
import socket
import threading
import select
import time
import signal
import sys
import json


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('192.168.4.1', 2024))
#server.setblocking(False);
server.listen()
sockets = []
print("Now listening on IP 192.168.4.1")


def signal_handler(sig, frame):
	print("Interrupted: closing all sockets.")
	for s in sockets:
		print("Closing", s[1])
		s[0].close()
	print("Closing listener")
	server.close()
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def establish_connection(sock, addr):
	connected = True
	state = "init"
	clap = False
	
	while connected:
		if clap && state == "waiting":
			state = "clapped"
		if (state == clapped):
			state = "waiting"
		try:
			ready = select.select([sock], [], [], 1)
			start = time.time()
			while (not ready[0]):
				if (time.time() - start >= 5):
					print("Timeout", addr)
					sock.close()
					connected = False 
				ready = select.select([sock], [], [], 10)

			if ready[0] and connected:
				data = sock.recv(1024)
				print("Received", data.decode())
				sock.send(("SmartSlate establish").encode('utf-8'))
				json_dict = data.decode()
				if json_dict["command"] == "SceneTakeNums":
					#put scene json into dict so we can display on slate
					scene_dict = "placeholder"
				elif json_dict["command"]:
					#create notes dict that includes current scene info + notes snd append that dict to a list of dicts
					
		except Exception as e:
			print("Lost", addr, e)
			sock.close()
			connected = False

while True:
	conn, addr = server.accept()
	sockets.append((conn, addr))
	print("Accepted connection from", addr[0], addr[1])
	handle = threading.Thread(target=establish_connection, args=(conn,addr))
	handle.daemon = True
	handle.start()
