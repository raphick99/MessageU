#pragma once

#include <array>
#include <string>
#include <cstdint>

class BasicInformation
{
public:
	using CLIENT_ID = std::array<uint8_t, 16>;

public:
	const CLIENT_ID client_id;
	const std::string name;

public:
	BasicInformation(CLIENT_ID, const std::string&);
	~BasicInformation() = default;
};

