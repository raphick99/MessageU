#pragma once

#include <string>
#include <boost/asio.hpp>
#include "project_exception.hpp"

class TcpClient
{
public:
	TcpClient(const std::string& host, const std::string& port);
	~TcpClient() = default;

	template<typename T>
	std::string read_prefixed_string()
	{
		T buffer_size = read_struct<T>();

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

	template<typename T>
	T read_struct()
	{
		T t{};
		std::size_t reply_length = boost::asio::read(socket, boost::asio::buffer(&t, sizeof(t)));
		if (reply_length == 0)
		{
			throw ProjectException(ProjectStatus::TcpClient_SocketClosedAtOtherEndpoint);
		}
		if (reply_length != sizeof(t))
		{
			throw ProjectException(ProjectStatus::TcpClient_IncompleteRead);
		}
		return t;
	}

	template<typename T>
	void write_prefixed_string(const std::string& string_to_write)
	{
		write_struct<T>(static_cast<T>(string_to_write.size()));
		std::size_t write_length = boost::asio::write(socket, boost::asio::buffer(string_to_write.data(), string_to_write.size()));
		if (write_length != string_to_write.size())
		{
			throw ProjectException(ProjectStatus::TcpClient_IncompleteWrite);
		}
	}

	template<typename T>
	void write_struct(const T& t)
	{
		std::size_t write_length = boost::asio::write(socket, boost::asio::buffer(&t, sizeof(t)));
		if (write_length != sizeof(t))
		{
			throw ProjectException(ProjectStatus::TcpClient_IncompleteWrite);
		}
	}

private:
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::resolver resolver;
};
