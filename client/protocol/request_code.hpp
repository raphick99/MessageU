#pragma once

#include <cstdint>

enum class RequestCode : uint16_t
{
	Register = 1000,
	ListUsers = 1001,
	GetPublicKey = 1002,
	SendMessage = 1003,
	PullMessages = 1004,
};

