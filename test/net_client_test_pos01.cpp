#include <iostream>
#include <net.hpp>

int main() {

  net::client c;
  c.connect("google.com", 80);
  c.write_line("GET /");

  std::string received = c.read_line();
  std::string expected = "HTTP/1.0 200 OK";

  if(received == expected) {
    return 0;
  }

  std::cout << "Expected: " << expected << std::endl;
  std::cout << "Received: " << received << std::endl;
  return -1;
}
