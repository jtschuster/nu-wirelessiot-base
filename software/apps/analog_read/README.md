Analog Read App
===============

Example of reading from an analog input and printing the result. Values are
printed in millivolts and the pin read by default is `P0.31`.

Note: the expected input value if the pin is not connected to anything could
be any value (i.e. the pin "floats"). To really test this, use a jumper wire
to connect the input pin to either "GND" or "VDD". Do **NOT** connect the input
to "5V" as the board runs at 3.3 volts.

