#include <utility>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/algorithm/hex.hpp>
#include "client.hpp"
#include "tcp_client.hpp"
#include "project_exception.hpp"
#include "config.hpp"
#include "protocol/request.hpp"
#include "protocol/response.hpp"
#include "protocol/register_request.hpp"
#include "protocol/register_response.hpp"
#include "protocol/list_client_response_entry.hpp"

Client::Client() :
	server_information(get_server_info(Config::server_info_filename)),
	//contacts{},
	client_information{}
{}

void Client::register_request()
{
	if (std::filesystem::exists(Config::me_info_filename))
	{
		std::cout << "\"" << Config::me_info_filename << "\" already exists. Cannot register again.\n";
		return;
	}

	// Shouldnt pass this condition, since if the client_information is valid, me.info should exist.
	if (is_client_registered())
	{
		std::cout << "Reached unreachable code. Someone probably deleted \"" << Config::me_info_filename <<
			"\" in the middle of a run. returning...\n";
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
	if (!received_expected_response_code(ResponseCode::Register, response_header.response_code))
	{
		return;
	}

	auto response = tcp_client.read_struct<RegisterResponse>();

	client_information.emplace(response.uuid, name, rsa_wrapper.getPrivateKey());

	ClientInformation::write_to_file(Config::me_info_filename, client_information.value());

	std::cout << "registered successfully\n";
}

void Client::client_list_request()
{
	if (is_client_registered())
	{
		std::cout << "Client must be registered. returning...\n";
		return;
	}

	TcpClient tcp_client(server_information.first, server_information.second);
	RequestHeader request_header{};

	std::copy(std::begin(client_information->uuid), std::end(client_information->uuid), std::begin(request_header.client_id));
	request_header.request_code = RequestCode::ListUsers;
	request_header.version = Config::version;
	request_header.payload_size = 0;  // No payload, only request

	tcp_client.write_struct(request_header);

	auto response_header = tcp_client.read_struct<ResponseHeader>();

	if (!received_expected_response_code(ResponseCode::ListUsers, response_header.response_code))
	{
		return;
	}

	size_t num_of_clients = static_cast<size_t>(response_header.payload_size) / sizeof(ListClientResponseEntry);

	for (size_t i = 0; i < num_of_clients; i++)
	{
		auto response = tcp_client.read_struct<ListClientResponseEntry>();
		std::string client_id, name;

		client_id.resize(response.client_id.size() * 2);  // multiply by 2 to account for the hex encoding.
		boost::algorithm::hex(response.client_id, std::begin(client_id));

		name.resize(response.name.size());
		std::copy_if(std::begin(response.name), std::end(response.name), std::begin(name), [](char c){ return c != '\0'; });
		auto end_of_name = name.find('\0');
		name.resize(end_of_name + 1);

		std::cout << "Name: " << name << "\nClient ID: " << client_id << "\n";
		std::cout << "========================================================\n";
	}
}

bool Client::is_client_registered()
{
	return client_information.has_value();
}

bool Client::received_expected_response_code(ResponseCode expected_response_code, ResponseCode received_response_code)
{
	if (received_response_code == ResponseCode::GeneralError)
	{
		std::cout << "server responsed with an error\n";
		return false;
	}
	if (received_response_code != expected_response_code)
	{
		std::cout << "server response code doesnt match request\n";
		return false;
	}
	return true;
}

std::pair<std::string, std::string> Client::get_server_info(const std::string& path)
{
	if (!(std::filesystem::exists(path)))
	{
		throw ProjectException(ProjectStatus::Client_FileDoesntExist);
	}
	std::string host, port;

	std::ifstream server_file(path);
	std::stringstream server_file_contents;
	server_file_contents << server_file.rdbuf();

	if (server_file_contents.str().find(':') == std::string::npos)
	{
		throw ProjectException(ProjectStatus::Client_InvalidServerInfo);
	}

	std::getline(server_file_contents, host, ':');
	std::getline(server_file_contents, port);

	return std::pair<std::string, std::string>(host, port);
}

