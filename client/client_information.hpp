#pragma once

#include <string>
#include <array>
#include <cstdint>
#include "cryptography/RSAWrapper.h"

class ClientInformation
{
public:
	const std::array<uint8_t, 16> client_id;
	const std::string name;
	RSAPrivateWrapper rsa_private_wrapper;

	ClientInformation(const std::array<uint8_t, 16>&, const std::string&, const std::string&);
	~ClientInformation() = default;

	static ClientInformation read_from_file(const std::string&);
	static void write_to_file(const std::string&, const ClientInformation&);
};
