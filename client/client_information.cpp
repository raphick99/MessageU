#include <filesystem>
#include <fstream>
#include <sstream>
#include <boost/algorithm/hex.hpp>
#include "client_information.hpp"
#include "project_exception.hpp"
#include "cryptography/Base64Wrapper.h"

ClientInformation::ClientInformation(const std::array<uint8_t, 16>& _client_id, const std::string& _name, const std::string& _key) :
    BasicInformation(_client_id, _name),
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
    std::string client_id;
    std::string key_chunk, key;

	std::getline(file_contents, name);
	std::getline(file_contents, client_id);
    while (std::getline(file_contents, key_chunk))
    {
        key += key_chunk;
        key.push_back('\n');
    }

    std::array<uint8_t, 16> unhexed_client_id;
    boost::algorithm::unhex(client_id, std::begin(unhexed_client_id));

    key = Base64Wrapper::decode(key);
    
    return ClientInformation(unhexed_client_id, name, key);
}

void ClientInformation::write_to_file(const std::string& path, const ClientInformation& client_information)
{
    std::ofstream file_to_write_to(path);
	std::string hexed_client_id, key;

    hexed_client_id.resize(client_information.client_id.size() * 2);  // multiply by 2 to account for the hex encoding.
    boost::algorithm::hex(client_information.client_id, std::begin(hexed_client_id));

    key = Base64Wrapper::encode(client_information.rsa_private_wrapper.getPrivateKey());

    file_to_write_to << client_information.name << "\n";
    file_to_write_to << hexed_client_id << "\n";
    file_to_write_to << key << "\n";
}

