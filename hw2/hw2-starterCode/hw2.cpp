/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: Hong Yu
*/                       

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;
using namespace glm;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float angle = 0.0; //rotate about y-axis
float angle2 = 0.0; //rotate about x-axis
float landRotate[3] = { 0.0f, 1.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
float eye[3] = { 0.0f, 0.0f, 1.0f };
float center[3] = { 0.0f, 0.0f, 0.0f };
float up[3] = { 0.0f, 1.0f, 0.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";
BasicPipelineProgram* pipelineProgram = new BasicPipelineProgram(); //pipeline for object shader
BasicPipelineProgram* pipelineProgram1 = new BasicPipelineProgram(); //pipeline for texture shader

ImageIO * heightmapImage;

//vao1 : rail and support structures//
//vao2 : crossbar//
//vao3~vao8 : texture mapping for ground,sky and forest//
GLuint vao1, vao2, vao3, vao4, vao5, vao6, vao7, vao8;
GLuint vbo1, vbo2, vbo3, vbo4, vbo5, vbo6, vbo7, vbo8;
GLuint elementbuffer; //index buffer//
GLuint program,program1;
GLuint texHandle[7];
int size1;
int numVertices, numVertices_bar;
int photo = 0; //global viriable for the screenshot//
vector<float> positions; //store positions//
vector<float> point; //store consecutive points//
vector<float> tangent; //store tangent//
vector<float> normal; //store normal//
vector<float> zeros; //zero vectors to reset positions and colors//
vector<float> texcoord; //store texture coordinate//
vector<float> crossbar; //store crossbar//
vector<float> crossbarnormal; //store texture map coordinate to render crossbar//
vector<unsigned int> indices; //store indices of vertices for glDrawElements//
OpenGLMatrix* openGLMatrix = new OpenGLMatrix();
GLint h_modelViewMatrix, h_projectionMatrix;
GLint h_modelViewMatrix1, h_projectionMatrix1;
char file[100]; //filename for screenshot//
vec3 result;
mat4 Basis, Hermite_Basis;
mat3x4 Control; //Control matrix//
//tang, N and B for riding//
vec3 tang;
vec3 N;
vec3 B;

int index = 0; //index for riding//
float hmax = 0; //max height of roller coaster//
float g = 9.8; //gravity constant//
int flag1 = 0; //to indicate whether at the end of spline, can be use to connect two splines//

// represents one control point along the spline 
struct Point
{
	double x;
	double y;
	double z;
};

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline
{
	int numControlPoints;
	Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;

int loadSplines(char * argv)
{
	char * cName = (char *)malloc(128 * sizeof(char));
	FILE * fileList;
	FILE * fileSpline;
	int iType, i = 0, j, iLength;

	// load the track file 
	fileList = fopen(argv, "r");
	if (fileList == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	// stores the number of splines in a global variable 
	fscanf(fileList, "%d", &numSplines);

	splines = (Spline*)malloc(numSplines * sizeof(Spline));

	// reads through the spline files 
	for (j = 0; j < numSplines; j++)
	{
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL)
		{
			printf("can't open file\n");
			exit(1);
		}

		// gets length for spline file
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		// allocate memory for all the points
		splines[j].points = (Point *)malloc(iLength * sizeof(Point));
		splines[j].numControlPoints = iLength;

		// saves the data to the struct
		while (fscanf(fileSpline, "%lf %lf %lf",
			&splines[j].points[i].x,
			&splines[j].points[i].y,
			&splines[j].points[i].z) != EOF)
		{
			i++;
		}
	}

	free(cName);

	return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
	// read the texture image
	ImageIO img;
	ImageIO::fileFormatType imgFormat;
	ImageIO::errorType err = img.load(imageFilename, &imgFormat);

	if (err != ImageIO::OK)
	{
		printf("Loading texture from %s failed.\n", imageFilename);
		return -1;
	}

	// check that the number of bytes is a multiple of 4
	if (img.getWidth() * img.getBytesPerPixel() % 4)
	{
		printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
		return -1;
	}

	// allocate space for an array of pixels
	int width = img.getWidth();
	int height = img.getHeight();
	unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

																		// fill the pixelsRGBA array with the image pixels
	memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++)
		{
			// assign some default byte values (for the case where img.getBytesPerPixel() < 4)
			pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
			pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
			pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
			pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

													   // set the RGBA channels, based on the loaded image
			int numChannels = img.getBytesPerPixel();
			for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
				pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
		}

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	// initialize the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

	// generate the mipmaps for this texture
	glGenerateMipmap(GL_TEXTURE_2D);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// query support for anisotropic texture filtering
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	printf("Max available anisotropic samples: %f\n", fLargest);
	// set anisotropic texture filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

	// query for any errors
	GLenum errCode = glGetError();
	if (errCode != 0)
	{
		printf("Texture initialization error. Error code: %d.\n", errCode);
		return -1;
	}

	// de-allocate the pixel array -- it is no longer needed
	delete[] pixelsRGBA;

	return 0;
}

void setTextureUnit(GLint unit)
{
	glActiveTexture(unit); // select the active texture unit
						   // get a handle to the “textureImage” shader variable
	GLint h_textureImage = glGetUniformLocation(program1, "textureImage");
	// deem the shader variable “textureImage” to read from texture unit “unit”
	glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

//initialize VBO//
void initVBO(GLuint &vbo, vector<float> &pos, vector<float> &norm)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*pos.size() + sizeof(float)*norm.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*pos.size(), pos.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*pos.size(), sizeof(float)*norm.size(), norm.data());
}


//Object: set the shader and layout of vao//
//used in initScene//
void setshader(int size, GLuint program)
{
	// get location index of the “position” shadervariable
	GLuint loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	const void * offset = (const void*)0;
	GLsizei stride = 0;
	GLboolean normalized = GL_FALSE;
	// set the layout of the “position” attribute data
	glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

	// get location index of the “color” shadervariable
	loc = glGetAttribLocation(program, "normal");
	glEnableVertexAttribArray(loc);
	offset = (const void*)(size);
	stride = 0;
	normalized = GL_FALSE;
	// set the layout of the “color” attribute data
	glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);
}

