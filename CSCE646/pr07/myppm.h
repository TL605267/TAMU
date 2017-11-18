#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

#define MY_DEBUG 0

using namespace std;

struct RGB {
	unsigned char red, green, blue;
	RGB () {}
	RGB (unsigned char red0, unsigned char green0, unsigned char blue0) {
		red = red0;
		green = green0;
		blue = blue0;
	}		
	void operator=(const RGB& RGB2) {
		red = RGB2.red;
		green  = RGB2.green;
		blue = RGB2.blue;
	}
};

struct XY {
	double x, y;
	XY () {};
	XY (double x0, double y0) {
		x = x0;
		y = y0;
	}
	void operator=(const XY& XY2) {
		x = XY2.x;
		y = XY2.y;
	}
	void printxy() {
		cout <<"("<<x<<", "<<y<<")"<<endl;
	}
};

class myppm {

public: 
	RGB *pixel_map;
	RGB *map1;
	RGB *output_map;
	int read_ppm (void);
	void clean_up(ifstream &file);
	int width, height, size;
	void ppm_init(string name);
	void transformation(double *mtrx, int dimension, XY point, char* argv);
	RGB getpixel(XY point);
	RGB bilinear(XY point); //https://en.wikipedia.org/wiki/Bilinear_interpolation

	double det_2x2 (double *mtrx);
	double det_2x2 (double a, double b, double c, double d);
	double det_3x3 (double *mtrx);
	void inv_mtrx_3x3 (double *mtrx, double *inv_mtrx);
private: 
	string ppm_name;
	int  RGB_range;
	fstream file_in;
	XY center, translation;
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
	
	size = width * height;
	
	center = XY(width/2, height/2);
	
	translation = XY(0,0);
	
	pixel_map = new RGB[size];
	clean_up(file_in);
	
