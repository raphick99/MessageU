import sqlite3
from datetime import datetime
import exceptions


DATABASE_FILE = 'server.db'


class Database:
    def __init__(self):
        self.connection = sqlite3.connect(DATABASE_FILE)
        self.connection.execute(
            'CREATE TABLE IF NOT EXISTS messages('
            'ID INTEGER PRIMARY KEY,'
            'ToClient BLOB,'
            'FromClient BLOB,'
            'Type INTEGER,'
            'Content BLOB)'
        )
        self.connection.execute(
            'CREATE TABLE IF NOT EXISTS clients('
            'ID BLOB PRIMARY KEY,'
            'Name TEXT UNIQUE,'
            'PublicKey BLOB,'
            'LastSeen DATE)'
        )
        self.connection.commit()

    def add_message(self, to_client, from_client, type, content):
        # todo need to update client last seen
        pass

    def add_client(self, client_id, name, public_key):
        try:
            self.connection.execute(
                'INSERT INTO clients values (?, ?, ?, ?)',
                (client_id, name, public_key, self.get_current_time())
            )
        except sqlite3.IntegrityError:
            raise exceptions.ClientWithRequestedNameAlreadyRegistered(name)
        self.connection.commit()

    def get_client_list(self):
        return self.connection.execute('SELECT ID, Name FROM clients').fetchall()

    @staticmethod
    def get_current_time():
        return datetime.now().strftime('%Y-%m-%d %X')
