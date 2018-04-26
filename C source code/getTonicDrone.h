#include<stdio.h>
#include<string.h>
#include<sndfile.h>
#include <time.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include<float.h>
#define PI	M_PI
#define TWOPI	(2.0*PI)

//libsnd dependency

/*
Fourier transform of samples.
Inputs -
Data:
real part of i-th data = data[2*i+1]
complex part of i-th data = data[2*i+2]
nn- a power of two and greater than number of elements
isign - Fourier if it is +1, Inverse if it is -1
*/
void four1(double data[], int nn, int isign);

/* Round up to next higher power of 2 (return x if it's already a power of 2).*/
int Pow2Roundup (int x);

/*return pitch for an array of fourier samplings*/
float CepPitch(double* EX,int winSize2, int rate,int maxim,int minim);

/*Get minimum maximum pitch given metadata*/
void GetMinMax(int metadata, int* minim,int* maxim);

/*Return pitch given Low Energy frames using ENMF*/
float PitchENMF(double** EXLowEng,int height,int winSize,int rate,int minim, int maxim);

/*Return pitch given Low Energy frames - Low Energy + Histogram method*/
int LowEnergyHist(double** EXLowEng,int winSize,int height,int rate, int maxim,int minim);

/*
GetTonicDrone
Inputs:
filename - Name of wav file
metadata - 1 male; 2 female; 3 Instrumental; 4 - not known; Default = 4
sec - No.of seconds of sampling to be taken
per - Percentage of low energy frames to be taken
algo - 1 for enmf, 2 for ceppitch
*/
int GetTonicDrone (char* filename, int metadata,int sec, int per, int algo);
