#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <math.h>

using namespace std;

int main() {
	double num = 3.1415926;
	unsigned char char2 = 10;

	char2 = num;
	num = char2;
	cout <<num << endl;
}
