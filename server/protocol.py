import struct
import enum
import dataclasses
from typing import List, Tuple

import exceptions


CLIENT_VERSION = 1
SERVER_VERSION = 1


class RequestCode(enum.Enum):
    Register = 1000
    ListUsers = 1001
    GetPublicKey = 1002
    SendMessage = 1003
    PullMessages = 1004


@dataclasses.dataclass
class RequestHeader:
    client_id: bytes
    code: RequestCode
    payload_size: int

    header_layout = struct.Struct('<16sBHI')

    @classmethod
    def parse_header(cls, connection):
        client_id, version, code, payload_size = cls.header_layout.unpack(connection.read(cls.header_layout.size))

        if version != CLIENT_VERSION:
            raise exceptions.NonSupportedClientVersion(version)

        # Make sure the request code is valid.
        code = RequestCode(code)

        return cls(client_id, code, payload_size)


@dataclasses.dataclass
class RegisterRequest(RequestHeader):
    name: str
    public_key: bytes

    layout = struct.Struct('<255s160s')

    @classmethod
    def parse(cls, header: RequestHeader, connection):
        name, public_key = cls.layout.unpack(connection.read(cls.layout.size))

        if b'\x00' not in name:
            raise exceptions.ReceivedNonNullTerminatedName(name)

        name = name.rstrip(b'\x00')

        if header.payload_size != cls.layout.size:
            raise exceptions.WrongPayloadSize(f'(payload_size={header.payload_size})(layout_size={cls.layout.size})')

        return cls(header.client_id, header.code, header.payload_size, name, public_key)


@dataclasses.dataclass
class ListUsersRequest(RequestHeader):
    @classmethod
    def parse(cls, header: RequestHeader, connection):
        if header.payload_size != 0:
            raise exceptions.PayloadSizeForListUsersRequestShouldBeZero()

        return cls(header.client_id, header.code, header.payload_size)


def parse(data):
    header = RequestHeader.parse_header(data)
    return {
        RequestCode.Register: RegisterRequest.parse,
        RequestCode.ListUsers: ListUsersRequest.parse,
    }[header.code](header, data)


class ResponseCode(enum.Enum):
    Register = 2000
    ListUsers = 2001
    GetPublicKey = 2002
    SendMessage = 2003
    PullMessages = 2004
    GeneralError = 9000


class MessageType(enum.Enum):
    RequestSymmetricKey = 1
    SendSymmetricKey = 2
    SendTextMessage = 3
    SendFile = 4


@dataclasses.dataclass
class ResponseHeader:
    header_layout = struct.Struct('<BHI')

    @property
    def code(self):
        raise NotImplementedError()

    def _build_payload(self):
        return b''

    def build(self):
        payload = self._build_payload()
        return self.header_layout.pack(SERVER_VERSION, self.code.value, len(payload)) + payload


@dataclasses.dataclass
class GeneralErrorResponse(ResponseHeader):
    @property
    def code(self):
        return ResponseCode.GeneralError


@dataclasses.dataclass
class RegisterResponse(ResponseHeader):
    client_id: bytes

    layout = struct.Struct('<16s')

    @property
    def code(self):
        return ResponseCode.Register

    def _build_payload(self):
        return self.layout.pack(self.client_id)


@dataclasses.dataclass
class ListUsersResponse(ResponseHeader):
    client_list: List[Tuple[bytes, bytes]]

    layout = struct.Struct('<16s255s')

    @property
    def code(self):
        return ResponseCode.ListUsers

    def _build_payload(self):
        payload = b''
        for client_id, name in self.client_list:
            payload += self.layout.pack(client_id, name)
        return payload
