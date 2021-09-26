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
	server_information(get_server_info()),
	//contacts{},
	client_information(get_client_info())
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
	if (name.length() > sizeof(Protocol::RegisterRequest::name) - 1)
	{
		std::cout << "Input is too long. cannot be more than " << sizeof(Protocol::RegisterRequest::name) - 1 << "\n";
		return;
	}

	// Create a private key.
	auto rsa_wrapper = RSAPrivateWrapper();

	std::string public_key = rsa_wrapper.getPublicKey();

	Protocol::RegisterRequest request{};
	std::copy(std::begin(name), std::end(name), std::begin(request.name));
	std::copy(std::begin(public_key), std::end(public_key), std::begin(request.public_key));

	Protocol::RequestHeader request_header{};
	request_header.request_code = Protocol::RequestCode::Register;
	request_header.version = Config::version;
	request_header.payload_size = sizeof(request);

	tcp_client.write_struct(request_header);
	tcp_client.write_struct(request);

	auto response_header = tcp_client.read_struct<Protocol::ResponseHeader>();
	if (!received_expected_response_code(Protocol::ResponseCode::Register, response_header.response_code))
	{
		return;
	}

	auto response = tcp_client.read_struct<Protocol::RegisterResponse>();

	client_information.emplace(response.client_id, name, rsa_wrapper.getPrivateKey());

	ClientInformation::write_to_file(Config::me_info_filename, client_information.value());

	std::cout << "registered successfully\n";
}

void Client::client_list_request()
{
	if (!is_client_registered())
	{
		std::cout << "Client must be registered. returning...\n";
		return;
	}

	TcpClient tcp_client(server_information.first, server_information.second);
	Protocol::RequestHeader request_header{};

	std::copy(std::begin(client_information->client_id), std::end(client_information->client_id), std::begin(request_header.client_id));
	request_header.request_code = Protocol::RequestCode::ListUsers;
	request_header.version = Config::version;
	request_header.payload_size = 0;  // No payload, only request

	tcp_client.write_struct(request_header);

	auto response_header = tcp_client.read_struct<Protocol::ResponseHeader>();

	if (!received_expected_response_code(Protocol::ResponseCode::ListUsers, response_header.response_code))
	{
		return;
	}

	size_t num_of_clients = static_cast<size_t>(response_header.payload_size) / sizeof(Protocol::ListClientResponseEntry);

	std::cout << "========================================================\n";
	for (size_t i = 0; i < num_of_clients; i++)
	{
		auto response = tcp_client.read_struct<Protocol::ListClientResponseEntry>();
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

bool Client::received_expected_response_code(Protocol::ResponseCode expected_response_code, Protocol::ResponseCode received_response_code)
{
	if (received_response_code == Protocol::ResponseCode::GeneralError)
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

std::optional<ClientInformation> Client::get_client_info()
{
	try
	{
		auto client_information = ClientInformation::read_from_file(Config::me_info_filename);
		return std::make_optional<ClientInformation>(
			client_information.client_id,
			client_information.name,
			client_information.rsa_private_wrapper.getPrivateKey()
			);
	}
	catch (const ProjectException& e)
	{
		if (e.status != ProjectStatus::ClientInformation_FileDoesntExist)
		{
			throw;
		}
	}
	return std::nullopt;
}

std::pair<std::string, std::string> Client::get_server_info()
{
	if (!(std::filesystem::exists(Config::server_info_filename)))
	{
		throw ProjectException(ProjectStatus::Client_FileDoesntExist);
	}
	std::string host, port;

	std::ifstream server_file(Config::server_info_filename);
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

