#pragma once

#include <string>
#include "basic_information.hpp"
#include "cryptography/RSAWrapper.h"

class ClientInformation
{
public:
	RSAPrivateWrapper rsa_private_wrapper;
	BasicInformation basic_information;

	ClientInformation(const std::string&, const BasicInformation&);
	~ClientInformation() = default;

	static ClientInformation read_from_file(const std::string&);
	static void write_to_file(const std::string&, const ClientInformation&);
};