//Texture: set the shader and layout of vao//
//used in initScene//
void setshadertex(int size, GLuint program)
{
	// get location index of the “position” shadervariable
	GLuint loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	const void * offset = (const void*)0;
	GLsizei stride = 0;
	GLboolean normalized = GL_FALSE;
	// set the layout of the “position” attribute data
	glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

	// get location index of the “texCoord” shadervariable
	loc = glGetAttribLocation(program, "texCoord");
	glEnableVertexAttribArray(loc);
	offset = (const void*)(size);
	stride = 0;
	normalized = GL_FALSE;
	// set the layout of the “texCoord” attribute data
	glVertexAttribPointer(loc, 2, GL_FLOAT, normalized, stride, offset);
}

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
	openGLMatrix->LoadIdentity();
	openGLMatrix->LookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);
	
	//upload LightDirection to GPU//
	float view[16];
	openGLMatrix->GetMatrix(view);
	GLint h_viewLightDirection = glGetUniformLocation(program, "viewLightDirection");
	vec3 lightDirection = { 0,3,0 };
	mat4 V = { view[0],view[1],view[2],view[3],
		view[4],view[5],view[6],view[7],
		view[8],view[9],view[10],view[11],
		view[12],view[13],view[14],view[15] };
	vec3 viewLightvector;
	viewLightvector = { (V * vec4(lightDirection, 0.0)).x,(V * vec4(lightDirection, 0.0)).y
		,(V * vec4(lightDirection, 0.0)).z };
	float viewLightDirection[3] = { viewLightvector.x,viewLightvector.y,viewLightvector.z };
	glUniform3fv(h_viewLightDirection, 1, viewLightDirection);

	openGLMatrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	openGLMatrix->Rotate(angle, 0, 1, 0);
	openGLMatrix->Rotate(angle2, 1, 0, 0);
	openGLMatrix->Scale(landScale[0], landScale[1], landScale[2]);

	//set the coefficient of La,Ld,Ls,ka,kd and ks for phong shading//
	float La[4] = { 0.5,0.5,0.5,0.5 };
	float Ld[4] = { 0.1,0.2,0.3,0.4 };
	float Ls[4] = { 0.3,0.4,0.5,0.6 };
	float ka[4] = { 0.5,0.5,0.5,0.5 };
	float kd[4] = { 0.2,0.3,0.4,0.5 };
	float ks[4] = { 0.6,0.7,0.3,0.4 };
	float alpha = 1;
	GLint h_La = glGetUniformLocation(program, "La");
	GLint h_Ld = glGetUniformLocation(program, "Ld");
	GLint h_Ls = glGetUniformLocation(program, "Ls");
	GLint h_ka = glGetUniformLocation(program, "ka");
	GLint h_kd = glGetUniformLocation(program, "kd");
	GLint h_ks = glGetUniformLocation(program, "ks");
	GLint h_alpha = glGetUniformLocation(program, "alpha");
	glUniform4fv(h_La, 1, La);
	glUniform4fv(h_Ld, 1, Ld);
	glUniform4fv(h_Ls, 1, Ls);
	glUniform4fv(h_ka, 1, ka);
	glUniform4fv(h_kd, 1, kd);
	glUniform4fv(h_ks, 1, ks);
	glUniform1f(h_alpha, alpha);

	
	GLboolean isRowMajor = GL_FALSE;

	//model-view matrix//
	float m[16];
	openGLMatrix->GetMatrix(m);
	
	//normal matrix//
	GLint h_normalMatrix = glGetUniformLocation(program, "normalMatrix");
	float n[16];
	openGLMatrix->GetNormalMatrix(n); // get normal matrix
	
	//projection matrix//
	openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
	float p[16];
	openGLMatrix->GetMatrix(p);
	
	//texture mapping for a cube : sky, forest and ground//
	pipelineProgram1->Bind();
	glBindVertexArray(vao3);
	setTextureUnit(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHandle[0]);
	glUniformMatrix4fv(h_modelViewMatrix1, 1, isRowMajor, m);
	glUniformMatrix4fv(h_projectionMatrix1, 1, isRowMajor, p);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, texHandle[1]);
	glBindVertexArray(vao4);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, texHandle[2]);
	glBindVertexArray(vao5);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, texHandle[3]);
	glBindVertexArray(vao6);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, texHandle[4]);
	glBindVertexArray(vao7);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, texHandle[5]);
	glBindVertexArray(vao8);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//Drawcrossbar//
	GLint first = 0;
	GLsizei count = numVertices_bar;
	glBindTexture(GL_TEXTURE_2D, texHandle[6]);
	glBindVertexArray(vao2);
	glDrawArrays(GL_TRIANGLES, first, count);

	//Drawobject//
	pipelineProgram->Bind();
	glBindVertexArray(vao1);
	first = 0;
	count = numVertices;
	glUniformMatrix4fv(h_normalMatrix, 1, isRowMajor, n);
	glUniformMatrix4fv(h_modelViewMatrix, 1, isRowMajor, m);
	glUniformMatrix4fv(h_projectionMatrix, 1, isRowMajor, p);
	glDrawArrays(GL_TRIANGLES, first, count);

	glutSwapBuffers();
}


