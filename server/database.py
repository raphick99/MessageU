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
        self.connection.execute('CREATE TABLE IF NOT EXISTS message_counter(Next INTEGER PRIMARY KEY)')
        if not self.connection.execute('SELECT * FROM message_counter').fetchone():
            self.connection.execute('INSERT INTO message_counter values(?)', (0, ))
        self.connection.commit()

    @multithread_protect
    def add_message(self, to_client, from_client, message_type, content):
        self._update_last_seen(from_client)
        message_id = self._get_next_message_id()
        self.connection.execute(
            'INSERT INTO messages values (?, ?, ?, ?, ?)',
            (message_id, to_client, from_client, message_type, content)
        )
        self.connection.commit()
        return message_id

    @multithread_protect
    def add_client(self, client_id, name, public_key):
        try:
            self.connection.execute(
                'INSERT INTO clients(ID, Name, PublicKey) values (?, ?, ?)',
                (client_id, name, public_key)
            )
        except sqlite3.IntegrityError:
            raise exceptions.ClientWithRequestedNameAlreadyRegistered(name)
        self._update_last_seen(client_id)
        self.connection.commit()

    @multithread_protect
    def get_client_list(self):
        return self.connection.execute('SELECT ID, Name FROM clients').fetchall()

    @multithread_protect
    def get_public_key_by_client_id(self, client_id):
        return self.connection.execute('SELECT PublicKey FROM clients WHERE ID = ?', (client_id, )).fetchone()

    @multithread_protect
    def extract_client_messages(self, client_id):
        self._update_last_seen(client_id)
        messages = self.connection.execute(
            'SELECT FromClient, ID, Type, Content FROM messages'
            ' WHERE ToClient = ?',
            (client_id,)
        ).fetchall()
        self.connection.execute('DELETE FROM messages WHERE ToClient = ?', (client_id,))
        self.connection.commit()
        return messages

    @multithread_protect
    def _get_next_message_id(self):
        next_message_id, = self.connection.execute('SELECT Next FROM message_counter').fetchone()
        self.connection.execute('UPDATE message_counter SET Next = ?', (next_message_id + 1,))
        self.connection.commit()
        return next_message_id

    @multithread_protect
    def _update_last_seen(self, client_id):
        last_seen = datetime.now().strftime('%Y-%m-%d %X')
        self.connection.execute('UPDATE clients SET LastSeen = ? WHERE ID = ?', (last_seen, client_id))
        self.connection.commit()
