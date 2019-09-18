#include <cstdlib>
#include <iostream>
using namespace std;

double det_3x3 (double *mtrx) {
	return mtrx[0]*(mtrx[4]*mtrx[8] - mtrx[5]*mtrx[7]) - mtrx[1]*(mtrx[3]*mtrx[8] - mtrx[5]*mtrx[6]) + mtrx[2]*(mtrx[3]*mtrx[7] - mtrx[4]*mtrx[6]);
}

double det_2x2 (double a, double b, double c, double d) {
	return a*d - b*c;
}
void inv_mtrx_3x3 (double *mtrx, double *inv_mtrx) { //http://mathworld.wolfram.com/MatrixInverse.html
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


int main() {
	double mtrx[9];
	double inv[9];
	mtrx[0] = 1;
	mtrx[1] = 0;
	mtrx[2] = 5;
	mtrx[3] = 1;
	mtrx[4] = 10;
	mtrx[5] = 5;
	mtrx[6] = 0;
	mtrx[7] = 0.1;
	mtrx[8] = 1;
	if (det_3x3(mtrx) == 0) {
		perror("Invalid matrix for inverse!");
		exit(-1);
	}
	inv_mtrx_3x3(mtrx,inv);
	cout << inv[0] << " " <<inv[1] << " " <<inv[2] <<endl;
	cout << inv[3] << " " <<inv[4] << " " <<inv[5] <<endl;
	cout << inv[6] << " " <<inv[7] << " " <<inv[8] <<endl;
	return 0;
}
