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
#include "protocol/get_public_key_request.hpp"
#include "protocol/get_public_key_response.hpp"
#include "protocol/send_message_request.hpp"
#include "protocol/send_message_response.hpp"
#include "protocol/pull_messages_response_entry.hpp"

Client::Client() :
	server_information(get_server_info()),
	client_information(get_client_info()),
	basic_client_information(),
	public_keys(),
	symmetric_keys()
{}

void Client::register_request()
{
	if (std::filesystem::exists(Config::me_info_filename))
	{
		std::cout << "\"" << Config::me_info_filename << "\" already exists. Cannot register again.\n";
		throw RecoverableProjectException(ProjectStatus::Client_AlreadyRegistered);
	}

	// Shouldnt pass this condition, since if the client_information is valid, me.info should exist.
	if (is_client_registered())
	{
		std::cout << "Reached unreachable code. Someone probably deleted \"" << Config::me_info_filename <<
			"\" in the middle of a run.\n";
		throw RecoverableProjectException(ProjectStatus::Client_UnreachableCodeInRegistration);
	}

	auto name = get_name();
	if (name.length() == 0)
	{
		std::cout << "Input length must be longer than 0.\n";
		throw RecoverableProjectException(ProjectStatus::Client_InputNameTooShort);
	}
	// Make sure input is less than the required size, counting on the \0.
	if (name.length() > sizeof(Protocol::RegisterRequest::name) - 1)
	{
		std::cout << "Input is too long. cannot be more than " << sizeof(Protocol::RegisterRequest::name) - 1 << "\n";
		throw RecoverableProjectException(ProjectStatus::Client_InputNameTooLong);
	}
	for (auto character : name)
	{
		if ((character > 0x7f) || (character < 0x20))
		{
			std::cout << "Input must be ascii, contains (0x" << std::hex << int(character) << ") which isnt ascii.\n";
			throw RecoverableProjectException(ProjectStatus::Client_InputContainsNonAsciiCharacter);
		}
	}

	// Create a private key.
	auto rsa_wrapper = RSAPrivateWrapper();

	std::string public_key = rsa_wrapper.getPublicKey();

	Protocol::RegisterRequest request{};
	std::copy(std::begin(name), std::end(name), std::begin(request.name));
	std::copy(std::begin(public_key), std::end(public_key), std::begin(request.public_key));

	TcpClient tcp_client(server_information.first, server_information.second);
	auto response_header = send_request<Protocol::RegisterRequest>(tcp_client, Protocol::RequestCode::Register, request);
	auto response = tcp_client.read_struct<Protocol::RegisterResponse>();

	client_information.emplace(response.client_id, name, rsa_wrapper.getPrivateKey());
	ClientInformation::write_to_file(Config::me_info_filename, client_information.value());

	std::cout << "Registered successfully!\n";
}

void Client::client_list_request()
{
	assert_client_registered();

	TcpClient tcp_client(server_information.first, server_information.second);
	auto response_header = send_request<Protocol::RequestHeader>(tcp_client, Protocol::RequestCode::ListUsers);

	size_t num_of_clients = static_cast<size_t>(response_header.payload_size) / sizeof(Protocol::ListClientResponseEntry);
	basic_client_information.clear();  // Keep only the most updated info.

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
		name.resize(end_of_name);

		std::cout << "Name: " << name << "\nClient ID: " << client_id << "\n";
		std::cout << "========================================================\n";

		basic_client_information.emplace(std::make_pair(name, response.client_id));  // Add to list.
	}
}

void Client::get_public_key_request()
{
	assert_client_registered();
	auto client_id = get_client_id();

	Protocol::GetPublicKeyRequest request{};
	std::copy(std::begin(client_id), std::end(client_id), std::begin(request.client_id));

	TcpClient tcp_client(server_information.first, server_information.second);

	auto response_header = send_request<Protocol::GetPublicKeyRequest>(tcp_client, Protocol::RequestCode::GetPublicKey, request);

	auto response = tcp_client.read_struct<Protocol::GetPublicKeyResponse>();
	assert_correct_client_id_in_response(request.client_id, response.client_id);

	std::string public_key;
	public_key.resize(response.public_key.size());
	std::copy(std::begin(response.public_key), std::end(response.public_key), std::begin(public_key));
	public_keys.emplace(std::make_pair(client_id, public_key));
	std::cout << "Public Key received successfully!\n";
}

