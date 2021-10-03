import struct
import enum
import dataclasses
from typing import List, Tuple


SERVER_VERSION = 2


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
