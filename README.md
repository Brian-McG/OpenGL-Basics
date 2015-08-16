Assignment 4: OpenGL Shader
-------------------------------------
##### **Author:** Brian Mc George (MCGBRI004)
##### **Date:** 16-08-2015
----------
## Important Notices
### Controls
Translation, rotation and scaling operations can be done via the mouse or via the mouse wheel.
In order to use the mouse, one must **click and hold** the mouse button (in the application window) and then move the mouse in the desired direction.

|Keyboard Option | Description                                                                          |
|:-------------- |:-------------------------------------------------------------------------------------|
|`r`             | Switches to rotation mode.<br>Each press toggles between the different rotation axes.|
|`s`             | Switches to scaling mode.                                                            |
|`t`             | Switches to translation mode.                                                        |
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

#### Mouse Wheel
All operations above (except x and y translation) can be done by rotating the mouse wheel.

### Normal mapping
|Keyboard Option | Action                |
|:-------------- |:------------          |
|`n`             | Toggle normal mapping |
The program will write to the output terminal of the application to indicate what the normal mapping state has been changed to.

### Model Options
The default model should be the cube.

|Keyboard Option | Model loaded|
|:-------------- |:------------|
|`1`             | Cube        |
|`2`             | F16         |

The normal map for the F16 is not designed for the model. As such the lighting (when normal mapping is applied) is not as crisp as the cube.
What occurs with the F16 (only when normal mapping is applied) is that you can see specular light on the model when the light is behind the model.
My understanding of why this occurs is that the normal map has perturbed the normals such that not all the normals extend away from the model. When the light is behind the model, those normals cause the specular light to show. I believe when you do shadow calculation it will correct this issue (which was not required for this practical).

Normal mapping is **on** by default for the **cube**.<br>
Normal mapping is **off** by default for the **F16**.

### Lighting
Three lights are included.<br>
The green light rotates about the x axis and starts at (0, 0, 10).<br>
The red light rotates about the y-axis and starts at (0, 0, 10).<br>
The blue light rotates about the z-axis and starts at (-10, 0, 0).<br>
The colors should combine when multiple lights fall on the same spot.

### To Run this program in Qt Creator
Simply open the project and click run.

### Default camera position
By default the camera sits at (0,0,3) and looks at (0,0,0).

###Additional project dependencies
The project depends on the *glm* package and is included in the project directory
