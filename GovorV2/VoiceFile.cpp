#include "VoiceFile.h"
#include <sndfile.h>
#include <sndfile.hh>
#include <iostream>
#include "libmfcc.h"
#include "fftw3.h"
using namespace std;


VoiceFile::VoiceFile()
{
}

VoiceFile::VoiceFile(vector<double> buf, int num)
{
	this->num = num;
	this->buf = buf;
}

VoiceFile::VoiceFile(char * path)
{
}


VoiceFile::~VoiceFile()
{
}
int DFT(int dir, int m, double *x1)
{
	long i, k;
	double arg;
	double cosarg, sinarg;
	double *x2 = NULL;

	x2 = (double*)malloc(m * sizeof(double));
	if (x2 == NULL)
		return(0);

	for (i = 0; i<m; i++) {
		x2[i] = 0;
		arg = -dir * 2.0 * 3.141592654 * (double)i / (double)m;
		for (k = 0; k<m; k++) {
			cosarg = cos(k * arg);
			sinarg = sin(k * arg);
			x2[i] += (x1[k] * cosarg);
		}
	}

	/* Copy the data back */
	if (dir == 1) {
		for (i = 0; i<m; i++) {
			x1[i] = x2[i] / (double)m;
		}
	}
	else {
		for (i = 0; i<m; i++) {
			x1[i] = x2[i];
		}
	}

	free(x2);
	return(1);
}
void VoiceFile::readFile(char* path)
{
	SNDFILE *sf;
	SF_INFO info;
	int num_channels;
	int num_items;
	int f, sr, c;

	info.format = 0;
	sf = sf_open(path, SFM_READ, &info);
	if (sf == NULL)
	{
		printf("Failed to open the file.\n");
		//system("pause");
		//exit(-1);
	}
	/* Print some of the info, and figure out how much data to read. */
	f = info.frames;
	sr = info.samplerate;
	c = info.channels;


	printf("frames=%d\n", f);
	printf("samplerate=%d\n", sr);
	printf("channels=%d\n", c);
	num_items = f*c;
	printf("num_items=%d\n", num_items);
	sampleRate = sr;

	/* Allocate space for the data to be read, then read it. */
	//buf = (double *)malloc(info.frames * sizeof(double));
	buf.resize(num_items);
	num = sf_read_double(sf, &buf[0], num_items);
	sf_close(sf);

	printf("Read %d items\n", num);
	//buf.shrink_to_fit();
	cout << "Size of buf vector" << buf.size() << endl;
}

