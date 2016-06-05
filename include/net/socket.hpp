/**
 * @file socket.hpp
 */
#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <unistd.h>

#include <net/socket_traits.hpp>

/**
 * @namespace net
 */
namespace net {
  /**
   * @brief Encapsulates a file descriptor used for socket communications.
   */
  class socket {
    public:
      /**
       * @brief Creates a new net::socket object.
       */
      socket() : fd_(0) {
      }

      /**
       * @brief Creates a new net::socket object and associates it with an existing file
       *        descriptor
       * @param[in] fd Existing file descriptor to manage.
       */
      socket(int fd) : fd_(fd) {
      }

      /**
       * @brief Create a new net::socket object and copies the shared file descriptor.
       */
      socket(const socket& s) : fd_(s.fd_) {
      }

      /**
       * @brief Disconnect the net::socket object from the foreign host and destroys it.
       */
      ~socket() {
        if(fd_ > 0) {
          if(::close(fd_) < 0) {
            std::ostringstream msg;
            msg << "Error closing socket: " << errno;
            throw socket_error(msg.str());
          }
        }
      }

      /**
       * @brief Reads a block of data from the connected host.
       * @param[in] buffer Buffer that receives the data.
       * @param[in] length Length of the buffer,
       * @param[in] peek   @c false to perform a normal read, @c true to read the data from the
       *                   connected host without removing it from the socket's input queue.
       * @returns The number of bytes actually read from the connected host.
       */
      unsigned read(char* buffer, const size_t length, const bool peek = false) const {
        ssize_t read_length = ::recv(fd_, buffer, length, peek ? MSG_PEEK : 0);
        if(read_length < 0) {
          std::ostringstream msg;
          msg << "Error reading from connected host: " << errno;
          throw socket_error(msg.str());
        }
        return unsigned(read_length);
      }

      /**
       * @brief Reads a line of text from the connected host.
       * @returns A line of text read from the connected host.
       */
      std::string read_line() const {
        static const size_t BUFFER_SIZE = 4096;
        static char buffer[BUFFER_SIZE];

        // Reads a chunk of data from the socket without removing the data from socket's internal
        // buffer.
        unsigned read_length = this->read(buffer, BUFFER_SIZE, true);

        if(read_length > 0) {
          // Look for the EOL marker (CRLF) in the buffer. If found, the CRLF characters are
          // changed to NULL characters to mark the end of the string.
          unsigned found_eol = 0;
          char* p = buffer;
          while((p - buffer) < read_length && found_eol < 2) {
            if(*p == '\r' || *p == '\n') {
              found_eol++;
            }
            p++;
          }

          if(found_eol < 2) {
            std::ostringstream msg;
            msg << "Line not found: " << found_eol;
            throw socket_error(msg.str());
          }

          read_length = this->read(buffer, size_t(p - buffer));
          return std::string(buffer, size_t(read_length - 2));
        }

        return std::string();
      }

      /**
       * @brief Writes a block of data to the connected host.
       * @param[in] buffer Buffer that contains the data.
       * @param[in] length Length of the buffer,
       * @returns The number of bytes actually sent to the connected host.
       */
      unsigned write(const void* buffer, const size_t length) const {
        if(!fd_) {
          throw socket_error("Not connected");
        }
        ssize_t write_length = ::send(fd_, buffer, length, 0);
        if(write_length < 0) {
          std::ostringstream msg;
          msg << "Error writing to connected host: " << errno;
          throw socket_error(msg.str());
        }
        return unsigned(write_length);
      }

      /**
       * @brief Writes a string to the socket followed by a CRLF.
       * @param[in] line String to send to the connected host.
       * @returns The number of bytes actually sent to the connected host.
       */
      unsigned write_line(const std::string& line) const {
        std::string buffer = line + std::string("\r\n");
        return write(buffer.c_str(), buffer.length());
      }

    protected:
      /**
       * @brief The actual socket (file) descriptor managed by the net::socket object.
       */
      int fd_;
  };

  /**
   * @brief Base implementation of an IP socket
   * @tparam T Derived type of the concrete socket implementation
   * @tparam traits A net::socket_traits object that defines the characteristics of the socket.
   */
  template<typename T, typename traits>
  class socket_impl : public socket {
    public:
      /**
       * @brief Constructs a new net::socket_impl object.
       */
      socket_impl() : socket(), res0_(nullptr) {
      }

      /**
       * @brief Destroys this net::socket_impl object.
       */
      ~socket_impl() {
        if(res0_) {
          freeaddrinfo(res0_);
        }
      }

      /**
       * @brief Creates an endpoint for communicating with @p name and @p port.
       * @param[in] name Hostname or IP address of the endpoint.
       * @param[in] port Port for the endpoint.
       *
       * Both servers and clients use this method to create an endpoint. The difference is that
       * clients provide information about a remote host and servers the local host.
       */
      void connect(std::string name, unsigned short port) {
        // Cannot create a socket if one already exists.
        if(fd_ > 0) {
          throw socket_error("Socket already exists.");
        }

        // The addrinfo structure found in netdb.h is used to define the socket type. Here the
        // socket is configured as a streaming TCP/IP server.
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_flags    = traits::flags();      // Socket will be passed to bind().
        hints.ai_family   = traits::family();     // Internet Protocol (IP) socket.
        hints.ai_socktype = traits::type();       // Streaming socket.
        hints.ai_protocol = traits::protocol();   // TCP protocol

        // Retrieves the socket address for the host. Here "localhost" is used so that the server
        // is available only on the current system. A real server would be accessible to the
        // outside world so the hostname would be either one that can be resolved to an external
        // IP, or simply NULL to be accessible on all attached networks. The second parameter is
        // the service (port number) that the server will listen on.
        int err = 0;
        if((err = ::getaddrinfo(name.c_str(), std::to_string(port).c_str(), &hints, &res0_))) {
          std::ostringstream msg;
          msg << "Could not resolve address: (" << err << ") " << gai_strerror(err);
          throw net::socket_error(msg.str());
        }

        // Now that all of the information for the socket is available, create it.
        fd_ = ::socket(res0_->ai_family, res0_->ai_socktype, res0_->ai_protocol);
        if(fd_ < 0) {
          std::ostringstream msg;
          msg << "Could not create socket: " << errno;
          throw net::socket_error(msg.str());
        }

        // Call the do_connect function in the derived class to finish connecting the socket to
        // the host.
        static_cast<T*>(this)->do_connect();
      }

    protected:

      unsigned char padding[4];
      /**
       * @brief Address of the endpoint.
       */
      struct addrinfo* res0_;
  };
}

