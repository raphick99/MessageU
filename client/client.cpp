#include <utility>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include "client.hpp"
#include "tcp_client.hpp"
#include "project_exception.hpp"
#include "config.hpp"
#include "protocol/request.hpp"
#include "protocol/response.hpp"
#include "protocol/register_request.hpp"
#include "protocol/register_response.hpp"

Client::Client() :
	server_information(get_server_info(Config::server_info_filename)),
	//contacts{},
	client_information{}
{}

void Client::register_client()
{
	if (std::filesystem::exists(Config::me_info_filename))
	{
		std::cout << "\"" << Config::me_info_filename << "\" already exists. Cannot register again.\n";
		return;
	}
	// Shouldnt pass this condition, since if the client_information is valid, me.info should exist.
	if (client_information.has_value())
	{
		std::cout << "Reached unreachable code. returning\n";
		return;
	}

	TcpClient tcp_client(server_information.first, server_information.second);

	std::string name;
	std::cout << "Enter name: ";
	std::cin.ignore();
	std::getline(std::cin, name);

	// Make sure input is less than the required size, counting on the \0.
	if (name.length() > sizeof(RegisterRequest::name) - 1)
	{
		std::cout << "Input is too long. cannot be more than " << sizeof(RegisterRequest::name) - 1 << "\n";
		return;
	}

	// Create a private key.
	auto rsa_wrapper = RSAPrivateWrapper();

	std::string public_key = rsa_wrapper.getPublicKey();

	RegisterRequest request{};
	std::copy(std::begin(name), std::end(name), std::begin(request.name));
	std::copy(std::begin(public_key), std::end(public_key), std::begin(request.public_key));

	RequestHeader request_header{};
	request_header.request_code = RequestCode::Register;
	request_header.version = Config::version;
	request_header.payload_size = sizeof(request);

	tcp_client.write_struct(request_header);
	tcp_client.write_struct(request);

	auto response_header = tcp_client.read_struct<ResponseHeader>();
	auto response = tcp_client.read_struct<RegisterResponse>();

	if (response_header.response_code == ResponseCode::GeneralError)
	{
		std::cout << "server responsed with an error\n";
		return;
	}
	if (response_header.response_code != ResponseCode::Register)
	{
		std::cout << "server response code doesnt match request\n";
		return;
	}

	client_information.emplace(response.uuid, name, rsa_wrapper.getPrivateKey());

	ClientInformation::write_to_file(Config::me_info_filename, client_information.value());
}

std::pair<std::string, std::string> Client::get_server_info(const std::string& path)
{
	if (!(std::filesystem::exists(path)))
	{
		throw ProjectException(ProjectStatus::Utility_FileDoesntExist);
	}
	std::string host, port;

	std::ifstream server_file(path);
	std::stringstream server_file_contents;
	server_file_contents << server_file.rdbuf();

	if (server_file_contents.str().find(':') == std::string::npos)
	{
		throw ProjectException(ProjectStatus::Utility_InvalidServerInfo);
	}

	std::getline(server_file_contents, host, ':');
	std::getline(server_file_contents, port);

	return std::pair<std::string, std::string>(host, port);
}

