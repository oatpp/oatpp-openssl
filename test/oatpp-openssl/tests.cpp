
#include "oatpp-test/UnitTest.hpp"

#include "FullTest.hpp"
#include "FullAsyncTest.hpp"
#include "FullAsyncClientTest.hpp"

#include "oatpp-openssl/Callbacks.hpp"

#include "oatpp/core/concurrency/SpinLock.hpp"
#include "oatpp/core/base/Environment.hpp"

#include <iostream>
#include <csignal>

namespace {

void runTests() {

  oatpp::openssl::init();

  /* set lockingCallback for openssl */
  oatpp::openssl::Callbacks::setDefaultCallbacks();

  /* ignore SIGPIPE */
  std::signal(SIGPIPE, SIG_IGN);

  {

    oatpp::test::openssl::FullTest test_virtual(0, 100);
    test_virtual.run();

    oatpp::test::openssl::FullTest test_port(8443, 10);
    test_port.run();

  }

  {

    oatpp::test::openssl::FullAsyncTest test_virtual(0, 100);
    test_virtual.run();

    oatpp::test::openssl::FullAsyncTest test_port(8443, 10);
    test_port.run();

  }

  {

    oatpp::test::openssl::FullAsyncClientTest test_virtual(0, 10);
    test_virtual.run(20); // - run this test 20 times.

    oatpp::test::openssl::FullAsyncClientTest test_port(8443, 10);
    test_port.run(1);

  }

}

}

int main() {

  oatpp::base::Environment::init();

  runTests();

  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";

  OATPP_ASSERT(oatpp::base::Environment::getObjectsCount() == 0);

  oatpp::base::Environment::destroy();

  return 0;
}

//#include "oatpp-openssl/Connection.hpp"
//#include "oatpp-openssl/client/ConnectionProvider.hpp"
//#include "oatpp-openssl/server/ConnectionProvider.hpp"
//
//#include <stdio.h>
//#include <unistd.h>
//#include <string.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <openssl/ssl.h>
//#include <openssl/err.h>
//
//#include <thread>
//
//#include "oatpp/web/client/HttpRequestExecutor.hpp"
//
//void init_openssl() {
//  SSL_load_error_strings();
//  OpenSSL_add_ssl_algorithms();
//}
//
//void cleanup_openssl() {
//  EVP_cleanup();
//}
//
//void clientExample() {
//  auto config = oatpp::openssl::Config::createDefaultClientConfigShared();
//  auto connectionProvider = oatpp::openssl::client::ConnectionProvider::createShared(config, {"oatpp.io", 443});
//
//  oatpp::web::client::HttpRequestExecutor executor(connectionProvider);
//
//  auto response = executor.execute("GET", "/", {}, nullptr, nullptr);
//
//  OATPP_LOGD("AAA", "response code=%d", response->getStatusCode());
//  auto html = response->readBodyToString();
//
//  OATPP_LOGD("AAA", "html=%s", html->c_str());
//
//}
//
//int main(int argc, char **argv) {
//
//  oatpp::base::Environment::init();
//  init_openssl();
//
//  clientExample();
//
//  auto config = oatpp::openssl::Config::createDefaultServerConfigShared(CERT_CRT_PATH, CERT_PEM_PATH);
//  auto connectionProvider = oatpp::openssl::server::ConnectionProvider::createShared(config, {"localhost", 4433});
//
//  OATPP_LOGD("Server", "running...");
//
//  while(1) {
//
//    auto connection = connectionProvider->get();
//
//    if(connection) {
//
//      OATPP_LOGD("Server", "tcp in... %d", connection.get());
//      connection->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
//      connection->getInputStreamContext().init();
//
//      OATPP_LOGD("Server", "write data");
//      oatpp::String data =
//        "HTTP/1.1 200 OK\r\n"
//        "Content-Length: 6\r\n\r\n"
//        "Hello!";
//
//      connection->writeExactSizeDataSimple(data->getData(), data->getSize());
//
//    }
//
////    struct sockaddr_in addr;
////    uint len = sizeof(addr);
////    SSL *ssl;
////    const char reply[] = "test\n";
////
////    int client = accept(sock, (struct sockaddr*)&addr, &len);
////    if (client < 0) {
////      perror("Unable to accept");
////      exit(EXIT_FAILURE);
////    }
////
////    ssl = SSL_new(ctx);
////    SSL_set_fd(ssl, client);
////
////    if (SSL_accept(ssl) <= 0) {
////      ERR_print_errors_fp(stderr);
////    }
////    else {
////      SSL_write(ssl, reply, strlen(reply));
////    }
////
////    SSL_shutdown(ssl);
////    SSL_free(ssl);
////    close(client);
//  }
//
//  //SSL_CTX_free(ctx);
//  cleanup_openssl();
//
//
//  oatpp::base::Environment::destroy();
//
//}
