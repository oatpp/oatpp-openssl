# oatpp-openssl [![Build Status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/oatpp.oatpp-openssl?branchName=master)](https://dev.azure.com/lganzzzo/lganzzzo/_build/latest?definitionId=32&branchName=master)

This submodule provides secure server and client connection providers for oatpp applications. Based on OpenSSL.

More about oat++:
- Website: [https://oatpp.io](https://oatpp.io)

## Requires

OpenSSL installed.

### Create server connection provider

```cpp

#include "oatpp-openssl/server/ConnectionProvider.hpp"
#include "oatpp-openssl/Config.hpp"

...

const char* pemFile = "path/to/file.pem";
const char* crtFile = "path/to/file.crt";

auto config = oatpp::openssl::Config::createDefaultServerConfigShared(pemFile, crtFile);
auto connectionProvider = oatpp::openssl::server::ConnectionProvider::createShared(config, {"localhost", 443});

```

### Create client connection provider

```cpp

#include "oatpp-openssl/client/ConnectionProvider.hpp"
#include "oatpp-openssl/Config.hpp"
#include "oatpp-openssl/configurer/TrustStore.hpp"

...

const char* trust = "path/to/truststore";

auto config = oatpp::openssl::Config::createDefaultClientConfigShared();
config->addContextConfigurer(std::make_shared<oatpp::openssl::configurer::TrustStore>(trust, nullptr));
auto connectionProvider = oatpp::openssl::client::ConnectionProvider::createShared(config, {"httpbin.org", 443});

```