void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // setup perspective matrix...
  openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix->LoadIdentity();
  openGLMatrix->Perspective(60.0, 1.0* w / h, 0.01, 100.0);
  openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (rightMouseButton)
      {
        // control z translation via the right mouse button
        landTranslate[2] += mousePosDelta[1] * 0.015f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
		angle += mousePosDelta[0];
      }
      if (rightMouseButton)
      {
		 angle2 += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (rightMouseButton)
      {
        // control z scaling via the right mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
		photo++;
		sprintf(file, "./animation/screenshot%d.jpg", photo);
        saveScreenshot(file);
    break;

	//Get a ride !//
	case 'q':
		int velocity = (int)(sqrt(2 * g * (hmax - point[index + 1])) + 1); //velocity based on real physics//
		index += 3 *2* velocity; //increase the speed to be faster//
		if (index < tangent.size())
		{
			//update tang,N and B and then use them to update the camera position//
			tang = { tangent[index],tangent[index + 1],tangent[index + 2] };
			N = normalize(cross(B, tang));
			B = normalize(cross(tang, N));
			up[0] = N[0];
			up[1] = N[1];
			up[2] = N[2];
			eye[0] = point[index] + 0.1*N[0];
			eye[1] = point[index + 1] + 0.1*N[1];
			eye[2] = point[index + 2] + 0.1*N[2];
			center[0] = eye[0] + tangent[index];
			center[1] = eye[1] + tangent[index + 1];
			center[2] = eye[2] + tangent[index + 2];

			//can see the height and velocity at the console//
			cout << "height:"<<point[index + 1] << endl;
			cout << "velocity" << velocity << endl;
		}

		//If at the end of rail, exit//
		else
		{
			cout << "At the end of Roller Coaster" << endl;
			exit(1);
		}
	break;
  }
}

//push vec3 into a vector//
void push(vector<float> &input, vec3 &v)
{
	input.push_back(v[0]);
	input.push_back(v[1]);
	input.push_back(v[2]);
}

