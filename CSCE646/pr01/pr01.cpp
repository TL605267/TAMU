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
void setPixels()
{
   for(int y = 0; y < height ; y++) {
     for(int x = 0; x < width; x++) {
       int i = (y * width + x) * 3; 
       pixmap[i++] = 0;
       pixmap[i++] = 0; //Do you know what "0xFF" represents? Google it!
       pixmap[i] = 0; //Learn to use the "0x" notation to your advantage.
     }
   }
}

void coolpicture() {
   for(int y = 0; y < height ; y++) {
     for(int x = 0; x < width; x++) {
       int i = (y * width + x) * 3; 
       int x0 = x*255/(double)height;
       int y0 = y*255/(double)width;
       pixmap[i++] = x0;
       pixmap[i++] = y0; //Do you know what "0xFF" represents? Google it!
       pixmap[i++] = (x0+y0)/2; //Learn to use the "0x" notation to your advantage.
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
static void ppmwindowDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glDrawPixels(ppm.width, ppm.height, GL_RGB, GL_UNSIGNED_BYTE, ppm.pixel_map);
  glFlush();
}
static void picwindowDisplay(void)
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
static void init(void) {  
	glClearColor(1,1,1,1); // Set background color.
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[]) {

	// initialize the global variables
	if(argc == 1) {
		width = 256;
		height = 256;
		pixmap = new unsigned char[width * height * 3];
		coolpicture();
		glutInit(&argc, argv);
		glutInitWindowPosition(100, 100); // Where the window will display on-screen.
		glutInitWindowSize(width, height);
		glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
		glutCreateWindow("my picture");
		init();
		glutReshapeFunc(windowResize);
		glutDisplayFunc(picwindowDisplay);
		glutMouseFunc(processMouse);
		glutMainLoop();
	}

	else {
		ppm.ppm_init(argv[1]);
		glutInit(&argc, argv);
		glutInitWindowPosition(100, 100); // Where the window will display on-screen.
		glutInitWindowSize(ppm.width, ppm.height);
		glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
		glutCreateWindow("pr01");
		init();
		glutReshapeFunc(windowResize);
		glutDisplayFunc(ppmwindowDisplay);
		glutMouseFunc(processMouse);
		glutMainLoop();
	}

	return 0; //This line never gets reached. We use it because "main" is type int.
}

