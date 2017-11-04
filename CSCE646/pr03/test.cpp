#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <cmath>


using namespace std;

int testcase  = 4;

struct RGB {
	int red, green, blue;
};

struct manipulation_map {
	int r_map[256];
	int g_map[256];
	int b_map[256];
};

struct xypair {
	double x;
	double y;
	xypair(){};
	xypair(double x0, double y0): x(x0), y(y0) {}
};

void cubic_curve_single(int* color_map, xypair point[4]){
	for (int i = 0; i < 256; i++) {
		double x = (double)color_map[i];
		double result = 0;
		for (int j = 0; j < 4; j++) {
			double fraction = point[j].y;
			for (int k = 0; k < 4; k++) {
				if (k != j) {
					fraction = fraction* double(x/255.0 - point[k].x) / double(point[j].x - point[k].x);
				}
			}
			result += fraction;
		}
		color_map[i] = int(result * 255.0);
		cout << i <<" " << color_map[i] <<endl;
	}
}

manipulation_map RGB_map;
double* linear_slope;
xypair* dot;
void piecewise_linear(int color_map[256], int n, double * rn) {
	linear_slope = new double[n];
	dot = new xypair[n+1];
	
	for(int i = 0; i < n; i++) {
		linear_slope[i] = (rn[i+1] - rn[i]) * n; // k = (y1-y0)/(x1-x0) = (y1-y0)/(1/n) = (y1-y0) * n
		dot[i].y = rn[i];
		dot[i].x = i / double(n);
	}
	dot[n].y = rn[n];
	dot[n].x = 1.0;
	//分多段，每段分别赋值：
	int rgb_index = 0;
		for (int i = 0; i < n; i++) { // [0-255]
			double l_boundary = dot[i].x * 255;
			double r_boundary = dot[i+1].x * 255;
			while (rgb_index >= l_boundary && rgb_index < r_boundary) {
				color_map[rgb_index] = (int)((linear_slope[i] * (rgb_index / 255.0 - dot[i].x) + dot[i].y) * 255);
				cout << rgb_index <<" => " << color_map[rgb_index] <<endl;
				rgb_index++;
			}
		}
	color_map[rgb_index] = 255 * dot[n].y;
	cout << rgb_index <<" => " << color_map[rgb_index] <<endl;
}


int main(){
	if (testcase == 1) { // test case #1
		double arr[6] = {0, 0.5, 0.4, 0.6, 0.8, 0.5}; //0 0.2 0.4 0.6 0.8 1
		int n = 5;
		while(true) {
			int RGB = 0;

			cout<<"Enter RGB"<<endl;
			cin>>RGB;
			
			//cout<<"new RGB value  = " << piecewise_linear(RGB, n, arr) <<endl;
		}
	}
	if (testcase == 2) {
		char a = '3';
		int b = (a-'0')%48;
		cout<<"char to int: "<<b<<endl;
	}
	
	if (testcase == 3) {
		double arr[6] = {0, 0.5, 0.4, 0.6, 0.8, 0.5}; //0 0.2 0.4 0.6 0.8 1
		int n = 5;
		manipulation_map new_rgb_map;
		piecewise_linear(new_rgb_map.r_map, n, arr);
	}
	
	if (testcase == 4) {
		xypair point[4] = {xypair(0,0.5), xypair(0.25,0.25), xypair(0.75,0.75), xypair(1.0,0.5)};

		int map[256];
		for (int i = 0; i < 256; i++) {
			map[i] = i;
		}
		cubic_curve_single(map, point);
	}
		return 0;

}



