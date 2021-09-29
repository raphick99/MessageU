#pragma once

#include <string>
#include <optional>
#include <utility>
#include <unordered_map>
#include "protocol/response_code.hpp"
#include "client_information.hpp"

class Client
{
private:
	std::pair<std::string, std::string> server_information;
	std::optional<ClientInformation> client_information;
	std::unordered_map<std::string, std::array<uint8_t, 16>> basic_client_information;
	std::unordered_map<std::string, RSAPublicWrapper> public_keys;

public:
	Client();
	~Client() = default;

public:
	void register_request();
	void client_list_request();
	void get_public_key_request();
	void send_symmetric_key_request();

private:
	std::string get_name();
	bool is_client_registered();
	static bool received_expected_response_code(Protocol::ResponseCode, Protocol::ResponseCode);
	static std::optional<ClientInformation> get_client_info();
	static std::pair<std::string, std::string> get_server_info();
};
