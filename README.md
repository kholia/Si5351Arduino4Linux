# Si5351Arduino4Linux - port of Si5351Arduino Library for Linux i2c

Linux port by Rafal Rozestwinski (callsign: SO2A, https://rozestwinski.com),
project home: https://github.com/webft8/Si5351Arduino4Linux

Based on an excellent: https://github.com/etherkit/Si5351Arduino

## Raspberry Pi

Tested on Raspberry Pi. Use i2c bus `1` (`/dev/i2c-1`) in the recent rpi revisions.

Si5351 module connection: 5V to 5V, GND to GND, SDA to SDA, SCK1 to SCK1.

Do not use RPI 3.3V for power (low current limit). Most Si5351 modules have
internal regulator 5V->3.3V anyway.

```
-----------------\
 [3.3V]  [ 5V ]  |
 [SDA1]  [ 5V ]  |
 [SCL1]  [GND ]  |
 [ IO ]  [ TX ]  |
 [ IO ]  [ RX ]  |
  ...    ...    .
```

## Quick start

```console
$ make build
g++ -ggdb -li2c -Wfatal-errors -Wall -std=c++11 *.cpp -o ./si5351_cli
$ ./si5351_cli
Usage: ./si5351_cli I2C_DEVICE_NUMBER FREQUENCY_CLK0_FLOAT CALIBRATION_FACTOR_INT
example:  $ ./si5351_cli 1 3600000.0 -100
                         ^    ^        ^
                        i2c   3.6MHz   calibration
                /dev/i2c-1
                         ^

$ ./si5351_cli 1 3600000.0 -100
Using i2c device number: 1
Setting SI5351 calibration to 100
Setting SI5351 CLK0 to f=7000000.000000
Done.
$
```

## API modifications versus Arduino upstream

Constructor takes two arguments. First argument is linux specific, i2c device number (/dev/i2c-1 = 1):

```c
// linux ctor api:
Si5351::Si5351(int i2c_linux_bus_device_number, uint8_t i2c_addr = SI5351_BUS_BASE_ADDR)

// instantiate:
int i2c_device_number = 1;
Si5351 si5351(i2c_device_number);
```

Other changes: added destructor for closing i2c device and members for tracking file descriptors.

## Integrate with your project

* Copy Si5351.cpp/.h to your project.
* Dependencies: `sudo apt-get install libi2c-dev`
* Link with lib i2c via `-li2c`
* Basic build command: `g++ -li2c example.cpp si5351.cpp -o ./si5351_cli`


## Verify if i2c connection works on Linux

Default Si5351 i2c address is 0x60. Scan bus for it:

```console
$ sudo apt-get install i2c-tools
$ i2cdetect 1
WARNING! This program can confuse your I2C bus, cause data loss and worse!
I will probe file /dev/i2c-1.
I will probe address range 0x03-0x77.
Continue? [Y/n] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: 60 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

## Debug options - define flags

`SI5351_DEBUG` - debug level logging

`SI5351_LOG=0` - disable error logging (enabled by default)

`SI5351_FATAL_ERRORS` - exit on error; to perform custom action, i.e. throw exception, set `SI5351_ON_ERROR` (please check code).

`SI5351_VERIFY_WRITES=1` - after every write, reads it again in order to verify success of write operation. Ignores some identified self-resetting registers.

Example:

```console
$ make debug_build
g++ -ggdb -std=c++11 -li2c -Wfatal-errors -Werror -Wall *.cpp -o ./si5351_cli -DSI5351_DEBUG -DSI5351_FATAL_ERRORS=1 '-DSI5351_ON_ERROR=throw 1;'
```

## Using the second (I2C 0) port on a Raspberry Pi

To enable I2C port 0, add the following line needs be added to the file `/boot/config.txt`.

```
dtparam=i2c_vc=on
```

```
root@radio:~# i2cdetect -y 0
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: 60 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```
