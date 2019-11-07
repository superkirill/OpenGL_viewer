# OpenGL_viewer
OpenGL-based 3D-model viewer with support of .stl, .json and partial support of .obj files.

1.	**To compile the code**:
		``qmake -qt=qt5 .. && make (from the folder “build”)``
2. **To run the code**:
		  ``./faces_viewer (From the folder “build”)``


## Functionalities

 1. **Transparency change**. Allows seeing the inner parts of the objects.
 ![transparency change](https://github.com/superkirill/OpenGL_viewer/blob/master/examples/transparency.png?raw=true)

2. **Z-Sorting of faces**. The faces that are farther from the camera are rendered first, and the faces that are closer to the camera are rendered last.

![Z-sorting example 1](https://github.com/superkirill/OpenGL_viewer/blob/master/examples/zsorting.png?raw=true)

![Z-Sorting example 2](https://github.com/superkirill/OpenGL_viewer/blob/master/examples/zsorting_2.png?raw=true)

3. **Drawing edges**. Allows to see the edges between the adjacent faces of the model.

![Drawing edges example](https://github.com/superkirill/OpenGL_viewer/blob/master/examples/edges.png?raw=true)

4. **Colorization.** Allows to paint the closed surfaces in the same color.

![Colorization example 1](https://github.com/superkirill/OpenGL_viewer/blob/master/examples/colorization.png?raw=true)


![Colorization example 2](https://github.com/superkirill/OpenGL_viewer/blob/master/examples/colorization2.png?raw=true)
