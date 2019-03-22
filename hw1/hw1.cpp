/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: <type your USC username here>
*/                       

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

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
float landScale[3] = { 0.6f, 0.6f, 0.6f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";
BasicPipelineProgram* pipelineProgram = new BasicPipelineProgram();

ImageIO * heightmapImage;

//vbo4,vao4 & vbo5,vao5 are for extra credit//
GLuint vao1,vao2,vao3,vao4,vao5;
GLuint vbo1,vbo2,vbo3,vbo4,vbo5;
GLuint elementbuffer; //index buffer//
GLuint program;
int size1, size2,size3,size4,size5;//size of positions//
int numVertices;
int mode = 1; //control which vao to be displayed//
int photo = 0; //global viriable for the screenshot//
vector<float> positions; //store positions//
vector<float> colors; //store colors//
vector<float> zeros; //zero vectors to reset positions and colors//
vector<unsigned int> indices; //store indices of vertices for glDrawElements//
OpenGLMatrix* openGLMatrix = new OpenGLMatrix();
GLint h_modelViewMatrix, h_projectionMatrix;
char file[100]; //filename for screenshot//

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
void initVBO(GLuint &vbo)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*positions.size() + sizeof(float)*colors.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*positions.size(), positions.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*positions.size(), sizeof(float)*colors.size(), colors.data());
}

//set the shader and layout of vao//
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
	loc = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(loc);
	offset = (const void*)(size);
	stride = 0;
	normalized = GL_FALSE;
	// set the layout of the “color” attribute data
	glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);
}

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
	openGLMatrix->LoadIdentity();
	//int zStudent = 3 + 4356762610 / 10000000000;
	openGLMatrix->LookAt(260, 180, 50, 128, 0, -128, 0, 1, 0);
	openGLMatrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	openGLMatrix->Rotate(angle, 0, 1, 0);
	openGLMatrix->Rotate(angle2, 1, 0, 0);
	openGLMatrix->Scale(landScale[0], landScale[1], landScale[2]);
	
	//model-view matrix//
	float m[16];
	openGLMatrix->GetMatrix(m);
	GLboolean isRowMajor = GL_FALSE;
	glUniformMatrix4fv(h_modelViewMatrix, 1, isRowMajor, m);

	//projection matrix//
	openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
	float p[16];
	openGLMatrix->GetMatrix(p);
	isRowMajor = GL_FALSE;
	glUniformMatrix4fv(h_projectionMatrix, 1, isRowMajor, p);

	GLint first = 0;
	GLsizei count = numVertices;
	//mode 4 and 5 are for extra credits//
	switch (mode)
	{
	case 1:
		glDrawArrays(GL_POINTS, first, count);
		break;
	case 2:
		count = size2 / (sizeof(float) * 3);
		glDrawArrays(GL_LINES, first, count);
		break;
	case 3:
		count = size3 / (sizeof(float) * 3);
		glDrawArrays(GL_TRIANGLES, first, count);
		break;
	case 4:
		count = size4 / (sizeof(float) * 3);
		glDrawArrays(GL_TRIANGLE_STRIP, first, count);
		break;
	case 5:
		count = size5 / (sizeof(float) * 3);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		break;
	default:
		break;
	}

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
  openGLMatrix->Perspective(60.0, 1.0* w / h, 0.01, 1500.0);
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
        landTranslate[0] += mousePosDelta[0] * 0.15f;
        landTranslate[1] -= mousePosDelta[1] * 0.15f;
      }
      if (rightMouseButton)
      {
        // control z translation via the right mouse button
        landTranslate[2] += mousePosDelta[1] * 0.15f;
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
		sprintf(file, "screenshot%d.jpg", photo);
        saveScreenshot(file);
    break;

	//point mode//
	case 'q':
		glBindVertexArray(vao1);
		mode = 1;
	break;

	//wireframe mode//
	case 'w':
		glBindVertexArray(vao2);
		mode = 2;
	break;

	//solid mode//
	case 'e':
		glBindVertexArray(vao3);
		mode = 3;
	
	//triangle strip mode//
	case 'r':
		glBindVertexArray(vao4);
		mode = 4;
	
    //glDrawElement mode//
	case 't':
		glBindVertexArray(vao5);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		mode = 5;
    break;
  }
}