void Client::pull_messages_request()
{
	assert_client_registered();

	TcpClient tcp_client(server_information.first, server_information.second);

	auto response_header = send_request<Protocol::RequestHeader>(tcp_client, Protocol::RequestCode::PullMessages);

	size_t already_read = 0;
	while (already_read < response_header.payload_size)
	{
		auto current_message_header = tcp_client.read_struct<Protocol::PullMessagesResponseEntry>();
		
		switch (current_message_header.message_type)
		{
		case Protocol::MessageType::RequestSymmetricKey:
			handle_symmetric_key_request(current_message_header);
			break;
		case Protocol::MessageType::SendSymmetricKey:
			handle_symmetric_key(current_message_header, tcp_client);
			break;
		case Protocol::MessageType::SendTextMessage:
			handle_text_message(current_message_header, tcp_client);
			break;
		case Protocol::MessageType::SendFile:
			handle_file(current_message_header, tcp_client);
			break;
		default:
			throw ProjectException(ProjectStatus::Client_InvalidMessageType);
			break;
		}
		already_read += sizeof(Protocol::PullMessagesResponseEntry) + current_message_header.payload_size;
	}
}

void Client::send_symmetric_key_request()
{
	assert_client_registered();
	auto client_id = get_client_id();
	send_message(client_id, Protocol::MessageType::RequestSymmetricKey);
}

void Client::send_symmetric_key()
{
	assert_client_registered();
	auto client_id = get_client_id();

	if (public_keys.find(client_id) == std::end(public_keys))
	{
		std::cout << "Cant send symmetric key, public key required.\n";
		return;
	}
	auto symmetric_key = AESWrapper::GenerateKey();
	symmetric_keys.emplace(std::make_pair(client_id, symmetric_key));
	auto encrypted_symmetric_key = public_keys.at(client_id).encrypt(symmetric_key);

	send_message(client_id, Protocol::MessageType::SendSymmetricKey, encrypted_symmetric_key);
}

void Client::send_text_message_request()
{
	assert_client_registered();
	auto client_id = get_client_id();

	if (symmetric_keys.find(client_id) == std::end(symmetric_keys))
	{
		std::cout << "can't send text message, symmetric key required.\n";
		return;
	}

	std::string message;
	std::cout << "Enter message:\n";
	std::getline(std::cin, message);

	auto encrypted_message = symmetric_keys.at(client_id).encrypt(message);

	send_message(client_id, Protocol::MessageType::SendTextMessage, encrypted_message);
}

void Client::send_file_request()
{
	assert_client_registered();
	auto client_id = get_client_id();

	if (symmetric_keys.find(client_id) == std::end(symmetric_keys))
	{
		std::cout << "can't send text message, symmetric key required.\n";
		return;
	}

	std::string file_path;
	std::cout << "Enter file path: ";
	std::getline(std::cin, file_path);


	if (!std::filesystem::exists(file_path))
	{
		std::cout << "file not found\n";
		throw RecoverableProjectException(ProjectStatus::Client_SendFileRequestNotFound);
	}

	std::ifstream file(file_path);
	std::stringstream file_contents;
	file_contents << file.rdbuf();
	auto encrypted_file_contents = symmetric_keys.at(client_id).encrypt(file_contents.str());

	send_message(client_id, Protocol::MessageType::SendFile, encrypted_file_contents);
}

void Client::handle_symmetric_key_request(const Protocol::PullMessagesResponseEntry& entry)
{
	print_message(entry.client_id, "Request for symmetric key");
}

void Client::handle_symmetric_key(const Protocol::PullMessagesResponseEntry& entry, TcpClient& tcp_client)
{
	auto encrypted_symmetric_key = tcp_client.read_string(entry.payload_size);
	auto symmetric_key = client_information->rsa_private_wrapper.decrypt(encrypted_symmetric_key);
	symmetric_keys.emplace(std::make_pair(entry.client_id, symmetric_key));

	print_message(entry.client_id, "symmetric key received");
}

void Client::handle_text_message(const Protocol::PullMessagesResponseEntry& entry, TcpClient& tcp_client)
{
	if (symmetric_keys.find(entry.client_id) == std::end(symmetric_keys))
	{
		print_message(entry.client_id, "can't decrypt message");
		tcp_client.read_string(entry.payload_size);  // remove payload from incoming buffer.
		return;
	}

	auto encrypted_message = tcp_client.read_string(entry.payload_size);
	auto message = symmetric_keys.at(entry.client_id).decrypt(encrypted_message);

	print_message(entry.client_id, message);
}

