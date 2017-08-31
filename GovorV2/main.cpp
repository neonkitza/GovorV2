#include "VoiceFile.h"
#include "sndfile.h"
#include "sndfile.hh"
#include "pahelper.h"
#include "pahelper.h"
#include <dirent.h>
#include "HMM.h"

bool checkFolder(const char * path);
bool readFolder(const char * path, const char * ekstenzija, std::vector<char*> &fileList);

void izbaciNewline(char *tekst)
{
	for (int i = 0; tekst[i] != '\0'; i++)
		if (tekst[i] == '\n')
			tekst[i] = '\0';
}


void saveWAV(const char* name, double *buf, int num, int sampleRate)
{
	SF_INFO sfinfo;
	sfinfo.channels = 1;
	sfinfo.samplerate = sampleRate;
	sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	SNDFILE * outfile = sf_open(name, SFM_WRITE, &sfinfo);
	sf_count_t count = sf_write_double(outfile, &buf[0], num);
	sf_write_sync(outfile);
	sf_close(outfile);
}
int saveWordMFCC(const char * fileName, std::vector<Window> word, int wordSize)
{
	FILE *f = fopen(fileName, "wb");
	if (f == NULL)
	{
		printf("Fail u otvaranju fajla");
		return 0;
	}

	uint32_t wordSize32 = wordSize;
	fwrite(&wordSize32, sizeof(uint32_t), 1, f);
	for (int i = 0; i < wordSize; i++)
	{
		for (int j = 0; j < 37; j++)

			printf("SAVE za word[%d] - MFCC[%d] = %f\n", i, j, word[i].mfcc.completeMfcc[j]);
	}
	for (int i = 0; i < wordSize; i++)
		fwrite(&word[i].mfcc.completeMfcc[0], sizeof(float), 37, f);

	fclose(f);

	return 1;
}
int readWordMFCC(const char * fileName, std::vector<Window>& word)
{
	uint32_t wordSize = 0;

	FILE *f = fopen(fileName, "rb");
	if (f == NULL)
	{
		printf("Fail u otvaranju fajla");
		return 0;
	}
	fread(&wordSize, sizeof(uint32_t), 1, f);
	//*word = (Window*)calloc(wordSize, sizeof(Window));
	word.resize(wordSize);
	//std::vector<Window> prozori;
	//prozori.resize(wordSize);
	for (int i = 0; i < wordSize; i++)
	{
		//(*word)[i].mfcc.completeMfcc.resize(37);
		//fread(&(*word)[i].mfcc.completeMfcc, sizeof(float), 37, f);
		//int temp = i;
		word[i].mfcc.completeMfcc.resize(37);
		fread(&word[i].mfcc.completeMfcc[0], sizeof(float), 37, f);
	}

	printf(" ----------------------\n");
	printf("MFCC za %s\n", fileName);
	for (int i = 0; i < wordSize; i++)
	{
		for (int j = 0; j < 37; j++)

			printf("READ za word[%d] - MFCC[%d] = %f\n", i, j, word[i].mfcc.completeMfcc[j]);
	}
	fclose(f);
	return wordSize;
}

char gNazivBaze[200];
char gDefaultBaza[200] = "velikaBaza";

bool traziPoBazi(std::vector<Window> word, int wordSize, std::vector<MFCCWord> &reci, char* nadjenaRec, double* minDTW)
{
	int minID = -1;
	*minDTW = 0.0;

	printf("\n\n=======================\n");
	for (int i = 0; i < reci.size(); i++)
	{

		//TODO: HMM 

		//DTW dtw(wordSize + 1, reci[i].brojProzora + 1); 
		
		/*
		double dtwResult = dtw.run(word, reci[i].windows, wordSize, reci[i].numOfWindows);
		printf("DTW rezultat za '%s' : %lf\n", reci[i].rec, dtwResult);

		if (minID < 0 || dtwResult < *minDTW)
		{
			minID = i;
			*minDTW = dtwResult;
		}
		*/
	}
	//printf("-----------------------\n");
	//printf("Minimum je za rec '%s' (DTW = %lf)\n", reci[minID].rec, minDTW);
	printf("=======================\n\n");

	if (*minDTW > 50) return false;
	strcpy(nadjenaRec, reci[minID].word);
	return true;
}

