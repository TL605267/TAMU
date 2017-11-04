#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

#include <cmath>
#include <random>

using namespace std;
#define PI 3.14159265
struct RGB {
	int red;
	int green;
	int blue;
	RGB(){};
	RGB(int r0, int g0, int b0): red(r0), green(g0), blue(b0) {}
};

struct xypair{
	double x;
	double y;
	xypair(){};
	xypair(double x0, double y0): x(x0), y(y0) {}
};

class myline {
public:
	myline(){};
	myline(xypair A, xypair B);
	// for ax+by+c=0: 
	double a;
	double b;
	double c;
	// for vector (x,y);
	xypair direction;
	bool is_left(xypair M);
};

myline::myline(xypair A, xypair B) {
	direction.x = B.x - A.x;
	direction.y = B.y - A.y;
	if (direction.x == 0) { // vertical
		a = 1;
		b = 0;
		c = -A.x;
	}
	
	else if (direction.y == 0) { // horizontal
		a = 0;
		b = 1;
		c = -A.y;
	}

	else {
		// [y = kx + b] aka [kx - y + b = 0]
		a = direction.y / direction.x;
		b = -1;
		c = A.y - a * A.x;
		if (a < 0) {
			a = -a;
			b = -b;
			c = -c;
		}
	}
}

bool myline::is_left (xypair M) {

	if(a * M.x + b* M.y + c >= 0 && direction.y < 0) {
		return true;
	}
	else if(a * M.x + b* M.y + c <= 0 && direction.y > 0) {
		return true;
	}
	else return false;
}

class mypolygon {
public:
	mypolygon (xypair* vertexes, int num_sides);
	bool is_fill(xypair M);
	//~mypolygon();
private:
	myline* sides;
	int num;
};

mypolygon::mypolygon (xypair* vertexes, int num_sides) {
	num = num_sides;
	sides = new myline[num];
	for (int i = 0; i < num - 1; i++) {
		sides[i] = myline(vertexes[i], vertexes[i+1]);
	}
	sides[num - 1] = myline(vertexes[num-1], vertexes[0]);
}
/*
mypolygon::~mypolygon(){
	delete sides;
}
*/
bool mypolygon::is_fill(xypair M) {
	for (int i = 0; i < num; i++) {
		if (!sides[i].is_left(M)) {
			return false;
		}
	}
	return true;
}

class myfunc {
public: 
	myfunc(int a0, double b0): a(a0), b(b0){};
	bool is_fill(xypair M);
private:
	int a;
	double b; 
};

bool myfunc::is_fill (xypair M) {
	if (a*sin(b*M.x) + 150< M.y) return false;
	else return true;
}

class myextra {
public:
	myextra(xypair c0, double r0): center(c0), radius(r0){};
	RGB color_fill (xypair M);
//private:
	xypair center;
	double radius;
	double distance (xypair M);
	double angle (xypair M);
};

double myextra::distance (xypair M) {
	double d = sqrt(pow((M.x - center.x), 2.0) + pow((M.y - center.y), 2.0));
	return d;
}

double myextra::angle (xypair M) {
	return atan2((M.y - center.y), (M.x - center.x));
}

RGB myextra::color_fill (xypair M) {
	RGB color;
	double r = distance(M);
	if (r >= radius) {
		color = RGB(255,255,255);
		return color;
	}
	double theta = angle(M);
	if (theta <= PI && theta >  1/3.0 * PI) {
		color.green = 255;
		if (theta >= 2/3.0 * PI) { // [2/3pi,pi]
			color.blue = int((theta - 2/3.0*PI)/(1/3.0*PI)*255);
			//color.blue = 255;
	 		color.red = 0;
		}
		else if (theta == PI) {
			color.red = 0;
			color.blue = 0;
		}
		else { // [1/3pi, 2/3pi]
			color.red = int((2/3.0*PI - theta)/(1/3.0*PI)*255);
	 		//color.red = 255;
	 		color.blue = 0;
 		}
		/*
		if (r >= radius / 2.0) {
			double t = (radius - r) / r;
			color.red = int (t* color.red + (1-t) * 255);
			color.green = int (t* color.green + (1-t) * 255);
			color.blue = int (t * color.blue + (1-t) * 255);
		}
		*/	
	}
	if (theta <= 1/3.0 * PI && theta > -1/3.0 * PI) {
		color.red = 255;
		if (theta < 0) { // [0,1/3pi]
			color.blue = int((2/3.0*PI - theta) / (1/3.0*PI)*255);
			//color.blue = 255;
			color.green = 0;
			/*
			if (r >= radius / 2.0) {
				double t = (radius - r) / r;
				color.red = int (t* color.red + (1-t) * 255);
				color.green = int (t* color.green + (1-t) * 255);
				//color.blue = int (t * color.blue + (1-t) * 255);
			}
			*/
			
		}
		
		else if (theta == 0) {
			color.blue = 0;
			color.green = 0;
		}
		
		else { // [-1/3pi,0]
	 		color.green = int((theta + 1/3.0*PI) / (1/3.0*PI)*255);
			//color.green = 255;
			color.blue = 0;
			/*
			if (r >= radius / 2.0) {
				double t = (radius - r) / r;
				color.red = int (t* color.red + (1-t) * 255);
				// color.green = int (t* color.green + (1-t) * 255);
				color.blue = int (t * color.blue + (1-t) * 255);
			}
			*/
 		}

	}
	if (theta <= -1/3.0 * PI && theta >= -PI) {
		color.blue = 255;
		if (theta >= -2/3.0 * PI) { // [-2/3pi, -1/3pi]
	 		color.red = int((theta + 2/3.0*PI) / (1/3.0*PI)*255);
			//color.red = 255;
			color.green = 0;
		}
		else { // [-pi, -2/3pi]
	 		color.green = int((-2/3.0*PI - theta) / (1/3.0*PI)*255);
	 		//color.green = 255;
			color.red = 0;
 		}
 		/*
		if (r >= radius / 2.0) {
			double t = (radius - r) / r;
			color.red = int (t* color.red + (1-t) * 255);
			color.green = int (t* color.green + (1-t) * 255);
			color.blue = int (t * color.blue + (1-t) * 255);
		}
		*/
	}

	return color;
}

	
/*=========================================
class mypolygon {
public:
	mypolygon (xypair A, xypair B, xypair C, xypair D);
	bool is_fill(xypair M);
private: 
	myline AB;
	myline BC;
	myline CD;
	myline DA;
};

mypolygon::mypolygon(xypair A, xypair B, xypair C, xypair D): AB(A,B), BC(B,C), CD(C,D), DA(D,A){}

bool mypolygon::is_fill (xypair M) {
	if (AB.is_left(M) == true && BC.is_left(M) == true && CD.is_left(M) == true && DA.is_left(M) == true) return true;
	else return false;
}

//===================================================
class mypic {
public:
	mypic(xypair C0, int r0): center(C0), radius(r0){};
	RGB color_fill(xypair M);
private: 
	xypair center;
	int radius;
	double distance(xypair N);
};

RGB mypic::color_fill(xypair M) {
	RGB color(0,0,0);
		
		color()
	}
}

double mypic::distance(xypair N) {
	double d = sqrt(pow((M.x - center.x), 2.0) + pow((M.y - center.y), 2.0));
	return d;
}
*/
