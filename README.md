
This repository contains both the eye detection script written in Python as well as the Arduino code for the ESP32 chip used to control sensors and actuators for the "Great Piece of Art Never Seen" installation.

Both the “Dlib” face detection model and eye blink script has more or less been taken from Adrian Rosebrockś blog [“Eye blink detection with OpenCV, Python, and dlib”](https://www.pyimagesearch.com/2017/04/24/eye-blink-detection-opencv-python-dlib/). The eye detection runs on and old laptop with an I3 processor and 4 gigs of ram with a 20 FPS performance. 

The code for the ESP32 utilizes both cores of the microcontroller. One for an wireless access point with which the system can be activated and monitored while developing as well as one core for the sensor and actuator handling.   


![alt text](img/GPOA.gif)
