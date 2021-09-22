import exceptions


PORT_FILE = 'port.info'


class Environment:
    def __init__(self):
        self.port = self._parse_port_file()

    @staticmethod
    def _parse_port_file():
        with open(PORT_FILE, 'r') as port_file:
            port = int(port_file.readline())
            if port < 0 or port > 2**16:  # make sure is in valid range
                raise exceptions.InvalidPortNumber(port)
        return port
