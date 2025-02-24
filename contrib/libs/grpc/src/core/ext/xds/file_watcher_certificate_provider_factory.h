//
//
// Copyright 2020 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//

#ifndef GRPC_CORE_EXT_XDS_FILE_WATCHER_CERTIFICATE_PROVIDER_FACTORY_H
#define GRPC_CORE_EXT_XDS_FILE_WATCHER_CERTIFICATE_PROVIDER_FACTORY_H

#include <grpc/support/port_platform.h>

#include "src/core/ext/xds/certificate_provider_factory.h"

namespace grpc_core {

class FileWatcherCertificateProviderFactory
    : public CertificateProviderFactory {
 public:
  class Config : public CertificateProviderFactory::Config {
   public:
    static RefCountedPtr<Config> Parse(const Json& config_json,
                                       grpc_error_handle* error);

    const char* name() const override;

    TString ToString() const override;

    const TString& identity_cert_file() const {
      return identity_cert_file_;
    }

    const TString& private_key_file() const { return private_key_file_; }

    const TString& root_cert_file() const { return root_cert_file_; }

    Duration refresh_interval() const { return refresh_interval_; }

   private:
    TString identity_cert_file_;
    TString private_key_file_;
    TString root_cert_file_;
    Duration refresh_interval_;
  };

  const char* name() const override;

  RefCountedPtr<CertificateProviderFactory::Config>
  CreateCertificateProviderConfig(const Json& config_json,
                                  grpc_error_handle* error) override;

  RefCountedPtr<grpc_tls_certificate_provider> CreateCertificateProvider(
      RefCountedPtr<CertificateProviderFactory::Config> config) override;
};

}  // namespace grpc_core

#endif  // GRPC_CORE_EXT_XDS_FILE_WATCHER_CERTIFICATE_PROVIDER_FACTORY_H