void VoiceFile::processAudio(bool printStuff = true)
{
	windowSizeInFrames = SIZE_IN_MS * sampleRate / 1000; //441
	//if (windowSizeInFrames % 2 != 0) windowSizeInFrames -= 1; //220
	
	//noise threshold
	calcNoiseThreshold();

	num = buf.size();

	dataIntoWindows();
	for (int i = 0; i < windows.size(); i++)
	{
		cout << windows[i].isVoice;
	}
	cout << endl;
	//lower
	lower(printStuff);

	for (int i = 0; i < windows.size(); i++)
	{
		cout << windows[i].isVoice;
	}
	cout << endl;
	//raise
	raise(printStuff);
	for (int i = 0; i < windows.size(); i++)
	{
		cout << windows[i].isVoice;
	}
	cout << endl;

	//calc beginning and end
	calcStartEnd();

	//ZCR
	// moraju se promeniti startID i endID ako se nesto promeni
	/*ZCR();
	for (int i = 0; i < windows.size(); i++)
	{
		cout << windows[i].isVoice;
	}*/

	//populate voiceWindows
	for (int i = startID; i < endID; i++)
	{

		voiceWindows.push_back(windows[i]);

	}
	voiceWindows.shrink_to_fit();
	calcMFCC();
/*	for (int i = 0; i < windows.size(); i++)
	{
		cout << "isVoice: " << windows[i].isVoice << endl;
	}*/
}
void VoiceFile::calcMFCC()
{
	//mozda mora fft/dft na datu pre
	for (int i = 0; i < voiceWindows.size(); i++)
	{
		DFT(1, voiceWindows[i].data.size(), &voiceWindows[i].data[0]); //trebalo bi fft a ne dft?
		voiceWindows[i].mfcc.calc(voiceWindows[i].data);
	}
}
void VoiceFile::calcStartEnd() 
{
	bool started = false, ended = false;
	for (int i = 0; i < windows.size(); i++)
	{
		if (windows[i].isVoice && !started)
		{
			startID = i;
			started = true;
		}
		else if (started && !windows[i].isVoice && !ended)
		{
			endID = i;
			ended = true;
			break;
		}
	}
}
void VoiceFile::ZCR()
{
	for (int i = 0; i < windows.size(); i++)
	{
		if (windows[i].signCount > noiseSgnCount)
			windows[i].isVoice = false;
	}
}
void VoiceFile::calcNoiseThreshold()
{
	double noiseSamples = sampleRate / 10; //number of frames in first 100ms

	double sum = 0;
	double avg = 0;
	bool sgn = true;
	//int sgnCount = 0;

	for (int i = 0; i < noiseSamples; i++)
	{
		if ((buf[i] < 0 && sgn) || (buf[i] >= 0 && !sgn)) noiseSgnCount++;
		if (buf[i] < 0) sgn = false;
		else sgn = true;
		avg += abs(buf[i]);
	}
	avg /= noiseSamples;
	for (int i = 0; i < noiseSamples; i++)
	{
		sum += pow(abs(buf[i]) - avg, 2);

	}
	noiseThreshold = avg + sqrt(sum / noiseSamples);
	noiseSgnCount *= SIZE_IN_MS / 100;
	//cout << "Noise threshold: " << noiseThreshold << endl;

}
/* Lower i raise ne rade skroz tacno*/
void VoiceFile::lower(bool printStuff = false)
{
	int count = 0;
	int woo = 200 / SIZE_IN_MS;
	for (int i = 0; i < windows.size(); i++)
	{
		if (windows[i].isVoice)
		{

			count = 0;
			for (int j = i - woo; j < i + woo + 1; j++)
			{
				
				if (j == i) continue;

				if (j >= windows.size() || j < 0)count++;
				else if (!windows[j].isVoice) count++;

			}
			double temp = (double)count / (2 * woo);
			if (printStuff) printf("LOWER - za i = %d ,temp je: %f, a count je: %d\n", i, temp, count);
			if (temp >= 0.55)
			{
				windows[i].isVoice = false;
				cout << "LOWERED id = " << i << endl;
			}
		}
	}
}
void VoiceFile::raise(bool printStuff = false)
{
	int count = 0;
	int woo = 200 / SIZE_IN_MS;
	//int temp = (woo * 2 * 6) / 10;
	int windowsSize = windows.size();

	for (int i = 0; i < windows.size(); i++)
	{
		
		count = 0;
		if (!windows[i].isVoice)
		{
			for (int j = i - woo; j < i + woo + 1; j++)
			{
				//if (j == windows.size()) break;
				if (j == i) continue;
				if (j >= windows.size() || j < 0) continue;
				else if (windows[j].isVoice)count++;
				
			}
			double temp = (double)count / (2*woo);
			if (printStuff) printf("RAISE - za i = %d ,temp je: %f, a count je: %d\n", i, temp, count, temp);
			if (temp >= 0.55)
			{
				windows[i].isVoice = true;
				cout << "RAISED id = " << i << endl;
			}
			
		}
	}
}
void VoiceFile::dataIntoWindows()
{
	int windowCount = 0;

	for (int i = 0; i < buf.size(); i += (int)windowSizeInFrames / 2) //windowSizeInFrames = 441
	{

		Window w;
		w.startFrame = i;
		w.endFrame = w.startFrame + windowSizeInFrames; //doesn't count
		if (w.endFrame >= num) w.endFrame = num;
		w.id = windowCount;
		w.lengthInFrames = windowSizeInFrames;
		w.lengthInMs = SIZE_IN_MS;

		windows.push_back(w);

		double sum = 0;
		int c = 0;

		bool sgn = true;
		int sgnCount = 0;

		for (int j = w.startFrame; j < w.endFrame; j++)
		{
			if (j >= num)
			{
				break;
				printf("BREAK IN dataIntoWindows!\n");
			}

			windows[windowCount].data.push_back(buf[j]);

			sum += std::abs(buf[j]);
			c++;

			if ((buf[j] < 0 && sgn) || (buf[j] >= 0 && !sgn)) sgnCount++;
			if (buf[j] < 0) sgn = false;
			else sgn = true;


		}
		windows[windowCount].avgValue = sum / windowSizeInFrames;
		if (windows[windowCount].avgValue > noiseThreshold) windows[windowCount].isVoice = true;
		windows[windowCount].signCount = sgnCount;
		hammingWindow(windows[windowCount].data); //hamming ovde???
		windowCount++;

	}
}
void VoiceFile::recordFile()
{

}
void VoiceFile::hammingWindow(std::vector<double>& bufOut)
{
	for (int i = 0; i < bufOut.size(); i++) {
		double multiplier = (0.54 - 0.46 * std::cos(2.0 * 3.14159265359 * i / windowSizeInFrames));
		bufOut[i] *= multiplier;
	}
}
