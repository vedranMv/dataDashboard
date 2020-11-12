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

## How to run
### Windows
On windows:
* Download QT Creator and compile the source from this repo
* Download a portable precompiled version from [Releases](https://github.com/vedranMv/dataDashboard/releases)

### Linux
There are several dependencies for running on linux systems, regardless of which method below you go with:
> ``sudo apt-get install libgl1-mesa-dev libfontconfig1 libxcb-randr0-dev libxcb-xtest0-dev libxcb-xinerama0-dev libxcb-shape0-dev libxcb-xkb-dev``

After that, it's possible to:
* Compile the project yourself, following the guide below 
* (Ubuntu) Download a precompiled, statically linked version from [Releases](https://github.com/vedranMv/)

### Compilation on linux

1. Download libraries and compilation tools (not available on older Ubuntu 18.04/16.04)
> ``sudo apt-get install qt5-default qt5-qmake libqt5datavisualization5-dev libqt5serialport5-dev libqt5core5a libqt5x11extras5-dev``

2. Clone this project
> ``git clone --depth=1 https://github.com/vedranMv/dataDashboard``

3. Make build directory inside the cloned one
> ``mkdir dataDashboard/build`` <br/> 
 ``cd dataDashboard/build``

4. Invoke qmake, then compile with make
> ``qmake -config release ../datadashboard.pro``<br/>
``make``

5. Run the program
> ``./datadashboard``