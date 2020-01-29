/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "ConnectionProvider.hpp"

#include "oatpp-openssl/Connection.hpp"

#include "oatpp/network/client/SimpleTCPConnectionProvider.hpp"

#include <openssl/crypto.h>

namespace oatpp { namespace openssl { namespace client {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  auto calback = CRYPTO_get_locking_callback();
  if(!calback) {
    OATPP_LOGD("[oatpp::openssl::client::ConnectionProvider::ConnectionProvider()]",
               "WARNING. openssl. CRYPTO_set_locking_callback is NOT set. "
               "This can cause problems using openssl in multithreaded environment! "
               "Please call oatpp::openssl::Callbacks::setDefaultCallbacks() or "
               "consider setting custom locking_callback.");
  }

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider) {
  return std::shared_ptr<ConnectionProvider>(new ConnectionProvider(config, streamProvider));
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config, const oatpp::String& host, v_uint16 port) {
  return createShared(config, oatpp::network::client::SimpleTCPConnectionProvider::createShared(host, port));
}

  
std::shared_ptr<oatpp::data::stream::IOStream> ConnectionProvider::getConnection(){

  Connection::TLSHandle tlsHandle = tls_client();
  tls_configure(tlsHandle, m_config->getTLSConfig());

  oatpp::String host;
  auto hostName = m_streamProvider->getProperty(oatpp::network::ConnectionProvider::PROPERTY_HOST);
  if(hostName) {
    host = hostName.toString();
  }

  auto tlsObject = std::make_shared<TLSObject>(tlsHandle, TLSObject::Type::CLIENT, host);
  auto connection = std::make_shared<Connection>(tlsObject, m_streamProvider->getConnection());

  connection->setOutputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
  connection->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);

  connection->initContexts();
  return connection;

}

oatpp::async::CoroutineStarterForResult<const std::shared_ptr<oatpp::data::stream::IOStream>&> ConnectionProvider::getConnectionAsync() {


  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const std::shared_ptr<oatpp::data::stream::IOStream>&> {
  private:
    std::shared_ptr<Config> m_config;
    std::shared_ptr<oatpp::network::ClientConnectionProvider> m_streamProvider;
  private:
    std::shared_ptr<oatpp::data::stream::IOStream> m_stream;
    std::shared_ptr<Connection> m_connection;
  public:

    ConnectCoroutine(const std::shared_ptr<Config>& config, const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
      : m_config(config)
      , m_streamProvider(streamProvider)
    {}

    Action act() override {
      /* get transport stream */
      return m_streamProvider->getConnectionAsync().callbackTo(&ConnectCoroutine::onConnected);
    }

    Action onConnected(const std::shared_ptr<oatpp::data::stream::IOStream>& stream) {
      /* transport stream obtained */
      m_stream = stream;
      return yieldTo(&ConnectCoroutine::secureConnection);
    }

    Action secureConnection() {

      Connection::TLSHandle tlsHandle = tls_client();
      tls_configure(tlsHandle, m_config->getTLSConfig());

      oatpp::String host;
      auto hostName = m_streamProvider->getProperty(oatpp::network::ConnectionProvider::PROPERTY_HOST);
      if(hostName) {
        host = hostName.toString();
      }

      auto tlsObject = std::make_shared<TLSObject>(tlsHandle, TLSObject::Type::CLIENT, host);
      m_connection = std::make_shared<Connection>(tlsObject, m_stream);

      m_connection->setOutputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
      m_connection->setInputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);

      return m_connection->initContextsAsync().next(yieldTo(&ConnectCoroutine::onSuccess));

    }

    Action onSuccess() {
      return _return(m_connection);
    }


  };

  return ConnectCoroutine::startForResult(m_config, m_streamProvider);


}
  
}}}
