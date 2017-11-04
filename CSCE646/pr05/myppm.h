#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <algorithm>    // std::max

using namespace std;
int i = 0;
#define Convolution 1
#define Derivative 2

struct RGB {
	unsigned char red, green, blue;
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
	RGB *new_map;
	double* hue_map;
	int kernel_size;
	int read_ppm (void);
	void clean_up(ifstream &file);
	int width, height;
	void ppm_init(string name);
	void ppm_init_hsv(string name);
	void conv_single_pixel(double* kernel, int size, double factor, int x, int y);
	void conv_blur(int size, double factor, int bias0, double* hue_map0);
	void morphological_single_pixel(double* kernel, int size, int x, int y);
	void dilation(int size, double* hue_map0);
	double get_hue(int r, int g, int b);
private: 
	int bias;
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
	new_map = new RGB[width * height];
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
			new_map[index].red = colors[0];
			new_map[index].green = colors[1];
			new_map[index].blue = colors[2];
		}
	}

	file_in.close();
}

void myppm::ppm_init_hsv(string name) {
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
	
	hue_map = new double[width * height];
	clean_up(file_in);
	
	//read bytes for pixel map (pixmap data)
	for(int i = height-1; i >= 0 ; i--) {
		for(int j = 0; j < width; j++) {
			int index = i * width + j;
			char colors[3];
			file_in.read(colors, sizeof colors);
			hue_map[index] = get_hue(colors[0], colors[1], colors[2]);
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

void myppm::conv_single_pixel(double* kernel, int size, double factor, int x, int y) { // unsigned char* pixel_map, unsigned char* new_map) {
	int accumulator_red = 0;
	int accumulator_green = 0;
	int accumulator_blue = 0;
	int conv_range = (size-1)/2; // size must be odd number
	//i++;
	//cout << i <<endl;
	int x0 = x - conv_range;
	int y0 = y - conv_range;
	for (int yk = 0; yk < size; yk++) {
		for (int xk = 0; xk < size; xk++) {
			accumulator_red += int(pixel_map[(x0 + xk) * width + (y0 + yk)].red) * kernel[xk * size + yk];
			accumulator_green += int(pixel_map[(x0 + xk) * width + (y0 + yk)].green) * kernel[xk * size + yk];
			accumulator_blue += int(pixel_map[(x0 + xk) * width + (y0 + yk)].blue) * kernel[xk * size + yk];

		}
	}
	accumulator_red = remainder((accumulator_red * factor + bias), 255);
	accumulator_green = remainder((accumulator_green * factor + bias), 255);
	accumulator_blue = remainder((accumulator_blue * factor + bias), 255);	
	/*
	if (accumulator_red < 0) { 
		cout << "Negative["<< x <<"," << y<<"]: " << accumulator_red << endl;
	}
	if (accumulator_green < 0) { 
		cout << "Negative["<< x <<"," << y<<"]: " << accumulator_green<< endl;
}	if (accumulator_blue < 0) { 
		cout << "Negative["<< x <<"," << y<<"]: " << accumulator_blue << endl;
	}*/
	//cout << " accumulator_red = " << accumulator_red << " " ;
	new_map[x * width + y].red = accumulator_red;
	new_map[x * width + y].green = accumulator_green;
	new_map[x * width + y].blue = accumulator_blue;
	
}
			


void myppm::conv_blur(int size, double factor, int bias0, double* hue_map0) {

	double kernel0[81] = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0
	};
	double kernel1[81] = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,2,
		0,0,0,0,0,0,2,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,2,0,0,0,0,0,0,
		2,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0
	};
	double kernel2[81] = {
		0,0,0,0,0,0,0,0,1,
		0,0,0,0,0,0,0,1,0,
		0,0,0,0,0,0,1,0,0,
		0,0,0,0,0,1,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,1,0,0,0,0,0,
		0,0,1,0,0,0,0,0,0,
		0,1,0,0,0,0,0,0,0,
		1,0,0,0,0,0,0,0,0
	};
	double kernel3[81] = {
		0,0,0,0,0,0,2,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,2,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,2,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,2,0,0,0,0,0,0
	};
	double kernel4[81] = {
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0
	};
	double kernel5[81] = {
		0,0,2,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,2,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,2,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,2,0,0
	};
	double kernel6[81] = {
		1,0,0,0,0,0,0,0,0,
		0,1,0,0,0,0,0,0,0,
		0,0,1,0,0,0,0,0,0,
		0,0,0,1,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,1,0,0,0,
		0,0,0,0,0,0,1,0,0,
		0,0,0,0,0,0,0,1,0,
		0,0,0,0,0,0,0,0,1
	};
	double kernel7[81] = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		2,0,0,0,0,0,0,0,0,
		0,0,2,0,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,0,2,0,0,
		0,0,0,0,0,0,0,0,2,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0
	};

	bias = bias0;
	int conv_range = (size-1)/2 + 3; // size must be odd number
	//cout <<"Upper range " << (height - conv_range) <<endl;

	for (int y = conv_range; y < height - conv_range; y++) {
		for (int x = conv_range; x < width - conv_range; x++) {
			//cout << x << ", " << y << endl;
			//cout << "x = "<<x<<"; y = "<<y<<endl;
			//cout << hue_map0[x * width + y] <<endl;
			if (hue_map0[y * width + x] < 22.5) conv_single_pixel (kernel0, size, factor, y, x);
			else if (hue_map0[y * width + x] < 45) conv_single_pixel (kernel1, size, factor, y, x);
			else if (hue_map0[y * width + x] < 67.5) conv_single_pixel (kernel2, size, factor, y, x);
			else if (hue_map0[y * width + x] < 90) conv_single_pixel (kernel3, size, factor, y, x);
			else if (hue_map0[y * width + x] < 112.5) conv_single_pixel (kernel4, size, factor, y, x);
			else if (hue_map0[y * width + x] < 135) conv_single_pixel (kernel5, size, factor, y, x);
			else if (hue_map0[y * width + x] < 157.5) conv_single_pixel (kernel6, size, factor, y, x);
			else if (hue_map0[y * width + x] < 180) conv_single_pixel (kernel7, size, factor, y, x);
			else if (hue_map0[y * width + x] < 202.5) conv_single_pixel (kernel0, size, factor, y, x);
			else if (hue_map0[y * width + x] < 225) conv_single_pixel (kernel1, size, factor, y, x);
			else if (hue_map0[y * width + x] < 247.5) conv_single_pixel (kernel2, size, factor, y, x);
			else if (hue_map0[y * width + x] < 270) conv_single_pixel (kernel3, size, factor, y, x);
			else if (hue_map0[y * width + x] < 292.5) conv_single_pixel (kernel4, size, factor, y, x);
			else if (hue_map0[y * width + x] < 315) conv_single_pixel (kernel5, size, factor, y, x);
			else if (hue_map0[y * width + x] < 337.5) conv_single_pixel (kernel6, size, factor, y, x);
			else  conv_single_pixel (kernel7, size, factor, y, x);

		}
	}
}


