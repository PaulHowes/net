/**
 * @file client.hpp
 */

#pragma once

#include <net/socket.hpp>

namespace net {
  /**
   * @brief Extends net::socket_impl to create a client socket.
   * @tparam traits A net::socket_traits object that defines the characteristics of the socket.
   */
  template<typename traits>
  class client : public socket_impl<client<traits>, traits> {
    public:
      /**
       * @brief Creates and initializes a new net::client object.
       */
      client() : socket_impl<client<traits>, traits>() { }

      /**
       * @brief Constructs a new net::client object and connects to a remote host.
       * @param[in] name Fully-qualified hostname of the remote host.
       * @param[in] port Service port to connect to.
       *
       * Example usage:
       *
       *     client c("google.com", 80);
       *     c.send_line("GET /");
       */
      client(std::string const& name, unsigned short port)
          : socket_impl<client<traits>, traits>() {
        this->connect(name, port);
      }

      /**
       * @brief Connects the newly-created socket to the remote host.
       *
       * *NOTE* This is an internal function that should not be directly invoked.
       */
      void do_connect() {
        if(::connect(this->fd_, this->res0_->ai_addr, this->res0_->ai_addrlen) < 0) {
          std::ostringstream msg;
          msg << "Connection failed: " << errno;
          throw net::socket_error(msg.str());
        }
      }
  };

  /**
   * @brief Specialization of client<T> for TCP/IP sockets.
   */
  typedef client<tcp_traits> tcp_client;

  /**
   * @brief Specialization of client<T> for UDP/IP sockets.
   */
  typedef client<udp_traits> udp_client;
}

