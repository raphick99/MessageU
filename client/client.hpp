#pragma once

#include <string>
#include <optional>
#include <utility>
#include <unordered_map>
#include <map>
#include <type_traits>
#include "client_information.hpp"
#include "config.hpp"
#include "tcp_client.hpp"
#include "cryptography/RSAWrapper.h"
#include "cryptography/AESWrapper.h"
#include "protocol/pull_messages_response_entry.hpp"
#include "protocol/request.hpp"
#include "protocol/response.hpp"
#include "protocol/response_code.hpp"
#include "protocol/client_id.hpp"

class Client
{
private:
	std::pair<std::string, std::string> server_information;
	std::optional<ClientInformation> client_information;
	std::unordered_map<std::string, Protocol::ClientID> basic_client_information;
	std::map<Protocol::ClientID, RSAPublicWrapper> public_keys;
	std::map<Protocol::ClientID, AESWrapper> symmetric_keys;

public:
	Client();
	~Client() = default;

	void register_request();
	void client_list_request();
	void get_public_key_request();
	void send_symmetric_key_request();
	void send_symmetric_key();
	void pull_messages_request();
	void send_text_message_request();

private:
	void handle_symmetric_key_request(const Protocol::PullMessagesResponseEntry&);
	void handle_symmetric_key(const Protocol::PullMessagesResponseEntry&, TcpClient&);
	void handle_text_message(const Protocol::PullMessagesResponseEntry&, TcpClient&);

	void print_message(const Protocol::ClientID&, const std::string&);
	Protocol::RequestHeader build_request(Protocol::RequestCode, size_t);
	bool is_client_registered();
	void assert_client_is_registered();
	Protocol::ClientID get_client_id();

	static std::string get_name();
	static void assert_received_expected_response_code(Protocol::ResponseCode, Protocol::ResponseCode);
	static std::pair<std::string, std::string> get_server_info();
	static std::optional<ClientInformation> get_client_info();

	template <typename Request>
	Protocol::ResponseHeader send_request(
		TcpClient& tcp_client,
		Protocol::RequestCode request_code,
		std::optional<std::reference_wrapper<const Request>> request = std::nullopt,
		std::optional<std::reference_wrapper<const std::string>> request_payload = std::nullopt
	)
	{
		size_t request_size = 0;
		if (request)
		{
			request_size += sizeof(request->get());
			if (request_payload)
			{
				request_size += request_payload->get().length();
			}
		}
		auto request_header = build_request(request_code, request_size);

		tcp_client.write_struct(request_header);
		if (request)
		{
			tcp_client.write_struct(request->get());
			if (request_payload)
			{
				tcp_client.write_string(request_payload->get());
			}
		}

		auto response_header = tcp_client.read_struct<Protocol::ResponseHeader>();

		auto expected_response_code = static_cast<Protocol::ResponseCode>(
			static_cast<std::underlying_type_t<Protocol::RequestCode>>(request_code) + 1000
		);
		assert_received_expected_response_code(expected_response_code, response_header.response_code);
		return response_header;
	}
};
