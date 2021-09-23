#pragma once

#include <array>
#include <string>
#include <cstdint>

class BasicInformation
{
public:
	using UUID = std::array<uint8_t, 16>;

public:
	const UUID uuid;
	const std::string name;

public:
	BasicInformation(UUID, std::string);
	~BasicInformation() = default;
};

