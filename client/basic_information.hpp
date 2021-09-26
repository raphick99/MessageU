#pragma once

#include <array>
#include <string>
#include <cstdint>

class BasicInformation
{
public:
	const std::array<uint8_t, 16> client_id;
	const std::string name;

public:
	BasicInformation(const std::array<uint8_t, 16>&, const std::string&);
	~BasicInformation() = default;
};

