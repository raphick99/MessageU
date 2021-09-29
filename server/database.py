import threading
import sqlite3
from datetime import datetime
import exceptions


DATABASE_FILE = 'server.db'
DATABASE_LOCK = threading.RLock()


def multithread_protect(func):
    def wrapper(*args, **kwargs):
        with DATABASE_LOCK:
            return func(*args, **kwargs)
    return wrapper


class Database:
    @multithread_protect
    def __init__(self):
        self.connection = sqlite3.connect(DATABASE_FILE)
        self.connection.text_factory = bytes
        self.connection.execute(
            'CREATE TABLE IF NOT EXISTS clients('
            'ID BLOB PRIMARY KEY,'
            'Name TEXT UNIQUE,'
            'PublicKey BLOB,'
            'LastSeen DATE)'
        )
        self.connection.execute(
            'CREATE TABLE IF NOT EXISTS messages('
            'ID INTEGER PRIMARY KEY,'
            'ToClient BLOB,'
            'FromClient BLOB,'
            'Type INTEGER,'
            'Content BLOB)'
        )
        self.connection.commit()

    @multithread_protect
    def add_message(self, message_id, to_client, from_client, message_type, content):
        self.connection.execute('UPDATE clients SET LastSeen = ? WHERE ID = ?', (self.get_current_time(), to_client))
        self.connection.execute(
            'INSERT INTO messages values (?, ?, ?, ?, ?)',
            (message_id, to_client, from_client, message_type, content)
        )
        self.connection.commit()

    @multithread_protect
    def add_client(self, client_id, name, public_key):
        try:
            self.connection.execute(
                'INSERT INTO clients values (?, ?, ?, ?)',
                (client_id, name, public_key, self.get_current_time())
            )
        except sqlite3.IntegrityError:
            raise exceptions.ClientWithRequestedNameAlreadyRegistered(name)
        self.connection.commit()

    @multithread_protect
    def get_client_list(self):
        return self.connection.execute('SELECT ID, Name FROM clients').fetchall()

    @multithread_protect
    def get_public_key_by_client_id(self, client_id):
        return self.connection.execute('SELECT PublicKey FROM clients WHERE ID = ?', (client_id, )).fetchone()

    @staticmethod
    def get_current_time():
        return datetime.now().strftime('%Y-%m-%d %X')