//push the position and color value of each vertex into the positions and colors vector//
void push(int i , int j)
{
	positions.push_back(i);
	positions.push_back(heightmapImage->getPixel(i, j, 0));
	positions.push_back(-j);
	colors.push_back(float(heightmapImage->getPixel(i, j, 0)) / 255);//make color change smoothly//
	colors.push_back(float(heightmapImage->getPixel(i, j, 0)) / 255);
	colors.push_back(1);
	colors.push_back(1);
}

void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);

  // do additional initialization here...
  pipelineProgram->Init("../openGLHelper-starterCode");
  pipelineProgram->Bind();


  program = pipelineProgram->GetProgramHandle();
  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");

  int width = heightmapImage->getWidth();
  int height = heightmapImage->getHeight();
  numVertices = width * height;

  //point//
  for (int i = 0; i < height; i++)
  {
	  for (int j = 0; j < width; j++)
	  {
		  push(i, j);
	  }
  }
  initVBO(vbo1);
  glGenVertexArrays(1, &vao1);
  glBindVertexArray(vao1);
  size1 = sizeof(float)*positions.size();
  setshader(size1, program);
  positions = zeros;
  colors = zeros;
  

  //wireframe//
  for (int i = 0; i < height; i++)
  {
      for (int j = 0; j < width - 1; j++)
      {
		  push(i, j);
		  push(i, j + 1);
      }
  }

  for (int j = 0; j < width; j++)
  {
	  for (int i = 0; i < height - 1; i++)
	  {
		  push(i, j);
		  push(i + 1, j);
	  }
  }
  initVBO(vbo2);
  glGenVertexArrays(1, &vao2);
  glBindVertexArray(vao2);
  size2 = sizeof(float)*positions.size();
  setshader(size2, program);
  positions = zeros;
  colors = zeros;
  

  //solid//
  for (int i = 0; i < height - 1; i++)
  {
	  for (int j = 0; j < width - 1; j++)
	  {
		  push(i, j);
		  push(i + 1, j);
		  push(i, j + 1);
		  push(i + 1, j);
		  push(i, j + 1);
		  push(i + 1,j + 1);
	  }
  }
  initVBO(vbo3);
  glGenVertexArrays(1, &vao3);
  glBindVertexArray(vao3);
  size3 = sizeof(float)*positions.size();
  setshader(size3, program);
  positions = zeros;
  colors = zeros;

  //triangle strip//
  for (int i = 0; i < height - 1; i++)
  {
	  for (int j = 0; j < width - 1; j++)
	  {
		  if (i % 2 == 0)
		  {
			  push(i, j);
			  push(i + 1, j);
		  }
		  else if (i % 2 == 1)
		  {
			  push(i + 1, width - 1 - j);
			  push(i, width - 1 - j);
		  }
	  }
  }
  initVBO(vbo4);
  glGenVertexArrays(1, &vao4);
  glBindVertexArray(vao4);
  size4 = sizeof(float)*positions.size();
  setshader(size4, program);
  positions = zeros;
  colors = zeros;

  //glDrawElements//
  for (int i = 0; i < height; i++)
  {
	  for (int j = 0; j < width; j++)
	  {
		  push(i, j);
	  }
  }

  initVBO(vbo5);
  glGenVertexArrays(1, &vao5);
  glBindVertexArray(vao5);
  size5 = sizeof(float)*positions.size();
  setshader(size5, program);
  for (int i = 0; i < height - 1; i++)
  {
	  for (int j = 0; j < width - 1; j++)
	  {
		  indices.push_back(i*height + j);
		  indices.push_back((i + 1)*height + j);
		  indices.push_back(i*height + j + 1);
		  indices.push_back((i + 1)*height + j);
		  indices.push_back(i*height + j + 1);
		  indices.push_back((i + 1)*height + j + 1);
	  }
  }
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  
  //intialize the mode with 5//
  mode = 5;
  
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

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