void myppm::morphological_single_pixel(double* kernel, int size, int x, int y) { // unsigned char* pixel_map, unsigned char* new_map) {
	int accumulator_red = 255;
	int accumulator_green = 255;
	int accumulator_blue = 255;
	int conv_range = (size-1)/2; // size must be odd number
	//i++;
	//cout << i <<endl;
	int x0 = x - conv_range;
	int y0 = y - conv_range;
	for (int yk = 0; yk < size; yk++) {
		for (int xk = 0; xk < size; xk++) {
			if (kernel[xk * size + yk] != 0) {
				accumulator_red = min(int(pixel_map[(x0 + xk) * width + (y0 + yk)].red), accumulator_red);
				accumulator_green = min(int(pixel_map[(x0 + xk) * width + (y0 + yk)].green), accumulator_green);
				accumulator_blue = min(int(pixel_map[(x0 + xk) * width + (y0 + yk)].blue), accumulator_blue);
			}
		}
	}
	/*
	if (accumulator_red < 0) { 
		cout << "Negative["<< x <<"," << y<<"]: " << accumulator_red << endl;
	}
	if (accumulator_green < 0) { 
		cout << "Negative["<< x <<"," << y<<"]: " << accumulator_green<< endl;
}	if (accumulator_blue < 0) { 
		cout << "Negative["<< x <<"," << y<<"]: " << accumulator_blue << endl;
	}*/
	//cout << " accumulator_red = " << accumulator_red << " " ;

	new_map[x * width + y].red = accumulator_red;
	new_map[x * width + y].green = accumulator_green;
	new_map[x * width + y].blue = accumulator_blue;
	
}
			


