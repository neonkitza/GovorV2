#include "MFCC.h"
#include "libmfcc.h"


MFCC::MFCC()
{
}


MFCC::~MFCC()
{
}

void MFCC::calc(std::vector<double> data)
{
	energy = 0;

	for (int i = 0; i < 12; i++)
	{
		mfcc[i] = GetCoefficient(&data[0], 44100, 48, data.size(), i);
		completeMfcc.push_back(mfcc[i]);
	}
	for (int i = 0; i < data.size(); i++)
	{
		energy += data[i] * data[i];
	}
	//delta
	for (int i = 0; i < 12; i++)
	{
		int low = i - 2;
		int high = i + 2;
		if (low < 0)low = 0;
		if (high > 11) high = 11;

		deltaMfcc[i] = mfcc[high] - mfcc[low];
		completeMfcc.push_back(deltaMfcc[i]);
	}
	//delta-delta
	for (int i = 0; i < 12; i++)
	{
		int low = i - 2;
		int high = i + 2;
		if (low < 0)low = 0;
		if (high > 11) high = 11;

		deltaDeltaMfcc[i] = deltaMfcc[high] - deltaMfcc[low];
		completeMfcc.push_back(deltaDeltaMfcc[i]);
	}

	completeMfcc.push_back(energy);
}

float MFCC::calcEuclideanDistance(MFCC m)
{
	float sum = 0;
	for (int i = 0; i < 37; i++)
	{
		sum += pow(completeMfcc[i] - m.completeMfcc[i], 2);
	}
	sum = sqrt(sum);
	return sum;
}

float MFCC::mahalanobisovoRastojanje(MFCC m,float sigma)
{
	float sum = 0;
	for (int i = 0; i < 37; i++)
	{
		sum += pow(completeMfcc[i] - m.completeMfcc[i], 2);
	}
	sum /= sigma;
	return sum;
}


	



