1. GPS used here is SPARKFUN GP-635T. This code only extracts time from the raw data. You can edit the ProcessingData function to get more useful info (longitude, latitude, altitude, etc).

2. Make sure you use at least 3xAAA batteries, as the minimum voltage for the GPS is 3.3v.

3. When compiling main_GPS_Sender.c, you need to exclude virtual_com_cmds.c and virtual_com_cmds.h. This could be done by right clicking these two files.

4. We just need 3 wires for this GPS communication. VCC, GND, TX(of GPS)->RX(of f2274, P3.5).