Data dashboard
=======
(QT based app for data visualization)

Data dashboard is a program based on QT framework that can be used to to visualize some of the common data types that one comes in contact when working with sensors. Although it was developed as during my work with IMU, and is mostly centered around it, it should be versatile enough for other sensors and purposes as well.

It's currently a work in progress, but once completed it should to be able to:
* Read data from serial port and or network socket (real input channel)
* Define data frame (specify start/end of frame character, as well as data separator char)
* Perform basic math (addition, subtraction, abs value) on multiple input channels and visualize result of the operations (virtual input channels)
* Log input channels into a file
* Visualize 3D orientation by taking either euler angles or quaternion as input
* Visualize linear data series (flexible number of input channels)
* Plot data in 3D scatter plot (or 2D by specifying one of the inputs as 0)
* Create dashboard consisting of any combination of the visualizations above on runtime

## Demo of current progress
/doc folder contains some of the screen shoot and a demo video below

https://vedran.ml/public/projects/datadashboard/demo.mp4