//push vec3 into positions and normal vector for phong shading//
void pushnorm(vector<float> &input1, vector<float> &input2, vec3 &v0, vec3 &v1, vec3 &v2)
{
	vec3 norm = normalize(cross(v2 - v1, v0 - v1));
	push(input1, norm);
	push(input1, norm);
	push(input1, norm);
	push(input2,v0);
	push(input2,v1);
	push(input2,v2);
}

//push position and texture mapping coordinate into vector for texture mapping//
void pushtex(vector<float> &input1, vector<float> &input2,vec3 &v0, vec3 &v1, vec3 &v2, vec3 &v3)
{
	push(input1, v0);
	push(input1, v1);
	push(input1, v2);
	push(input1, v0);
	push(input1, v2);
	push(input1, v3);
	input2.push_back(0);
	input2.push_back(0);
	input2.push_back(1);
	input2.push_back(0);
	input2.push_back(1);
	input2.push_back(1);
	input2.push_back(0);
	input2.push_back(0);
	input2.push_back(1);
	input2.push_back(1);
	input2.push_back(0);
	input2.push_back(1);
}

//read an image for texture mapping//
void uploadtex(const char* filename, GLuint h)
{
	int code = initTexture(filename, h);
	if (code != 0)
	{
		cout << "Error loading the texture image.\n" << endl;
		exit(EXIT_FAILURE);
	}
}

