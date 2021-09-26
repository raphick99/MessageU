import socketserver
import itertools
import logging
import uuid
import database
import exceptions
import protocol


log = logging.getLogger(__name__)


class ClientHandler(socketserver.StreamRequestHandler):
    message_counter = itertools.count()

    def handle_register_request(self, request):
        db = database.Database()

        client_id = uuid.uuid4().bytes
        db.add_client(client_id, request.name, request.public_key)

        return protocol.RegisterResponse(
            code=protocol.ResponseCode.Register,
            client_id=client_id,
        )

    def handle_client_list_request(self, request):
        db = database.Database()

        client_list = [
            (client_id, name) for client_id, name in db.get_client_list()
            if client_id != request.client_id
        ]

        return protocol.ListUsersResponse(
            code=protocol.ResponseCode.ListUsers,
            client_list=client_list,
        )

    def handle(self):
        request = protocol.parse(self.rfile)
        log.debug(f'received: {request}')

        try:
            response = {
                protocol.RequestCode.Register: self.handle_register_request,
                protocol.RequestCode.ListUsers: self.handle_client_list_request,
            }[request.code](request)

        except exceptions.GeneralServerException as e:
            log.info(f'caught exception: {type(e).__name__}, {e.args}')
            response = protocol.ResponseHeader(code=protocol.ResponseCode.GeneralError)

        log.debug(f'sending: {response}')
        self.wfile.write(response.build())
