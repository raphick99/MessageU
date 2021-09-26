#include "basic_information.hpp"

BasicInformation::BasicInformation(const std::array<uint8_t, 16>& _client_id, const std::string& _name) :
	client_id(_client_id),
	name(_name)
{}

