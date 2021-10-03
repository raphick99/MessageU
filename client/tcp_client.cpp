#include "tcp_client.hpp"

TcpClient::TcpClient(const std::string& host, const std::string& port) : 
	io_context(),
	socket(io_context),
	resolver(io_context)
{
	boost::asio::connect(socket, resolver.resolve(host, port));
}

std::string TcpClient::read_string(size_t buffer_size)
{
	if (buffer_size == 0)
	{
		return std::string();
	}

	std::string buffer;
	buffer.resize(buffer_size);

	std::size_t reply_length = boost::asio::read(socket, boost::asio::buffer(buffer.data(), buffer_size));
	if (reply_length == 0)
	{
		throw ProjectException(ProjectStatus::TcpClient_SocketClosedAtOtherEndpoint);
	}
	if (reply_length != buffer_size)
	{
		throw ProjectException(ProjectStatus::TcpClient_IncompleteRead);
	}
	return buffer;
}

void TcpClient::write_string(const std::string& string_to_write)
{
	std::size_t write_length = boost::asio::write(socket, boost::asio::buffer(string_to_write.data(), string_to_write.size()));
	if (write_length != string_to_write.size())
	{
		throw ProjectException(ProjectStatus::TcpClient_IncompleteWrite);
	}
}