void myppm::dilation(int size, double* hue_map0) {
	
	double kernel0[81] = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0
	};
	double kernel1[81] = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,2,
		0,0,0,0,0,0,2,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,2,0,0,0,0,0,0,
		2,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0
	};
	double kernel2[81] = {
		0,0,0,0,0,0,0,0,1,
		0,0,0,0,0,0,0,1,0,
		0,0,0,0,0,0,1,0,0,
		0,0,0,0,0,1,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,1,0,0,0,0,0,
		0,0,1,0,0,0,0,0,0,
		0,1,0,0,0,0,0,0,0,
		1,0,0,0,0,0,0,0,0
	};
	double kernel3[81] = {
		0,0,0,0,0,0,2,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,2,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,2,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,2,0,0,0,0,0,0
	};
	double kernel4[81] = {
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,0
	};
	double kernel5[81] = {
		0,0,2,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,2,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,2,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,2,0,0
	};
	double kernel6[81] = {
		1,0,0,0,0,0,0,0,0,
		0,1,0,0,0,0,0,0,0,
		0,0,1,0,0,0,0,0,0,
		0,0,0,1,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,1,0,0,0,
		0,0,0,0,0,0,1,0,0,
		0,0,0,0,0,0,0,1,0,
		0,0,0,0,0,0,0,0,1
	};
	double kernel7[81] = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		2,0,0,0,0,0,0,0,0,
		0,0,2,0,0,0,0,0,0,
		0,0,0,0,1,0,0,0,0,
		0,0,0,0,0,0,2,0,0,
		0,0,0,0,0,0,0,0,2,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0
	};

	int conv_range = (size-1)/2; // size must be odd number
	//cout <<"Upper range " << (height - conv_range) <<endl;
	for (int y = conv_range; y < height - conv_range; y++) {
		for (int x = conv_range; x < width - conv_range; x++) {
			//cout << x << ", " << y << endl;
			if (hue_map0[y * width + x] < 22.5) morphological_single_pixel (kernel0, size, y, x);
			else if (hue_map0[y * width + x] < 45) morphological_single_pixel (kernel1, size, y, x);
			else if (hue_map0[y * width + x] < 67.5) morphological_single_pixel (kernel2, size, y, x);
			else if (hue_map0[y * width + x] < 90) morphological_single_pixel (kernel3, size, y, x);
			else if (hue_map0[y * width + x] < 112.5) morphological_single_pixel (kernel4, size, y, x);
			else if (hue_map0[y * width + x] < 135) morphological_single_pixel (kernel5, size, y, x);
			else if (hue_map0[y * width + x] < 157.5) morphological_single_pixel (kernel6, size, y, x);
			else if (hue_map0[y * width + x] < 180) morphological_single_pixel (kernel7, size, y, x);
			else if (hue_map0[y * width + x] < 202.5) morphological_single_pixel (kernel0, size, y, x);
			else if (hue_map0[y * width + x] < 225) morphological_single_pixel (kernel1, size, y, x);
			else if (hue_map0[y * width + x] < 247.5) morphological_single_pixel (kernel2, size, y, x);
			else if (hue_map0[y * width + x] < 270) morphological_single_pixel (kernel3, size, y, x);
			else if (hue_map0[y * width + x] < 292.5) morphological_single_pixel (kernel4, size, y, x);
			else if (hue_map0[y * width + x] < 315) morphological_single_pixel (kernel5, size, y, x);
			else if (hue_map0[y * width + x] < 337.5) morphological_single_pixel (kernel6, size, y, x);
			else morphological_single_pixel (kernel7, size, y, x);
		}
	}
}


double myppm::get_hue(int r, int g, int b) {

	double RED, GREEN, BLUE;
	double MAX, MIN, delta;
	double h, s, v;
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
	//cout <<"Hue = "<<h<<endl;
	return h;
}
