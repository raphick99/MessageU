#pragma once

#include <string>
#include "cryptography/RSAWrapper.h"
#include "protocol/client_id.hpp"

class ClientInformation
{
public:
	const Protocol::ClientID client_id;
	const std::string name;
	RSAPrivateWrapper rsa_private_wrapper;

	ClientInformation(const Protocol::ClientID&, const std::string&, const std::string&);
	~ClientInformation() = default;

	static ClientInformation read_from_file(const std::string&);
	static void write_to_file(const std::string&, const ClientInformation&);
};

