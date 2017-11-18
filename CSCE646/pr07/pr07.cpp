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
myppm ppm;
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
  glDrawPixels(ppm.width, ppm.height, GL_RGB, GL_UNSIGNED_BYTE, ppm.output_map);
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

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[]) {

	// initialize the global variables
	if(argc == 1) {
		cout << "Not enough arguments! Please check out the README." <<endl;
		exit(-1);
	}

	else {
		ppm.ppm_init(argv[2]);
		double mtrx[9];
		int dem;
		XY translation_point(0,0);	
		if ((string)argv[1] == "rotation"||(string)argv[1] == "r") {
			dem = 2;
			double PI = 3.1415926;
			double theta = PI/12; // 30deg
			//translation_point  =XY(255, 170);
			mtrx[0] = cos(theta);;
			mtrx[1] = -sin(theta);
			mtrx[2] = sin(theta);
			mtrx[3] = cos(theta);

	  	}
		if ((string)argv[1] == "scaling") {
			dem = 2;
			mtrx[0] = 2;
			mtrx[1] = 0;
			mtrx[2] = 0;
			mtrx[3] = 2;
	  	}
		if ((string)argv[1] == "shear") {
			dem = 2;
			translation_point  =XY(255, 170);
			mtrx[0] = 0.7;
			mtrx[1] = 0.1;
			mtrx[2] = 0.1;
			mtrx[3] = 0.7;
	  	}
		if ((string)argv[1] == "mirror" || (string)argv[1] == "m") {
			dem = 2;
			translation_point  =XY(255, 170);
			mtrx[0] = 1;
			mtrx[1] = 0;
			mtrx[2] = 0;
			mtrx[3] = -1;
	  	}
		if ((string)argv[1] == "translation" || (string)argv[1] == "t") {
			dem = 3;
			translation_point  =XY(-225, -170);
			mtrx[0] = 1;
			mtrx[1] = 0;
			mtrx[2] = translation_point.x;
			mtrx[3] = 0;
			mtrx[4] = 1;
			mtrx[5] = translation_point.y;
			mtrx[6] = 0;
			mtrx[7] = 0;
			mtrx[8] = 1;
	  	}	
		if ((string)argv[1] == "perspective" || (string)argv[1] == "p") {
			dem = 3;

			mtrx[0] = 1.255;
			mtrx[1] = 0.255;
			mtrx[2] = -108.375;
			mtrx[3] = 0.17;
			mtrx[4] = 1.17;
			mtrx[5] = -72.25;
			mtrx[6] = 0.001;
			mtrx[7] = 0.001;
			mtrx[8] = 0.575;

	  	}
	  	
		if ((string)argv[1] == "bilinear" || (string)argv[1] == "b") {
			dem = 3;
			
			mtrx[0] = 0.526549;
			mtrx[1] = -0.156458;
			mtrx[2] = 187.085;
			mtrx[3] = 0.26632;
			mtrx[4] = 0.44932;
			mtrx[5] = 32.64;
			mtrx[6] = 0.0005;
			mtrx[7] = 0.0005;
			mtrx[8] = 1;

	  	}
	  	
		if ((string)argv[1] == "special" || (string)argv[1] == "s") {
			dem = 3;
			
			mtrx[0] = 1;
			mtrx[1] = 0;
			mtrx[2] = 0;
			mtrx[3] = 0;
			mtrx[4] = 1;
			mtrx[5] = 0;
			mtrx[6] = 0;
			mtrx[7] = 0;
			mtrx[8] = 1;

	  	}
	  	
	  	ppm.transformation(mtrx, dem, translation_point, argv[1] );
		glutInit(&argc, argv);
		glutInitWindowPosition(100, 100); // Where the window will display on-screen.
		glutInitWindowSize(ppm.width, ppm.height);
		glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
		glutCreateWindow("pr06");
		init();
		glutReshapeFunc(windowResize);
		glutDisplayFunc(ppmwindowDisplay);
		glutMouseFunc(processMouse);
		glutMainLoop();
	}

	return 0; //This line never gets reached. We use it because "main" is type int.
}

