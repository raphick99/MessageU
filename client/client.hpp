#pragma once

#include <string>
#include <optional>
#include <utility>
//#include <unordered_map>
#include "contact_information.hpp"
#include "client_information.hpp"

class Client
{
private:
	std::pair<std::string, std::string> server_information;
	//std::unordered_map<BasicInformation::UUID, ContactInformation> contacts;
	std::optional<ClientInformation> client_information;

public:
	Client();
	~Client() = default;

public:
	void register_client();

private:
	static std::pair<std::string, std::string> get_server_info(const std::string&);
};
