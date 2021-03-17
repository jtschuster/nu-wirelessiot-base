RTT-to-Python Example
=====================

A Python script capable of capturing the output from RTT. This uses Telnet to
connect to the RTT stream.

Expects you to be running an application on the end device that prints to RTT.
The printf application works nicely.

Warning: this is a bit jank. It works fine if there's a single board, but for
multiple it fails AND leaves a JLinkExe zombie process lying around (that you
have to clean up manually). Use this as an example to build your own, reliable
script, not as a finished product.

Note that `telnetlib` is a part of the python package and doesn't have to be
installed via pip.

