Assignment #3: Ray tracing

FULL NAME: Hong Yu


MANDATORY FEATURES
------------------

<Under "Status" please indicate whether it has been implemented and is
functioning correctly.  If not, please explain the current status.>

Feature:                                 Status: finish? (yes/no)
-------------------------------------    -------------------------
1) Ray tracing triangles                  yes

2) Ray tracing sphere                     yes

3) Triangle Phong Shading                 yes

4) Sphere Phong Shading                   yes

5) Shadows rays                           yes

6) Still images                           yes
   
7) Extra Credit (up to 20 points)
   1.Recursive reflection
     Set the variable "times" in function "draw_scene" to determine how many times for recursion.
	 For example, if times = 1, there's no recursion,just shoot ray once; times = 2, one recursion and so on.
   
   2.Good antialiasing
     Fire multiple rays in one pixel and then calculate the average color. In the assignment, I fire 4
	 rays in one pixel. Set the variable "antialiasing" in function "draw_scene" to determine whether using
	 antialiasing. For example, if antialiasing = 0, no antialiasing; antialiasing = 1, there will be antialiasing.
   
   3.Softshadow
     As for light source L(x,y,z), I create more point light source around L: L1(x+0.05,y,z),L2(x-0.05,y,z)
	 L3(x,y+0.05,z),L4(x,y-0.05,z),L5(x,y,z+0.05),L6(x,y,z-0.05), so those 7 points can be regarded as a "sphere
	 light source". Shoot shadow rays to every point of those 7 points, weighting them by the brightness, adding them
	 together to get the result.
	 Set the variable "softshadow" in function "draw_scene" to determine whether using softshadow. 
	 For example, if "softshadow" = 0, no softshadow; "softshadow" = 1, there will be softshadow.
   
   
8) Stil_image
   I attatch some of the still images in the "Image" folder. The name of the image illustrates the features
   of the images. For example, "siggraph_recursive.jpg" means 1 recursive reflection for siggraph.scene, "table_softshadow+antialiasing.jpg"
   means softshadow and antiailiasing for table.scene.
   
!!!!!!Important variables for extra credits:
void draw_scene()
{
    int times = 1(times >= 1)
	int antiailiasing = 0
	int softshadow = 0
}
When extra features are implemented, it will be slower, as long as not too many extra are implemented,
the speed is acceptable.

Set those variables randomly to get different extra features, have fun!!


   