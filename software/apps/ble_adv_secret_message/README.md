# Secret message app

Transmits a secret message over a BLE advertisement. The device's BLE address
is set to `C0:98:E5:4E:03:97`.

Note: this must be loaded on a board that already has the softdevice installed.
To ensure this, load another BLE app on the board first, check that it's
working, then come back to this repo and run the upload command below.

Upload application to device:
```
$ JLinkExe -device nrf52840_xxaa -if swd -speed 4000 flash.jlink
```

