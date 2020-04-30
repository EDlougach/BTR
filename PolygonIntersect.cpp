#include <fstream>
#include <complex>
#include <algorithm>
using namespace std;

ifstream cin("input.txt");
ofstream cout("output.txt");

typedef complex<double> Point;
typedef Point *Polygon;
struct Line {
	Point p1, p2;
};

double crossmul(Point v1, Point v2) // Vector multiplication (x1*y2-x2*y1)
{
	return imag(conj(v1)*v2);
}

const double EPS = 1e-16;

bool intersect(Line l1, Line l2, Point &res) // Finds intersection point of two line segments
{
	// Checking if those segments cross
	if (crossmul(l1.p2-l1.p1, l2.p1-l1.p1) * crossmul(l1.p2-l1.p1, l2.p2-l1.p1) > -EPS) return false;
	if (crossmul(l2.p2-l2.p1, l1.p1-l2.p1) * crossmul(l2.p2-l2.p1, l1.p2-l2.p1) > -EPS) return false;
	// Using Krahmer rule of solving system of equations
	double A1, A2, B1, B2, C1, C2;
	A1 = imag(l1.p2-l1.p1);
	B1 = real(l1.p1-l1.p2);
	C1 = A1*real(l1.p1)+B1*imag(l1.p1);
	A2 = imag(l2.p2-l2.p1);
	B2 = real(l2.p1-l2.p2);
	C2 = A2*real(l2.p1)+B2*imag(l2.p1);
	double D, Dx, Dy;
	D = A1*B2-A2*B1;
	Dx = C1*B2-C2*B1;
	Dy = A1*C2-A2*C1;
	// returning result
	res = Point(Dx/D, Dy/D);
	return true;
}

bool inside(Polygon p, int n, Point v) // Checking if the point is inside (or on the border of) the polygon
{
	int l, r;
	l = 0; r = n;
	v-=p[0];
	if ((crossmul(v, p[1]-p[0])>EPS)||(crossmul(v, p[n-1]-p[0])<-EPS)) return false;
	// binary search
	while (r-l!=1)
	{
		int m = (r+l)/2;
		double val = crossmul(v, p[m]-p[0]);
		if (val>0)
		{
			r = m;
		} else
		{
			l = m;
		}
	}
	v+=p[0]-p[l];
	return (crossmul(v, p[r]-p[l])<EPS);
}

// special comparison routine for sorting resulting polygon
Point center;
bool operator<(Point p1, Point p2)
{
	p1-=center;
	p2-=center;
	return (arg(p2)-arg(p1)>EPS);
}

// main routine
void intersect(Polygon p1, int n1, Polygon p2, int n2, Polygon res, int &nres)
{
	nres = 0;
	// First of all we put to the result points of first polygon, which are inside the second one
	for (int i = 0; i < n1; i++)
	{
		if (inside(p2, n2, p1[i]))
		{
			res[nres++]=p1[i];
		}
	}
	// the same for points of second polygon (inside first)
	for (int i = 0; i < n2; i++)
	{
		if (inside(p1, n1, p2[i]))
		{
			res[nres++]=p2[i];
		}
	}
	// Adding lines' segments intersections
	for (int i = 0; i < n1; i++)
	{
		for (int j = 0; j < n2; j++)
		{
			Line l1, l2;
			l1.p1 = p1[i];
			l1.p2 = p1[(i==n1-1)?0:(i+1)];
			l2.p1 = p2[j];
			l2.p2 = p2[(j==n2-1)?0:(j+1)];
			Point pt;
			if (intersect(l1, l2, pt))
				res[nres++] = pt;
		}
	}
	// finding mass center
	center = Point(0, 0);
	for (int i = 0; i < nres; i++)
	{
		center+=res[i];
	}
	center /= (double)nres;
	// sorting in counterclockwise direction
	sort(res, res+nres);
}

int main(void)
{
	Polygon p1, p2;
	Polygon res;
	int n1, n2;
	// reading data
	cin >> n1;
	p1 = new Point[n1];
	for (int i = 0; i < n1; i++)
	{
		double x, y;
		cin >> x >> y;
		p1[i] = Point(x, y);
	}
	cin >> n2;
	p2 = new Point[n2];
	for (int i = 0; i < n2; i++)
	{
		double x,y;
		cin >> x >> y;
		p2[i] = Point(x, y);
	}
	res = new Point[n1+n2+1];
	int nres = 0;
	// Call to the main routine
	intersect(p1, n1, p2, n2, res, nres);
	// Printing the result
	cout << nres << endl;
	for (int i = 0; i < nres; i++)
	{
		cout << real(res[i]) << " " << imag(res[i]) << endl;
	}
	delete[] p1;
	delete[] p2;
	delete[] res;
	return 0;
}
