#pragma once

#include "basic_information.hpp"
#include "cryptography/AESWrapper.h"
#include "cryptography/RSAWrapper.h"

class ContactInformation : public BasicInformation
{
public:
	AESWrapper aes_wrapper;
	RSAPublicWrapper rsa_public_wrapper;
};
