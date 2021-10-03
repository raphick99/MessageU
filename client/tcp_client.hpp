#pragma once

#include <string>
#include <boost/asio.hpp>
#include "project_exception.hpp"

class TcpClient
{
public:
	TcpClient(const std::string& host, const std::string& port);
	~TcpClient() = default;

	std::string read_string(size_t);
	void write_string(const std::string&);

	// templated function, must be in header
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

