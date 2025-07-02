import serial
import time
import json
import numpy as np
import csv
from datetime import datetime

PORT = '/dev/ttyACM0'  # Update this to your serial port


ser = serial.Serial(PORT, 9600, timeout=1)
log_file = open("data/data_log.csv", "w", newline='')
csv_writer = csv.writer(log_file)
PWM_CHANNELS = 8
NUM_TMP = 6
NUM_PH = 2
NUM_ORP = 2 

csv_writer.writerow(["Timestamp", *[f"TMP(kpa) {i+1}" for i in range(NUM_TMP)],
                     *[f"PH{i+1}" for i in range(NUM_PH)],
                     *[f"ORP{i+1}" for i in range(NUM_ORP)],
                     *[f"Motor{i+1}" for i in range(PWM_CHANNELS)]])

while True:
    try:
        line = ser.readline().decode('utf-8').strip()
        if line:
            data = json.loads(line)

            if  ("log" in data and data["log"] == "on"):
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                row = row = [timestamp] + data["TMP_data"] + data["PH_data"] + data["ORP_data"] + [int(entry) for entry in data["pwm_output"]]
                csv_writer.writerow(row)
                log_file.flush()
                print (f"Logged data")
            
    except Exception as e:
        print(f"Serial read error: {e}")
