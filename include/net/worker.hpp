/**
 * @file worker.hpp
 */

#pragma once

namespace net {
  /**
   * @brief Extends net::socket to handle incoming connections from a server.
   *
   * Note that this is derived from socket, not socket_impl as the underlying file descriptor was
   * created by the OS when server::accept() was called.
   */
  class worker : public socket {
    public:
      /**
       * @brief Creates a new net::worker from an existing socket file descriptor.
       * @param[in] s File descriptor for the worker socket.
       * @param[in] a Address information for the connected client.
       *
       * This method should be invoked only by net::server.
       */
      worker(int s, struct sockaddr_in a) :
          socket(s), _client_address(a) {
      }

      /**
       * @brief Retrieves the hostname for the connected client.
       *
       * @returns Hostname for the connected client.
       */
      std::string client_hostname() const {
        struct hostent* client_host = gethostbyaddr(&_client_address.sin_addr.s_addr,
            sizeof(_client_address.sin_addr.s_addr), AF_INET);
        if(!client_host) {
          std::ostringstream msg;
          msg << "Could not get client hostname: " << h_errno << " " << hstrerror(h_errno);
          throw socket_error(msg.str());
        }
        return client_host->h_name;
      }

      /**
       * @brief Retrieves the IP address for the connected client.
       * @returns IP address for the connected client.
       */
      std::string client_ip() const {
        char* client_ip = inet_ntoa(_client_address.sin_addr);
        if(!client_ip) {
          throw socket_error("Could not get client IP.");
        }
        return client_ip;
      }

    private:
      /**
       * @brief Receives information about the connected client when net::server::accept() is
       *        called.
       */
      struct sockaddr_in _client_address;
  };
}
