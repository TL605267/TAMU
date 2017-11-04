#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

using namespace std;

struct RGB {
	unsigned char red, green, blue;
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
class myppm {

public: 
	RGB *pixel_map;
	int read_ppm (void);
	void clean_up(ifstream &file);
	int width, height;
	manipulation_map RGB_map;
	void ppm_init(string name);
	void piecewise_linear_single(int* color_map, int n, double * rn); // so far, single channel only
	void piecewise_linear_RGB(int n1, double* rn1, int n2, double* rn2, int n3, double* rn3);
	void cubic_curve_single(int* color_map, xypair point[4]); // single channel cubic curve
	void cubic_curve_RGB(xypair R_point[4], xypair G_point[4], xypair B_point[4]);
	void color_manipulation (void);

private: 
	double* linear_slope;
	xypair* dot;
	string ppm_name;
	int  RGB_range;
	fstream file_in;
		
};



void myppm::ppm_init(string name) {
	ppm_name = name;
	ifstream file_in(name, ifstream::binary);

	//clean_up(file_in);
	
	string current_line;
	getline(file_in, current_line);

	
	if (current_line != "P6") {
		cout<<"Invalid image file!"<<endl;
		file_in.close();
		exit(-1);
	}
	
	clean_up(file_in);
	
	getline(file_in, current_line);
	istringstream(current_line) >> width >> height;
	
	clean_up(file_in);
	
	getline(file_in, current_line);

	RGB_range = stoi(current_line);
	
	pixel_map = new RGB[width * height];
	
	clean_up(file_in);
	
	//read bytes for pixel map (pixmap data)
	for(int i = height-1; i >= 0 ; i--) {
		for(int j = 0; j < width; j++) {
			int index = i * width + j;
			char colors[3];
			file_in.read(colors, sizeof colors);
			pixel_map[index].red = colors[0];
			pixel_map[index].green = colors[1];
			pixel_map[index].blue = colors[2];
		}
	}

	file_in.close();

}


void myppm::clean_up(ifstream &file) {
	string dumpster;
	while(file.peek() == '#') {
		getline(file, dumpster);
	}
}

void myppm::piecewise_linear_single(int* color_map, int n, double * rn) {
	linear_slope = new double[n];
	dot = new xypair[n+1];
	
	for(int i = 0; i < n; i++) {
		linear_slope[i] = (rn[i+1] - rn[i]) * n; // k = (y1-y0)/(x1-x0) = (y1-y0)/(1/n) = (y1-y0) * n
		dot[i].y = rn[i];
		dot[i].x = i / double(n);
	}
	dot[n].y = rn[n];
	dot[n].x = 1.0;

	int rgb_index = 0;
		for (int i = 0; i < n; i++) { // [0-255]
			double l_boundary = dot[i].x * 255;
			double r_boundary = dot[i+1].x * 255;
			while (rgb_index >= l_boundary && rgb_index < r_boundary) {
				color_map[rgb_index] = (int)((linear_slope[i] * (rgb_index / 255.0 - dot[i].x) + dot[i].y) * 255);
				//cout << rgb_index <<" => " << color_map[rgb_index] <<endl;
				rgb_index++;
			}
		}
	color_map[rgb_index] = 255 * dot[n].y;
	//cout << rgb_index <<" => " << color_map[rgb_index] <<endl;
	return;
}

void myppm::piecewise_linear_RGB(int n1, double* rn1, int n2, double* rn2, int n3, double* rn3) {
	piecewise_linear_single(RGB_map.r_map, n1, rn1);
	piecewise_linear_single(RGB_map.g_map, n2, rn3);
	piecewise_linear_single(RGB_map.b_map, n3, rn3);
	return;
}

void myppm::cubic_curve_single(int* color_map, xypair point[4]){
	for (int i = 0; i < 256; i++) {
		double result = 0;
		for (int j = 0; j < 4; j++) {
			double fraction = point[j].y;
			for (int k = 0; k < 4; k++) {
				if (k != j) {
					fraction = fraction* double(i/255.0 - point[k].x) / double(point[j].x - point[k].x);
				}
			}
			result += fraction;
		}
		color_map[i] = int(result * 255.0);
		//cout << i <<" " << color_map[i] <<endl;
	}
}

void myppm::cubic_curve_RGB(xypair R_point[4], xypair G_point[4], xypair B_point[4]){
	cubic_curve_single(RGB_map.r_map, R_point);
	cubic_curve_single(RGB_map.g_map, G_point);
	cubic_curve_single(RGB_map.b_map, B_point);
}


void myppm::color_manipulation (void){
	for(int y = 0; y < height ; y++) {
		for(int x = 0; x < width; x++) {
		int i = y * width + x; 
		pixel_map[i].red = RGB_map.r_map[pixel_map[i].red];
		pixel_map[i].green = RGB_map.g_map[pixel_map[i].green];
		pixel_map[i].blue = RGB_map.b_map[pixel_map[i].blue];
		}
	}
	return;
}
