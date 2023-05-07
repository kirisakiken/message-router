import threading
import socket

HOST = 'localhost'
PORT = 8080

def listen_for_messages(sock):
    while True:
        data = sock.recv(1024).decode()
        if not data:
            break
        print(data)

def send_message(sock):
    while True:
        message = input()
        if message == "EOF":
            return
        sock.send(message.encode())

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

listen_thread = threading.Thread(target=listen_for_messages, args=(sock,))
send_thread = threading.Thread(target=send_message, args=(sock,))

listen_thread.start()
send_thread.start()

listen_thread.join()
send_thread.join()

sock.close()