bool ucitajBazu(const char *nazivBaze, std::vector<MFCCWord> &reci)
{
	char mfccFolder[200];
	strcpy(mfccFolder, nazivBaze);
	strcat(mfccFolder, "/MFCC");

	if (!checkFolder(mfccFolder))
	{
		printf("Ova baza nema MFCC koeficijente, probajte opciju 3 prvo.\n\n");
		return false;
	}

	std::vector<char*> dataFajlovi;
	if (!readFolder(mfccFolder, ".dat", dataFajlovi))
	{
		printf("Citanje MFCC koeficijenata nije uspelo iz nekog razloga.\n\n");
		return false;
	}

	if (dataFajlovi.size() == 0)
	{
		printf("Folder za MFCC postoji, ali nema nijedne reci u njemu, probajte opciju 3 prvo.\n\n");
		return false;
	}

	reci.clear();
	for (int i = 0; i < dataFajlovi.size(); i++)
	{
		MFCCWord rec;
		char putanja[200];
		strcpy(putanja, mfccFolder);
		strcat(putanja, "/");
		strcat(putanja, dataFajlovi[i]);
		rec.numOfWindows = readWordMFCC(putanja, rec.windows);

		if (rec.numOfWindows == 0)
		{
			printf("Greska u ucitavanju koeficijenata iz '%s'!\n", putanja);
			continue;
		}

		strcpy(rec.word, dataFajlovi[i]);
		rec.word[strlen(rec.word) - 4] = '\0';

		printf("Rec iz MFCC baze : '%s'\n", rec.word);
		reci.push_back(rec);
	}

	return true;
}

void trainHMM(HMM& hmm)
{
	std::vector<MFCCWord> reci;
	
	ucitajBazu("trainBaza\\hello",reci);
	hmm.train(reci);
	//
}

void compare()
{
	std::vector<MFCCWord> reci;
	if (!ucitajBazu(gNazivBaze, reci))
		return;

	printf("Baza ucitana, pripremite se za snimanje, Enter za pocetak, Tab za kraj...");
	getchar();

	double *buf = (double*)malloc(10 * 44100 * sizeof(double));
	int num = 0;
	int sr = 44100;

	PAH_RecordMonoSamples(buf, 10 * 44100, 44100, true, &num);

	if (num > 44100 / 100)
		num -= 44100 / 100;

	saveWAV("zadnji-snimak.wav", buf, num, 44100);
	VoiceFile vf;
	vf.readFile("zadnji-snimak.wav");
	Window * word;
	vf.processAudio(false);
	int wordSize = vf.voiceWindows.size();

	double minDTW = 0.0;
	char nadjenaRec[50];

	if (traziPoBazi(vf.voiceWindows, vf.voiceWindows.size(), reci, nadjenaRec, &minDTW))
	{
		printf("Minimum je za rec '%s' (DTW = %lf)\n", nadjenaRec, minDTW);
	}
	else
	{
		printf("Nismo nasli odgovarajucu rec u bazi.\n");
	}
}

