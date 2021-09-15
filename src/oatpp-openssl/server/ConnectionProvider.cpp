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

#include <memory>

namespace oatpp { namespace openssl { namespace server {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  instantiateTLSServer();

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
{
  return std::make_shared<ConnectionProvider>(config, streamProvider);
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
}

void ConnectionProvider::instantiateTLSServer() {

  struct ssl_ctx_deleter {
    void operator()(struct ssl_ctx_st * p){
      SSL_CTX_free(p);
    }
  };

  auto method = SSLv23_server_method();

  auto ctx = std::shared_ptr<SSL_CTX>(SSL_CTX_new(method), ssl_ctx_deleter());
  if (!ctx) {
    throw std::runtime_error("[oatpp::openssl::server::ConnectionProvider::instantiateTLSServer()]. Error. Can't create context.");
  }

  m_config->configureContext(ctx);

}

void ConnectionProvider::stop() {
  m_streamProvider->stop();
}

std::shared_ptr<data::stream::IOStream> ConnectionProvider::get(){

  auto transportStream = m_streamProvider->get();

  if(transportStream) {
    return std::make_shared<Connection>(m_config, transportStream);
  }

  return nullptr;

}

void ConnectionProvider::invalidate(const std::shared_ptr<data::stream::IOStream>& connection) {

  auto c = std::static_pointer_cast<oatpp::openssl::Connection>(connection);

  /********************************************
   * WARNING!!!
   *
   * c->closeTLS(); <--- DO NOT
   *
   * DO NOT CLOSE or DELETE TLS handles here.
   * Remember - other threads can still be
   * waiting for TLS events.
   ********************************************/

  /* Invalidate underlying transport */
  auto s = c->getTransportStream();
  m_streamProvider->invalidate(s);

}

void ConnectionProvider::updateConfig(const std::shared_ptr<Config> &config) {
  m_config = config;
  instantiateTLSServer();
}

}}}

