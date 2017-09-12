#include <vector>
#pragma once
class MFCC
{

public:

	double mfcc[12], deltaMfcc[12], deltaDeltaMfcc[12];
	double energy, deltaEnergy, deltaDeltaEnergy;
	std::vector<float> completeMfcc;
	int indexInWord = 0;
	MFCC();
	~MFCC();
	void calc(std::vector<double> data);
	MFCC operator+(const MFCC & mfcc)
	{
		MFCC m;
		m.completeMfcc.resize(37);
		for (int i = 0; i < 37; i++)
		{
			m.completeMfcc[i] = this->completeMfcc[i] + mfcc.completeMfcc[i];
		}
		return m;
	}
	void operator+=(const MFCC & mfcc)
	{
		
		this->completeMfcc.resize(37);
		for (int i = 0; i < 37; i++)
		{
			this->completeMfcc[i] = this->completeMfcc[i] + mfcc.completeMfcc[i];
		}
	}
	MFCC operator/(int n)
	{
		MFCC m;
		for (int i = 0; i < 37; i++)
		{
			m.completeMfcc[i] = (float)this->completeMfcc[i]/n;
		}
		return m;
	}
	MFCC operator*(double n)
	{
		MFCC m;
		for (int i = 0; i < 37; i++)
		{
			m.completeMfcc[i] = (float)this->completeMfcc[i] * n;
		}
		return m;
	}
	MFCC operator/(double n)
	{
		MFCC m;
		for (int i = 0; i < 37; i++)
		{
			m.completeMfcc[i] = (float)this->completeMfcc[i] / n;
		}
		return m;
	}
	MFCC operator-(MFCC mfcc)
	{
		MFCC m;
		m.completeMfcc.resize(37);
		for (int i = 0; i < 37; i++)
		{
			m.completeMfcc[i] = this->completeMfcc[i] - mfcc.completeMfcc[i];
		}
		return m;
	}
	/*float operator-(const MFCC & mfcc)
	{



		return 0.0f;
	}*/
	float MFCC::calcEuclideanDistance(MFCC m);
	float mahalanobisovoRastojanje(MFCC m,float sigma);
};

