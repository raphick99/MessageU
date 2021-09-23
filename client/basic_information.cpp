#include "basic_information.hpp"

BasicInformation::BasicInformation(UUID _uuid, std::string _name) :
	uuid(_uuid),
	name(std::move(_name))
{}

