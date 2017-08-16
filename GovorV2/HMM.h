#include <opencv2/core/core.hpp>
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

	MFCC modelVektorSegmenta;

	float kovarijansa;
	

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
	void poravnavanjeRadiTreniranja(std::vector<std::vector<Sablon>>& newWords, std::vector<MFCCWord>& reci);
public:
	const int N = NUMBER_OF_STATES;
	
	/*states*/
	Segment segmenti[30];
	//Sablon segmenti[30];

	HMM();
	~HMM();
	void init();
	void train(std::vector<MFCCWord>& reci);
	
};

