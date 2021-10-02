#pragma once

/*
* Represents the possible internal statuses that may arrise.
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

	ClientInformation_FileDoesntExist,

};

