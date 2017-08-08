#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include <Windows.h>

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
//#define SAMPLE_RATE  (44100)
//#define FRAMES_PER_BUFFER (512)
//#define NUM_SECONDS     (5)
//#define NUM_CHANNELS    (1)

typedef struct
{
	int          frameIndex;  /* Index into sample array. */
	int          maxFrameIndex;
	float      *recordedSamples;
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	paTestData *data = (paTestData*)userData;
	const float *rptr = (const float*)inputBuffer;
	float *wptr = &data->recordedSamples[data->frameIndex];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	(void)outputBuffer; /* Prevent unused variable warnings. */
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	if (framesLeft < framesPerBuffer)
	{
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if (inputBuffer == NULL)
	{
		for (i = 0; i<framesToCalc; i++)
		{
			*wptr++;
			//if(NUM_CHANNELS == 2) *wptr++;
		}
	}
	else
	{
		for (i = 0; i<framesToCalc; i++)
		{
			*wptr++ = *rptr++;  /* left */
								//if(NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
		}
	}

	data->frameIndex += framesToCalc;
	return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	paTestData *data = (paTestData*)userData;
	float *rptr = &data->recordedSamples[data->frameIndex];
	float *wptr = (float*)outputBuffer;
	unsigned int i;
	int finished;
	unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

	(void)inputBuffer; /* Prevent unused variable warnings. */
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	if (framesLeft < framesPerBuffer)
	{
		/* final buffer... */
		for (i = 0; i<framesLeft; i++)
		{
			*wptr++ = *rptr++;  /* left */
								//if(NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
		}
		for (; i<framesPerBuffer; i++)
		{
			*wptr++ = 0;  /* left */
						  //if(NUM_CHANNELS == 2) *wptr++ = 0;  /* right */
		}
		data->frameIndex += framesLeft;
		finished = paComplete;
	}
	else
	{
		for (i = 0; i<framesPerBuffer; i++)
		{
			*wptr++ = *rptr++;  /* left */
								//if(NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
		}
		data->frameIndex += framesPerBuffer;
		finished = paContinue;
	}
	return finished;
}

bool PAH_RecordMonoSamples(double* buffer, int maxSamples, int sampleRate, bool interruptable, int* samplesRecorded)
{
	bool ok = true;
	*samplesRecorded = 0;

	PaStreamParameters  inputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	paTestData          data;
	int                 i;
	int                 totalFrames;
	int                 numSamples;
	int                 numBytes;

	printf("Recording start...\n");
	fflush(stdout);

	data.maxFrameIndex = totalFrames = maxSamples; /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames;
	numBytes = numSamples * sizeof(float);
	data.recordedSamples = (float *)malloc(numBytes); /* From now on, recordedSamples is initialised. */
	if (data.recordedSamples == NULL)
	{
		printf("Could not allocate record array.\n");
		fflush(stdout);
		ok = false;
		goto done;
	}
	for (i = 0; i<numSamples; i++) data.recordedSamples[i] = 0;

	err = Pa_Initialize();
	if (err != paNoError) goto done;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice)
	{
		fprintf(stderr, "Error: No default input device.\n");
		goto done;
	}
	inputParameters.channelCount = 1;                    /* mono input */
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		sampleRate,
		512,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		recordCallback,
		&data);
	if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto done;

	printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

	if (interruptable)
	{
		while (GetAsyncKeyState(VK_RETURN))
			Pa_Sleep(10);
	}

	while ((err = Pa_IsStreamActive(stream)) == 1)
	{
		Pa_Sleep(10);

		if (interruptable)
		{
			// Ako je TAB pritisnut
			if (GetAsyncKeyState(VK_RETURN))
			{
				printf("Recording stopped at index = %d\n", data.frameIndex);
				numSamples = data.frameIndex;
				data.maxFrameIndex = data.frameIndex;
				fflush(stdout);

				err = Pa_StopStream(stream);
				if (err != paNoError) goto done;
				break;
			}
		}
		//printf("index = %d\n", data.frameIndex); fflush(stdout);
	}
	if (err < 0) goto done;

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto done;

done:
	Pa_Terminate();
	if (data.recordedSamples)       /* Sure it is NULL or valid. */
	{
		int startSample = sampleRate / 10;
		for (int i = 0; i < data.maxFrameIndex - startSample; i++)
			buffer[i] = (double)data.recordedSamples[i + startSample];

		free(data.recordedSamples);
		*samplesRecorded = data.maxFrameIndex - startSample;
	}
	if (err != paNoError)
	{
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		err = 1;          /* Always return 0 or 1, but no other return codes. */
		ok = false;
	}

	return ok;
}

bool PAH_PlayMonoSamples(double* buffer, int numSamples, int sampleRate, bool interruptable)
{
	PaStreamParameters  outputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	paTestData          data;
	int                 i;
	int                 totalFrames;
	int                 numBytes;
	float               max, val;
	double              average;

	printf("Playback start...\n");
	fflush(stdout);

	data.maxFrameIndex = totalFrames = numSamples; /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames * 1;
	numBytes = numSamples * sizeof(float);
	data.recordedSamples = (float *)malloc(numBytes); /* From now on, recordedSamples is initialised. */
	if (data.recordedSamples == NULL)
	{
		printf("Could not allocate record array.\n");
		goto done;
	}

	for (i = 0; i<numSamples; i++) data.recordedSamples[i] = (float)buffer[i];

	err = Pa_Initialize();
	if (err != paNoError) goto done;

	/* Playback recorded data.  -------------------------------------------- */
	data.frameIndex = 0;

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice)
	{
		fprintf(stderr, "Error: No default output device.\n");
		goto done;
	}
	outputParameters.channelCount = 1;                     /* mono output */
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	printf("\n=== Now playing back. ===\n"); fflush(stdout);
	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		sampleRate,
		512,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		playCallback,
		&data);
	if (err != paNoError) goto done;

	if (stream)
	{
		err = Pa_StartStream(stream);
		if (err != paNoError) goto done;

		printf("Waiting for playback to finish.\n"); fflush(stdout);

		while ((err = Pa_IsStreamActive(stream)) == 1)
		{
			Pa_Sleep(10);
			if (interruptable)
			{
				// Ako je TAB pritisnut
				if (GetAsyncKeyState(VK_RETURN))
				{
					printf("Playback stopped at index = %d\n", data.frameIndex);
					fflush(stdout);

					err = Pa_StopStream(stream);
					if (err != paNoError) goto done;
					break;
				}
			}
		}
		if (err < 0) goto done;

		err = Pa_CloseStream(stream);
		if (err != paNoError) goto done;

		printf("Done.\n"); fflush(stdout);
	}

done:
	Pa_Terminate();
	if (data.recordedSamples)       /* Sure it is NULL or valid. */
		free(data.recordedSamples);
	if (err != paNoError)
	{
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}

	return err == paNoError;
}