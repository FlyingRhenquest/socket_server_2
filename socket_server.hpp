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
 * Socket server - holds a server_body and provides a consistent interface
 * to it. Anything that provides an interface like server_body's
 * (Constructor accepts owner, port and interface and class
 * implements operator()()) will work as the server body template
 * parameter.
 */

#ifndef _HPP_SOCKET_SERVER
#define _HPP_SOCKET_SERVER

#include <arpa/inet.h>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>
#include <errno.h>
#include <netinet/in.h>
#include "server_interface.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <boost/thread.hpp>
#include <unistd.h>

namespace fr {

  namespace socket {

    template <typename body>
    class socket_server : public server_interface {
      bool shut_down;
      int port;
      uint32_t interface;
    public:
      socket_server(int port, uint32_t interface = INADDR_ANY) : shut_down(false), port(port), interface(interface)
      {
      }

      boost::thread *start()
      {
	// Notify listeners we're about to start the
	// server
	server_start();
	body srv(dynamic_cast<server_interface *>(this), port, interface);
	boost::thread *retval = new boost::thread(srv);
	// Yield to allow body time to start
	boost::this_thread::yield();
	return retval;
      }

      // Assumes body implementations notice and obey is_done()
      // within a reasonable period of time. It's better to have
      // your server shut down gracefully than get kill -9'd.

      void shutdown()
      {       
	shut_down = true;
      }

      bool is_done()
      {
	return shut_down;
      }

    };

  }

}

#endif
