class GeneralServerException(Exception):
    pass


class InvalidPortNumber(GeneralServerException):
    pass


class NonsupportedClientVersion(GeneralServerException):
    pass


class ReceivedNonNullTerminatedName(GeneralServerException):
    pass


class WrongPayloadSize(GeneralServerException):
    pass


class ClientWithRequestedNameAlreadyRegistered(GeneralServerException):
    pass
