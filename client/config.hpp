#pragma once

#include <cstdint>
#include <string>

/*
* Config namespace. declares configurations for the project
*/
namespace Config
{
	static const uint8_t version = 1;
	static const std::string server_info_filename("server.info");
	static const std::string me_info_filename("me.info");
}
