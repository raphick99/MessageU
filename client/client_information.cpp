#include <filesystem>
#include <fstream>
#include <sstream>
#include <boost/algorithm/hex.hpp>
#include "client_information.hpp"
#include "project_exception.hpp"
#include "cryptography/Base64Wrapper.h"

ClientInformation::ClientInformation(const BasicInformation::UUID& _uuid, const std::string& _name, const std::string& _key) :
    BasicInformation(_uuid, _name),
    rsa_private_wrapper(_key)
{}

ClientInformation ClientInformation::read_from_file(const std::string& path)
{
    if (!(std::filesystem::exists(path)))
    {
        throw ProjectException(ProjectStatus::ClientInformation_FileDoesntExist);
    }

	std::ifstream f(path);
	std::stringstream file_contents;
	file_contents << f.rdbuf();

    std::string name;
    std::string uuid;
    std::string key_chunk, key;

	std::getline(file_contents, name);
	std::getline(file_contents, uuid);
    while (std::getline(file_contents, key_chunk))
    {
        key += key_chunk;
        key.push_back('\n');
    }

    BasicInformation::UUID unhexed_uuid;
    boost::algorithm::unhex(uuid, std::begin(unhexed_uuid));

    key = Base64Wrapper::decode(key);
    
    return ClientInformation(unhexed_uuid, name, key);
}

void ClientInformation::write_to_file(const std::string& path, const ClientInformation& client_information)
{
    std::ofstream file_to_write_to(path);
	std::string hexed_uuid, key;

    boost::algorithm::hex(client_information.uuid, std::begin(hexed_uuid));
    key = Base64Wrapper::encode(client_information.rsa_private_wrapper.getPrivateKey());

    file_to_write_to << client_information.name << "\n";
    file_to_write_to << hexed_uuid << "\n";
    file_to_write_to << key << "\n";
}

