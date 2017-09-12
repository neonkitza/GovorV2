#include "HMM.h"
#include <cmath>
#include <algorithm>
#include <iterator>

#define _USE_MATH_DEFINES
#include <math.h>


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
						reci[i].windows[count].mfcc.indexInWord = count;
						newWords[i][j].vektoriUSablonu.push_back(reci[i].windows[count].mfcc);
						//countUStanju++;
						count++;
						countVecihStanja++;	
					}
					
					break;
					
				}
				else
				{
					reci[i].windows[count].mfcc.indexInWord = count;
					newWords[i][j].vektoriUSablonu.push_back(reci[i].windows[count].mfcc);
					count++;
					
				}
				countUStanju++;
			}
		}
	}

	//kreiramo model vektore segmenata
	createModelVectors(newWords);



	//idemo po trening recima, izracunamo BW parametre za svaku i delimo
	//ovo sve treba u petlju, ovo je BM
	std::vector<std::vector<std::vector<double>>> beta, alpha;
	beta.resize(reci.size());
	alpha.resize(reci.size());
	for (int i = 0; i < 30; i++) //idemo po stanjima
	{
		double novaVerPrelaska = 0, novaVerEmis = 0, imenilacA = 0, delilacA = 0, imenilacB = 0, delilacB = 0;
		
		

		for (int j = 0; j < reci.size(); j++) //po recima
		{
			forward(alpha[j], reci[j]);
			backward(beta[j], reci[j]);
			
			for (int t = 0; t < reci[j].windows.size(); t++) 
			{
				if (t < reci[j].windows.size() - 1)
				{
					imenilacA += jointProb(i, alpha[j], beta[j], reci[j], t);
					delilacA += gamma(i, alpha[j], beta[j], reci[j], t);
				}
				else
				{
					delilacB = delilacA + gamma(i, alpha[j], beta[j], reci[j], t);
				}


			}

			//novaVerPrelaska += transProbUpdate(i, alpha[j], beta[j], reci[j]);
			
		}
		segmenti[i].verovatnocaPrelaska = imenilacA/delilacA;
		segmenti[i].verovatnocaOstajanja = 1 - segmenti[i].verovatnocaPrelaska;
		
	}
	for (int i = 0; i < N; i++) //po segmentima
	{

		MFCC newMedianVector;
		double divThingie = 0.0;
		double newCov[37];
		double covImenilac = 0.0;
		memset(segmenti[i].kovarijansa, 0, 37 * sizeof(double)); //TEST
		segmenti[i].covMatrixSum = 1;
		for (int j = 0; j < reci.size(); j++) //po recima
		{
			for (int k = 0; k < reci[j].windows.size(); k++) //po prozorima (svim ili po segmentima????)
			{
				double thingie = gamma(i, alpha[j], beta[j], k);
				newMedianVector += reci[j].windows[k].mfcc * thingie;
				divThingie += thingie;

				for (int kovCount = 0; kovCount < reci[j].windows[k].mfcc.completeMfcc.size(); kovCount++)
				{
					segmenti[i].kovarijansa[kovCount] += (pow((reci[j].windows[k].mfcc.completeMfcc[kovCount] - segmenti[i].modelVektorSegmenta.completeMfcc[kovCount]), 2)*thingie);
				}

			}
		}
		newMedianVector = newMedianVector/divThingie;
		segmenti[i].modelVektorSegmenta = newMedianVector;
		for (int j = 0; j < 37; j++)
		{
			segmenti[i].kovarijansa[j] /= divThingie;
			segmenti[i].covMatrixSum *= segmenti[i].kovarijansa[j];
		}
	}
	

}
double HMM::transProbUpdate(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, MFCCWord o)
{
	double sumI = 0.0;
	double sumD = 0.0;
	for (int t = 0; t < o.windows.size(); t++)
	{
		if (t == o.windows.size() || whichState == 29) break;
		sumI += alpha[t][whichState] * segmenti[whichState].verovatnocaPrelaska*getEmissionProb(segmenti[whichState], o.windows[t + 1].mfcc)*beta[t + 1][whichState + 1];
		sumD += alpha[t][whichState] * beta[t][whichState];

	}
	return sumI / sumD;
}

double HMM::gamma(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, int t)
{
	double sumD = 0.0;
	for (int i = 0; i < N; i++)
	{
		sumD += alpha[t][i] * beta[t][i];
		//sum += jointProb(j, alpha, beta, o, t);
	}
	return alpha[t][whichState]*beta[t][whichState]/sumD;
}

