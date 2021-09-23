#pragma once

#include <cstdint>

enum class MessageType : uint8_t
{
	RequestSymmetricKey = 1,
	SendSymmetricKey = 2,
	SendTextMessage = 3,
	SendFile = 4,
};

