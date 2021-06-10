#########################################################################################################################################################################
####################################################### FOR BEST RESULTS RUN IN PYTHON 3.8.X AND IDLE ###################################################################
import serial
import threading
import tkinter as tk
from tkinter import *

bg = tk.Tk()
bg.configure(background="black")
bg.title("LED MONITORING APPLICATION")

#Store the old state, print the new state only if old state is changed
#Tracking the State of the LED's
def track_state():
    curr = uart.read()
    while 1:
        new = uart.read()
        if new != curr:
            curr = new
            count = int.from_bytes(curr, "big")
            state = calculate_state(count)
            print("CURRENT STATE: Count - " + str(count) + ", LED State - " + str(state) + "\n")
            
#Cycle to the next state    
def next():
    print("We're moving on to the next state\n")
    uart.write(str.encode('i'))
    
#Cycle to the previous state    
def prev():
    print("We're moving back to the previous state\n")
    uart.write(str.encode('d'))

#Turn of the LED's
def off():
    print("Final STATE: LED's are OFF\n")
    uart.write(str.encode('o'))

#Quit Application    
def quit():
    print("\n***** EXIT SUCCESSFULLY *****")
    uart.write(str.encode('q'))
    uart.close()
    bg.destroy()

#Fetch State Counter from MSP432
def fetch_count():
    try:
        current = uart.read()
        print(current)
        current = int.from_bytes(current, "big")
        print(current)
        state = calculate_state(current)
        print(state)
        print("Your current state is: " + str(current) +" "+ state + "\n")
    except:
        print("ERROR: INVALID STATE" + "\n")

#Calculate State given the State Counter
def calculate_state(data):
    if data == 0:
        status = "OFF"
    if data == 1:
        status = "RED"
    if data == 2:
        status = "GREEN"
    if data == 3:
        status = "BLUE"
    if data == 4:
        status = "CYAN"
    return status
    
b1 = tk.Button(bg, text="NEXT", command=next, bg="#00fc2e", fg="black", font=("Book Antiqua", 25))
b2 = tk.Button(bg, text="PREVIOUS", command=prev, bg="#3aadf4", fg="black", font=("Book Antiqua", 25))
###Uncoment b3 to use Button to explicitly Grab current state
b3 = tk.Button(bg, text=" STATE", command=fetch_count, bg="#6d41fc", fg="black", font=("Book Antiqua", 25))
b4 = tk.Button(bg, text="OFF", command=off, bg="#51c6b9", fg="black", font=("Book Antiqua", 25))
b5 = tk.Button(bg, text="EXIT", command=quit, bg="#fc0a0e", fg="black", font=("Book Antiqua", 25))

b1.grid(row=1, column=2, padx=15, pady=15)
b2.grid(row=2, column=2, padx=15, pady=15)
###b3 is an additional functionality I was trying to implement
b3.grid(row=4, column=2, padx=15, pady=15)
b4.grid(row=5, column=2, padx=15, pady=15)
b5.grid(row=6, column=2, padx=15, pady=15)

uart = serial.Serial(port='COM5', baudrate=9600, bytesize=8, parity=serial.PARITY_NONE, stopbits=1)

thread = threading.Thread(target=track_state).start()

bg.mainloop()
