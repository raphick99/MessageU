#pragma once

#include <exception>
#include "project_status.hpp"

/*
* class for our internal project exceptions.
*/
class ProjectException : public std::exception
{
public:
	const ProjectStatus status;
	ProjectException(ProjectStatus);
};

