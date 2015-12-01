This is the Arduino sketch for the RideData Android app v1.
You can use any Bluetooth shield or IMU sensor you'd like, you don't even have to use an Arduino.
As long as your code, in the case of an Arduino, your sketch, sends the right commands over Bluetooth, the RideData app will read it.

The app was designed to work with this sketch and the following parts:

Arduino Uno
SeeedStudio Bluetooth Shield v2
http://www.seeedstudio.com/wiki/Bluetooth_Shield_V2.0

SeeedStudio Grove IMU10DOF
http://www.seeedstudio.com/depot/Grove-IMU-10DOF-p-2386.html

See http://ridedata.net for details about hardware currently supported configuration and detail instructions

If you write your own it just has to send a total of 14 data points over Bluetooth with a comma separating each.

It should go:
Gyro X axis, Y, Z, Accelerometer X, Y, Z, Magnetometer X, Y, Z, heading, tiltheading, temperature, pressure, atm, altitude

Look at the included sketch, even if your hardware is different, it will give you an idea for the pattern to follow.
