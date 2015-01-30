/**
 * @file server.hpp
 */

#pragma once

namespace net {
  /**
   * @brief Extends net::socket_impl to create a server socket.
   */
  class server : public socket_impl<server> {
    public:
      /**
       * @brief Creates and initializes a new net::server object.
       */
      server() : socket_impl() { }

      /**
       * @brief Constructs a new net::client object and connects to a remote host.
       * @param[in] name Fully-qualified hostname of the remote host.
       * @param[in] port Service port to connect to.
       *
       * Example usage:
       *
       *     void start() {
       *       server s("localhost", 8080);
       *       while(true) {
       *         net::worker w = s.accept();
       *         // Handle the new connection through w
       *       }
       *     }
       *
       *     int main() {
       *       std::thread t(server);
       *       ...
       *       t.join()
       *       return 0;
       *     }
       */
      server(std::string const& name, unsigned short port) : socket_impl() { 
        connect(name, port);
      }

      /**
       * @brief Binds the newly-created socket to the local address and listens for incoming
       *        connections.
       */
      void do_connect() {
        // Allows the server to re-bind to the socket if the server is terminated and restarted
        // quickly (within the TIME_WAIT interval) as it takes time for the OS to notice that
        // this has happened. If "address in use" errors are seen, not using SO_REUSEADDR is
        // usually the cause.
        int reuse = 1;
        if(::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
          std::ostringstream msg;
          msg << "Could not configure socket: " << errno;
          throw net::socket_error(msg.str());
        }

        // Connects (binds) this process to the socket created by socket_impl.
        if(::bind(fd_, res0_->ai_addr, res0_->ai_addrlen) < 0) {
          std::ostringstream msg;
          msg << "Could not bind to socket: " << errno;
          throw net::socket_error(msg.str());
        }

        // Listens for incoming connections. The second paremeter (backlog) is set to 10,000.
        // This is the number of connections that the operating system can queue up while a
        // request is being serviced. If the queue is full, then clients will receive a
        // "connection refused" error. If requests are not handled quickly enough, then queued
        // requests may time out.
        if(::listen(fd_, 10000) < 0) {
          std::ostringstream msg;
          msg << "Could not listen for incoming connections: " << errno;
          throw net::socket_error(msg.str());
        }
      }

      /**
       * @brief Waits for a client connection.
       * 
       * Note that this is a blocking call if the net::server is not marked as non-blocking, or
       * not used within a @c select or @c poll construct.
       */
      worker accept() {
        struct sockaddr_in addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int s = ::accept(fd_, reinterpret_cast<struct sockaddr*>(&addr), &len);
        if(s < 0) {
          std::ostringstream msg;
          msg << "Could not accept incoming connection: " << errno;
          throw socket_error(msg.str());
        }
        return worker(s, addr, len);
      }
  };
}
