/**
 * An echo service class for test.
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

#ifndef _HPP_ECHO_SERVER
#define _HPP_ECHO_SERVER

#include "fdes_stream.hpp"
#include <iostream>
#include "server_interface.hpp"
#include "service_class_interface.hpp"
#include <string>

// Simple echo service. I'm just going to ignore the incoming address on
// this

class echo_service : public fr::socket::service_class_interface {

public:

  echo_service(fr::socket::server_interface *owner, int fdes, sockaddr *incoming_address, size_t addr_size) : service_class_interface(owner, fdes, incoming_address, addr_size)
  {
  }

  void operator()()
  {
    // If you create fdes_stream before now, you'll get hung up on
    // it not having a copy constructor. This is much easier.
    fr::socket::fdes_stream streams(this->fdes);
    while(!this->owner->is_done() && streams.stream_in.good() && streams.stream_out.good()) {
      std::string from_sock;
      std::getline(streams.stream_in, from_sock);
      streams.stream_out << from_sock << std::endl;
    }
  }

};

#endif