void testCases()
{
	char srcFolder[200];

	printf("\nIz kog foldera testiramo? ");
	fgets(srcFolder, 200, stdin);
	izbaciNewline(srcFolder);

	if (!checkFolder(srcFolder))
	{
		printf("Nema foldera '%s', nema testiranja.\n");
		return;
	}

	std::vector<char*> fajlovi;
	if (!readFolder(srcFolder, ".wav", fajlovi))
	{
		printf("Nismo uspeli naci fajlove iz '%s'.", srcFolder);
		return;
	}

	if (fajlovi.size() == 0)
	{
		printf("Nemamo nijedan .WAV fajl u '%s'.\n", srcFolder);
		return;
	}

	std::vector<MFCCWord> reci;
	if (!ucitajBazu(gNazivBaze, reci))
		return;

	int tacni = 0;
	int ukupno = fajlovi.size();

	for (int i = 0; i < fajlovi.size(); i++)
	{
		char fajl[200];
		strcpy(fajl, srcFolder);
		strcat(fajl, "/");
		strcat(fajl, fajlovi[i]);
		/*
		Window* prozori;
		SNDFILE *sf;
		SF_INFO info;
		double *buf;
		int num;

		// Open the WAV file. 
		info.format = 0;
		sf = sf_open(fajl, SFM_READ, &info);
		if (sf == NULL)
		{
			printf("Greska u otvaranju '%s', prolazimo dalje.\n", fajl);
			ukupno--;
			continue;
		}

		// Allocate space for the data to be read, then read it. 
		buf = (double *)malloc(info.frames * sizeof(double));
		num = sf_read_double(sf, buf, info.frames);
		sf_close(sf);
		*/
		VoiceFile vf;
		vf.readFile(fajl);
		vf.processAudio(false);
		int brojProzora = vf.voiceWindows.size();
		if (brojProzora > 0)
		{
			char nadjenaRec[50];
			double minDTW = 0.0;

			char rec[50];
			strcpy(rec, fajlovi[i]);
			rec[strlen(fajlovi[i]) - 4] = '\0';

			bool imaUBazi = false;
			for (int i = 0; i < reci.size(); i++)
			{
				if (strcmp(reci[i].word, rec) == 0)
				{
					imaUBazi = true;
					break;
				}
			}

			if (traziPoBazi(vf.voiceWindows, vf.voiceWindows.size(), reci, nadjenaRec, &minDTW))
			{
				printf("%s -> ", rec);
				printf("%s [DTW = %.2lf] : ", nadjenaRec, minDTW);

				if (imaUBazi && strcmp(rec, nadjenaRec) == 0)
				{
					printf("TACNO\n");
					tacni++;
				}
				else
				{
					printf("NETACNO\n");
				}
			}
			else
			{
				printf("%s -> ", rec);
				printf("N/A : ");

				if (!imaUBazi)
				{
					printf("TACNO\n");
					tacni++;
				}
				else
				{
					printf("NETACNO\n");
				}
			}

		}
		else
		{
			printf("Greska u procesiranju '%s', prolazimo dalje.\n", fajl);
			ukupno--;
			continue;
		}

		//if (prozori != NULL) free(prozori);
		//if (buf != NULL) free(buf);
	}

	printf("\nTacnih %d od %d (%.2lf%%)\n\n", tacni, ukupno, (double)tacni / (double)ukupno * 100.0);
}

void dbInput()
{
	char rec[200];
	printf("Koju rec snimamo? (samo mala slova pls) ");
	fgets(rec, 200, stdin);
	izbaciNewline(rec);

	if (strlen(rec) < 1)
	{
		printf("Moramo imati rec.");
		return;
	}

	bool ok = true;
	for (int i = 0; rec[i] != '\0'; i++)
	{
		if (rec[i] < 'a' || rec[i] > 'z')
		{
			ok = false;
			break;
		}
	}

	if (!ok)
	{
		printf("Samo mala slova, PLS!");
		return;
	}

	//double *buf = (double*)malloc(10 * 44100 * sizeof(double));
	std::vector<double> buf;
	int num = 0;
	int sr = 44100;

	printf("Pripremite se za snimanje (Enter za start, Tab za stop)...");
	getchar();

	PAH_RecordMonoSamples(&buf[0], 10 * 44100, 44100, true, &num);

	if (num > 44100 / 100)
		num -= 44100 / 100;

	//Window * word;
	VoiceFile vf(buf, num);
	vf.processAudio(false);
	int wordSize = vf.voiceWindows.size();

	if (wordSize == 0)
	{
		printf("Nemamo govora u ovoj reci?");
		//free(word);
		//free(buf);
		return;
	}

	char baseFileName[256];
	strcpy(baseFileName, gNazivBaze);
	strcat(baseFileName, "/MFCC/");
	strcat(baseFileName, rec);
	strcat(baseFileName, ".dat");

	char wavFileName[256];
	strcpy(wavFileName, gNazivBaze);
	strcat(wavFileName, "/");
	strcat(wavFileName, rec);
	strcat(wavFileName, ".wav");

	int result = saveWordMFCC(baseFileName, vf.voiceWindows, wordSize);

	if (result)
	{
		printf("Rec '%s' procesirana i sacuvana u bazu! Wee!\n", rec);
		saveWAV(wavFileName, &buf[0], num, 44100);
	}
	else
		printf("Imamo fail u snimanju reci '%s'\n", baseFileName);

	//free(word);
	//free(buf);
}

