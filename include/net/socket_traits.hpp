/**
 * @file socket_traits.hpp
 */

#pragma once

namespace net {
  template<int flags_, int family_, int type_, int protocol_>
  struct socket_traits {
    static inline int flags() { return flags_; }
    static inline int family() { return family_; }
    static inline int type() { return type_; }
    static inline int protocol() { return protocol_; }
  };

  struct tcp_traits : socket_traits<AI_PASSIVE, PF_INET, SOCK_STREAM, IPPROTO_TCP> { };
  struct udp_traits : socket_traits<AI_PASSIVE, PF_INET, SOCK_DGRAM,  IPPROTO_UDP> { };
}
