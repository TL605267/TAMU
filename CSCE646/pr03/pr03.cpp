// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
//
// Project 1
// Tong Lu
// Template is created by Anton Agana based from Ariel Chisholm's template
// 09.23.2017
//
// This file is supplied with an associated makefile. Put both files in the same
// directory, navigate to that directory from the Linux shell, and type 'make'.
// This will create a program called 'pr01' that you can run by entering
// 'homework0' as a command in the shell.
//
// If you are new to programming in Linux, there is an
// excellent introduction to makefile structure and the gcc compiler here:
//
// http://www.cs.txstate.edu/labs/tutorials/tut_docs/Linux_Prog_Environment.pdf
//
// =============================================================================

#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <math.h>
#include "myppm.h"
using namespace std;

// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height;
unsigned char *pixmap;
myppm ppm;

// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================

// =============================================================================
// OpenGL Display and Mouse Processing Functions.
//
// You can read up on OpenGL and modify these functions, as well as the commands
// in main(), to perform more sophisticated display or GUI behavior. This code
// will service the bare minimum display needs for most assignments.
// =============================================================================
static void windowResize(int w, int h)
{   
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,(w/2),0,(h/2),0,1); 
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity() ;
}
static void ppmwindowDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glDrawPixels(ppm.width, ppm.height, GL_RGB, GL_UNSIGNED_BYTE, ppm.pixel_map);
  glFlush();
}


static void processMouse(int button, int state, int x, int y)
{
  if(state == GLUT_UP)
  exit(0);               // Exit on mouse click.
}
static void init(void) {  
	glClearColor(1,1,1,1); // Set background color.
}

void RGBtoHSV(int r, int g, int b, double &h, double &s, double &v) {

	double RED, GREEN, BLUE;
	double MAX, MIN, delta;

	RED = r / 255.0; GREEN = g / 255.0; BLUE = b / 255.0;  // Normalize RGB Values (0-1)

	MAX = max(RED, max(GREEN, BLUE));
	MIN = min(RED, min(GREEN, BLUE));

	v = MAX;        // value is maximum of r, g, b

	if (MAX == 0){    // saturation and hue 0 if value is 0
		s = 0;
		h = 0;
	}
	else{
		s = (MAX - MIN) / MAX;           // Saturation (0-1)

		delta = MAX - MIN;
		if (delta == 0)                    // Hue will be 0 if Saturation is 0
			h = 0;
		else{
			if (RED == MAX)                  // Hue (0-360)
				h = (GREEN - BLUE) / delta;
			else if (GREEN == MAX)
				h = 2.0 + (BLUE - RED) / delta;
			else
				h = 4.0 + (RED - GREEN) / delta;
			h = h * 60.0;
			if (h < 0)
				h = h + 360.0;
		}
	}
}

void HSVtoRGB(double h, double s, double v, int &r, int &g, int &b) {

	//initial values 
	double intermediate1 = s * v;
	double intermediate2 = intermediate1 * (1 - abs(fmod((h / 60.0), 2) - 1));
	double n = v - intermediate1;
	double R, G, B;
	//analyzing h value based on its range
	if (h < 120)
	{
		B = 0;
		R = (h < 60) ? 1 : intermediate2;
		G = (h < 60) ? intermediate2 : 1;
	}
	else if (h < 240)
	{
		R = 0;
		B = (h < 180) ? intermediate2 : 1;
		G = (h < 180) ? 1 : intermediate2;
	}
	else if (h > 240)
	{
		G = 0;
		B = (h < 300) ? 1 : intermediate2;
		R = (h < 300) ? intermediate2 : 1;
	}
	//computer final r, g, b values
	r = int((R + n) * 255);
	g = int((G + n) * 255);
	b = int((B + n) * 255);
}

//use control image hue values to manipulated image
void change_hue(myppm& control, myppm& output) {
	//manipulate pixel one by one
	for (int y = 0; y < control.height; y++)
	{
		for (int x = 0; x < control.width; x++)
		{
			int i = y * control.width + x;

			double Hue = 0;
			double Saturation = 0;
			double Value = 0;
			double HueFinal = 0;
			double SaturationFinal = 0;
			double ValueFinal = 0;

			RGBtoHSV(control.pixel_map[i].red, control.pixel_map[i].green, control.pixel_map[i].blue, Hue, Saturation, Value);

			RGBtoHSV(output.pixel_map[i].red, output.pixel_map[i].green, output.pixel_map[i].blue, HueFinal, SaturationFinal, ValueFinal);

			// Set output image values equel to control values
			HueFinal = Hue;

			int r = 0;
			int g = 0;
			int b = 0;

			HSVtoRGB(HueFinal, SaturationFinal, ValueFinal, r, g, b);

			output.pixel_map[i].red = r;
			output.pixel_map[i].green = g;
			output.pixel_map[i].blue = b;
		}
	}
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[]) {
	// initialize the global variables
	if(argc == 1 || argc == 2) {
		cout<<"Not enough arguments!"<<endl;
		cout<<"Please check README"<<endl;
		exit(-1);
	}

	else {
		if ((string)argv[1] == "linear") {
			double arr1[6] = {0, 0.5, 0.6, 0.8, 0.9, 1.0}; //0 0.2 0.4 0.6 0.8 1	
			int n1 = 5;
			double arr2[6] = {0, 0.2, 0.4, 0.6, 0.8, 1.0}; //0 0.2 0.4 0.6 0.8 1
			int n2 = 5;
			double arr3[6] = {0, 0.2, 0.4, 0.6, 0.8, 1.0}; //0 0.2 0.4 0.6 0.8 1
			int n3 = 5;
			ppm.ppm_init(argv[2]);
			ppm.piecewise_linear_RGB(n1,arr1,n2,arr2,n3,arr3);
			ppm.color_manipulation();

		}
		if ((string)argv[1] == "cubic") {
			xypair R_point[4] = {xypair(0,0.5), xypair(0.25,0.75), xypair(0.75,0.25), xypair(1.0,0.5)};
			xypair G_point[4] = {xypair(0,0.5), xypair(0.25,0.75), xypair(0.75,0.25), xypair(1.0,0.5)};
			xypair B_point[4] = {xypair(0,0.5), xypair(0.25,0.75), xypair(0.75,0.25), xypair(1.0,0.5)};
			ppm.ppm_init(argv[2]);
			ppm.cubic_curve_RGB(R_point, G_point, B_point);
			ppm.color_manipulation();

		}
		if ((string)argv[1] == "hue") {
			myppm control;
			control.ppm_init(argv[2]);
			ppm.ppm_init(argv[3]);
			change_hue(control, ppm);

		}
		
					
		glutInit(&argc, argv);
		glutInitWindowPosition(100, 100); // Where the window will display on-screen.
		glutInitWindowSize(ppm.width, ppm.height);
		glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
		glutCreateWindow("pr03");
		init();
		glutReshapeFunc(windowResize);
		glutDisplayFunc(ppmwindowDisplay);
		glutMouseFunc(processMouse);
		glutMainLoop();
	}

	
	return 0; //This line never gets reached. We use it because "main" is type int.
}