void initScene(int argc, char *argv[])
{

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);

  // do additional initialization here...
  pipelineProgram->Init("../openGLHelper-starterCode");
  pipelineProgram->Bind();

  //program for object shader//
  program = pipelineProgram->GetProgramHandle();
  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");

  float s = 0.5; //s for catmull-Rom splines//
  float alpha = 0.05; // coefficient to render a T-shape rail//
  int flag = 0; //An indicator to render crossbar at constant step//
  
  //Catmull-Rom Basis//
  Basis[0] = vec4(-s, 2 * s, -s, 0);
  Basis[1] = vec4(2 - s, s - 3, 0, 1);
  Basis[2] = vec4(s - 2, 3 - 2 * s, s, 0);
  Basis[3] = vec4(s, -s, 0, 0);

  //Hermite Basis//
  Hermite_Basis[0] = vec4(2, -3, 0, 1);
  Hermite_Basis[1] = vec4(-2, 3, 0, 0);
  Hermite_Basis[2] = vec4(1, -2, 1, 0);
  Hermite_Basis[3] = vec4(1, -1, 0, 0);
 
  result[0] = splines[0].points[1].x;
  result[1] = splines[0].points[1].y;
  result[2] = splines[0].points[1].z;
  

  //initialize lookat function//
  vec3 V = { 1.0,0.0,0.0 };
  tang = { splines[0].points[2].x - splines[0].points[0].x ,splines[0].points[2].y - splines[0].points[0].y ,
	  splines[0].points[2].z - splines[0].points[0].z };
  push(tangent, tang);

  N = normalize(cross(tang, V));
  B = normalize(cross(tang, N));
  up[0] = N[0];
  up[1] = N[1];
  up[2] = N[2];

  eye[0] = result[0] + 0.25*B[0] + 0.1 * N[0];
  eye[1] = result[1] + 0.25*B[1] + 0.1 * N[1];
  eye[2] = result[2] + 0.25*B[2] + 0.1 * N[2];

  center[0] = eye[0] + tang[0];
  center[1] = eye[1] + tang[1];
  center[2] = eye[2] + tang[2];

  push(point, result + float(0.25)*B);
  hmax = max(hmax, (result + float(0.25)*B).y);

  //Draw rail//
  vec3 v0, v1, v2, v3, v4, v5, v6, v7; //for T-shape rail front-end//
  vec3 v8, v9, v10, v11, v12, v13, v14, v15; //for T-shape rail back-end//
  vec3 v16, v17, v18, v19, v20, v21, v22, v23; //for crossbar//
  vec3 v24, v25, v26, v27, v28, v29, v30, v31; //for supporting structure//
  vec3 norm;

  //initialize the T-shape rail//
  v0 = result + alpha * (-N + B);
  v1 = result + alpha * B;
  v2 = result + (float)0.5 * alpha * B;
  v3 = result + alpha * (N + (float)0.5*B);
  v4 = result - alpha * (-N + (float)0.5*B);
  v5 = result - (float)0.5 * alpha * B;
  v6 = result - alpha * B;
  v7 = result - alpha * (N + B);

  pushnorm(normal, positions, v7, v0, v1);
  pushnorm(normal, positions, v1, v6, v7);
  pushnorm(normal, positions, v5, v2, v3);
  pushnorm(normal, positions, v3, v4, v5);
  v0 = v0 + float(0.5) * B; v1 = v1 + float(0.5) * B; v2 = v2 + float(0.5) * B; v3 = v3 + float(0.5) * B;
  v4 = v4 + float(0.5) * B; v5 = v5 + float(0.5) * B; v6 = v6 + float(0.5) * B; v7 = v7 + float(0.5) * B;
  pushnorm(normal, positions, v7, v0, v1);
  pushnorm(normal, positions, v1, v6, v7);
  pushnorm(normal, positions, v5, v2, v3);
  pushnorm(normal, positions, v3, v4, v5);
  v0 = v0 - float(0.5) * B; v1 = v1 - float(0.5) * B; v2 = v2 - float(0.5) * B; v3 = v3 - float(0.5) * B;
  v4 = v4 - float(0.5) * B; v5 = v5 - float(0.5) * B; v6 = v6 - float(0.5) * B; v7 = v7 - float(0.5) * B;

  
  for (int i = 0; i < numSplines; i++)
  {
	  flag1 = 0;
	  for (int j = 0; j < splines[i].numControlPoints - 3; j++)
	  {
		  // If at the beginning of the next spline, connect the previous spline//
		  if (i > 0 && flag1 == 0)
		  {
			  Control[0] = vec4(result.x, splines[i].points[j + 1].x, tang.x, 0.5 *(splines[i].points[j + 2].x - splines[i].points[j].x));
			  Control[1] = vec4(result.y, splines[i].points[j + 1].y, tang.y, 0.5 *(splines[i].points[j + 2].y - splines[i].points[j].y));
			  Control[2] = vec4(result.z, splines[i].points[j + 1].z, tang.z, 0.5 *(splines[i].points[j + 2].z - splines[i].points[j].z));
			  j = -1;
		  }

		  else
		  {
			  Control[0] = vec4(splines[i].points[j].x, splines[i].points[j + 1].x, splines[i].points[j + 2].x, splines[i].points[j + 3].x);
			  Control[1] = vec4(splines[i].points[j].y, splines[i].points[j + 1].y, splines[i].points[j + 2].y, splines[i].points[j + 3].y);
			  Control[2] = vec4(splines[i].points[j].z, splines[i].points[j + 1].z, splines[i].points[j + 2].z, splines[i].points[j + 3].z);
		  }
		  
		  //use brute force to render rail//
		  for (float u = 0.001; u <= 1.0; u = u + 0.001)
		  {
			  //Use hermite spline to connect points in differtent spline//
			  if (i > 0 && flag1 == 0)
			  {
				  result = vec4(u * u * u, u * u, u, 1)*Hermite_Basis*Control;
				  tang = vec4(3 * u * u, 2 * u, 1, 0)*Hermite_Basis*Control;
			  }

			  //Use catmull-Rom spline to connect points in the same spline//
			  else
			  {
				  result = vec4(u * u * u, u * u, u, 1)*Basis*Control;
				  tang = vec4(3 * u * u, 2 * u, 1, 0)*Basis*Control;
			  }
			  
			  N = normalize(cross(B, tang));
			  B = normalize(cross(tang, N));

			  push(point, result + float(0.25)*B);
			  hmax = max(hmax, (result + float(0.25)*B).y);
			  push(tangent, tang);
			  
			  //Gain vertex of the back-end of T-shape rail//
			  v8 = result + alpha * (-N + B);
			  v9 = result + alpha * B;
			  v10 = result + (float)0.5 * alpha * B;
			  v11 = result + alpha * (N + (float)0.5*B);
			  v12 = result - alpha * (-N + (float)0.5*B);
			  v13 = result - (float)0.5 * alpha * B;
			  v14 = result - alpha * B;
			  v15 = result - alpha * (N + B);


			  pushnorm(normal, positions, v2, v1, v9);
			  pushnorm(normal, positions, v9, v10, v2);
			  pushnorm(normal, positions, v4, v3, v11);
			  pushnorm(normal, positions, v11, v12, v4);
			  pushnorm(normal, positions, v6, v5, v13);
			  pushnorm(normal, positions, v13, v14, v6);
			  pushnorm(normal, positions, v0, v8, v10);
			  pushnorm(normal, positions, v10, v2, v0);
			  pushnorm(normal, positions, v2, v10, v11);
			  pushnorm(normal, positions, v11, v3, v2);
			  pushnorm(normal, positions, v15, v7, v5);
			  pushnorm(normal, positions, v5, v13, v15);
			  pushnorm(normal, positions, v13, v5, v4);
			  pushnorm(normal, positions, v4, v12, v13);
			  pushnorm(normal, positions, v0, v8, v15);
			  pushnorm(normal, positions, v15, v7, v0);

			  v0 = v0 + float(0.5) * B; v1 = v1 + float(0.5) * B; v2 = v2 + float(0.5) * B; v3 = v3 + float(0.5) * B;
			  v4 = v4 + float(0.5) * B; v5 = v5 + float(0.5) * B; v6 = v6 + float(0.5) * B; v7 = v7 + float(0.5) * B;
			  v8 = v8 + float(0.5) * B + float(0.02) * tang; v9 = v9 + float(0.5) * B + float(0.02) * tang; v10 = v10 + float(0.5) * B + float(0.02) * tang; v11 = v11 + float(0.5) * B + float(0.02) * tang;
			  v12 = v12 + float(0.5) * B + float(0.02) * tang; v13 = v13 + float(0.5) * B + float(0.02) * tang; v14 = v14 + float(0.5) * B + float(0.02) * tang; v15 = v15 + float(0.5) * B + float(0.02) * tang;

			  pushnorm(normal, positions, v2, v1, v9);
			  pushnorm(normal, positions, v9, v10, v2);
			  pushnorm(normal, positions, v4, v3, v11);
			  pushnorm(normal, positions, v11, v12, v4);
			  pushnorm(normal, positions, v6, v5, v13);
			  pushnorm(normal, positions, v13, v14, v6);
			  pushnorm(normal, positions, v0, v8, v10);
			  pushnorm(normal, positions, v10, v2, v0);
			  pushnorm(normal, positions, v2, v10, v11);
			  pushnorm(normal, positions, v11, v3, v2);
			  pushnorm(normal, positions, v15, v7, v5);
			  pushnorm(normal, positions, v5, v13, v15);
			  pushnorm(normal, positions, v13, v5, v4);
			  pushnorm(normal, positions, v4, v12, v13);
			  pushnorm(normal, positions, v0, v8, v15);
			  pushnorm(normal, positions, v15, v7, v0);

			  v0 = v8 - float(0.5) * B - float(0.02) * tang; v1 = v9 - float(0.5) * B - float(0.02) * tang; v2 = v10 - float(0.5) * B - float(0.02) * tang; v3 = v11 - float(0.5) * B - float(0.02) * tang;
			  v4 = v12 - float(0.5) * B - float(0.02) * tang; v5 = v13 - float(0.5) * B - float(0.02) * tang; v6 = v14 - float(0.5) * B - float(0.02) * tang; v7 = v15 - float(0.5) * B - float(0.02) * tang;
		      
			  //Draw crossbar every 100 steps//
			  flag++;
			  if (flag == 100)
			  {
				  v16 = result - float(0.05)*B - alpha * N;
				  v17 = v16 - float(0.05)*N;
				  v18 = v17 + float(0.6)*B;
				  v19 = v16 + float(0.6)*B;
				  v20 = v16 + float(0.005)*tang;
				  v21 = v20 - float(0.05)*N;
				  v22 = v21 + float(0.6)*B;
				  v23 = v20 + float(0.6)*B;

				  //Draw crossbar using wooden texture//
				  pushtex(crossbar, crossbarnormal, v16, v17, v18, v19);
				  pushtex(crossbar, crossbarnormal, v16, v19, v23, v20);
				  pushtex(crossbar, crossbarnormal, v18, v22, v23, v19);
				  pushtex(crossbar, crossbarnormal, v17, v18, v22, v21);
				  pushtex(crossbar, crossbarnormal, v17, v21, v20, v16);
				  pushtex(crossbar, crossbarnormal, v21, v22, v23, v20);

				  //Draw crossbar using phong shading//
				  /*pushnorm(crossbarnormal, crossbar, v17, v18, v19);
				  pushnorm(crossbarnormal, crossbar, v19, v16, v17);
				  pushnorm(crossbarnormal, crossbar, v18, v22, v23);
				  pushnorm(crossbarnormal, crossbar, v23, v19, v18);
				  pushnorm(crossbarnormal, crossbar, v16, v19, v23);
				  pushnorm(crossbarnormal, crossbar, v23, v20, v16);
				  pushnorm(crossbarnormal, crossbar, v17, v16, v20);
				  pushnorm(crossbarnormal, crossbar, v20, v21, v17);
				  pushnorm(crossbarnormal, crossbar, v22, v18, v17);
				  pushnorm(crossbarnormal, crossbar, v17, v21, v22);
				  pushnorm(crossbarnormal, crossbar, v23, v22, v21);
				  pushnorm(crossbarnormal, crossbar, v21, v20, v23);*/
				  flag = 0;
			  }
		  }

		  //Draw supporting structure//
		  if (j == 0 || j == 20 || j == 33 || j == 35)
		  {
			  v24 = v17;
			  v25 = v24 + float(0.1)*B;
			  v26 = { v25.x,-10,v25.z };
			  v27 = { v24.x,-10,v24.z };
			  v28 = v24 + float(0.005)*tang;
			  v29 = v25 + float(0.005)*tang;
			  v30 = v26 + float(0.005)*tang;
			  v31 = v27 + float(0.005)*tang;
			  pushnorm(normal, positions, v27, v26, v25);
			  pushnorm(normal, positions, v25, v24, v27);
			  pushnorm(normal, positions, v26, v30, v29);
			  pushnorm(normal, positions, v29, v25, v26);
			  pushnorm(normal, positions, v24, v25, v29);
			  pushnorm(normal, positions, v29, v28, v24);
			  pushnorm(normal, positions, v27, v31, v30);
			  pushnorm(normal, positions, v30, v26, v27);
			  pushnorm(normal, positions, v31, v28, v29);
			  pushnorm(normal, positions, v29, v30, v31);
			  pushnorm(normal, positions, v31, v27, v24);
			  pushnorm(normal, positions, v24, v28, v31);

			  v25 = v18;
			  v24 = v25 - float(0.1)*B;
			  v26 = { v25.x,-10,v25.z };
			  v27 = { v24.x,-10,v24.z };
			  v28 = v24 + float(0.01)*tang;
			  v29 = v25 + float(0.01)*tang;
			  v30 = v26 + float(0.01)*tang;
			  v31 = v27 + float(0.01)*tang;
			  pushnorm(normal, positions, v27, v26, v25);
			  pushnorm(normal, positions, v25, v24, v27);
			  pushnorm(normal, positions, v26, v30, v29);
			  pushnorm(normal, positions, v29, v25, v26);
			  pushnorm(normal, positions, v24, v25, v29);
			  pushnorm(normal, positions, v29, v28, v24);
			  pushnorm(normal, positions, v27, v31, v30);
			  pushnorm(normal, positions, v30, v26, v27);
			  pushnorm(normal, positions, v31, v28, v29);
			  pushnorm(normal, positions, v29, v30, v31);
			  pushnorm(normal, positions, v31, v27, v24);
			  pushnorm(normal, positions, v24, v28, v31);
		  }

		  flag1 = 1;
	  }
  }

  //set the default value of N and B to get the start of riding//
  tang = { splines[0].points[2].x - splines[0].points[0].x ,splines[0].points[2].y - splines[0].points[0].y ,
	  splines[0].points[2].z - splines[0].points[0].z };
  N = normalize(cross(tang, V));
  B = normalize(cross(tang, N));

  //initialize vbo1 for rail, supporting structure//
  initVBO(vbo1, positions, normal);
  glGenVertexArrays(1, &vao1);
  glBindVertexArray(vao1);
  size1 = sizeof(float)*positions.size();
  setshader(size1, program);
  numVertices = positions.size() / 3;
  positions = zeros;
  normal = zeros;

  //**Initialize texture map**//
  pipelineProgram1->Init("../texture_shader");//pipeline program for texture mapping//
  pipelineProgram1->Bind();
  program1 = pipelineProgram1->GetProgramHandle();

  h_modelViewMatrix1 = glGetUniformLocation(program1, "modelViewMatrix");
  h_projectionMatrix1 = glGetUniformLocation(program1, "projectionMatrix");

  glGenTextures(7, texHandle);
  
  uploadtex("ground.jpg", texHandle[0]);
  uploadtex("sky.jpg", texHandle[1]);
  uploadtex("forest1.jpg", texHandle[2]);
  uploadtex("forest2.jpg", texHandle[3]);
  uploadtex("forest3.jpg", texHandle[4]);
  uploadtex("forest4.jpg", texHandle[5]);
  uploadtex("wood.jpg", texHandle[6]);

  //initialize vbo2 for crossbar//
  initVBO(vbo2, crossbar, crossbarnormal);
  glGenVertexArrays(1, &vao2);
  glBindVertexArray(vao2);
  size1 = sizeof(float)*crossbar.size();
  setshadertex(size1, program1);
  numVertices_bar = crossbar.size() / 3;

  //world positions of the cube for texture mapping//
  v0 = { -35,-10,-35 };
  v1 = { 35,-10,-35 };
  v2 = { 35,-10,35 };
  v3 = { -35,-10,35 };
  v4 = { -35,40,-35 };
  v5 = { 35,40,-35 };
  v6 = { 35,40,35 };
  v7 = { -35,40,35 };
  
  //ground//
  pushtex(positions,texcoord,v0, v1, v2, v3);
  initVBO(vbo3, positions, texcoord);
  glGenVertexArrays(1, &vao3);
  glBindVertexArray(vao3);
  size1 = sizeof(float)*positions.size();
  setshadertex(size1, program1);
  positions = zeros;
  texcoord = zeros;

  //sky//
  pushtex(positions, texcoord, v4, v5, v6, v7);
  initVBO(vbo4, positions, texcoord);
  glGenVertexArrays(1, &vao4);
  glBindVertexArray(vao4);
  size1 = sizeof(float)*positions.size();
  setshadertex(size1, program1);
  positions = zeros;
  texcoord = zeros;

  //forest1//
  pushtex(positions, texcoord, v0, v1, v5, v4);
  initVBO(vbo5, positions, texcoord);
  glGenVertexArrays(1, &vao5);
  glBindVertexArray(vao5);
  size1 = sizeof(float)*positions.size();
  setshadertex(size1, program1);
  positions = zeros;
  texcoord = zeros;

  //forest2//
  pushtex(positions, texcoord, v1, v2, v6, v5);
  initVBO(vbo6, positions, texcoord);
  glGenVertexArrays(1, &vao6);
  glBindVertexArray(vao6);
  size1 = sizeof(float)*positions.size();
  setshadertex(size1, program1);
  positions = zeros;
  texcoord = zeros;

  //forest3//
  pushtex(positions, texcoord, v2, v3, v7, v6);
  initVBO(vbo7, positions, texcoord);
  glGenVertexArrays(1, &vao7);
  glBindVertexArray(vao7);
  size1 = sizeof(float)*positions.size();
  setshadertex(size1, program1);
  positions = zeros;
  texcoord = zeros;

  //forest4//
  pushtex(positions, texcoord, v3, v0, v4, v7);
  initVBO(vbo8, positions, texcoord);
  glGenVertexArrays(1, &vao8);
  glBindVertexArray(vao8);
  size1 = sizeof(float)*positions.size();
  setshadertex(size1, program1);
  positions = zeros;
  texcoord = zeros;
  
}


