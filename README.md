Introduction
--
For our 4180 project we decided to make our own version of an IOT enabled slate used for film productions. These "Smart Slates", as we have called them, have 6 primary functions.
They are:

1) Display the Roll, Scene, and Take number.
  
2) Keep track of the current time relative to the start of the shoot.
   
3) Keep track of the timestamp of the start of each scene.
   
4) Make it easy for the Assistant Director(will now be referenced as AD) to take notes on the current scene and save them for use while editing.
   
5) Make a nice clap sound in order to sync up the video and audio tracks.
    
6) Automatic time start and stop based on clap

For the purposes of designing this project we broke the problem up into 3 segments: detecting the clap and displaying relevant info, controling the slate and inputting relevant info, and the means with which the AD interfaces with the Smart Slate™️. To accomplish this we decided to not only create a Smart Slate™️ but also a desktop companion app that the AD can use to take notes and interface with the slate.

//-- Video Demo --//

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

Connect the parts as dictated in the wiring diagram, pin numbers are labeled appropriately. The 7 Segment display circuitry along with the CD4511BE are repeated 8 times for each individual part.

![SmartSlate](https://github.com/eollins/SmartSlateCompanion/assets/91165948/a9c38e4f-9ac8-4668-8196-3a34d50bfc50)


Print out the two .stl files in the repository and fit components as shown. Secure parts with tape as neccessary.
[SmartSlateBody](smartslatebody.stl)
[SmartSlateHand](smartslateslapper.stl)


INSERT PICTURES OF SLATE FRONT AND BACK

Software
-


