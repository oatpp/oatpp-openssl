
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
