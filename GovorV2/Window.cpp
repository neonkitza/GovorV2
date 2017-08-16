#include "Window.h"
#include <stdlib.h>
#include <math.h>
#include "libmfcc.h"

Window::Window()
{
}


Window::~Window()
{
}
float absolute(float a)
{
	if (a < 0)
		return -a;
	else
	{
		return a;
	}
}

/*
double Window::calcEuclideanDistance(Window w)
{
	double sum = 0;
	for (int i = 0; i < 12; i++)
	{
		sum += pow(lpc[i] - w.lpc[i], 2);
	}
	sum = sqrt(sum);
	return sum;
}
*/
