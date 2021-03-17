#! /usr/bin/env python3

import subprocess
import telnetlib
import time

PORT=42477

# Open JLinkExe connection
jlink_cmd = "JLinkExe -device nrf52840_xxaa -if swd -speed 4000 -RTTTelnetPort {} -AutoConnect 1".format(PORT)
proc = subprocess.Popen(jlink_cmd.split(' '), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

# Wait until it is ready
input("Hit enter after selecting a device in JLink ")

try:
    # Open Telnet to RTT port and start printing data
    telnet = telnetlib.Telnet(host="localhost", port=PORT, timeout=1)
    while True:
        rx_data = telnet.read_very_eager()
        if rx_data:
            print(rx_data)
except KeyboardInterrupt:
    pass
except Exception as e:
    print(e)
finally:
    proc.kill()
