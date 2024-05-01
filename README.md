Introduction
--
Team Members: Ethan Ollins, Jack Cochran, Alex Peng

For our 4180 project we decided to make our own version of an IOT enabled slate used for film productions. These "Smart Slates", as we have called them, have 6 primary functions.
They are:

1) Display the Roll, Scene, and Take number.
  
2) Keep track of the current time relative to the start of the shoot.
   
3) Keep track of the timestamp of the start of each scene.
   
4) Make it easy for the Assistant Director(will now be referenced as AD) to take notes on the current scene and save them for use while editing.
   
5) Make a nice clap sound in order to sync up the video and audio tracks.
    
6) Automatic time start and stop based on clap

For the purposes of designing this project we broke the problem up into 3 segments: detecting the clap and displaying relevant info, controling the slate and inputting relevant info, and the means with which the AD interfaces with the Smart Slate™️. To accomplish this we decided to not only create a Smart Slate™️ but also a desktop companion app that the AD can use to take notes and interface with the slate.

<a href="http://youtu.be/_YN7Nneg3RY" target="_blank">//-- Video Demo --//</a>


Electrical and Hardware Design
-

List of Parts:
* Raspberry Pi 3b+
* Mbed microcontroller
* 8x 7 segment displays
* 1x Magnet Sensor
* 3x uLCD
* 8x CD4511BE
* 56x 330 Ohm Resistors
* ABS Filament
* Jumper Wires
* Electrical Tape

Connect the parts as dictated in the wiring diagram, pin numbers are labeled appropriately. The 7 Segment display circuitry along with the CD4511BE are repeated 8 times for each individual part. Each resistor in the diagram is a 330 Ohm resistor for each of the led segments in the 7 segment displays.

![SmartSlate](https://github.com/eollins/SmartSlateCompanion/assets/91165948/37397345-6701-40be-8acd-53e69f9c927e)



The ULCD's display the Roll, Scene, and Take number. The 7 segment displays displays the time and when the magnet sensor is opened and closed the time is stopped and recorded in the companion software.

Print out the two .stl files in the repository and fit components as shown. Secure parts with tape as neccessary.

[SmartSlateBody](smartslatebody.stl)
[SmartSlateHand](smartslateslapper.stl)

![Slate Front](https://github.com/eollins/SmartSlateCompanion/assets/91165948/4287fa16-3772-4246-b1cf-44e2e2c96084)
![Slate Back](https://github.com/eollins/SmartSlateCompanion/assets/91165948/dc97f9d7-b2ea-45a1-a74e-e8c866eb5b9c)


Software
--
To successfully recreate this project you will need to do the following 5 things.

1) Clone this github repo

2) Set up the raspberry pi to be an access point for a LAN

3) Take the contents of the folder named raspberry pi and copy them onto your raspberry pi

4) Take the contents of the folder named mbed and copy them onto your mbed

5) Download QT Creator and then copy the contents of the folder named gui into a new QT project and compile

The SmartSlate works via three distinct components.

1) Raspberry Pi "connectToCompanion" script
The Raspberry Pi inside the slate hosts a wireless access point using the hostapd and dnsmasq libraries. Through its wireless interface, it uses a static local IP address, 192.168.4.1. The connectToCompanion.py script runs as a background service on the Pi, and it simultaneously listens for and accepts TCP connections from users, monitors connections for heartbeats (allowing for timeout detection), maintains a "slate state," and communicates with the mbed. The Raspberry Pi script is a state machine of sorts, cycling between the states of "init" (when the slate is waiting for take information), "ready" (when the slate has received take information and is waiting for the operator to use the clapper), and "clapped" (when the slate has been clapped, the timecode has been reported, and the slate is waiting for notes to be published). The Pi script additionally saves all notes and updates made to them in a local tab-separated values file, which it additionally sends to the companion app. In summary, the primary purpose of the connectToCompanion script is to accept companion connections, communicate with the mbed's sensors and displays, and centralize the notes.

2) SmartSlateCompanion Qt5 C++ application
The SmartSlateCompanion application is used by the second-assistant director (2AD) to push roll, scene, and take information to the slate, as well as to record notes for each take that is completed (along with other useful information such as audio track number and audio device). The SmartSlateCompanion app, written in C++ using the Qt UI library, repeatedly attempts to open a TCP connection to the Raspberry Pi's static IP address until a successful connection is made. This is only possible once the user has connected their computer to the SmartSlate LAN, which is hosted by the Raspberry Pi. Once the companion has connected via port 2024, the user is able to monitor the current state of the slate (init, ready, or clapped). When the user inputs a roll, scene, and take number, the information is sent to the Raspberry Pi over TCP, and the state is updated to "ready." The 2AD then remains idle until the "clapped" state has been reached, after which the 2AD may enter take notes at any time (intended for when the director has called "cut"). Once this takes place, the slate's state is reset, and the relevant notes appear in the companion app. The 2AD may update, clear, or download the notes at any time. In summary, the companion is in the domain of the second assistant director, who is responsible for compiling notes.

3) mbed code
The mbed on the inside of the slate governs the display of received information as well as some elements of the state machine progression. It is connected to the Raspberry Pi via USB, allowing for communication via serial as well as allowing the mbed to receive power. The connectToCompanion script does not begin until the mbed has been properly detected over the serial port. Once connected, the mbed maintains a timecode relative to when it has powered on, using a separate thread to display this precise time on the array of seven-segment displays (with the aid of seven-segment driver chips that reduce the number of pins needed). Additionally, the mbed waits for serial input from the Pi, and once it receives a full set of roll/scene/take numbers (sent by the Pi in a specific format), it displays the received information on the uLCD displays. This then enables the mbed to begin listening for claps, which are detected by a magnet sensor embedded into the clapper arm itself. Once the clap has been registered, the timecode is frozen on the displays and sent back to the Pi, which records the timecode and updates its state to "clapped." Once the Pi sends a "clear" signal over serial, the mbed unfreezes the timecode display and erases the uLCD, and begins waiting for the next take. In summary, the mbed is primarily responsible for managing the physical displays on the slate, receiving pertinent display information from the Pi, and sending pertinent state information back in return.

Setting up a Raspberry PI as an access point
--
The Raspberry Pi, running the Raspbian OS, was configured to serve as a wireless access point. Though it is not connected to a router and therefore does not have access to the Internet, this provides the benefit of allowing hosts, such as a user's laptop, to connect to the Pi wirelessly and communicate via TCP sockets. Through the hostapd and dnsmasq libraries, the Raspberry Pi serves as an access point using its wireless connectivity interface and uses the fixed local IP address of 192.168.4.1. Communication between a user running the SmartSlateCompanion app and the slate itself takes place when the user connects to the SmartSlate network, and the companion application automatically establishes a TCP connection. The Raspberry Pi then serves as a middleman between the user and the visible elements of the slate, which are controlled by an internal mbed.
