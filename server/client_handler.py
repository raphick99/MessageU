import socketserver
import logging
import uuid
import database
import exceptions
import protocol


log = logging.getLogger(__name__)


class ClientHandler(socketserver.StreamRequestHandler):
    def handle_register_request(self, request):
        db = database.Database()

        client_id = uuid.uuid4().bytes
        db.add_client(client_id, request.name, request.public_key)

        return protocol.response.RegisterResponse(
            client_id=client_id,
        )

    def handle_client_list_request(self, request):
        db = database.Database()

        client_list = [
            (client_id, name) for client_id, name in db.get_client_list()
            if client_id != request.client_id
        ]

        return protocol.response.ListUsersResponse(
            client_list=client_list,
        )

    def handle_get_public_key_request(self, request):
        db = database.Database()

        result = db.get_public_key_by_client_id(request.requested_client_id)
        if not result:
            raise exceptions.NoClientWithRequestedClientID(request.requested_client_id)

        public_key, = result
        return protocol.response.GetPublicKeyResponse(
            client_id=request.requested_client_id,
            public_key=public_key,
        )

    def handle_send_message_request(self, request):
        db = database.Database()
        message_id = db.add_message(request.to_client, request.client_id, request.message_type.value, request.content)
        return protocol.response.SendMessageResponse(
            client_id=request.to_client,
            message_id=message_id,
        )

    def handle_pull_messages_request(self, request):
        db = database.Database()

        messages = db.extract_client_messages(request.client_id)

        return protocol.response.PullMessagesResponse(
            message_list=messages,
        )

    def handle(self):
        request = protocol.request.Request.parse_request(self.rfile)
        log.debug(f'received: {request}')

        try:
            response = {
                protocol.request.RequestCode.Register: self.handle_register_request,
                protocol.request.RequestCode.ListUsers: self.handle_client_list_request,
                protocol.request.RequestCode.GetPublicKey: self.handle_get_public_key_request,
                protocol.request.RequestCode.SendMessage: self.handle_send_message_request,
                protocol.request.RequestCode.PullMessages: self.handle_pull_messages_request,
            }[request.code](request)

        except exceptions.GeneralServerException as e:
            log.info(f'caught exception: {type(e).__name__}, {e.args}')
            response = protocol.response.GeneralErrorResponse()

        log.debug(f'sending: {response}')
        self.wfile.write(response.build())
