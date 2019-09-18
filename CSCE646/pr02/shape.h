#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <algorithm>    // std::max
    
#define PI 3.14159265

struct XY {
	double x;
	double y;
	XY(){};
	XY(double x0, double y0): x(x0), y(y0) {};
	void operator=(const XY &xy0){
		this->x = xy0.x;
		this->y = xy0.y;
	}
};

struct RT {
	double rho;
	double theta;
};

class my_poly {
public:
	my_poly();
	my_poly(XY c0) { center = c0;}
	my_poly(XY* v0, int n0, XY c0) {
		v_rt = new RT[n0];
		vertex = v0;
		v_num = n0;
		center = c0;
		for (int i = 0; i < n0; i++) {
			v_rt[i] = get_polar(v0[i]);
		}
	}
	double get_a(XY p1, XY p2); // ax+by-1 = 0
	double get_b(XY p1, XY p2); // ax+by-1 = 0
	RT get_polar(XY p);
	bool is_fill(XY p);
private:
	XY* vertex;
	RT* v_rt;
	XY center;
	int v_num;
};

double my_poly::get_a(XY p1, XY p2) {
	double det = p1.x*p2.y-p1.y*p2.x;
	if (det == 0) {
		printf ("Line pass center!");
		exit(-1);
	}
	else return (p2.y - p1.y)/det;
}

double my_poly::get_b(XY p1, XY p2) {
	double det = p1.x*p2.y-p1.y*p2.x;
	if (det == 0) {
		printf ("Line pass center!");
		exit(-1);
	}
	else return (p1.x - p2.x)/det;
}

RT my_poly::get_polar (XY p) {
	double px = p.x - center.x;
	double py = p.y - center.y;

	RT p_rt;
	p_rt.rho = sqrt(px*px + py*py); // distance to center
	p_rt.theta = atan(py/px); // get angle from arctangent
	if (px < 0 && py > 0) { // II || III
		// II: -pi/4 => 3pi/4
		// III: pi/4 => 5pi/4
		p_rt.theta += PI; 
	}
	else if (px > 0 && py < 0) { // IV
		//p_rt.phi -pi/4 => 7pi/4
		p_rt.theta += 2 * PI;
	}
	printf("p(%f, %f) => (%f, %f)is at %f degrees.\n", p.x, p.y, px, py, p_rt.theta * 180 / PI); // degree is easier to check
	return p_rt;
}

bool my_poly::is_fill(XY p) {
	RT p_rt = get_polar(p);	
	for (int i = 0; i < v_num; i++) {
		RT p1 = v_rt[i%v_num];
		RT p2 = v_rt[(i+1)%v_num];
		XY p1_xy = vertex[i%v_num];
		XY p2_xy = vertex[(i+1)%v_num];

		if (p_rt.theta >= p1.theta && p_rt.theta < p2.theta) { // is theta is in the range
			double a = get_a(p1_xy, p2_xy);
			double b = get_b(p1_xy, p2_xy);
			// my formula
			double rho_center = abs(-1 / (a*cos(p_rt.theta) + b*sin(p_rt.theta)));
			if (p_rt.rho > rho_center) return false;
			else return true;
		}
	}
}
