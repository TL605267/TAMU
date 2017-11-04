// std::real example
#include <iostream>     // std::cout
#include <complex>      // std::complex, std::real
#include <cmath>

using namespace std;
double w (double x){return (3 *sqrt(1 - (x/7.)*(x/7.)));}
  double l (double x)
    {return (6./7. *sqrt(10.) + (3 + x)/2. - 3./7.* sqrt(10.)* sqrt(4 - (x + 1)*(x+1)));}
  std::complex<double> h (double x)
    {return (3*(abs(x - 1./2) + abs(x + 1./2) + 6) -11*(abs(x + 0.75) + abs(x - 0.75)))/2.0;}
  double r (double x)
    {return (6./7 *sqrt(10.) + (3 - x)/2. - 3./7 *sqrt(10.) *sqrt(4 - (x - 1)*(x-1)));}
  double H (double x)
    {if(x<0) return 0; else return 1;}
  double sign(double x)
    {if (x>0) return 1; else if(x<0) return -1;else return 0;}
  double f (double x) {
      std::complex<double> a;
      a =  (w(x) + (l(x) - w(x)) *H(x + 3) + (h(x) - l(x))* H(x + 1) +(r(x) - h(x)) *H(x - 1) + (w(x) - r(x)) *H(x - 3));
      double b = std::real(a);
      return b;
  }
  double g (double x) {
      double b;
      b = 	1./2*(sign(x + 4) - sign(x - 4))*(3 *sqrt(1 - (x/7.)*(x/7.)) + sqrt(1 - (abs(abs(x) - 2) - 1)*(abs(abs(x) - 2) - 1)) + abs(x/2.) -(3 *sqrt(33.) - 7)/112.* x*x - 3) 
    - 3*sqrt(1 - (x/7.)*(x/7.));
      return b;
  }

	double z (double x) {
		x = sqrt(1 - (abs(abs(x) - 2) - 1)*(abs(abs(x) - 2) - 1));
		return x;
		}
int main ()
{
	for (double x = -7.0; x < 7.1; x+=0.1) {
		cout << z(x) << ", " << x<<endl;
	}

  return 0;
}
