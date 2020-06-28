Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: < Hong Yu >
USC ID 		: < 4356762610 >
File        : hw2-starterCode/hw2.cpp

Description: In this assignment, we use Catmull-Rom splines along with OpenGL core profile shader-based texture mapping and Phong shading to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - Y

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

4. Run at interactive frame rate (>15fps at 1280 x 720) - Y

5. Understandably written, well commented code - Y

6. Attached an Animation folder containing not more than 1000 screenshots - Y

7. Attached this ReadMe File - Y

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - Y

2. Render a Double Rail - Y

3. Made the track circular and closed it with C1 continuity - N

4. Any Additional Scene Elements? (list them here) - Y (forest, wooden crossbars,supporting structures)

5. Render a sky-box - Y

6. Create tracks that mimic real world roller coaster - N

7. Generate track from several sequences of splines - Y
   (I connect goodRide.sp and goodRide2.sp to get a new track, also you can change the order to get a different track)

8. Draw splines using recursive subdivision - N

9. Render environment in a better manner - N

10. Improved coaster normals - N

11. Modify velocity with which the camera moves - Y

12. Derive the steps that lead to the physically realistic equation of updating u - Y
(I think 11 and 12 are implemented together)
(You can check the velocity and height in console when riding)


Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
1. "q" : get a ride , press 'q' continuously can get a continuous ride
    When running at the end of the track, the program will exit.
   
2. Translation:
   Ctrl + mouseleftbutton: control x,y translation;
   Ctrl + mouserightbutton: control z translation;

3. Scaling:
   Shift + mouseleftbutton: control x,y scaling;
   Shift + mouserightbutton: control z scaling;

4. Rotation:
   mouseleftbutton: rotate about y axis;
   mouserightbutton: rotate about x axis;

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
1. When connecting different splines together, because we know the tangent value and position of the end point 
   of the previous one and the start point of the later one. I use Hermite spline to connect those two points,so
   it can be continuous.

2. As for T-shaped rail. Just like the same method, I use tangent,N and B to draw the "T" shape. The only difference
   is that "T" has 8 points around the major point. Moreover I inverse the shape of "T" to mimic the real world rail.


Comments : If you have done translation, scaling and rotation by mouse, please do not ride.
                     If you want to get a ride, just press "q" and dont do any other operations!

Have fun!
