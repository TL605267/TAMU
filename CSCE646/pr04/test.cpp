#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <cmath>


using namespace std;

int testcase  = 1;
int width = 5;
int height = 5;
struct RGB {
	unsigned char red, green, blue;
};

struct xypair {
	double x;
	double y;
	xypair(){};
	xypair(double x0, double y0): x(x0), y(y0) {}
};

void conv_single_pixel(int* kernel, int size, int x, int y, unsigned char* pixel_map, unsigned char* new_map) {
	int accumulator_red = 0;
	int accumulator_green = 0;
	int accumulator_blue = 0;
	int conv_range = (size-1)/2; // size must be odd number
	/*
	if (x < conv_range || y < conv_range) {
		cout << "Error: convolution reach the edge!" << endl;
	}*/
	int x0 = x - conv_range;
	int y0 = y - conv_range;
	for (int yk = 0; yk < size; yk++) {
		for (int xk = 0; xk < size; xk++) {
			//cout << pixel_map[(x - conv_range + xk) * width + (yk - conv_range + yk)] << " ";

			accumulator_red += int(pixel_map[(x0 + xk) * width + (y0 + yk)]) * kernel[xk * size + yk];

		}
	}
	accumulator_red = int (accumulator_red / ((double) (size * size) ));
	//cout << " accumulator_red = " << accumulator_red << " " ;
	new_map[x * width + y] = accumulator_red;
	
}

int main(){
	if (testcase == 1) { // test case #1
		int kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
		unsigned char map[25] = {'0', '0', '0', '0', '0', '0', '9', '9', '9', '0', '0', '9', '9', '9', '0', '0', '9', '9', '9', '0', '0', '0', '0', '0', '0'};
		unsigned char new_map[25] = {'0', '0', '0', '0', '0', '0', '9', '9', '9', '0', '0', '9', '9', '9', '0', '0', '9', '9', '9', '0', '0', '0', '0', '0', '0'};
		cout << "Before: " << endl;
		for (int i = 0; i < 25; i++) {
			cout << map[i] << "  ";
			if (i == 4 || i == 9 || i == 14 || i == 19 || i == 24) {
				cout << endl;
			}
		}
		cout << "After: " << endl;
		for (int i = 1; i < 4; i++) {
			for (int j = 1; j < 4; j++) {
				conv_single_pixel (kernel, 3, i, j, map, new_map);
			}
			cout << endl;
		}
		
		for (int i = 0; i < 25; i++) {
			cout << new_map[i] << "  ";
			if (i == 4 || i == 9 || i == 14 || i == 19 || i == 24) {
				cout << endl;
			}
		}
		
		
	}

	return 0;

}



