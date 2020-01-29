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

#include "oatpp/network/server/SimpleTCPConnectionProvider.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

namespace oatpp { namespace openssl { namespace server {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
  , m_closed(false)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  m_tlsObject = instantiateTLSServer();

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider){
  return std::shared_ptr<ConnectionProvider>(new ConnectionProvider(config, streamProvider));
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config, v_uint16 port) {
  return createShared(config, oatpp::network::server::SimpleTCPConnectionProvider::createShared(port));
}

ConnectionProvider::~ConnectionProvider() {
  close();
}

std::shared_ptr<TLSObject> ConnectionProvider::instantiateTLSServer() {

  Connection::TLSHandle handle = tls_server();

  if(handle == NULL) {
    throw std::runtime_error("[oatpp::openssl::server::ConnectionProvider::instantiateTLSServer()]: Failed to create tls_server");
  }

  if (tls_configure(handle, m_config->getTLSConfig()) < 0) {
    OATPP_LOGD("[oatpp::openssl::server::ConnectionProvider::instantiateTLSServer()]", "Error on call to 'tls_configure'. %s", tls_error(handle));
    throw std::runtime_error( "[oatpp::openssl::server::ConnectionProvider::instantiateTLSServer()]: Failed to configure tls_server");
  }

  return std::make_shared<TLSObject>(handle, TLSObject::Type::SERVER, nullptr);

}

void ConnectionProvider::close() {
  if(!m_closed) {
    m_closed = true;
    if(m_tlsObject) {
      m_tlsObject->close();
    }
    m_streamProvider->close();
  }
}

std::shared_ptr<oatpp::data::stream::IOStream> ConnectionProvider::getConnection(){
  auto transportStream = m_streamProvider->getConnection();
  if(transportStream) {
    return std::make_shared<Connection>(m_tlsObject, transportStream);
  }
  return nullptr;
}

}}}

