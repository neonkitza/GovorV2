#include "HMM.h"



void HMM::initInitialStateDistribution()
{
	
}

HMM::HMM()
{
}


HMM::~HMM()
{
}

void HMM::init()
{
}
void resizeVec(std::vector<std::vector<Window> > &vec, const unsigned short ROWS, const unsigned short COLUMNS)
{
	vec.resize(ROWS);
	for (auto &it : vec)
	{
		it.resize(COLUMNS);
	}
}
void HMM::train(std::vector<MFCCWord>& reci)
{
	//reci[i].size(),30
	std::vector<std::vector<Sablon>> newWords;
	//std::vector<TrainingWord> trainingWords;
	bool varSize = false;
	
	//inicijalizacija trening sablona za sve reci
	newWords.resize(reci.size());
	for (int i = 0; i < reci.size(); i++)
	{
		for (int j = 0; j < N; j++)
		{
			Sablon s;
			newWords[i].push_back(s);
		}
	}
	//idemo rec po rec
	for (int i = 0; i < reci.size(); i++)
	{	
		int countUStanju = 0;
		int count = 0;
		int countVecihStanja = 0;

		int velicinaStanja = reci[i].numOfWindows / N;
		int vecihStanja = reci[i].numOfWindows % N;
		
		//setamo po sablonima
		//j = sablonCount 
		for (int j = 0; j < N; j++)
		{
			countUStanju = 0;
			while (1)
			{
				if (countUStanju == velicinaStanja)
				{
					if (countVecihStanja < vecihStanja)
					{
						newWords[i][j].vektoriUSablonu.push_back(reci[i].windows[count].mfcc);
						//countUStanju++;
						count++;
						countVecihStanja++;	
					}
					
					break;
					
				}
				else
				{
					newWords[i][j].vektoriUSablonu.push_back(reci[i].windows[count].mfcc);
					count++;
					
				}
				countUStanju++;
			}
		}
	}

	//kreiramo model vektore segmenata
	createModelVectors(newWords);





}
void HMM::poravnavanjeRadiTreniranja(std::vector<std::vector<Sablon>>& newWords, std::vector<MFCCWord>& reci)
{

	//ici po recima
	for (int i = 0; i < reci.size(); i++)
	{
		cv::Mat matrica(N, reci[i].numOfWindows, CV_32FC1);
		for (int j = 0; j < reci[i].numOfWindows; j++)
		{

		}
	}
}
void HMM::createModelVectors(std::vector<std::vector<Sablon>>& newWords)
{
	for (int j = 0; j < 30; j++)
	{
		MFCC sumInSegment;
		int numOfVectorsInSegment = 0;
		for (int i = 0; i < newWords.size(); i++)
		{
			for (int k = 0; k < newWords[i][j].vektoriUSablonu.size(); k++)
			{
				sumInSegment += newWords[i][j].vektoriUSablonu[k];
				numOfVectorsInSegment++;

			}
		}
		segmenti[j].modelVektorSegmenta = sumInSegment / numOfVectorsInSegment;
		segmenti[j].verovatnocaPrelaska = newWords.size() / numOfVectorsInSegment;
		segmenti[j].verovatnocaOstajanja = 1 - segmenti[j].verovatnocaPrelaska;
	}
}