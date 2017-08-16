#include <vector>
#include "MFCC.h"
#pragma once

class Window
{
public:

	//float mfcc[12], deltaMfcc[12], deltaDeltaMfcc[12];
	//float energy, deltaEnergy, deltaDeltaEnergy;
	//std::vector<float> completeMfcc;
	MFCC mfcc;
	int kojiState = 0;
	int lengthInFrames, lengthInMs;
	bool isVoice = false;
	std::vector<double> data;
	int id;
	int startFrame, endFrame;
	double avgValue;
	int signCount;
	//double mfcc[39];
	//MFCC mfcc;
	//float lpc[12];
	
	//TODO: MFCC

	Window();
	~Window();
	double calcEuclideanDistance(Window w);
};

