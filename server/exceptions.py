class GeneralServerException(Exception):
    pass


class InvalidPortNumber(GeneralServerException):
    pass


class NonSupportedClientVersion(GeneralServerException):
    pass


class ReceivedNonNullTerminatedName(GeneralServerException):
    pass


class WrongPayloadSize(GeneralServerException):
    pass


class ClientWithRequestedNameAlreadyRegistered(GeneralServerException):
    pass


class PayloadSizeForListUsersRequestShouldBeZero(GeneralServerException):
    pass


class NoClientWithRequestedClientID(GeneralServerException):
    pass


class PayloadSizeForPullMessagesRequestShouldBeZero(GeneralServerException):
    pass
