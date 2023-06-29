#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <thread>
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void handleClientRequest(tcp::socket clientSocket) {
  try {
    // Read the request from the client
    beast::flat_buffer buffer;
    http::request<http::dynamic_body> request;
    http::read(clientSocket, buffer, request);

    // Create a connection to the target server
    net::io_context ioContext;
    tcp::socket serverSocket(ioContext);
    tcp::resolver resolver(ioContext);
    auto const results = resolver.resolve("localhost", "8082");
    net::connect(serverSocket, results.begin(), results.end());

    // Forward the request to the target server
    http::write(serverSocket, request);

    // Receive the response from the target server
    beast::flat_buffer serverBuffer;
    http::file_body::value_type file;
    beast::error_code ec;
    file.open("tmp", beast::file_mode::write, ec);

    http::response<http::file_body> res{std::piecewise_construct,
                                        std::make_tuple(std::move(file))};
    http::response_parser<http::file_body> parser{std::move(res)};

    parser.body_limit((std::numeric_limits<std::uint64_t>::max)());
    http::read(serverSocket, serverBuffer, parser);

    // Forward the response to the client
    auto tmp = parser.release();
    tmp.body().seek(0, ec);
    http::write(clientSocket, tmp);
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
  }
}

int main() {
  // Create the server endpoint
  net::io_context ioContext;
  tcp::endpoint endpoint(tcp::v4(), 8081);

  // Create and bind the acceptor
  tcp::acceptor acceptor(ioContext, endpoint);
  acceptor.listen();

  std::cout << "Proxy server started. Listening on port 8081." << std::endl;

  // Accept client connections and handle requests
  while (true) {
    tcp::socket clientSocket(ioContext);
    acceptor.accept(clientSocket);

    std::thread t(&handleClientRequest, std::move(clientSocket));
    t.detach();
  }

  return 0;
}
