#pragma once

#include <utility>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include "project_exception.hpp"

namespace Utility
{
	std::pair<std::string, std::string> get_server_info(const std::string& path)
	{
		if (!(std::filesystem::exists(path)))
		{
			throw ProjectException(ProjectStatus::Utility_FileDoesntExist);
		}
		std::string host, port;

		std::ifstream server_file(path);
		std::stringstream server_file_contents;
		server_file_contents << server_file.rdbuf();

		if (server_file_contents.str().find(':') == std::string::npos)
		{
			throw ProjectException(ProjectStatus::Utility_InvalidServerInfo);
		}

		std::getline(server_file_contents, host, ':');
		std::getline(server_file_contents, port);

		return std::pair<std::string, std::string>(host, port);
	}
}

