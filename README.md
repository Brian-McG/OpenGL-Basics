Assignment 2: OpenGL Intro
-------------------------------------
##### **Author:** Brian Mc George (MCGBRI004)
##### **Date:** 27-04-2015
----------
## Important Notices
### Controls
Translation, rotation and scaling operations can be done via the mouse or via the mouse wheel.
In order to use the mouse, one must **click and hold** the mouse button (in the application window) and then move the mouse in the desired direction.
|Keyboard Option | Description|
|:-------------- |:------------|
|`r`             | Switches to rotation mode.<br>Each press toggles between the different rotation axes.|
|`s`             | Switches to scaling mode.|
|`t`             | Switches to translation mode.|
The program will write to the output terminal of the application which mode it is in. 

#### Rotation via mouse
##### **X-Rotation**
To rotate about the x-axis one must move the mouse **up / down** to affect the rotation.

##### **Y-Rotation**
To rotate about the y-axis one must move the mouse **left / right** to affect the rotation.
##### **Z-Rotation**
To rotate about the z-axis one must move the mouse **left / right** to affect the rotation.

#### Scaling via mouse
To increase the size of the model the mouse must be moved upwards.
Similarly, to decrease the size of the model the mouse must be moved downwards.

#### Translation
x-translations are done via the mouse by moving it left / right.
y-translations are done via the mouse by moving it up / down.
z-translations are done via the mouse wheel.

#### Mouse Wheel
All operations above (except x and y translation) can be done by rotating the mouse wheel.

### Default model
The default model should be the bunny. If for some reason the bunny fails to load or cannot be located the program will default to a cube model. The program has only been tested to run on Ubuntu. 

### Colour Options
 1. White
 2. Red
 3. Green
 4. Blue
 5. Lavender purple

### File Menu Options
|Menu Option | Description|
|:--------------|:------------|
|`New`        | Opens a new application window|
|`Open`            | Reads an stl file from system|
|`Reset`            | Resets the scene to default state|

### To Run this program
1. Open project in Qt Creator
2. Click Run in Qt Creator

### Default camera position
By default the camera sits at (0,0,3) and looks at (0,0,0).

###Additional project dependencies
The project depends on the *glm* package and is included in the project directory

### Files Included

 - .gitignore
 - Games_3_GL3.2_Template.pro
 - README.md
 - README.html
 - bunny.stl
 - glheader.h
 - main.cpp
 - resources.qrc
 - simple.frag
 - simple.vert
 - window.cpp
 - window.h
 - glwidget.h
 - glwidget.cpp
 - glm


