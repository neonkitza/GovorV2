#include "Window.h"
#include "pahelper.h"
#include <vector>
#pragma once

#define SIZE_IN_MS 20

class VoiceFile
{
public:

	char * path;
	std::vector<double> buf;
	int num; //buf.size() pretty much
	std::vector<Window> windows;
	std::vector<Window> voiceWindows;
	int sampleRate;
	int windowSizeInFrames;
	double noiseThreshold;
	int noiseSgnCount = 0;
	int startID, endID;

	VoiceFile();
	VoiceFile(std::vector<double> buf, int num);
	//VoiceFile(double*buf, int num);
	VoiceFile(char*);


	~VoiceFile();
	void readFile(char * path);
	void processAudio(bool printStuff);
	void calcMFCC();
	void calcStartEnd();
	void ZCR();
	void calcNoiseThreshold();
	void lower(bool);
	void raise(bool);
	void dataIntoWindows();
	void recordFile();
	void hammingWindow(std::vector<double>& bufOut);
	void izbaciNewline(char * tekst);
	void saveWAV(const char * name, double * buf, int num, int sampleRate);
};

