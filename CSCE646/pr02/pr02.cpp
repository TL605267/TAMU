// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 0
// Created by Anton Agana based from Ariel Chisholm's template
// 05.23.2011
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


// =============================================================================
// The following are the self-defined classes: 
// =============================================================================

#include "shape.h"

using namespace std;

// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height;
unsigned char *pixmap;
//string option = "concave";

// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================

void setPixels() {
	for(int y = 0; y < height ; y++) {
		for(int x = 0; x < width; x++) {
			int i = (y * width + x) * 3; 
			pixmap[i++] = 255;
			pixmap[i++] = 0xFF; //Do you know what "0xFF" represents? Google it!
			pixmap[i] = 0x00; //Learn to use the "0x" notation to your advantage.
		}
	}
}

void draw_Concave(mypolygon P, int sample, RGB color_in, RGB color_out) {

	int M = sqrt(sample);
	int N = sqrt(sample);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int count = 0;
			int randx = rand();
			int randy = rand();
			for (int m=0; m<M; m++) {
				for (int n = 0; n < N; n++) {
					double x_sample = x + (double)m / (double)M + ((double)randx/RAND_MAX)/N;
					double y_sample = y + (double)n / (double)N + ((double)randy/RAND_MAX)/N;
					xypair rand_sample(x_sample,y_sample);
					if(P.is_fill(rand_sample)) count++;
				}
			}
			double t = (double)count / sample;
			int coordinates = (y * width + x) * 3;
			pixmap[coordinates++] = t * color_in.red + (1-t) * color_out.red;
			pixmap[coordinates++] = t * color_in.green + (1-t) * color_out.green;
			pixmap[coordinates] = t * color_in.blue + (1-t) * color_out.blue;
		}
	}
}


void draw_function(myfunc P, int sample, RGB color_in, RGB color_out) {

	int M = sqrt(sample);
	int N = sqrt(sample);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int count = 0;
			int randx = rand();
			int randy = rand();
			for (int m=0; m<M; m++) {
				for (int n = 0; n < N; n++) {
					double x_sample = x + (double)m / (double)M + ((double)randx/RAND_MAX)/N;
					double y_sample = y + (double)n / (double)N + ((double)randy/RAND_MAX)/N;
					xypair rand_sample(x_sample,y_sample);
					if(P.is_fill(rand_sample)) {
						count++;
					}
				}
			}
			double t = (double)count / sample;
			int coordinates = (y * width + x) * 3;
			pixmap[coordinates++] = t * color_in.red + (1-t) * color_out.red;
			pixmap[coordinates++] = t * color_in.green + (1-t) * color_out.green;
			pixmap[coordinates] = t * color_in.blue + (1-t) * color_out.blue;
		}
	}
}

void draw_extra(myextra P) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			xypair M(x,y);
			int coordinates = (y * width + x) * 3;
			pixmap[coordinates++] = P.color_fill(M).red;
			pixmap[coordinates++] = P.color_fill(M).green;
			pixmap[coordinates] = P.color_fill(M).blue;
		}
	}
}




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
static void windowDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixmap);
  glFlush();
}
static void processMouse(int button, int state, int x, int y)
{
  if(state == GLUT_UP)
  exit(0);               // Exit on mouse click.
}
static void init(void)
{
  glClearColor(1,1,1,1); // Set background color.
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[]) {
	if (argc == 1) cout << "Please enter a command: convex, function, or extracredit" <<endl;
	else {
		//initialize the global variables
		width = 300;
		height = 300;
		pixmap = new unsigned char[width * height * 3];  //Do you know why "3" is used?
	
		if ((string)argv[1] == "convex") {
			int size = 4;
			xypair convex[size] = {
				xypair(50,50),
				xypair(200,100),
				xypair(250,250),
				xypair(100,200),
			};
			mypolygon T(convex, size);
			RGB in (100, 100, 200);
			RGB out (200, 200, 100);
			int sample = 4;
			draw_Concave(T, sample, in, out);
		}

		if((string)argv[1] == "function") {
			myfunc Func(50,0.1);
			RGB in(100,100,200);
			RGB out(200,200,100);
			int sample = 1;
			draw_function(Func, sample, in, out);
		}

		if((string)argv[1] == "extracredit") {
			xypair C(150,150);
			myextra extra(C,150.0);
			draw_extra(extra);
		}

		// OpenGL Commands:
		// Once "glutMainLoop" is executed, the program loops indefinitely to all
		// glut functions.  
		glutInit(&argc, argv);
		glutInitWindowPosition(100, 100); // Where the window will display on-screen.
		glutInitWindowSize(width, height);
		glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
		glutCreateWindow("Pr02");
		init();
		glutReshapeFunc(windowResize);
		glutDisplayFunc(windowDisplay);
		glutMouseFunc(processMouse);
		glutMainLoop();
	}
	return 0; //This line never gets reached. We use it because "main" is type int.
}

