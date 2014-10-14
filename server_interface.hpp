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
 *
 * Virtual server interface. I use this so I can keep "owner" in my
 * server body class without it having to be templated.
 */

#ifndef _HPP_SERVER_INTERFACE
#define _HPP_SERVER_INTERFACE

#include <arpa/inet.h>
#include <boost/signals2/signal.hpp>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <boost/thread.hpp>
#include <unistd.h>

namespace fr {

  namespace socket {

    class server_interface {
    public:

      // Include some boost signals, for great justice

      // Notify listeners server is about to start
      boost::signals2::signal<void()> server_start;
 
      // Signals that the (socket) listener has started listening
      boost::signals2::signal<void()> start_listening;

      // Signals getting sockaddr * should make a copy of the address
      // immediately if they want to use them. You can assume the
      // pointer becomes invalid as soon as the handler terminates.

      // Signals that a connection request has taken place. This occurs
      // right after accept(), with the address and length of the
      // address structure from accept

      boost::signals2::signal<void (sockaddr *, size_t)> connection_request;

      // Optional signal may be called by service class prior to exiting to
      // inform the owner that the connection for this address has closed

      boost::signals2::signal<void (sockaddr *, size_t)> hangup;


      // Start listening for connections. Returns a thread pointer in case
      // you want to join on it later.

      virtual boost::thread *start() = 0;

      // Wait for listener body to complete. You don't need to call this if
      // you want to do additional processing in your main thread. If you
      // just want to wait and exit, you can use this.

      // Shut server down cleanly -- this signals the server body that a
      // shutdown has been requested. The body in turn can stop processing,
      // finish processing current connections and exit.

      virtual void shutdown() = 0;

      // Will return false until shutdown is called, then returns true.

      virtual bool is_done() = 0;

    };

  }

}

#endif
