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
import spidev
import binascii
import serial

# Establish a socket to listen on the static IP.
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('192.168.4.1', 2024))
server.listen()
sockets = []
print("Now listening on IP 192.168.4.1")

# Global clap variable.
clap = False

hatOn = False

debug = None
if hatOn:
	debug = SenseHat()

found = False
dev = 0
while not found:
	try:
		mbed = serial.Serial('/dev/ttyACM' + str(dev), 9600)
		found = True
	except:
		print("Didn't find", str(dev) + ", trying other")

		if dev == 0:
			dev = 1
		else:
			dev = 0
print("Found mbed")

# Initialize slate state.
state = {
	"state": "init",
	"roll": "",
	"scene": "",
	"take": "",
	"timecode": "",
	"notes": ""
}

# Thread for checking for temporary claps.
def check_for_claps():
	global clap
	global state

	while True:
		try:
			if state["state"] == "ready":
				print("Reading mbed")
				msg = mbed.readline()
				print("Received", msg)
				if msg[-5:-1].decode() == "CLAP":
					print("Clap detected.")
					if state["state"] == "ready":
						print("Set to clapped!")
						state["state"] = "clapped"
					
					print("Waiting for timecode")
					timecode = mbed.readline()
					print("Got the timecode")
					state["timecode"] = timecode.decode()[:-1]
		except:
			print("mbed not found")

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
	if hatOn:
		debug.clear((0, 255, 0))
	
	while connected:
		if state['state'] == "ready" and hatOn:
			debug.clear((0, 255, 0))
		elif state['state'] == "clapped" and hatOn:
			debug.clear((0, 0, 255))
		elif state['state'] == "init" and hatOn:
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
						state["state"] = "ready"

						try:
							m = "@R" + state["roll"] + "~S" + state["scene"] + "~T" + state["take"] + "~"
							print(m)
							mbed.write((m).encode('utf-8'))
							print("Sent to displays")
						except:
							print("mbed was gone")

					elif json_dict["command"] == "PushNotes":
						if state["state"] == "clapped":
							state["state"] = "init"
							notes_to_write = json_dict["payload"]
							d = date.today()
							file_name = "/home/pi/" + d.strftime("%d_%m_%y.txt")

							if not os.path.exists(file_name):
								file = open(file_name, 'x')
								file.close()

							file = open(file_name, 'a+')

							if state["roll"] != None and state["scene"] != None and state["take"] != None and notes_to_write["audio"] != None and notes_to_write["device"] != None and notes_to_write["notes"] != None:
								file.write(state["roll"] + "\t")
								file.write(state["scene"] + "\t")
								file.write(state["take"] + "\t")
								file.write(notes_to_write["audio"] + "\t")
								file.write(notes_to_write["device"] + "\t")
								file.write(state["timecode"] + "\t")
								file.write(notes_to_write["notes"] + "\n")

							file.close()
							print(json_dict["payload"])
							print("Took notes", json_dict["payload"])
			
							try:
								mbed.write(("@").encode('utf-8'))
								print("Sent reset")
							except:
								print("mbed was gone")

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
						file = "/home/pi/" + d.strftime("%d_%m_%y.txt")
						f = open(file, "w")
						f.write(newNotes)
						f.close()
						state["notes"] = newNotes

					elif json_dict["command"] == "ClearNotes":
						d = date.today()
						file = "/home/pi/" + d.strftime("%d_%m_%y.txt")
						f = open(file, "w")
						f.write("")
						f.close()
						state["notes"] = ""

				d = date.today()
				file = "/home/pi/" + d.strftime("%d_%m_%y.txt")
				if os.path.exists(file):
					f = open(file, "r")
					notes = f.read()
					f.close()
					state["notes"] = notes
				else:
					state["notes"] = "No note file"

				sock.send((json.dumps(state)).encode('utf-8'))

		except Exception as e:
			if hatOn:
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
