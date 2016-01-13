# PC script for communication with Arduino v_irsender.
# Customise your serial port OS name in code below.
#
# Usage:
# v_irsender <same_command_as_for_Arduino_v_irsender>
# for example:
# v_irsender LG 4 197
# v_irsender "SONY-RAW A90 12"
#
# v1.0 (2016-01-13)
# Copyright 2016 Valerii Vyshniak
# http://www.vi-soft.com.ua

import sys
import serial

s = serial.Serial()
s.port = "COM6"
s.baudrate = 115200
s.timeout = 5
# prevent arduino to restart on DTR signal
s.setDTR(False)
s.open()

cmd = " ".join(sys.argv[1:])
cmd += "\n"

s.reset_input_buffer()
s.reset_output_buffer()

s.write(bytearray(cmd,'utf-8'))

resultOK = "Res: OK"
resultError = "Res: ER"
result = ""
outText = ""
ret = None

# wait for IR send status
while True:
    rd = s.read(1).decode('utf-8')
    if len(rd) == 0:
        break
    result += rd
    outText += rd
    if len(result) == len(resultOK):
        if result == resultOK:
            ret = True
            break
        if result == resultError:
            ret = False
            break
        result = result[-(len(resultOK) - 1):]

# skip rest data of status until end of line
if ret is not None:
    while True:
        rd = s.read(1).decode('utf-8')
        if len(rd) == 0:
            break
        outText += rd
        if rd == "\n":
            break

s.close()

print(outText, end = "")
sys.exit(0 if ret else 1)