void Client::handle_file(const Protocol::PullMessagesResponseEntry& entry, TcpClient& tcp_client)
{
	if (symmetric_keys.find(entry.client_id) == std::end(symmetric_keys))
	{
		print_message(entry.client_id, "can't decrypt message");
		tcp_client.read_string(entry.payload_size);  // remove payload from incoming buffer.
		return;
	}

	auto encrypted_file = tcp_client.read_string(entry.payload_size);
	auto file_contents = symmetric_keys.at(entry.client_id).decrypt(encrypted_file);

	auto file_name = generate_random_filename() + ".MessageU";
	auto path = std::filesystem::temp_directory_path() / file_name;

	std::ofstream file(path);
	file << file_contents;
	file.close();

	auto file_received_message = "received file, saved at " + path.string();
	print_message(entry.client_id, file_received_message);
}

void Client::print_message(const Protocol::ClientID& client_id, const std::string& content)
{
	std::string name("<unknown>");
	for (const auto& [current_name, current_client_id] : basic_client_information)
	{
		if (client_id == current_client_id)
		{
			name = current_name;
			break;
		}
	}

	std::cout << "From: " << name << "\n";
	std::cout << "Content:\n";
	std::cout << content << "\n";
	std::cout << "-----<EOM>-----\n";
}

Protocol::RequestHeader Client::build_request(Protocol::RequestCode request_code, size_t payload_size)
{
	Protocol::RequestHeader request_header{};

	if (is_client_registered())
	{
		std::copy(std::begin(client_information->client_id), std::end(client_information->client_id), std::begin(request_header.client_id));
	}
	request_header.version = Config::client_version;
	request_header.request_code = request_code;
	request_header.payload_size = static_cast<uint32_t>(payload_size);

	return request_header;
}

void Client::send_message(
	const Protocol::ClientID& client_id,
	Protocol::MessageType message_type,
	std::optional<std::reference_wrapper<const std::string>> content
)
{
	Protocol::SendMessageRequest request{};
	std::copy(std::begin(client_id), std::end(client_id), std::begin(request.client_id));
	request.messsage_type = message_type;
	request.payload_size = 0;
	if (content)
	{
		request.payload_size = content->get().length();
	}

	TcpClient tcp_client(server_information.first, server_information.second);
	auto response_header = send_request<Protocol::SendMessageRequest>(tcp_client, Protocol::RequestCode::MessageSend, request, content);
	auto response = tcp_client.read_struct<Protocol::SendMessageResponse>();
	assert_correct_client_id_in_response(request.client_id, response.client_id);
	std::cout << "Message ID: " << response.message_id << "\n";
}

std::string Client::generate_random_filename()
{
	std::srand(std::time(nullptr));
	std::stringstream random_filename;
	char letters_and_numbers[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	for (int i = 0; i < 32; i++)
	{
		random_filename << letters_and_numbers[std::rand() % (sizeof(letters_and_numbers) - 1)];
	}

	return random_filename.str();
}

std::string Client::get_name()
{
	std::string name;
	std::cout << "Enter name: ";
	std::cin.ignore();
	std::getline(std::cin, name);

	return name;
}

bool Client::is_client_registered()
{
	return client_information.has_value();
}

void Client::assert_client_registered()
{
	if (!is_client_registered())
	{
		std::cout << "Client must be registered.\n";
		throw RecoverableProjectException(ProjectStatus::Client_ClientNotRegistered);
	}
}

void Client::assert_correct_client_id_in_response(const Protocol::ClientID& request_client_id, const Protocol::ClientID& response_client_id)
{
	if (response_client_id != request_client_id)
	{
		std::cout << "Received response with wrong client id\n";
		throw RecoverableProjectException(ProjectStatus::Client_WrongClientIDInResponse);
	}
}

Protocol::ClientID Client::get_client_id()
{
	auto name = get_name();
	if (basic_client_information.find(name) == std::end(basic_client_information))
	{
		std::cout << "No client with that name. try refreshing the client information.\n";
		throw RecoverableProjectException(ProjectStatus::Client_UnknownClientName);
	}

	return basic_client_information.at(name);
}

void Client::assert_received_expected_response_code(Protocol::ResponseCode expected_response_code, Protocol::ResponseCode received_response_code)
{
	if (received_response_code == Protocol::ResponseCode::GeneralError)
	{
		std::cout << "server responsed with an error\n";
		throw RecoverableProjectException(ProjectStatus::Client_ServerResponsedWithError);
	}
	if (received_response_code != expected_response_code)
	{
		std::cout << "server response code doesnt match request\n";
		throw RecoverableProjectException(ProjectStatus::Client_UnexpectedResponseCode);
	}
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

