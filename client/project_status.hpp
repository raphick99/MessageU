#pragma once

/*
* Represents the possible statuses that may be thrown.
*/
enum class ProjectStatus
{
	Uninitialized = -1,
	Success = 0,

	TcpClient_SocketClosedAtOtherEndpoint,
	TcpClient_IncompleteRead,
	TcpClient_IncompleteWrite,

	Client_FileDoesntExist,
	Client_InvalidServerInfo,
	Client_InvalidMessageType,
	Client_UnexpectedResponseCode,
	Client_ServerResponsedWithError,
	Client_ClientNotRegistered,
	Client_UnknownClientName,
	Client_InputNameTooShort,
	Client_InputNameTooLong,
	Client_SendFileRequestNotFound,
	Client_AlreadyRegistered,
	Client_UnreachableCodeInRegistration,
	Client_WrongClientIDInResponse,

	ClientInformation_FileDoesntExist,
};