int main(int argc, char *argv[])
{
    if (argc<2)
	{
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	// load the splines from the provided filename
	loadSplines(argv[1]);

	printf("Loaded %d spline(s).\n", numSplines);
	for (int i = 0; i<numSplines; i++)
		printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

    cout << "Initializing GLUT..." << endl;
    glutInit(&argc,argv);

    cout << "Initializing OpenGL..." << endl;

    #ifdef __APPLE__
      glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    #else
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    #endif

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);  
    glutCreateWindow(windowTitle);

    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    // tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
    // perform animation inside idleFunc
    glutIdleFunc(idleFunc);
    // callback for mouse drags
    glutMotionFunc(mouseMotionDragFunc);
    // callback for idle mouse movement
    glutPassiveMotionFunc(mouseMotionFunc);
    // callback for mouse button changes
    glutMouseFunc(mouseButtonFunc);
    // callback for resizing the window
    glutReshapeFunc(reshapeFunc);
    // callback for pressing the keys on the keyboard
    glutKeyboardFunc(keyboardFunc);

    // init glew
    #ifdef __APPLE__
      // nothing is needed on Apple
    #else
      // Windows, Linux
      GLint result = glewInit();
      if (result != GLEW_OK)
      {
        cout << "error: " << glewGetErrorString(result) << endl;
        exit(EXIT_FAILURE);
      }
    #endif

    // do initialization
    initScene(argc, argv);

    // sink forever into the glut loop
    glutMainLoop();
}