
import socket
import threading 
import select 
import time 
import signal 
import sys 
import json 
import traceback 
import os
from datetime import date 
from sense_hat import SenseHat

# Establish a socket to listen on the static IP.
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('192.168.4.1', 2025))
server.listen()
sockets = []
print("Now listening on IP 192.168.4.1")

# Global clap variable.
clap = False
#comment 
debug = SenseHat()

# Initialize slate state.
state = {
	"state": "init",
	"roll": "",
	"scene": "",
	"take": "",
	"notes": ""
}

# Thread for checking for temporary claps.
def check_for_claps():
	global clap
	global state

	while True:
		if len(debug.stick.get_events()) > 0:
			if state["state"] == "ready":
				state["state"] = "clapped"
				debug.clear((0, 0, 255))
				for s in sockets:
					s[0].send((json.dumps(state)).encode('utf-8'))
				print("Sent clap")

# Closes all sockets once interrupted.
def signal_handler(sig, frame):
	print("Interrupted: closing all sockets.")
	for s in sockets:
		print("Closing", s[1])
		s[0].close()
	print("Closing listener")
	server.close()
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

# Connection thread.
def establish_connection(sock, addr):
	global clap
	global state

	connected = True
	debug.clear((0, 255, 0))
	
	while connected:
		if state['state'] == "ready":
			debug.clear((0, 255, 0))
		elif state['state'] == "clapped":
			debug.clear((0, 0, 255))
		elif state['state'] == "init":
			debug.clear((255, 255, 0))

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

				if "Heartbeat" not in data.decode():
					print("Received", data.decode())

				data_s = data.decode()
				if data_s is not None:
					json_dict = json.loads(data_s)
					if json_dict["command"] == "PushToSlate":
						state["roll"] = json_dict["payload"]["roll"]
						state["scene"] = json_dict["payload"]["scene"]
						state["take"] = json_dict["payload"]["take"]
						state["message"] = "Successfully updated scene/take nums"
						print("Updated take info to", state["roll"], state["scene"], state["take"])
                        			#ulcd displays roll, scene, and take
						state["state"] = "ready"

					elif json_dict["command"] == "PushNotes":
						if state["state"] == "clapped":
							state["state"] = "init"
                            				#payload contains audio, device, and notes
							notes_to_write = json_dict["payload"]
							d = date.today()
							file_name = d.strftime("%d_%m_%y.txt")

							if not os.path.exists(file_name):
								file = open(file_name, 'x')
								#file.write("ROLL\tSCENE\tTAKE\tAUDIO\tREC\tNOTES\n")
								file.close()

							file = open(file_name, 'a+') #if this ever fails put in try catch block and do error handling

							if state["roll"] != None and state["scene"] != None and state["take"] != None and notes_to_write["audio"] != None and notes_to_write["device"] != None and notes_to_write["notes"] != None:
								file.write(state["roll"] + "\t")
								file.write(state["scene"] + "\t")
								file.write(state["take"] + "\t")
								file.write(notes_to_write["audio"] + "\t")
								file.write(notes_to_write["device"] + "\t")
								file.write(notes_to_write["notes"] + "\n")

							file.close()
							print(json_dict["payload"])
							print("Took notes", json_dict["payload"])

					elif json_dict["command"] == "ResetTake":
						if state["state"] != "init":
							state["state"] = "init"
							state["roll"] = ""
							state["scene"] = ""
							state["take"] = ""
							state["message"] = ""
							print("Reset take info")

					elif json_dict["command"] == "UpdateNotes":
						pl = json_dict["payload"]
						notes = state["notes"]
						lines = notes.split("\n")
						cols = lines[int(pl["row"])].split('\t')
						cols[int(pl["column"])] = pl["new"]
						newLine = "\t".join(cols)
						lines[int(pl["row"])] = newLine
						newNotes = "\n".join(lines)

						d = date.today()
						file = d.strftime("%d_%m_%y.txt")
						f = open(file, "w")
						f.write(newNotes)
						f.close()
						state["notes"] = newNotes

				d = date.today()
				file = d.strftime("%d_%m_%y.txt")
				if os.path.exists(file):
					f = open(file, "r")
					notes = f.read()
					f.close()
					state["notes"] = notes
				else:
					state["notes"] = "No note file"

				sock.send((json.dumps(state)).encode('utf-8'))
					
		except Exception as e:
			debug.clear((255, 0, 0))
			traceback.print_exc()
			print("Lost", addr, e)
			sock.close()
			connected = False

clap_checker = threading.Thread(target=check_for_claps)
clap_checker.daemon = True
clap_checker.start()

while True:
	conn, addr = server.accept()
	sockets.append((conn, addr))
	print("Accepted connection from", addr[0], addr[1])
	handle = threading.Thread(target=establish_connection, args=(conn,addr))
	handle.daemon = True
	handle.start()
