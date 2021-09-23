#pragma once

#include <string>
#include "basic_information.hpp"
#include "cryptography/RSAWrapper.h"

class ClientInformation : public BasicInformation
{
public:
	RSAPrivateWrapper rsa_private_wrapper;

	ClientInformation(const BasicInformation::UUID&, const std::string&, const std::string&);
	~ClientInformation() = default;

	static ClientInformation read_from_file(const std::string&);
	static void write_to_file(const std::string&, const ClientInformation&);
};
