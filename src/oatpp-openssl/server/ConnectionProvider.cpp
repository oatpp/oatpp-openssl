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

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

namespace oatpp { namespace openssl { namespace server {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
  , m_closed(false)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  instantiateTLSServer();

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
{
  return std::shared_ptr<ConnectionProvider>(new ConnectionProvider(config, streamProvider));
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const network::Address& address,
                                                                     bool useExtendedConnections)
{
  return createShared(
    config,
    network::tcp::server::ConnectionProvider::createShared(address, useExtendedConnections)
  );
}

ConnectionProvider::~ConnectionProvider() {
  stop();
}

void ConnectionProvider::instantiateTLSServer() {

  auto method = SSLv23_server_method();

  m_ctx = SSL_CTX_new(method);
  if (!m_ctx) {
    throw std::runtime_error("[oatpp::openssl::server::ConnectionProvider::instantiateTLSServer()]. Error. Can't create context.");
  }

  SSL_CTX_set_ecdh_auto(m_ctx, 1);
  m_config->configureContext(m_ctx);

}

void ConnectionProvider::stop() {
  if(!m_closed) {
    m_closed = true;
    m_streamProvider->stop();
  }
}

std::shared_ptr<data::stream::IOStream> ConnectionProvider::get(){

  auto transportStream = m_streamProvider->get();

  if(transportStream) {

    auto ssl = SSL_new(m_ctx);
    SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_set_accept_state(ssl);

    return std::make_shared<Connection>(ssl, transportStream);

  }

  return nullptr;

}

}}}

