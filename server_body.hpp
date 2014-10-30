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
 * This is my server body. It implements the chunky bits of the socket
 * setup and control. It's templated with a service class, which gets
 * started when an inbound connection comes in. The service class should
 * accept a server_interface pointer, an int file descriptor and a
 * sockaddr pointer for the connection and the size of the sockaddr
 * storage.
 */

#ifndef _HPP_SERVER_BODY
#define _HPP_SERVER_BODY

#include "server_interface.hpp"
#include <stdexcept>
#include <stdint.h>
#include <boost/thread.hpp>

namespace fr {

  namespace socket {
  
    template <typename service_class>
    class server_body {
      server_interface *owner;
      int port;
      uint32_t interface;

    public:

      // Server body takes owner, port and optionally the network
      // interface to listen on. If interface isn't specified, it's
      // assumed to be all of them.  This should correspond to the
      // address of the interface you want to listen on, in network
      // byte order IIRC.

      server_body(server_interface *owner, int port, uint32_t interface = INADDR_ANY) : owner(owner), port(port), interface(interface)
      {
      }

      server_body(const server_body &copy) : owner(copy.owner), port(copy.port), interface(copy.interface)
      {
      }

      void operator()()
      {
	int errbuflen = 255;
	int backlog = 20;
	int retval = 0;
	// Create socket (Which is really just a file descriptor)
	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
	char errbuf[errbuflen];
	if (0 > sock) {
	  strerror_r(errno, errbuf, errbuflen);
	  throw std::logic_error(std::string(errbuf));
	}

	// Set up address to listen on
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = interface;
	
	// Bind address to socket. Note bind can accept any address
	// format as long as you tell it the size of the structure
	// you're passing it. So I'm really passing it a sockaddr_in
	// but it probably doesn't really know anything about it
	// (other than its size)
	retval = bind(sock, (sockaddr *) &addr, sizeof(sockaddr_in));

	if (0 > retval) {
	  strerror_r(errno, errbuf, errbuflen);
	  throw std::logic_error(std::string(errbuf));
	}

	// We can have backlog connections waiting to be accepted
	// before (I assume) the system just stops accepting them.
	// The man page says the client "may" receive an ECONNREFUSED
	// error after that.

	retval = listen(sock, backlog);

	// Ok! We're listening! Of course, we still have to actually
	// ACCEPT in-bound requests, but now we're listening for them!
	// Send start listening signal

	owner->start_listening();

	while(!owner->is_done()) {
	  // use select to poll the socket with a timeout of 1 second.
	  // That allows us to check every second to see if the owner
	  // was shut down. We could also do additional processing here
	  // if we had any we wanted to do. Note that if you block
	  // SIGINT in your signal handler, select will probably not
	  // work correctly
	  fd_set set;
	  timeval tv;
	  tv.tv_sec = 1;
	  tv.tv_usec = 0;
	  FD_SET(sock, &set);
	  char address_buffer[SOCK_MAXADDRLEN];
	  sockaddr_in *incoming_address = (sockaddr_in *) address_buffer;;

	  select(sock + 1, &set, NULL, NULL, &tv);
	  
	  // If there are bytes to be read on the socket, it means
	  // someone's trying to connect to us. So accept it...
	  
	  if (FD_ISSET(sock, &set)) {
	    socklen_t size = SOCK_MAXADDRLEN;
	    // So what this does is create a NEW file descriptor to
	    // handle the freshly-arrived connection. This allows the
	    // existing socket to continue listening. The new file
	    // descriptor will be closed when the connection
	    // we're accepting right now goes away.
	    int fdes =  accept(sock, (sockaddr *) &incoming_address, &size);
	    if (0 > fdes) {
	      // I'm going to treat this as non-fatal
	      perror("Error accepting connection");
	    } else {
	      // Anything wanting to do anything with incoming_address MUST copy the
	      // address, either when this signal is received or in the service_class
	      // constructor, below. If you derive your service class from
	      // service_class_interface, the address will be copied for you.

	      owner->connection_request((sockaddr *) &incoming_address, size);
	      // This has to be instantiated prior to creating a
	      // thread with it. If service class plans to use the
	      // address for anything, it should copy it in the
	      // constructor. You can pretty much assume this
	      // pointer is invalid at any point after that.
	      service_class serve_it(owner, fdes, (sockaddr *) &incoming_address, size);

	      // Note: For non-trivial tasks, I should probably use a
	      // thread pool. I'll get around to that when I'm writing
	      // some internets.

	      boost::thread *thrd = new boost::thread(serve_it);
	      // TODO: Implement non-leaking thread monitoring and
	      // cleanup class
	      owner->thread_spawned(thrd); // Now somebody else's problem!
	    }
	  }

	}
      }

    };

  }

}

#endif
