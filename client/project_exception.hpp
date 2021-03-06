#pragma once

#include <exception>
#include "project_status.hpp"

class ProjectException : public std::exception
{
public:
	const ProjectStatus status;
	ProjectException(ProjectStatus);
};

class RecoverableProjectException : public ProjectException
{
	using ProjectException::ProjectException;
};

