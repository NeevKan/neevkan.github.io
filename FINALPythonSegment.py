import time
import tkinter as tk
import serial

# Replace with your actual port name from Arduino IDE
PORT = "/dev/cu.usbserial-A107ZEVA"
BAUD = 115200

print("Connecting to Arduino...")
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)
print("Connected!")

# Create a small window to capture keypresses safely on Mac
root = tk.Tk()
root.title("Stepper & Magnet Controller")
root.geometry("350x200")

label = tk.Label(
    root,
    text=(
        "Click this window, then press:\n\n[ UP / DOWN ] : X Axis\n[ W / S ]"
        "       : Y Axis\n[ M ]           : Toggle Magnet"
    ),
    font=("Arial", 11),
    justify="left",
)
label.pack(expand=True, padx=20, pady=20)


def handle_keypress(event):
  key = event.keysym
  if key.lower() == "w":
    ser.write(b"W")
    print("Sent: X Forward")
  elif key.lower() == "s":
    ser.write(b"S")
    print("Sent: X Backward")
  elif key.lower() == "a":
    ser.write(b"D")
    print("Sent: Y Forward")
  elif key.lower() == "d":
    ser.write(b"A")
    print("Sent: Y Backward")
  elif key.lower() == "m":
    ser.write(b"M")
    print("Sent: Toggle Magnet")
  elif key == 'Up':
      ser.write(b"L")
      print("Sent: Z Up")
  elif key == 'Down':
      ser.write(b"P")
      print("Sent: Z Down")


# Bind keyboard events to the window
root.bind("<Key>", handle_keypress)

root.mainloop()
ser.close()