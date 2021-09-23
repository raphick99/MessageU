#include "tcp_client.hpp"

TcpClient::TcpClient(const std::string& host, const std::string& port) : 
	io_context(),
	socket(io_context),
	resolver(io_context)
{
	boost::asio::connect(socket, resolver.resolve(host, port));
}