		//read bytes for pixel map (pixmap data)
	for(int i = height-1; i >= 0 ; i--) {
		for(int j = 0; j < width; j++) {
			int index = i * width + j;
			char colors[3];
			file_in.read(colors, sizeof colors);
			pixel_map[index] = RGB(colors[0], colors[1], colors[2]);
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

void myppm::transformation(double *mtrx, int dimension, XY point, char* argv) {

	if (dimension != 2 && dimension != 3) {
		cout <<"Invalid dimension!" <<endl;
		exit(-1);
	}
	else {
		translation = point;
		output_map = new RGB[width * height];
		RGB red = RGB(255,0,0);
		if (dimension == 2) { // Rotation, Scaling, Shear, Mirror
			double x, y;
			int index, new_index;
			//center = XY(1920,1080);
			for(int i = 0; i < width; i++) {
				for(int j = 0; j < height; j++) {
					index = j * width + i;
					//x = mtrx[0]*i + mtrx[1]*j; // [-1/2, 1/2]
					//y = mtrx[2]*i + mtrx[3]*j;
					x = mtrx[0]*(i - center.x) + mtrx[1]*(j - center.y) + center.x; // [-1/2, 1/2]
					y = mtrx[2]*(i - center.x) + mtrx[3]*(j - center.y) + center.y;
					//cout << "x = " << x << "y = " << y << endl;
					if (x >= 0 && x < width && y >= 0 && y < height) {
						new_index = int(y) * width + int(x);
						if (MY_DEBUG) {
							cout<<"Width ="<<width<<endl;
							cout << "("<<i<<", "<<j<<") => ("<<x<<", "<<y<<"); [index = "<< index<<"]"<<endl;
						}
						if (new_index < size && new_index >= 0) {
							output_map[new_index] = pixel_map[index];
						}
					}
				}
			}	
			// inverse mapping
			if (MY_DEBUG) cout << "===========Inverse==========="<<endl;
			double x_origin, y_origin, index_origin, tx, ty;
			double det = mtrx[0]*mtrx[3] - mtrx[1]*mtrx[2];
			if (MY_DEBUG) cout <<"det = "<<det<<endl;

			for(int i = 0; i < width; i++) {
				for(int j = 0; j < height; j++) {
					index = j * width + i;
					if (output_map[index].red == 0 && output_map[index].green == 0 && output_map[index].blue == 0) {
						//x_origin = i;
						//y_origin = j;
						//x_origin = (mtrx[3]*i - mtrx[1]*j)/det;
						//y_origin = (-mtrx[2]*i + mtrx[0]*j)/det;
						//cout << "a = " <<mtrx[3]<<"; b = "<<-mtrx[1]<<"; c = "<<-mtrx[2]<<"; d = "<<mtrx[0]<<endl;
						//cout <<"det = "<<det<<"; i = "<<i <<"; j = "<< j<< "| x_origin = "<< x_origin <<"; y_origin = " << y_origin << endl;
						//x_origin = i - center.x;
						//y_origin = j - center.y;

						x_origin = (mtrx[3]*(i - center.x) - mtrx[1]*(j - center.y))/det + center.x;
						y_origin = (-mtrx[2]*(i - center.x) + mtrx[0]*(j - center.y))/det + center.y;
						//tx = x_origin - floor(x_origin);
						//ty = y_origin - floor(y_origin);
						if (x_origin >= 0 && y_origin >= 0 && x_origin <= double(width) && y_origin <= double(height)) {
							index_origin = int(y_origin) * width + int(x_origin);
							if (index_origin >= 0 && index_origin < size) {
								//output_map[index] = red;
								if (MY_DEBUG) {
									cout<<"Width ="<<width<<endl;
									cout << "("<<i<<", "<<j<<") => ("<<x_origin<<", "<<y_origin<<"); [index_origin = "<< int(index_origin)<<"]"<<endl;
								}
								XY index_xy = XY(x_origin,y_origin);
								//output_map[index] = pixel_map[int(index_origin)];
								output_map[index] = bilinear(index_xy);
								if(output_map[index].red == 0 && output_map[index].green == 0 && output_map[index].blue == 0) {
									cout << "Black : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
								if(output_map[index].red == 255 && output_map[index].green == 0 && output_map[index].blue == 0) {
									cout << "Red : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
								if(output_map[index].red == 0 && output_map[index].green == 255 && output_map[index].blue == 0) {
									cout << "Green : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
								if(output_map[index].red == 0 && output_map[index].green == 255 && output_map[index].blue == 255) {
									cout << "Blue : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
							}
						}
					}
				}		
			}
			
			
		}
		if (dimension == 3) {

			double x, y, z;
			int index, new_index;

			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					index = j * width + i;

					x = mtrx[0]*i  + mtrx[1]*j + mtrx[2];
					y = mtrx[3]*i  + mtrx[4]*j + mtrx[5];
					z = mtrx[6]*i  + mtrx[7]*j + mtrx[8];					

					x = x/z;
					y = y/z;

					new_index = int(y) * width + int(x);
					if (new_index < width * height && x > 0 && y > 0 && y < height && x < width) {
						output_map[new_index] = pixel_map[index];
					}
				}
			}
			// inverse mapping
			
			if (MY_DEBUG) cout << "===========Inverse==========="<<endl;
			double x_origin, y_origin, z_origin, index_origin, tx, ty;
			double *inv_mtrx = new double[9];
			inv_mtrx_3x3(mtrx, inv_mtrx);

			for(int i = 0; i < width; i++) {
				for(int j = 0; j < height; j++) {
					index = j * width + i;
					if (output_map[index].red == 0 && output_map[index].green == 0 && output_map[index].blue == 0) {

						x_origin = inv_mtrx[0]*i  + inv_mtrx[1]*j + inv_mtrx[2];
						y_origin = inv_mtrx[3]*i  + inv_mtrx[4]*j + inv_mtrx[5];
						z_origin = inv_mtrx[6]*i  + inv_mtrx[7]*j + inv_mtrx[8];					

						x_origin = x_origin/z_origin;
						y_origin = y_origin/z_origin;



						if (x_origin >= 0 && y_origin >= 0 && x_origin <= double(width) && y_origin <= double(height)) {
							index_origin = int(y_origin) * width + int(x_origin);
							if (index_origin >= 0 && index_origin < size) {

								if (MY_DEBUG) {
									cout<<"Width ="<<width<<endl;
									cout << "("<<i<<", "<<j<<") => ("<<x_origin<<", "<<y_origin<<"); [index_origin = "<< int(index_origin)<<"]"<<endl;
								}
								XY index_xy = XY(x_origin,y_origin);
								//output_map[index] = red;
								//output_map[index] = pixel_map[int(index_origin)];
								output_map[index] = bilinear(index_xy);
								if(output_map[index].red == 0 && output_map[index].green == 0 && output_map[index].blue == 0) {
									cout << "Black : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
								if(output_map[index].red == 255 && output_map[index].green == 0 && output_map[index].blue == 0) {
									cout << "Red : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
								if(output_map[index].red == 0 && output_map[index].green == 255 && output_map[index].blue == 0) {
									cout << "Green : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
								if(output_map[index].red == 0 && output_map[index].green == 255 && output_map[index].blue == 255) {
									cout << "Blue : ["<< x_origin << ", "<< y_origin << "]"<<endl;
								}
							}
						}
					}
				}
			}			
		}		
	}
}

RGB myppm::getpixel(XY point) {
	int index = int(point.y) * width + int(point.x);
	if (index >= 0 && index <= width * height) {
		return pixel_map[index];
	}
	else {
		cout << width<<height<<endl;
		cout << "("<<point.x<<", "<<point.y<<")"<<endl;
		perror("Error: getpixel() is out of range!");
		exit(-1);
	}
}
	

RGB myppm::bilinear(XY point) { //https://en.wikipedia.org/wiki/Bilinear_interpolation
	double x, y, x1, x2, y1, y2;
	XY Q11, Q12, Q21, Q22;
	RGB xy;
	
	x = point.x;
	y = point.y;

	x1 = floor(x);
	x2 = x1 + 1;
	y1 = floor(y);
	y2 = y1 + 1;

	Q11 = XY(x1, y1);
	Q12 = XY(x1, y2);
	Q21 = XY(x2, y1);
	Q22 = XY(x2, y2);

	if (x2 < width && y2 < height) {
		//cout << "x = "<<x<<", y = "<<y<<", x1 = "<<x1<<", x2 = "<<x2<<", y1 = "<<y1<<", y2 = "<<y2<<endl;
		xy.red = (y2-y)/(y2-y1) * (getpixel(Q11).red * (x2-x)/(x2-x1) + getpixel(Q21).red * (x-x1)/(x2-x1)) + (y-y1)/(y2-y1) * (getpixel(Q12).red * (x2-x)/(x2-x1) + getpixel(Q22).red * (x-x1)/(x2-x1)) + 0.5;
		xy.green = (y2-y)/(y2-y1) * (getpixel(Q11).green * (x2-x)/(x2-x1) + getpixel(Q21).green * (x-x1)/(x2-x1)) + (y-y1)/(y2-y1) * (getpixel(Q12).green * (x2-x)/(x2-x1) + getpixel(Q22).green * (x-x1)/(x2-x1)) + 0.5;
		xy.blue = (y2-y)/(y2-y1) * (getpixel(Q11).blue * (x2-x)/(x2-x1) + getpixel(Q21).blue * (x-x1)/(x2-x1)) + (y-y1)/(y2-y1) * (getpixel(Q12).blue * (x2-x)/(x2-x1) + getpixel(Q22).blue * (x-x1)/(x2-x1)) + 0.5;
		return xy;
	}
	
	else if (x2 == width && y2 < height) {
		xy.red = (y2-y)/(y2-y1) * getpixel(Q11).red + (y-y1)/(y2-y1) * getpixel(Q12).red + 0.5;
		xy.green = (y2-y)/(y2-y1) * getpixel(Q11).green +  (y-y1)/(y2-y1) * getpixel(Q12).green + 0.5;
		xy.blue = (y2-y)/(y2-y1) * getpixel(Q11).blue + (y-y1)/(y2-y1) * getpixel(Q12).blue + 0.5;
	}
	
	else if (x2 < width && y2 == width) {
		xy.red = getpixel(Q11).red * (x2-x)/(x2-x1) + getpixel(Q21).red * (x-x1)/(x2-x1) + 0.5;
		xy.green = getpixel(Q11).green * (x2-x)/(x2-x1) + getpixel(Q21).green * (x-x1)/(x2-x1) + 0.5;
		xy.blue = getpixel(Q11).blue * (x2-x)/(x2-x1) + getpixel(Q21).blue * (x-x1)/(x2-x1) + 0.5;
	}
	else {
		xy = output_map[width * width - 1];
	} 
}

double myppm::det_2x2 (double *mtrx) {
	return mtrx[0]*mtrx[3] - mtrx[1]*mtrx[2];
}

double myppm::det_2x2 (double a, double b, double c, double d) {
	return a*d - b*c;
}

double myppm::det_3x3 (double *mtrx) {
	return mtrx[0]*(mtrx[4]*mtrx[8] - mtrx[5]*mtrx[7]) - mtrx[1]*(mtrx[3]*mtrx[8] - mtrx[5]*mtrx[6]) + mtrx[2]*(mtrx[3]*mtrx[7] - mtrx[4]*mtrx[6]);
}

void myppm::inv_mtrx_3x3 (double *mtrx, double *inv_mtrx) { //http://mathworld.wolfram.com/MatrixInverse.html
	double a11 = mtrx[0];
	double a12 = mtrx[1];
	double a13 = mtrx[2];
	double a21 = mtrx[3];
	double a22 = mtrx[4];
	double a23 = mtrx[5];
	double a31 = mtrx[6];
	double a32 = mtrx[7];
	double a33 = mtrx[8];
	
	double det = det_3x3(mtrx);
	inv_mtrx[0] = det_2x2(a22,a23,a32,a33) / det;
	inv_mtrx[1] = det_2x2(a13,a12,a33,a32) / det;
	inv_mtrx[2] = det_2x2(a12,a13,a22,a23) / det;
	inv_mtrx[3] = det_2x2(a23,a21,a33,a31) / det;
	inv_mtrx[4] = det_2x2(a11,a13,a31,a33) / det;
	inv_mtrx[5] = det_2x2(a13,a11,a23,a21) / det;
	inv_mtrx[6] = det_2x2(a21,a22,a31,a32) / det;
	inv_mtrx[7] = det_2x2(a12,a11,a32,a31) / det;
	inv_mtrx[8] = det_2x2(a11,a12,a21,a22) / det;
}
