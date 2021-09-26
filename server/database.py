import sqlite3
from datetime import datetime
import itertools


DATABASE_FILE = 'server.db'


class Database:
    message_counter = itertools.count()

    def __init__(self):
        self.connection = sqlite3.connect(DATABASE_FILE)
        cur = self.connection.cursor()
        cur.execute('CREATE TABLE IF NOT EXISTS messages(ID INTEGER, ToClient BLOB, FromClient BLOB, Type INTEGER, Content BLOB)')
        cur.execute('CREATE TABLE IF NOT EXISTS clients(ID BLOB, Name TEXT, PublicKey BLOB, LastSeen DATE)')

    def add_message(self, to_client, from_client, type, content):
        # todo need to update client last seen
        pass

    def add_client(self, client_id, name, public_key):
        cur = self.connection.cursor()
        cur.execute('INSERT INTO clients values (?, ?, ?, ?)', (client_id, name, public_key, self.get_current_time()))
        self.connection.commit()

    def check_client_exists_by_name(self, name):
        cur = self.connection.cursor()
        return len(cur.execute('SELECT * FROM clients WHERE Name = ?', (name,)).fetchall()) > 0

    def get_client_list(self):
        cur = self.connection.cursor()
        return cur.execute('SELECT ID, Name FROM clients').fetchall()

    @staticmethod
    def get_current_time():
        return datetime.now().strftime('%Y-%m-%d %X')
