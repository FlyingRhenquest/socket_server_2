/**
 * Copyright 2014 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <boost/thread.hpp>
#include "echo_service.hpp"
#include "server_body.hpp"
#include "socket_client.hpp"
#include "socket_server.hpp"

class test_client_server : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(test_client_server);
  CPPUNIT_TEST(test_echo_service);
  CPPUNIT_TEST_SUITE_END();

public:

  void test_echo_service()
  {
    // Create a server on port 12345
    fr::socket::socket_server<fr::socket::server_body<echo_service> > my_echo_server(12345);
    boost::thread *server_thread = my_echo_server.start();
    // Give the server time to start
    boost::this_thread::yield();
    fr::socket::socket_client echo_client("localhost", 12345);
    std::string expected("Hello, sockets!");
    std::string actual;
    echo_client.streams->stream_out << expected << std::endl;
    getline(echo_client.streams->stream_in, actual);
    //    std::cout << std::endl << "Expected: \"" << expected << "\"" << std::endl;
    //    std::cout << "Actual: \"" << actual << "\"" << std::endl;
    CPPUNIT_ASSERT(expected == actual);
    my_echo_server.shutdown();
    // Poke the echo server so it exits. IRL using blocking reads in
    // your server code opens a sack of pain. For this test, it was
    // a significantly smaller sack of pain than having to deal with
    // select and signals or polling the stream and building the
    // string up that way. No matter how you go about it, there's always
    // a good bit of suck associated with threaded I/O.
    echo_client.streams->stream_out << std::endl;
    server_thread->join();
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_client_server);
