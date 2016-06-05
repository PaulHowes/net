#include <chrono>
#include <iostream>
#include <thread>
#include <net.hpp>

void server() {
  net::tcp_server s;
  s.connect("localhost", 1234);
  net::worker w = s.accept();
  w.write_line("foobar");
}

int main() {
  std::thread t(server);

  // Give the server a chance to start.
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  net::tcp_client c;
  c.connect("localhost", 1234);

  std::string received = c.read_line();
  std::string expected = "foobar";

  t.join();

  if(received == expected) {
    return 0;
  }

  std::cout << "Expected: " << expected << std::endl;
  std::cout << "Received: " << received << std::endl;
  return -1;
}
