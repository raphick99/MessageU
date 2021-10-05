#include "tcp_client.h"

TcpClient::TcpClient(const std::string& host, const std::string& port) : 
	io_context(),
	socket(io_context),
	resolver(io_context)
{
	boost::asio::connect(socket, resolver.resolve(host, port));
}

std::string TcpClient::read_string(size_t nbytes)
{
	if (nbytes == 0)
	{
		return std::string();
	}

	std::string data;

	while (data.length() < nbytes)
	{
		std::string buffer;
		if (nbytes - data.length() > CHUNK_SIZE)
		{
			buffer.resize(CHUNK_SIZE);  // read in chunks
		}
		else
		{
			buffer.resize(nbytes - data.length());
		}
		std::size_t reply_length = boost::asio::read(socket, boost::asio::buffer(buffer.data(), buffer.length()));
		if (reply_length == 0)
		{
			throw ProjectException(ProjectStatus::TcpClient_SocketClosedAtOtherEndpoint);
		}
		if (reply_length != buffer.length())
		{
			throw ProjectException(ProjectStatus::TcpClient_IncompleteRead);
		}
		data += buffer;
	}
	return data;
}

void TcpClient::write_string(const std::string& string_to_write)
{
	size_t already_written = 0;
	while (already_written < string_to_write.length())
	{
		std::string current_chunk = string_to_write.substr(already_written, CHUNK_SIZE);
		std::size_t write_length = boost::asio::write(socket, boost::asio::buffer(current_chunk.data(), current_chunk.length()));
		if (write_length != current_chunk.length())
		{
			throw ProjectException(ProjectStatus::TcpClient_IncompleteWrite);
		}
		already_written += write_length;
	}
}

