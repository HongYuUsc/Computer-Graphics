/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: Hong Yu
 * *************************
*/

#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#include <imageIO.h>

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0
#define Pi 3.14159265

unsigned char buffer[HEIGHT][WIDTH][3];
using namespace std;

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct Triangle
{
  Vertex v[3];
};

struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};

struct Light
{
  double position[3];
  double color[3];
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

//intersection points of sphere//
double x_sphere = 0;
double y_sphere = 0;
double z_sphere = 0;

//intersection points of triangles//
double x_triangle = 0;
double y_triangle = 0;
double z_triangle = 0;

//normals of triangles//
double x_normal = 0;
double y_normal = 0;
double z_normal = 0;

//specular value of triangle//
double x_specular = 0;
double y_specular = 0;
double z_specular = 0;

//barycentric coordinates//
double alpha = 0;
double beta = 0;
double gama = 0;

//flag = 0 when there is no intersection//
int flag = 0; 

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);


void cross_3product(const double a1[], const double a2[], double result[])
{
	result[0] = a1[1] * a2[2] - a1[2] * a2[1];
	result[1] = a1[2] * a2[0] - a1[0] * a2[2];
	result[2] = a1[0] * a2[1] - a1[1] * a2[0];
}

//determine if a ray intersects a sphere//
bool is_sphere_intersection(double x0, double y0, double z0, double xd, double yd, double zd,struct Sphere &sphere)
{
	double length = sqrt(pow(xd, 2) + pow(yd, 2) + pow(zd, 2));
	double xd_n = xd / length;
	double yd_n = yd / length;
	double zd_n = zd / length;
	double xc = sphere.position[0];
	double yc = sphere.position[1];
	double zc = sphere.position[2];
	double r = sphere.radius;
	double b = 2 * (xd_n*(x0 - xc) + yd_n * (y0 - yc) + zd_n * (z0 - zc));
	double c = pow((x0 - xc), 2) + pow((y0 - yc), 2) + pow((z0 - zc), 2) - pow(r, 2);
	double delta = pow(b, 2) - 4 * c;
	if (delta >= 0)
	{
		double t = (-b - sqrt(delta)) / 2;
		if (t > 0)
		{
			x_sphere = x0 + xd_n * t;
			y_sphere = y0 + yd_n * t;
			z_sphere = z0 + zd_n * t;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

//phong shading of sphere//
void sphere_illumination(double eye[], double intersection[], struct Light &light, struct Sphere &sphere, double I[])
{
	double length = sqrt(pow(light.position[0] - intersection[0], 2) + pow(light.position[1] - intersection[1], 2) 
		+ pow(light.position[2] - intersection[2], 2));

	//light//
	double L[3];
	L[0] = (light.position[0] - intersection[0]) / length;
	L[1] = (light.position[1] - intersection[1]) / length;
	L[2] = (light.position[2] - intersection[2]) / length;
	double b = 2 * (L[0]*(intersection[0] - sphere.position[0]) + L[1] * (intersection[1] - sphere.position[1]) + L[2] * (intersection[2] - sphere.position[2]));
	if (b < 0)
	{
		I[0] = 0;
		I[1] = 0;
		I[2] = 0;
	}

	//phong model//
	else
	{
		double r = sphere.radius;
		double N[3] = { (intersection[0] - sphere.position[0]) / r,(intersection[1] - sphere.position[1]) / r,(intersection[2] - sphere.position[2]) / r };
		length = sqrt(pow((eye[0] - intersection[0]), 2) + pow((eye[1] - intersection[1]), 2) + pow((eye[2] - intersection[2]), 2));
		double V[3] = { (eye[0]- intersection[0]) / length, (eye[1] - intersection[1]) / length, (eye[2] - intersection[2]) / length };
		double R[3];
		double ans = 2 * (L[0] * N[0] + L[1] * N[1] + L[2] * N[2]);
		R[0] = ans * N[0] - L[0];
		R[1] = ans * N[1] - L[1];
		R[2] = ans * N[2] - L[2];

		double theta = L[0] * N[0] + L[1] * N[1] + L[2] * N[2];
		if (theta < 0)
			theta = 0;
		double phi = R[0] * V[0] + R[1] * V[1] + R[2] * V[2];
		if (phi < 0)
			phi = 0;

		I[0] = light.color[0] * (sphere.color_diffuse[0] * theta + sphere.color_specular[0] * pow(phi, sphere.shininess));
		I[1] = light.color[1] * (sphere.color_diffuse[1] * theta + sphere.color_specular[1] * pow(phi, sphere.shininess));
		I[2] = light.color[2] * (sphere.color_diffuse[2] * theta + sphere.color_specular[2] * pow(phi, sphere.shininess));
	}
}

//determine if a ray intersects a triangle//
bool is_triangle_intersection(double x0, double y0, double z0, double xd, double yd, double zd, struct Triangle &triangle)
{
	int sign = 0; //if sign = 1, point is out of triangle//
	double a1[3], a2[3], a3[3], a4[3], a5[3], T[3], N[3];
	double normal[3], Area_ABC[3], Area_ABT[3], Area_TCB[3], Area_ATC[3];
	double d, t, area;
	double length = sqrt(pow(xd, 2) + pow(yd, 2) + pow(zd, 2));
	xd = xd / length;
	yd = yd / length;
	zd = zd / length;

	a1[0] = triangle.v[1].position[0] - triangle.v[0].position[0];
	a1[1] = triangle.v[1].position[1] - triangle.v[0].position[1];
	a1[2] = triangle.v[1].position[2] - triangle.v[0].position[2];
	a2[0] = triangle.v[2].position[0] - triangle.v[0].position[0];
	a2[1] = triangle.v[2].position[1] - triangle.v[0].position[1];
	a2[2] = triangle.v[2].position[2] - triangle.v[0].position[2];
	
	cross_3product(a1, a2, normal);
	length = sqrt(pow(normal[0], 2) + pow(normal[1], 2) + pow(normal[2], 2));
	normal[0] /= length;
	normal[1] /= length;
	normal[2] /= length;
	if (normal[0] * xd + normal[1] * yd + normal[2] * zd == 0)
	{
		return false;
	}
		

	d = -(normal[0] * triangle.v[0].position[0] + normal[1] * triangle.v[0].position[1] + normal[2] * triangle.v[0].position[2]);
	t = -(normal[0] * x0 + normal[1] * y0 + normal[2] * z0 + d) / (normal[0] * xd + normal[1] * yd + normal[2] * zd);
	if (t <= 0)
	{
		return false;
	}
    else
	{
		T[0] = x0 + t * xd;
		T[1] = y0 + t * yd;
		T[2] = z0 + t * zd;
		a3[0] = T[0] - triangle.v[0].position[0];
		a3[1] = T[1] - triangle.v[0].position[1];
		a3[2] = T[2] - triangle.v[0].position[2];
		a4[0] = triangle.v[1].position[0] - T[0];
		a4[1] = triangle.v[1].position[1] - T[1];
		a4[2] = triangle.v[1].position[2] - T[2];
		a5[0] = triangle.v[2].position[0] - T[0];
		a5[1] = triangle.v[2].position[1] - T[1];
		a5[2] = triangle.v[2].position[2] - T[2];

		//3D barycentric coordinates//
		cross_3product(a1, a2, Area_ABC);
		cross_3product(a1, a3, Area_ABT);
		cross_3product(a4, a5, Area_TCB);
		cross_3product(a3, a2, Area_ATC);
		if (Area_ABC[0] * Area_ABT[0] + Area_ABC[1] * Area_ABT[1] + Area_ABC[2] * Area_ABT[2] < 0)
			sign = 1;
		else if (Area_ABC[0] * Area_TCB[0] + Area_ABC[1] * Area_TCB[1] + Area_ABC[2] * Area_TCB[2] < 0)
			sign = 1;
		else if (Area_ABC[0] * Area_ATC[0] + Area_ABC[1] * Area_ATC[1] + Area_ABC[2] * Area_ATC[2] < 0)
			sign = 1;

		area = sqrt(pow(Area_ABC[0], 2) + pow(Area_ABC[1], 2) + pow(Area_ABC[2], 2));
		alpha = sqrt(pow(Area_TCB[0], 2) + pow(Area_TCB[1], 2) + pow(Area_TCB[2], 2)) / area;
		beta = sqrt(pow(Area_ATC[0], 2) + pow(Area_ATC[1], 2) + pow(Area_ATC[2], 2)) / area;
		gama = sqrt(pow(Area_ABT[0], 2) + pow(Area_ABT[1], 2) + pow(Area_ABT[2], 2)) / area;

		N[0] = alpha * triangle.v[0].normal[0] + beta * triangle.v[1].normal[0] + gama * triangle.v[2].normal[0];
		N[1] = alpha * triangle.v[0].normal[1] + beta * triangle.v[1].normal[1] + gama * triangle.v[2].normal[1];
		N[2] = alpha * triangle.v[0].normal[2] + beta * triangle.v[1].normal[2] + gama * triangle.v[2].normal[2];
		length = sqrt(pow(N[0], 2) + pow(N[1], 2) + pow(N[2], 2));
		N[0] /= length;
		N[1] /= length;
		N[2] /= length;

		if (sign == 1)
			return false;

		x_specular = alpha * triangle.v[0].color_specular[0] + beta * triangle.v[1].color_specular[0] + gama * triangle.v[2].color_specular[0];
		y_specular = alpha * triangle.v[0].color_specular[1] + beta * triangle.v[1].color_specular[1] + gama * triangle.v[2].color_specular[1];
		z_specular = alpha * triangle.v[0].color_specular[2] + beta * triangle.v[1].color_specular[2] + gama * triangle.v[2].color_specular[2];
		x_normal = N[0];
		y_normal = N[1];
		z_normal = N[2];
		x_triangle = T[0];
		y_triangle = T[1];
		z_triangle = T[2];
		return true;
	}
}

//phong shading of triangle//
void triangle_illumination(double eye[], double intersection[], double a, double b, double g, double normal[], double I[], struct Light &light, struct Triangle &triangle)
{
	double N[3], tri_d[3], tri_s[3];
	double tri_shi, length;
	N[0] = normal[0];
	N[1] = normal[1];
	N[2] = normal[2];

	tri_d[0] = a * triangle.v[0].color_diffuse[0] + b * triangle.v[1].color_diffuse[0] + g * triangle.v[2].color_diffuse[0];
	tri_d[1] = a * triangle.v[0].color_diffuse[1] + b * triangle.v[1].color_diffuse[1] + g * triangle.v[2].color_diffuse[1];
	tri_d[2] = a * triangle.v[0].color_diffuse[2] + b * triangle.v[1].color_diffuse[2] + g * triangle.v[2].color_diffuse[2];

	tri_s[0] = a * triangle.v[0].color_specular[0] + b * triangle.v[1].color_specular[0] + g * triangle.v[2].color_specular[0];
	tri_s[1] = a * triangle.v[0].color_specular[1] + b * triangle.v[1].color_specular[1] + g * triangle.v[2].color_specular[1];
	tri_s[2] = a * triangle.v[0].color_specular[2] + b * triangle.v[1].color_specular[2] + g * triangle.v[2].color_specular[2];
	tri_shi = a * triangle.v[0].shininess + b * triangle.v[1].shininess + g * triangle.v[2].shininess;

	length = sqrt(pow(light.position[0] - intersection[0], 2) + pow(light.position[1] - intersection[1], 2)
		+ pow(light.position[2] - intersection[2], 2));

	//light//
	double L[3];
	I[0] = 0;
	I[1] = 0;
	I[2] = 0;

	L[0] = (light.position[0] - intersection[0]) / length;
	L[1] = (light.position[1] - intersection[1]) / length;
	L[2] = (light.position[2] - intersection[2]) / length;
	if (L[0] * N[0] + L[1] * N[1] + L[2] * N[2] < 0)
	{
		return;
	}

	//phong model//
	else
	{
		length = sqrt(pow(eye[0]-intersection[0], 2) + pow(eye[1] - intersection[1], 2) + pow(eye[2] - intersection[2], 2));
		double V[3] = { (eye[0] - intersection[0]) / length, (eye[1] - intersection[1]) / length, (eye[2] - intersection[2]) / length };
		double R[3];
		double ans = 2 * (L[0] * N[0] + L[1] * N[1] + L[2] * N[2]);
		R[0] = ans * N[0] - L[0];
		R[1] = ans * N[1] - L[1];
		R[2] = ans * N[2] - L[2];

		double theta = L[0] * N[0] + L[1] * N[1] + L[2] * N[2];
		if (theta < 0)
			theta = 0;
		double phi = R[0] * V[0] + R[1] * V[1] + R[2] * V[2];
		if (phi < 0)
			phi = 0;

		I[0] = light.color[0] * (tri_d[0] * theta + tri_s[0] * pow(phi, tri_shi));
		I[1] = light.color[1] * (tri_d[1] * theta + tri_s[1] * pow(phi, tri_shi));
		I[2] = light.color[2] * (tri_d[2] * theta + tri_s[2] * pow(phi, tri_shi));

	}
}

//The recursive ray tracer//
void ray_tracer(double eye[],double direction[],double result[], int triangle_index,int sphere_index,int time,int softshadow)
{
	double color[3] = { 0,0,0 };
	double f_color[3] = { 0,0,0 };
	result[0] = 0;
	result[1] = 0;
	result[2] = 0;
	double reflection[3] = { 0,0,0 };
	double view[3] = { 0,0,0 };
	double normal[3] = { 0,0,0 };
	double specular[3] = { 0,0,0 };
	double ans,a,b,g;
	flag = 0;
	int shadow = 0;
	double intersection[3] = { 1000 * direction[0], 1000 * direction[1], 1000 * direction[2] };
	
	if (time == 0)
		return;

	//Draw Triangle//
	for (int i = 0; i < num_triangles; i++)
	{
		if (i != triangle_index && is_triangle_intersection(eye[0], eye[1], eye[2], direction[0], direction[1], direction[2], triangles[i]))
		{
			if ((x_triangle - intersection[0]) * intersection[0] + (y_triangle - intersection[1]) * intersection[1] + (z_triangle - intersection[2]) * intersection[2] <= 0)
			{
				intersection[0] = x_triangle;
				intersection[1] = y_triangle;
				intersection[2] = z_triangle;
				specular[0] = x_specular;
				specular[1] = y_specular;
				specular[2] = z_specular;
				a = alpha;
				b = beta;
				g = gama;
				normal[0] = x_normal;
				normal[1] = y_normal;
				normal[2] = z_normal;
				view[0] = eye[0] - intersection[0];
				view[1] = eye[1] - intersection[1];
				view[2] = eye[2] - intersection[2];
				ans = 2 * (view[0] * x_normal + view[1] * y_normal + view[2] * z_normal);
				reflection[0] = ans * x_normal - view[0];
				reflection[1] = ans * y_normal - view[1];
				reflection[2] = ans * z_normal - view[2];
				//Do ray tracing recursively, determined by time//
				ray_tracer(intersection, reflection, color, i, num_spheres, time - 1, softshadow);
				f_color[0] += specular[0] * color[0];
				f_color[1] += specular[1] * color[1];
				f_color[2] += specular[2] * color[2];

				//hard shadow//
				if (softshadow == 0)
				{
					//loop for every light//
					for (int j = 0; j < num_lights; j++)
					{

						//shadow rays//
						double xd = lights[j].position[0] - intersection[0];
						double yd = lights[j].position[1] - intersection[1];
						double zd = lights[j].position[2] - intersection[2];

						//check if blocked by spheres//
						for (int k = 0; k < num_spheres; k++)
						{
							if (is_sphere_intersection(intersection[0], intersection[1], intersection[2], xd, yd, zd, spheres[k]))
							{
								double d1 = pow(x_sphere - intersection[0], 2) + pow(y_sphere - intersection[1], 2) + pow(z_sphere - intersection[2], 2);
								double d2 = pow(xd, 2) + pow(yd, 2) + pow(zd, 2);
								if (d1 <= d2)
								{
									shadow = 1;
									break;
								}
							}
						}

						//check if blocked by triangles//
						if (shadow == 0)
						{
							for (int k = 0; k < num_triangles; k++)
							{
								if (k != i)
								{
									if (is_triangle_intersection(intersection[0], intersection[1], intersection[2], xd, yd, zd, triangles[k]))
									{
										double d1 = pow(x_triangle - intersection[0], 2) + pow(y_triangle - intersection[1], 2) + pow(z_triangle - intersection[2], 2);
										double d2 = pow(xd, 2) + pow(yd, 2) + pow(zd, 2);
										if (d1 <= d2)
										{
											shadow = 1;
											break;
										}
									}
								}
							}
						}


						if (shadow == 1)
						{
							shadow = 0;
							continue;
						}

						triangle_illumination(eye, intersection, a, b, g, normal, color, lights[j], triangles[i]);
						if (time == 1)
						{
							f_color[0] += color[0];
							f_color[1] += color[1];
							f_color[2] += color[2];
						}

						else
						{
							f_color[0] += (1 - specular[0]) * color[0];
							f_color[1] += (1 - specular[1]) * color[1];
							f_color[2] += (1 - specular[2]) * color[2];
						}
					}
				}

				//softshadow//
				else
				{
					//loop for every light//
					for (int j = 0; j < num_lights; j++)
					{
						double xd[7];
						double yd[7];
						double zd[7];

						//shadow rays//
						xd[0] = lights[j].position[0] - intersection[0];
						yd[0] = lights[j].position[1] - intersection[1];
						zd[0] = lights[j].position[2] - intersection[2];
						xd[1] = lights[j].position[0] - intersection[0] + 0.05;
						yd[1] = lights[j].position[1] - intersection[1];
						zd[1] = lights[j].position[2] - intersection[2];
						xd[2] = lights[j].position[0] - intersection[0] - 0.05;
						yd[2] = lights[j].position[1] - intersection[1];
						zd[2] = lights[j].position[2] - intersection[2];
						xd[3] = lights[j].position[0] - intersection[0];
						yd[3] = lights[j].position[1] - intersection[1] + 0.05;
						zd[3] = lights[j].position[2] - intersection[2];
						xd[4] = lights[j].position[0] - intersection[0];
						yd[4] = lights[j].position[1] - intersection[1] - 0.05;
						zd[4] = lights[j].position[2] - intersection[2];
						xd[5] = lights[j].position[0] - intersection[0];
						yd[5] = lights[j].position[1] - intersection[1];
						zd[5] = lights[j].position[2] - intersection[2] + 0.05;
						xd[6] = lights[j].position[0] - intersection[0];
						yd[6] = lights[j].position[1] - intersection[1];
						zd[6] = lights[j].position[2] - intersection[2] - 0.05;

						for (int m = 0; m < 7; m++)
						{
							//check if blocked by spheres//
							for (int k = 0; k < num_spheres; k++)
							{
								if (is_sphere_intersection(intersection[0], intersection[1], intersection[2], xd[m], yd[m], zd[m], spheres[k]))
								{
									double d1 = pow(x_sphere - intersection[0], 2) + pow(y_sphere - intersection[1], 2) + pow(z_sphere - intersection[2], 2);
									double d2 = pow(xd[m], 2) + pow(yd[m], 2) + pow(zd[m], 2);
									if (d1 <= d2)
									{
										shadow = 1;
										break;
									}
								}
							}

							//check if blocked by triangles//
							if (shadow == 0)
							{
								for (int k = 0; k < num_triangles; k++)
								{
									if (k != i)
									{
										if (is_triangle_intersection(intersection[0], intersection[1], intersection[2], xd[m], yd[m], zd[m], triangles[k]))
										{
											double d1 = pow(x_triangle - intersection[0], 2) + pow(y_triangle - intersection[1], 2) + pow(z_triangle - intersection[2], 2);
											double d2 = pow(xd[m], 2) + pow(yd[m], 2) + pow(zd[m], 2);
											if (d1 <= d2)
											{
												shadow = 1;
												break;
											}
										}
									}
								}
							}


							if (shadow == 1)
							{
								shadow = 0;
								continue;
							}

							triangle_illumination(eye, intersection, a, b, g, normal, color, lights[j], triangles[i]);
							if (time == 1)
							{
								f_color[0] += color[0] / 7;
								f_color[1] += color[1] / 7;
								f_color[2] += color[2] / 7;
							}

							else
							{
								f_color[0] += (1 - specular[0]) * color[0] / 7;
								f_color[1] += (1 - specular[1]) * color[1] / 7;
								f_color[2] += (1 - specular[2]) * color[2] / 7;
							}
						}
					}
				}

				result[0] = f_color[0];
				result[1] = f_color[1];
				result[2] = f_color[2];
				f_color[0] = 0;
				f_color[1] = 0;
				f_color[2] = 0;
				flag = 1;
			}
		}
	}

	//Draw sphere//
	for (int i = 0; i < num_spheres; i++)
	{
		if (i != sphere_index && is_sphere_intersection(eye[0], eye[1], eye[2], direction[0], direction[1], direction[2], spheres[i]))
		{
			if ((x_sphere - intersection[0]) * intersection[0] + (y_sphere - intersection[1]) * intersection[1] + (z_sphere - intersection[2]) * intersection[2] <= 0)
			{
				intersection[0] = x_sphere;
				intersection[1] = y_sphere;
				intersection[2] = z_sphere;
				view[0] = eye[0] - intersection[0];
				view[1] = eye[1] - intersection[1];
				view[2] = eye[2] - intersection[2];
				ans = 2 * (view[0] * (intersection[0] - spheres[i].position[0]) + view[1] * (intersection[1] - spheres[i].position[1]) + view[2] * (intersection[2] - spheres[i].position[2]));
				reflection[0] = ans * (intersection[0] - spheres[i].position[0]) - view[0];
				reflection[1] = ans * (intersection[1] - spheres[i].position[1]) - view[1];
				reflection[2] = ans * (intersection[2] - spheres[i].position[2]) - view[2];
				//Do ray tracing recursively, determined by time//
				ray_tracer(intersection, reflection, color, num_triangles, i, time - 1, softshadow);
				f_color[0] += spheres[i].color_specular[0] * color[0];
				f_color[1] += spheres[i].color_specular[1] * color[1];
				f_color[2] += spheres[i].color_specular[2] * color[2];

				//hardshadow//
				if (softshadow == 0)
				{
					for (int j = 0; j < num_lights; j++)
					{
						//shadow rays//
						double xd = lights[j].position[0] - intersection[0];
						double yd = lights[j].position[1] - intersection[1];
						double zd = lights[j].position[2] - intersection[2];
						
						//check if blocked by spheres//
						for (int k = 0; k < num_spheres; k++)
						{
							if (k != i)
							{
								if (is_sphere_intersection(intersection[0], intersection[1], intersection[2], xd, yd, zd, spheres[k]))
								{
									double d1 = pow(x_sphere - intersection[0], 2) + pow(y_sphere - intersection[1], 2) + pow(z_sphere - intersection[2], 2);
									double d2 = pow(xd, 2) + pow(yd, 2) + pow(zd, 2);
									if (d1 <= d2)
									{
										shadow = 1;
										break;
									}
								}
							}
						}

						//check if blocked by triangles//
						if (shadow == 0)
						{
							for (int k = 0; k < num_triangles; k++)
							{
								if (is_triangle_intersection(intersection[0], intersection[1], intersection[2], xd, yd, zd, triangles[k]))
								{
									double d1 = pow(x_triangle - intersection[0], 2) + pow(y_triangle - intersection[1], 2) + pow(z_triangle - intersection[2], 2);
									double d2 = pow(xd, 2) + pow(yd, 2) + pow(zd, 2);
									if (d1 <= d2)
									{
										shadow = 1;
										break;
									}
								}
							}
						}

						if (shadow == 1)
						{
							shadow = 0;
							continue;
						}

						sphere_illumination(eye, intersection, lights[j], spheres[i], color);
						if (time == 1)
						{
							f_color[0] += color[0];
							f_color[1] += color[1];
							f_color[2] += color[2];
						}

						else
						{
							f_color[0] += (1 - spheres[i].color_specular[0]) * color[0];
							f_color[1] += (1 - spheres[i].color_specular[1]) * color[1];
							f_color[2] += (1 - spheres[i].color_specular[2]) * color[2];
						}
					}
				}

				else
				{
					//loop for every light//
					for (int j = 0; j < num_lights; j++)
					{
						double xd[7];
						double yd[7];
						double zd[7];
						//shadow rays//
						xd[0] = lights[j].position[0] - intersection[0];
						yd[0] = lights[j].position[1] - intersection[1];
						zd[0] = lights[j].position[2] - intersection[2];
						xd[1] = lights[j].position[0] - intersection[0] + 0.05;
						yd[1] = lights[j].position[1] - intersection[1];
						zd[1] = lights[j].position[2] - intersection[2];
						xd[2] = lights[j].position[0] - intersection[0] - 0.05;
						yd[2] = lights[j].position[1] - intersection[1];
						zd[2] = lights[j].position[2] - intersection[2];
						xd[3] = lights[j].position[0] - intersection[0];
						yd[3] = lights[j].position[1] - intersection[1] + 0.05;
						zd[3] = lights[j].position[2] - intersection[2];
						xd[4] = lights[j].position[0] - intersection[0];
						yd[4] = lights[j].position[1] - intersection[1] - 0.05;
						zd[4] = lights[j].position[2] - intersection[2];
						xd[5] = lights[j].position[0] - intersection[0];
						yd[5] = lights[j].position[1] - intersection[1];
						zd[5] = lights[j].position[2] - intersection[2] + 0.05;
						xd[6] = lights[j].position[0] - intersection[0];
						yd[6] = lights[j].position[1] - intersection[1];
						zd[6] = lights[j].position[2] - intersection[2] - 0.05;

						for (int m = 0; m < 7; m++)
						{
							//check if blocked by spheres//
							for (int k = 0; k < num_spheres; k++)
							{
								if (k != i)
								{
									if (is_sphere_intersection(intersection[0], intersection[1], intersection[2], xd[m], yd[m], zd[m], spheres[k]))
									{
										double d1 = pow(x_sphere - intersection[0], 2) + pow(y_sphere - intersection[1], 2) + pow(z_sphere - intersection[2], 2);
										double d2 = pow(xd[m], 2) + pow(yd[m], 2) + pow(zd[m], 2);
										if (d1 <= d2)
										{
											shadow = 1;
											break;
										}
									}
								}
							}

							//check if blocked by triangles//
							if (shadow == 0)
							{
								for (int k = 0; k < num_triangles; k++)
								{
									if (is_triangle_intersection(intersection[0], intersection[1], intersection[2], xd[m], yd[m], zd[m], triangles[k]))
									{
										double d1 = pow(x_triangle - intersection[0], 2) + pow(y_triangle - intersection[1], 2) + pow(z_triangle - intersection[2], 2);
										double d2 = pow(xd[m], 2) + pow(yd[m], 2) + pow(zd[m], 2);
										if (d1 <= d2)
										{
											shadow = 1;
											break;
										}
									}
								}
							}

							if (shadow == 1)
							{
								shadow = 0;
								continue;
							}

							sphere_illumination(eye, intersection, lights[j], spheres[i], color);
							if (time == 1)
							{
								f_color[0] += color[0] / 7;
								f_color[1] += color[1] / 7;
								f_color[2] += color[2] / 7;
							}

							else
							{
								f_color[0] += (1 - spheres[i].color_specular[0]) * color[0] / 7;
								f_color[1] += (1 - spheres[i].color_specular[1]) * color[1] / 7;
								f_color[2] += (1 - spheres[i].color_specular[2]) * color[2] / 7;
							}
						}
					}
				}
				result[0] = f_color[0];
				result[1] = f_color[1];
				result[2] = f_color[2];
				f_color[0] = 0;
				f_color[1] = 0;
				f_color[2] = 0;
				flag = 1;
			}
		}
	}

	if (flag == 0)
	{
		result[0] = 0;
		result[1] = 0;
		result[2] = 0;
	}
}

//MODIFY THIS FUNCTION
void draw_scene()
{
  double eye[3] = { 0,0,0 };
  double direction[3];
  double color[3] = { 0,0,0 };
  double pixel[3] = { 0,0,0 };

  //times for Recursive reflection//
  //antiailiasing for Good antialiasing//
  //softshadow for softshadow//
  //Those 3 parameters for extra credits//

  int times = 1; //time = 1, only 1 view ray, no reflection ray//
                 //time = 2, relect once//
                 //time = 3, reflect twice, and so on//
                 //time >= 1//
  int antiailiasing = 0;//0 : no antiailiasing, 1 : antiailiasing//
  int softshadow = 0;//0: no softshadow, 1 : softshadow//

  //a simple test output
  for(int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(int y=0; y<HEIGHT; y++)
    {
		//no antiailiasing//
		if (antiailiasing == 0)
		{
			direction[0] = x - 320;
			direction[1] = y - 240;
			direction[2] = -240 / tan(Pi * fov / 360);
			ray_tracer(eye, direction, color, num_triangles, num_spheres, times,softshadow);
			if (flag == 0)
			{
				pixel[0] += 255;
				pixel[1] += 255;
				pixel[2] += 255;
			}
			else
			{
				pixel[0] += (color[0] + ambient_light[0]) * 255;
				pixel[1] += (color[1] + ambient_light[1]) * 255;
				pixel[2] += (color[2] + ambient_light[2]) * 255;
			}

			plot_pixel(x, y, min((int)pixel[0], 255), min((int)pixel[1], 255), min((int)pixel[2], 255));
			pixel[0] = 0;
			pixel[1] = 0;
			pixel[2] = 0;
		}
		
		//fire 4 rays to do anti-ailiasing//
		else
		{
			//1//
			direction[0] = 2 * x - 640;
			direction[1] = 2 * y - 480;
			direction[2] = -480 / tan(Pi * fov / 360);
			ray_tracer(eye, direction, color, num_triangles, num_spheres, times, softshadow);
			if (flag == 0)
			{
				pixel[0] += 255;
				pixel[1] += 255;
				pixel[2] += 255;
			}
			else
			{
				pixel[0] += (color[0] + ambient_light[0]) * 255;
				pixel[1] += (color[1] + ambient_light[1]) * 255;
				pixel[2] += (color[2] + ambient_light[2]) * 255;
			}

			//2//
			direction[0] = 2 * x + 1 - 640;
			direction[1] = 2 * y - 480;
			direction[2] = -480 / tan(Pi * fov / 360);
			ray_tracer(eye, direction, color, num_triangles, num_spheres, times,softshadow);
			if (flag == 0)
			{
			pixel[0] += 255;
			pixel[1] += 255;
			pixel[2] += 255;
			}
			else
			{
			pixel[0] += (color[0] + ambient_light[0]) * 255;
			pixel[1] += (color[1] + ambient_light[1]) * 255;
			pixel[2] += (color[2] + ambient_light[2]) * 255;
			}

			//3//
			direction[0] = 2 * x - 640;
			direction[1] = 2 * y + 1 - 480;
			direction[2] = -480 / tan(Pi * fov / 360);
			ray_tracer(eye, direction, color, num_triangles, num_spheres, times, softshadow);
			if (flag == 0)
			{
			pixel[0] += 255;
			pixel[1] += 255;
			pixel[2] += 255;
			}
			else
			{
			pixel[0] += (color[0] + ambient_light[0]) * 255;
			pixel[1] += (color[1] + ambient_light[1]) * 255;
			pixel[2] += (color[2] + ambient_light[2]) * 255;
			}

			//4//
			direction[0] = 2 * x + 1 - 640;
			direction[1] = 2 * y + 1 - 480;
			direction[2] = -480 / tan(Pi * fov / 360);
			ray_tracer(eye, direction, color, num_triangles, num_spheres, times, softshadow);
			if (flag == 0)
			{
			pixel[0] += 255;
			pixel[1] += 255;
			pixel[2] += 255;
			}
			else
			{
			pixel[0] += (color[0] + ambient_light[0]) * 255;
			pixel[1] += (color[1] + ambient_light[1]) * 255;
			pixel[2] += (color[2] + ambient_light[2]) * 255;
			}

			pixel[0] /= 4;
			pixel[1] /= 4;
			pixel[2] /= 4;

			plot_pixel(x, y, min((int)pixel[0], 255), min((int)pixel[1], 255), min((int)pixel[2], 255));
			pixel[0] = 0;
			pixel[1] = 0;
			pixel[2] = 0;
		}
	}
    glEnd();
    glFlush();
  }
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = r;
  buffer[y][x][1] = g;
  buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in Saving\n");
  else 
    printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parse error, abnormal abortion\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

