/**
 * Service class interface. Service class must implement a constructor that takes:
 *
 *   * server_interface *owner - Server that initiated this service class
 *   * int file descriptor for this socket
 *   * Sockaddr * - incoming address
 *   * size_t sockaddr size 
 *
 * Service class must also implement operator ()
 *
 * Your service class doesn't technically HAVE to inherit from this class -- Any type that implements
 * that type of constructor and operator() will do. Keep in mind if you write your own that in the
 * constructor you must copy the sockaddr * you got, if you intend to use it. As soon as operator()
 * is called, I can no longer guarantee that the original address is still valid.
 */

#ifndef _HPP_SERVICE_CLASS_INTERFACE
#define _HPP_SERVICE_CLASS_INTERFACE

#include "server_interface.hpp"
#include <stdint.h>
#include <sys/socket.h>

#ifndef SOCK_MAXADDRLEN
// Well WHY THE HELL NOT?!
#define SOCK_MAXADDRLEN 255
#endif

namespace fr {

  namespace socket {

    class service_class_interface {
    protected:

      server_interface *owner;
      int fdes;
      sockaddr *incoming_address;
      size_t addr_size;

    public:

      service_class_interface(server_interface *owner, int fdes, sockaddr *address, size_t addr_size) : owner(owner), fdes(fdes), incoming_address(nullptr), addr_size(addr_size)
      {
	incoming_address = (sockaddr *) malloc(addr_size);
	// Ah, ah ah! I hear you cry! You didn't test your malloc for null! You're a terrible
	// programmer! Well turns out Linux as configured will never actually return NULL
	// when it runs out of memory. It just crashes the OS. So testing this for null is
	// kind of pointless. Your process or your computer has already died (Well it doesn't,
	// Until I do this memcpy...)
	memcpy(incoming_address, address, addr_size);
      }

      // Make a new copy of address, since boost::thread will free the old one when it destroys the
      // copy of this object. Oh yeah, boost::thread copies your object and destroys the old one.
      // Gotta watch out for that sort of thing...
      service_class_interface(const service_class_interface &copy) : owner(copy.owner), fdes(copy.fdes), incoming_address(nullptr), addr_size(copy.addr_size)
      {
	incoming_address = (sockaddr *) malloc(addr_size);
	memcpy(incoming_address, copy.incoming_address, addr_size);
      }

      virtual ~service_class_interface()
      {
	free(incoming_address);
      }

      virtual void operator()() = 0;

    };

  }

}

#endif
