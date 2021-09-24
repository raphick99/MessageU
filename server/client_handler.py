import socketserver
import traceback
import logging
import uuid
import database
import exceptions
import protocol


log = logging.getLogger(__name__)


class ClientHandler(socketserver.StreamRequestHandler):
    def handle_register_request(self, request):
        db = database.Database()
        if db.check_client_exists_by_name(request.name):
            raise exceptions.ClientWithRequestedNameAlreadyRegistered(request.name)

        client_id = uuid.uuid4().bytes
        db.add_client(client_id, request.name, request.public_key)

        return protocol.RegisterResponse(
            code=protocol.ResponseCode.Register,
            uuid=client_id,
        )

    def handle(self):
        request = protocol.parse(self.rfile)
        log.debug(f'received: {request}')

        try:
            response = {
                protocol.RequestCode.Register: self.handle_register_request
            }[request.code](request)

        except exceptions.GeneralServerException as e:
            log.info(f'caught exception: {type(e).__name__}, {e.args}')
            response = protocol.ResponseHeader(code=protocol.ResponseCode.GeneralError)

        log.debug(f'sending: {response}')
        self.wfile.write(response.build())