bool readFolder(const char * path, const char * ekstenzija, std::vector<char*> &fileList)
{
	DIR *dir;
	int i = 0;
	struct dirent *ent;
	int duzinaEkstenzije = 0;
	if (ekstenzija != NULL) duzinaEkstenzije = strlen(ekstenzija);

	if ((dir = opendir(path)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 && strlen(ent->d_name) >= 5)
			{
				if (duzinaEkstenzije == 0 || strcmp(ent->d_name + strlen(ent->d_name) - duzinaEkstenzije, ekstenzija) == 0)
				{
					char * fileName = (char *)malloc(strlen(ent->d_name) + 1);
					strcpy(fileName, ent->d_name);
					fileList.push_back(fileName);
					//	printf("%s\n", ent->d_name);
				}
			}
		}
		closedir(dir);
		return true;
	}
	else {
		/* could not open directory */
		perror("");
		printf("Coult not open directory!");
		return false;
	}
}

bool checkFolder(const char * path)
{
	DIR *dir;
	if ((dir = opendir(path)) != NULL)
	{
		closedir(dir);
		return true;
	}
	else
	{
		return false;
	}
}

/*name - naziv foldera*/
void calcMFCCforFolder(const char * name)
{
	std::vector<char*> fileList; //lista wav fajlova u folderu
	readFolder(name, ".wav", fileList);

	char command[256];
	strcpy(command, "mkdir ");
	strcat(command, name);
	strcat(command, "\\MFCC");
	system(command);

	for (int i = 0; i < fileList.size(); i++)
	{
		//Window* prozori = NULL;

		char path[200];
		strcpy(path, name);
		strcat(path, "/");
		strcat(path, fileList[i]);
		
		VoiceFile vf;
		vf.readFile(path);
		vf.processAudio(true);

		int brojProzora = vf.voiceWindows.size();
		if (brojProzora > 0)
		{
			printf("Nesto smo procitali iz '%s', %d prozora!\n", fileList[i], brojProzora);

			char outputFile[256];
			strcpy(outputFile, name);
			strcat(outputFile, "/MFCC/");
			strcat(outputFile, fileList[i]);
			outputFile[strlen(outputFile) - 4] = '\0'; // ubijanje ekstenzije
			strcat(outputFile, ".dat");

			saveWordMFCC(outputFile, vf.voiceWindows, brojProzora);

		}
		else
		{
			printf("NOPE %s\n", fileList[i]);
		}

		//if (prozori != NULL) free(prozori);
		//if (buf != NULL) free(buf);
	}
}

int main()
{
	//VoiceFile vf;
	//vf.readFile("bukis.wav");
	//vf.processAudio(false);
	//saveWordMFCC("bukisMFCC", vf.voiceWindows, vf.voiceWindows.size());
	//std::vector<Window> word;
	//readWordMFCC("bukisMFCC", word);
	
	
	HMM hmm;
	trainHMM(hmm);
	calcMFCCforFolder("trainBaza\\hello");
	system("pause");
}