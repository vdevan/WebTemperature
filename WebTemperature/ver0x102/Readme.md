# Version 0x102

This version uses multiple reads of Temperature file to a fixed buffer size of 512 bytes,
instead of having a String variable to read a file. At times this could be close to 5K

Using fixed buffer for reading file improves performance of 8266 6WebServer. 
Also using cookies to store user preference on temperature measurement. 
Selecting Centigrade or Farenheit is now persistent 
