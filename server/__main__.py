import socketserver
import logging
import environment
import client_handler

logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger(__name__)

def main():
    with socketserver.ThreadingTCPServer(('', environment.Environment().port), client_handler.ClientHandler) as server:
        server.serve_forever()


if __name__ == '__main__':
    main()
