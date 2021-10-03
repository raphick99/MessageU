import struct
import enum
import dataclasses
from typing import List, Tuple

import exceptions


CLIENT_VERSION = 1
SERVER_VERSION = 2


class RequestCode(enum.Enum):
    Register = 1000
    ListUsers = 1001
    GetPublicKey = 1002
    SendMessage = 1003
    PullMessages = 1004


class MessageType(enum.Enum):
    RequestSymmetricKey = 1
    SendSymmetricKey = 2
    SendTextMessage = 3
    SendFile = 4


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


@dataclasses.dataclass
class GetPublicKeyRequest(RequestHeader):
    requested_client_id: bytes

    layout = struct.Struct('<16s')

    @classmethod
    def parse(cls, header: RequestHeader, connection):
        client_id, = cls.layout.unpack(connection.read(cls.layout.size))

        if header.payload_size != cls.layout.size:
            raise exceptions.WrongPayloadSize(f'(payload_size={header.payload_size})(layout_size={cls.layout.size})')

        return cls(header.client_id, header.code, header.payload_size, client_id)


@dataclasses.dataclass
class SendMessageRequest(RequestHeader):
    to_client: bytes
    message_type: MessageType
    content: bytes

    layout = struct.Struct('<16sBI')

    @classmethod
    def parse(cls, header: RequestHeader, connection):
        to_client, message_type, content_size = cls.layout.unpack(connection.read(cls.layout.size))
        message_type = MessageType(message_type)
        content = connection.read(content_size)  # TODO should possibly read in chunks
        return cls(header.client_id, header.code, header.payload_size, to_client, message_type, content)


@dataclasses.dataclass
class PullMessagesRequest(RequestHeader):
    @classmethod
    def parse(cls, header: RequestHeader, connection):
        if header.payload_size != 0:
            raise exceptions.PayloadSizeForPullMessagesRequestShouldBeZero()

        return cls(header.client_id, header.code, header.payload_size)


def parse(data):
    header = RequestHeader.parse_header(data)
    return {
        RequestCode.Register: RegisterRequest.parse,
        RequestCode.ListUsers: ListUsersRequest.parse,
        RequestCode.GetPublicKey: GetPublicKeyRequest.parse,
        RequestCode.SendMessage: SendMessageRequest.parse,
        RequestCode.PullMessages: PullMessagesRequest.parse,
    }[header.code](header, data)


class ResponseCode(enum.Enum):
    Register = 2000
    ListUsers = 2001
    GetPublicKey = 2002
    SendMessage = 2003
    PullMessages = 2004
    GeneralError = 9000


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


@dataclasses.dataclass
class GetPublicKeyResponse(ResponseHeader):
    client_id: bytes
    public_key: bytes

    layout = struct.Struct('<16s160s')

    @property
    def code(self):
        return ResponseCode.GetPublicKey

    def _build_payload(self):
        return self.layout.pack(self.client_id, self.public_key)


@dataclasses.dataclass
class SendMessageResponse(ResponseHeader):
    client_id: bytes
    message_id: int

    layout = struct.Struct('<16sI')

    @property
    def code(self):
        return ResponseCode.SendMessage

    def _build_payload(self):
        return self.layout.pack(self.client_id, self.message_id)


@dataclasses.dataclass
class PullMessagesResponse(ResponseHeader):
    message_list: List[Tuple[bytes, int, int, bytes]]

    layout = struct.Struct('<16sIBI')

    @property
    def code(self):
        return ResponseCode.PullMessages

    def _build_payload(self):
        payload = b''
        for client_id, message_id, message_type, content in self.message_list:
            payload += self.layout.pack(client_id, message_id, message_type, len(content)) + content
        return payload
