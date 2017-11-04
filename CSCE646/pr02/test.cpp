#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

#include "shape.h"

using namespace std;

int testcase  = 3;

int main(){
	if (testcase == 1) { // test case #1
		xypair A(0,-1);
		xypair B(1,0);
		myline L1 = myline(A,B);
		cout << "L1 is "<< L1.a << "x + " << L1.b << "y + " << L1.c << " = 0" <<endl;
		cout << "Direction of L1 is " << "(" << L1.direction.x << "," << L1.direction.y << ")" <<endl;
		xypair M(0,0);
		cout<< L1.is_left(M)<<endl;
	}
	/*
	else if (testcase == 2) {
		xypair A(5,0);
		xypair B(0,5);
		xypair C(-5,0);
		xypair D(0,-5);
		xypair M;
		cout<<"Enter x"<<endl;
		cin>>M.x;
		cout<<"Enter y"<<endl;
		cin>>M.y;
		mypolygon Poly(A,B,C,D);
		if(Poly.is_fill(M)){
			cout<<"M is inside polygon"<<endl;
		}
		else cout<<"M is outside polygon"<<endl;
	}
	*/
	if (testcase == 3) {
		xypair trangle[3] = {xypair(0,0), xypair(5,5), xypair(0,10)};
		mypolygon T(trangle, 3);
		xypair M;
		while (1) {
			cout<<"Enter x"<<endl;
			cin>>M.x;
			cout<<"Enter y"<<endl;
			cin>>M.y;
			if (T.is_fill(M)) cout<<"M is in Trangle!"<<endl;
			else cout <<"M is outside Trangle!" <<endl;
		}
	}
	if (testcase == 4) {
		xypair C(150, 150);
		myextra (C, 150.0);
		xypair M;
		while (1) {
			cout<<"Enter x"<<endl;
			cin>>M.x;
			cout<<"Enter y"<<endl;
			cin>>M.y
	return 0;
}



