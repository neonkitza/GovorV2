#include <vector>
#pragma once

class Window
{
public:

	int lengthInFrames, lengthInMs;
	bool isVoice = false;
	std::vector<double> data;
	int id;
	int startFrame, endFrame;
	double avgValue;
	int signCount;
	double mfcc[12];
	//float lpc[12];
	
	//TODO: MFCC

	Window();
	~Window();
	void calcMFCC();
	double calcEuclideanDistance(Window w);
};

