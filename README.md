# SEVKE_FAN
This is exhaust fan with some fun futures for gaming room.

CAD: https://www.thingiverse.com/thing:6567706

Hello,
This project is made to send the bad odors in the room to the outside and is equipped with rgb led and game details because it will be used in the game room.

Necessary hardware:
1) Nodemcu v2 (code may need to be modified if ESP32 is used)
2) WS2812b addressable led.
3) NF-A14 Noctua silent fan x3
4) MQ2 Gas sensor
5) BMP180 Pressure sensor
6) USB C breakout

Usage:
Edit the code, enter your wi-fi details and upload the code. When the device is turned on and connected to wifi, you will see a rainbow animation with led. The device will give you an ip address using the leds. After connecting from your browser with this ip address, you will access a panel with buttons for clean air and dirty air calibration. Calibrate the device in dirty air and clean air environments by pressing these buttons. This is not mandatory unless you want to use it in automatic mode. You can adjust the fan speeds from the same panel.

When the device is in normal operation, the LEDs on the device will create an animation showing you the quality of the air from red to green. You must not skip calibrations for this animation to work.

The pressure sensor is passive. You can read its value on the panel. If you prefer to use a pressure sensor, you can use it for automation purposes. 

Getting IP adress:
192.168.xxx.xxx is default.
Count the blue blinks until they turn red or yellow. The next yellow blink indicates the next digit. A red blink indicates a dot. For example;
1 blue blink + red blink + 3 blue blinks + yellow blink + yellow blink + yellow blink + 4 blue blinks = 192.168.1.304
