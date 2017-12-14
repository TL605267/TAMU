#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <algorithm>    // std::max
#include "shape.h"

using namespace std;


struct RGB {
	unsigned char red, green, blue;
};

class myppm {

public: 
	RGB *pixel_map;
	RGB *output_map;
	myppm();
	myppm(char* shape, int width0, int height0);
	int width, height;
	void draw_poly(XY* vertex, int n0);
private: 
	char* type;
	XY center;
	XY* vertex;
};

myppm::myppm(char* shape, int width0, int height0) {
	if (shape == "star") {
		vertex* = new XY[3];
		vertex[0] = XY(2,0);
		vertex[1] = XY(-2,2);
		vertex[2] = XY(-3,-2);
	}
	width = width0;
	height = height0;
	pixel_map = new RGB[width * height];
	output_map = new RGB[width * height];
	center.x = width / 2;
	center.y = height / 2;
}

void myppm::draw_poly (void) {
	my_poly polygon = my_poly(vertex, n0, center);
	for(int i = 0; i < width; i++) {
		for(int j = 0; j < height; j++) {
			int index = j * width + i;
			XY A = XY(i,j);
			if (polygon.is_fill(A)) output_map[index].red = 255;
			else output_map[index].blue = 255;
		}
	}
}