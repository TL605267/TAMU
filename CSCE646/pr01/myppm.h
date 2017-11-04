#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

using namespace std;

struct RGB {
	unsigned char red, green, blue;
};

class myppm {

public: 
	RGB *pixel_map;
	int read_ppm (void);
	void clean_up(ifstream &file);
	int width, height;
	void ppm_init(string name);
private: 
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

