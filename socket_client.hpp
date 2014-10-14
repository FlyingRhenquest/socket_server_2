/**
 * Socket client. Connect to a network socket and provide stream objects
 * for it. Right now I'm working with ipv4 sockets and am not working
 * too hard to abstract this further. Once I get some support in for
 * other protocol types, I might revisit this and make it more abstract.
 *
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

#ifndef _HPP_SOCKET_CLIENT
#define _HPP_SOCKET_CLIENT

#include <arpa/inet.h>
#include <errno.h>
#include "fdes_stream.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "time/string_converts.hpp"
#include <unistd.h>

namespace fr {

  namespace socket {

    /**
     * Create client with either name and port and then access the
     * streams from streams. Yeah, this isn't a super-spiffy
     * protocol-speaking object. I'm not entirely sure there'd be a
     * lot of value in trying to create a generic framework for that
     * sort of thing. Pretty much everything you'd want to do would be
     * protocol-specific. Of course, it wouldn't be terribly difficult
     * to add a specific protocol (Like http) on top of this using
     * boost::signals2, and reimplement something like lynx...
     */

    class socket_client {
      sockaddr *address;
      size_t address_len;
      int port;
      int fd;
      int domain, type, protocol;

      // Call after address, address_len and port are set up
      void instrument_connection()
      {
	fd = ::socket(domain, type, protocol);
	if (0 > fd) {
	  // You know... why don't I write an exception to wrap errno
	  // one of these days? Assuming someone hasn't done it already
	  free(address);
	  address = nullptr;
	  throw std::logic_error("Error while requesting socket");
	}

	if (connect(fd, address, address_len) != 0) {
	  free(address);
	  close(fd);
	  throw std::logic_error("Could not connect");
	}
	
	streams = new fdes_stream(fd);
	
      }
      
    public:
      fdes_stream *streams;

      // Can throw an exception if name lookup fails
      socket_client(const std::string &host_name, int port, int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0) : address(nullptr), address_len(0l), port(port), fd(-1), domain(domain), type(type), protocol(protocol), streams(nullptr)
      {       
	// It might not be a bad idea to sanitize that string before
	// passing it...
	struct addrinfo hints = { 0 };
	struct addrinfo *result;
	int err;
	std::string port_s;
	port_s = fr::time::to_string<int>()(port); // :-/

	hints.ai_family = domain;
	hints.ai_socktype = type;
	hints.ai_flags = 0;
	hints.ai_protocol = protocol;

	err = getaddrinfo(host_name.c_str(), port_s.c_str(), &hints, &result);

	if (err != 0) {
	  std::string err("Error: Could not get address info for ");
	  err.append(host_name);
	  throw std::logic_error(err);
	}

	// Client is ust going to try to connect to the first one
	// it can find
	address_len = result->ai_addrlen;
	address = (sockaddr *) malloc(address_len);
	memcpy(address, result->ai_addr, address_len);
	freeaddrinfo(result);

	// Now I can set up the connection
	instrument_connection();
      }

      // I have enough info to let the copy constructor create a
      // new connection to the remote, but it's really better just
      // to pass this as a pointer or reference
      socket_client(const socket_client &copy) = delete;

      ~socket_client()
      {
	if (nullptr != address) {
	  free(address);
	}
	if (nullptr != streams) {
	  delete streams; // Closes fd
	}
      }

    };

  }

}

#endif
