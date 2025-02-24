#include "read_filter_merger.h"

namespace NKikimr::NOlap::NIndexedReader {

bool TSortableBatchPosition::IsSameSchema(const std::shared_ptr<arrow::Schema> schema) const {
    if (Fields.size() != (size_t)schema->num_fields()) {
        return false;
    }
    for (ui32 i = 0; i < Fields.size(); ++i) {
        if (Fields[i]->type() != schema->field(i)->type()) {
            return false;
        }
        if (Fields[i]->name() != schema->field(i)->name()) {
            return false;
        }
    }
    return true;
}

void TMergePartialStream::PutControlPoint(std::shared_ptr<TSortableBatchPosition> point) {
    Y_VERIFY(point);
    Y_VERIFY(point->IsSameSchema(SortSchema));
    Y_VERIFY(++ControlPoints == 1);
    SortHeap.emplace_back(TBatchIterator(*point));
    if (SortHeap.size() > 1) {
        Y_VERIFY(SortHeap.front().GetKeyColumns().Compare(SortHeap.back().GetKeyColumns()) != std::partial_ordering::greater);
    }
    std::push_heap(SortHeap.begin(), SortHeap.end());
}

void TMergePartialStream::AddPoolSource(const std::optional<ui32> poolId, std::shared_ptr<arrow::RecordBatch> batch, std::shared_ptr<NArrow::TColumnFilter> filter) {
    if (!batch || !batch->num_rows()) {
        return;
    }
    Y_VERIFY_DEBUG(NArrow::IsSorted(batch, SortSchema));
    if (!poolId) {
        IndependentBatches.emplace_back(batch);
        AddNewToHeap(poolId, batch, filter, true);
    } else {
        auto it = BatchPools.find(*poolId);
        if (it == BatchPools.end()) {
            it = BatchPools.emplace(*poolId, std::deque<TIteratorData>()).first;
        }
        it->second.emplace_back(batch, filter);
        if (it->second.size() == 1) {
            AddNewToHeap(poolId, batch, filter, true);
        }
    }
}

void TMergePartialStream::AddNewToHeap(const std::optional<ui32> poolId, std::shared_ptr<arrow::RecordBatch> batch, std::shared_ptr<NArrow::TColumnFilter> filter, const bool restoreHeap) {
    if (!filter || filter->IsTotalAllowFilter()) {
        SortHeap.emplace_back(TBatchIterator(batch, nullptr, SortSchema->field_names(), Reverse, poolId));
    } else if (filter->IsTotalDenyFilter()) {
        return;
    } else {
        SortHeap.emplace_back(TBatchIterator(batch, filter, SortSchema->field_names(), Reverse, poolId));
    }
    if (restoreHeap) {
        std::push_heap(SortHeap.begin(), SortHeap.end());
    }
}

void TMergePartialStream::RemoveControlPoint() {
    Y_VERIFY(ControlPoints == 1);
    Y_VERIFY(ControlPointEnriched());
    Y_VERIFY(-- ControlPoints == 0);
    std::pop_heap(SortHeap.begin(), SortHeap.end());
    SortHeap.pop_back();
}

}
