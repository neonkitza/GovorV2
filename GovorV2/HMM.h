
#include <map>
#include "VoiceFile.h"

#pragma once

#define NUMBER_OF_STATES 30;


struct Segment
{
	int id;
	//matrica a prakticno (tranzicione)
	// 1-verOstajanja = verovatnocaPrelaska;
	double verovatnocaOstajanja;
	double verovatnocaPrelaska; //1-the one above

	//emisione (gaus)
	double emisionaVerovatnoca;


	double initialProb;

	MFCC modelVektorSegmenta;

	double kovarijansa[37];
	double covMatrixSum;
	int numOfVectorsInSegment;
	

}; 
struct Sablon
{
	std::vector<MFCC> vektoriUSablonu;

	//MFCC modelVektor;

};

class HMM
{
private:
	void initInitialStateDistribution();
	void HMM::createModelVectors(std::vector<std::vector<Sablon>>& newWords);
	//void gaussMesavina(Window w);
	void calcObsProbability(Segment & segment, MFCC x);
	double getEmissionProb(Segment segment, MFCC x);
	//double getEmissionProb(MFCC ot, int whichState);
	//double getEmissionProb(MFCC ot);
	//double getEmissionProb(Window w);
	//void calcObsProbability(double kovarijansa[37]);
	//void calcObsProbability(std::vector<std::vector<Sablon>>& newWords);

	//alpha and beta
	std::vector<std::vector<double>> alpha, beta;
	std::vector<std::vector<double>> B;

public:
	const int N = NUMBER_OF_STATES;
	
	/*states*/
	Segment segmenti[30];
	//Sablon segmenti[30];

	HMM();
	~HMM();
	void init();
	void train(std::vector<MFCCWord>& reci);

	double transProbUpdate(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, MFCCWord o);

	double gamma(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, int t);

	double gamma(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, MFCCWord o, int t);

	//double jointProb(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, MFCCWord o);

	double jointProb(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, MFCCWord o, int t);

	double forward(std::vector<std::vector<double>>& alpha, MFCCWord o);

	double backward(std::vector<std::vector<double>>& beta, MFCCWord o);

	//double forward(int whichState, MFCCWord o);

	//double backward(int whichState, MFCCWord o);
	
};

