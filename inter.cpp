#include <cstdio>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

struct fVal {
    double x, y, f;
    fVal(double nx, double ny, double nf) {
	x=nx;
	y=ny;
	f=nf;
    }
};

#define sqr(a) ((a)*(a))

inline double nfunc(double x1, double x2, double y1, double y2)
{
    // It should be fast-working function, which returns 0
    // if and only if x1==x2 and y1==y2
    return sqr(x1-x2)+sqr(y1-y2);
}

double interpolate(vector<fVal> p, double x, double y)
{
    int N = p.size();
    double res = 0;
    int i, j;
    for (i = 0; i < N; i++)
    {
	double curval = p[i].f;
	for (j = 0; j < N; j++)
	{
	    if (j==i) continue;
	    curval*=nfunc(x, p[j].x, y, p[j].y);
	    curval/=nfunc(p[i].x, p[j].x, p[i].y, p[j].y);
	    // NOTICE: the last line doesn't include x and y
	    // That means, you can precount production of such values:
	    // r(i) = P_{i!=j}nfunc(x[i]. x[j], y[i], y[j])
	}
	res+=curval;
    }
    return res;
}

int main(void)
{
    vector<fVal> p;
    p.push_back(fVal(0.0, 0.0, 0.0));
    p.push_back(fVal(1.0, 0.0, 1.0));
    p.push_back(fVal(0.0, 1.0, 1.0));
    p.push_back(fVal(1.0, 1.0, 2.0));
    printf("%.4lf\n", interpolate(p, 0.5, 0.5));
    // Interpolated polynom is x*x+y*y
    return 0;
}

