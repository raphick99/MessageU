#pragma once

#include <exception>
#include "project_status.h"

/*
 * class for project exceptions. contains a status, which is the reason for the exception.
 */
class ProjectException : public std::exception
{
public:
	const ProjectStatus status;
	ProjectException(ProjectStatus);
};

/*
 * class for recoverable project exceptions. the same as ProjectException, 
 *  except that its recoverable, meaning it doesnt kill the client.
 */
class RecoverableProjectException : public ProjectException
{
	using ProjectException::ProjectException;
};

