import socketserver
import logging
import environment
import client_handler


logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger(__name__)


def main():
    env = environment.Environment()
    log.info(f'listening on {env.port}')
    with socketserver.ThreadingTCPServer(('', env.port), client_handler.ClientHandler) as server:
        server.serve_forever()


if __name__ == '__main__':
    main()
