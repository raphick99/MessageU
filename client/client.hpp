#pragma once

#include <string>
#include <optional>
#include <utility>
#include <unordered_map>
#include <map>
#include "protocol/response_code.hpp"
#include "client_information.hpp"
#include "tcp_client.hpp"
#include "cryptography/RSAWrapper.h"
#include "cryptography/AESWrapper.h"
#include "protocol/pull_messages_response_entry.hpp"

class Client
{
private:
	std::pair<std::string, std::string> server_information;
	std::optional<ClientInformation> client_information;
	std::unordered_map<std::string, std::array<uint8_t, 16>> basic_client_information;
	std::map<std::array<uint8_t, 16>, RSAPublicWrapper> public_keys;
	std::map<std::array<uint8_t, 16>, AESWrapper> symmetric_keys;

public:
	Client();
	~Client() = default;

public:
	void register_request();
	void client_list_request();
	void get_public_key_request();
	void send_symmetric_key_request();
	void send_symmetric_key();
	void pull_messages_request();

private:
	void handle_symmetric_key_request(const Protocol::PullMessagesResponseEntry&);
	void handle_symmetric_key(const Protocol::PullMessagesResponseEntry&, TcpClient&);
	void handle_text_message(const Protocol::PullMessagesResponseEntry&, TcpClient&);

private:
	void print_message(const std::array<uint8_t, 16>&, const std::string&);
	std::string get_name();
	bool is_client_registered();
	static bool received_expected_response_code(Protocol::ResponseCode, Protocol::ResponseCode);
	static std::optional<ClientInformation> get_client_info();
	static std::pair<std::string, std::string> get_server_info();
};
