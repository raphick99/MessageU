import socketserver


class ClientHandler(socketserver.StreamRequestHandler):
    def handle(self):
