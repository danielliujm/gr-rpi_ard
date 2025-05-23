import tkinter as tk
from tkinter import ttk
import serial
import threading
import time
import json
import numpy as np
import csv
from datetime import datetime

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
root = tk.Tk()
root.title("JSON Sensor Monitor & PWM Sender")

log_file = open("data/data_log.csv", "w", newline='')
csv_writer = csv.writer(log_file)

PWM_CHANNELS = 8
NUM_TMP = 6
NUM_PH = 2
NUM_ORP = 2 

# Write header row
csv_writer.writerow(["Timestamp", *[f"TMP{i+1}" for i in range(NUM_TMP)],
                     *[f"PH{i+1}" for i in range(NUM_PH)],
                     *[f"ORP{i+1}" for i in range(NUM_ORP)],
                     *[f"Motor{i+1}" for i in range(PWM_CHANNELS)]])





TMP_values = [tk.StringVar(value="--") for _ in range(NUM_TMP)]
PH_values = [tk.StringVar(value="--") for _ in range(NUM_PH)]
ORP_values = [tk.StringVar(value="--") for _ in range(NUM_ORP)]

pwm_entries = []


def send_pwm():
    
    try: 
        pwm_list = [int(entry.get()) for entry in pwm_entries]
        if all(0 <= val <= 100 for val in pwm_list):
            message = json.dumps({"pwm": pwm_list}) + '\n'
            ser.write(message.encode('utf-8'))
            print(f"Sent PWM values: {pwm_list}")
        else:
            print("Invalid PWM values, must be between 0 and 100")
    except ValueError:
        print("Invalid PWM input")

def read_serial():
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                data = json.loads(line)
                if "TMP_data" in data:
                    for i in range( len(data["TMP_data"])):
                        TMP_values[i].set(data["TMP_data"][i])
                if "PH_data" in data:
                    for i in range(len(data["PH_data"])):
                        PH_values[i].set(data["PH_data"][i])
                if "ORP_data" in data:
                    for i in range(len(data["ORP_data"])):
                        ORP_values[i].set(data["ORP_data"][i])
                if  ("log" in data and data["log"] == "on"):
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    row = row = [timestamp] + data["TMP_data"] + data["PH_data"] + data["ORP_data"] + [int(entry.get()) for entry in pwm_entries]
                    csv_writer.writerow(row)
                    log_file.flush()
        except Exception as e:
            print(f"Serial read error: {e}")
        time.sleep(0.1)

def ESTOP():
    try:
        for entry in pwm_entries:
            entry.delete(0, tk.END)
            entry.insert(0, "0")
        send_pwm()
        print ("E-STOP activated, all PWM values set to 0")
    except Exception as e:
        print(f"E-STOP error: {e}")

for i in range(NUM_TMP):
    ttk.Label(root, text=f"TMP {i+1}:").grid(row=i, column=0)
    ttk.Label(root, textvariable=TMP_values[i]).grid(row=i, column=1)

for i in range(NUM_PH):
    ttk.Label(root, text=f"PH {i+1}:").grid(row=i + NUM_TMP, column=0)
    ttk.Label(root, textvariable=PH_values[i]).grid(row=i + NUM_TMP, column=1)
    
for i in range(NUM_ORP):
    ttk.Label(root, text=f"ORP {i+1}:").grid(row=i + NUM_TMP + NUM_PH, column=0)
    ttk.Label(root, textvariable=ORP_values[i]).grid(row=i + NUM_TMP + NUM_PH, column=1)


for i in range(PWM_CHANNELS):
    ttk.Label(root, text=f"PWM {i+1}:").grid(row=i, column=2)
    entry = ttk.Entry(root, width=5)
    entry.insert(0, "0")
    entry.grid(row=i, column=3)
    pwm_entries.append(entry)

root.bind ('<Return>', lambda event: send_pwm())
root.bind ('<k>' , lambda event: ESTOP())
root.bind ('<Control-c>' , lambda event: root.quit())

ttk.Button(root, text="Send PWM", command=send_pwm).grid(row=NUM_TMP+NUM_PH+NUM_ORP, column=2, columnspan=2)
ttk.Button(root, text = 'E-STOP', command = lambda: ESTOP()).grid(row=NUM_TMP+NUM_PH+NUM_ORP, column=0, columnspan=2)

threading.Thread(target=read_serial, daemon=True).start()
root.mainloop()
