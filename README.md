### Install packages
to run once: 
```
pip install -r requirements.txt
```

### Arduino-CLI 
When changes are made to an Arduino .ino script, compile it with
```
arduino-cli compile --fqbn arduino:avr:mega {full_file_name}
```

upload the Arduino script with 
```
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega {full_file_name}
```
The port could be ``/dev/ttyACM0`` or ``/dev/ttyACM1``, check which port the arduino is connected to with 
```
arduino-cli board list
```

### Running the GUI 
```
cd gr_rpi_ard
python3 rpi_gui_files/gui_bringup.py
```
Change the port name in gui_bringup.py accordingly. 

### Calibrating PH and ORP sensors 
Calibration of the PH and ORP sensors consist of the following steps (in sensor_calibration/sensor_calibration.ino)
List of sensor addresses:
```
PH1_ADDR = 0x65;
PH2_ADDR = 0x67;
ORP1_ADDR = 0x66;
ORP2_ADDR = 0x68;
```
1, activate the sensor 
```
activateSensor (sensor_address, 0x01)
```
(to deactivate the sensor, run)
```
activateSensor (sensor_address, 0x00)
```
2, send the calibration values to the calibration register 
For PH:
```
sendPHCalibration(PH_address, PH_value)
```
For ORP 
```
sendORPCalibration(ORP_address, ORP_value)
```
3, request calibration 
For PH:
```
requestCalibration (PH_address, calibration_node)
```
where 
```
calibration_code = 1  ---- clear all previous calibrations 
calibration_code = 2  ---- low point calibration (PH = 4.0)
calibration_code = 3  ---- mid point calibration (PH = 7.0)
calibration_code = 4  ---- high point calibration (PH = 10.0)
```
For ORP: 
```
requestCalibration (ORP_address, calibration_node)
```
where 
```
calibration_code = 1  ---- clear all previous calibrations 
calibration_code = 2  ---- single point calibration
```
4, confirm calibration success
```
confirmCalibration (sensor_address)
```
output table 
```
    PH 
    HI              MID         LO       OUTPUT
    0               0           0          0
    0               0           1          1
    0               1           0          2
    0               1           1          3
    1               0           0          4
    1               0           1          5
    1               1           0          6
    1               1           1          7

    ORP 
    0       No calibration
    1       Calibration
    
```
### Conversion of TMP sensor reading 
The TMP sensor reading outputs 4-20mA where 4mA corresponds to -15 psig and 20mA corresponds to +60 psig. The current passes through a 270 $\Omega$ resistor, so the current output gets mapped to 1.08V - 5.4V. To convert from voltage reading (V) back to pressure (P), use:
$$ P = ({{V*1000}\over{270}}-4)*{{75}\over{16}}+(-15)$$