double HMM::jointProb(int whichState, std::vector<std::vector<double>> alpha, std::vector<std::vector<double>> beta, MFCCWord o,int t)
{
	double sumI = 0.0;
	double sumD = 0.0;
	sumI = alpha[t][whichState]*segmenti[whichState].verovatnocaPrelaska*getEmissionProb(segmenti[whichState],o.windows[t+1].mfcc)*beta[t+1][whichState+1];
	for (int i = 0; i < N; i++)
	{
		sumD += alpha[t][i] * beta[t][i];

	}
	return sumI / sumD;

}
double HMM::forward(std::vector<std::vector<double>>& alpha, MFCCWord o)
{
	//init
	double sum = 0.0;
	int Tmax = o.windows.size();

	
	alpha.resize(Tmax);
	for (int i = 0; i < 30; i++)
	{
		alpha[i].resize(30);
	}

	//init
	int t = 0;
	for (int i = 0; i < 30; i++)
		alpha[t][i] = segmenti[i].initialProb*getEmissionProb(segmenti[i],o.windows[t].mfcc);

	//rekurzija (valjda)
	for (t = 0; t < Tmax; t++)
	{
		for (int j = 0; j < N; j++)
		{
			sum = 0.0;
			for (int i = 0; i < N; i++)
			{
				if (i == j)
				{
					sum += alpha[t][i] * segmenti[i].verovatnocaOstajanja;
				}
				else if (i == j + 1)
				{
					sum += alpha[t][i] * segmenti[i].verovatnocaPrelaska;
				}
				
				//sum += alpha[t][i] * segmenti[i].verovatnocaPrelaska;
			}
			if(t < Tmax-1) alpha[t + 1][j] = sum*getEmissionProb(segmenti[j],o.windows[t+1].mfcc);
		};
	}
	t = Tmax-1;
	sum = 0.0;
	for (int i = 0; i < N; i++)
		sum += alpha[t][i];

	return sum;

}
double HMM::backward(std::vector<std::vector<double>>& beta, MFCCWord o)
{
	//init
	double sum = 0.0;
	int Tmax = o.windows.size();

	
	beta.resize(Tmax);
	for (int i = 0; i < 30; i++)
	{
		beta[i].resize(30);
	}

	int t = Tmax;
	for (int i = 0; i < N; i++)
		beta[t][i] = 1.0;

	// Recursion
	for (t = Tmax - 1; t >= 1; t--)
	{
		for (int i = 0; i < N; i++)
		{
			sum = 0.0;
			if (i < N - 1)
			{
				sum += segmenti[i].verovatnocaPrelaska*getEmissionProb(segmenti[i + 1], o.windows[t + 1].mfcc)*beta[t + 1][i + 1];
				sum += segmenti[i].verovatnocaOstajanja*getEmissionProb(segmenti[i], o.windows[t + 1].mfcc)*beta[t + 1][i];

			}
			//for (int j = i; j <= i+1; j++) //da li po svim ili samo moguci (i i i+1)
				//sum += segmenti[i].verovatnocaPrelaska*getEmissionProb(segmenti[j], o.windows[t + 1].mfcc)*beta[t + 1][j];

			beta[t][i] = sum;
		};
	}

	// Termination
	t = 0;
	sum = 0.0;
	for (int i = 0; i < N; i++)
		sum += segmenti[i].initialProb*getEmissionProb(segmenti[i],o.windows[t].mfcc)*beta[t][i];

	return sum;

}
void HMM::createModelVectors(std::vector<std::vector<Sablon>>& newWords)
{
	for (int j = 0; j < 30; j++)
	{
		MFCC sumInSegment;
		double sumCov = 0;
		int numOfVectorsInSegment = 0;
		double covSum = 0;
		for (int i = 0; i < newWords.size(); i++)
		{
			for (int k = 0; k < newWords[i][j].vektoriUSablonu.size(); k++)
			{
				sumInSegment += newWords[i][j].vektoriUSablonu[k];
				numOfVectorsInSegment++;
				

			}
		}
		if (j == 0)
		{
			segmenti[j].initialProb = 1;
		}
		else segmenti[j].initialProb = 0;
		segmenti[j].modelVektorSegmenta = sumInSegment / numOfVectorsInSegment;
		segmenti[j].verovatnocaPrelaska = newWords.size() / numOfVectorsInSegment;
		segmenti[j].verovatnocaOstajanja = 1 - segmenti[j].verovatnocaPrelaska;
		segmenti[j].numOfVectorsInSegment = numOfVectorsInSegment;
	}

	for (int j = 0; j < 30; j++) //po segmentima
	{
		double sumCov = 0;
		int kolikoDodali = 0;
		for (int i = 0; i < newWords.size(); i++) //po trening recima
		{
			for (int k = 0; k < newWords[i][j].vektoriUSablonu.size(); k++) //po vektorima u sablonu(segmentu)
			{
				for (int kovCount = 0; kovCount < newWords[i][j].vektoriUSablonu[k].completeMfcc.size(); kovCount++)
				{
					segmenti[j].kovarijansa[kovCount] += pow((newWords[i][j].vektoriUSablonu[k].completeMfcc[kovCount] - segmenti[j].modelVektorSegmenta.completeMfcc[kovCount]),2);		
				}
				kolikoDodali++;
			}
		}
		for (int i = 0; i < 37; i++)
		{
			segmenti[j].kovarijansa[i] /= kolikoDodali;
		}

	}
	
}

//gauss
double HMM::getEmissionProb(Segment segment, MFCC x)
{
	segment.covMatrixSum = 1; //determinanta

	double expPow = 0;

	for (int i = 0; i < 37; i++)
	{
		segment.covMatrixSum *= segment.kovarijansa[i];
		expPow += pow(x.completeMfcc[i] - segment.modelVektorSegmenta.completeMfcc[i], 2);
	}
	
	expPow /= (2 * segment.covMatrixSum);
	//segment.emisionaVerovatnoca = (1 / (sqrt(2 * M_PI)*segment.covMatrixSum))*exp(-expPow);
	return (1 / (sqrt(2 * M_PI)*segment.covMatrixSum))*exp(-expPow);
}

