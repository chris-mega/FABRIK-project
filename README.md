COMP 4490 
Author: Christian Melendez
OS: Windows 10
Graphics hardware: Nvidia GEForce GTX 960M + Intel HD Graphics 530

- Used Visual Studio 2017
- See src folder for cpp and glsl files

Question 1: (running_robot)

- Primitives: cube, icosphere and cylinder
- For cylinder (used in legs):
	* Got an idea for how to draw it in https://stackoverflow.com/questions/32883572/how-to-draw-cylinder-in-modern-opengl
		Set the algorithm to something similar to the one provided for icosphere
- To move the "camera" use WASD (only works for X and Y axis, so there may be some weird behaviour)

Question 2: (fire)

- Array "colours" stores a lot of random values 
- Particles go up until a random value and wrap to initial position when done. Color will change as they go up.
- To move the "camera" use WASD (only works for X and Y axis, so there may be some weird behaviour)
