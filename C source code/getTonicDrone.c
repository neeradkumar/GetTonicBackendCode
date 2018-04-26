#include "getTonicDrone.h"
/*
Fourier transform of samples.
Inputs -
Data:
real part of i-th data = data[2*i+1]
complex part of i-th data = data[2*i+2]
nn- a power of two and greater than number of elements
isign - Fourier if it is +1, Inverse if it is -1
*/
void four1(double data[], int nn, int isign)
{
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
	if (j > i) {
	    tempr = data[j];     data[j] = data[i];     data[i] = tempr;
	    tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
	}
	m = n >> 1;
	while (m >= 2 && j > m) {
	    j -= m;
	    m >>= 1;
	}
	j += m;
    }
    mmax = 2;
    while (n > mmax) {
	istep = 2*mmax;
	theta = TWOPI/(isign*mmax);
	wtemp = sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi = sin(theta);
	wr = 1.0;
	wi = 0.0;
	for (m = 1; m < mmax; m += 2) {
	    for (i = m; i <= n; i += istep) {
		j =i + mmax;
		tempr = wr*data[j]   - wi*data[j+1];
		tempi = wr*data[j+1] + wi*data[j];
		data[j]   = data[i]   - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp = wr)*wpr - wi*wpi + wr;
	    wi = wi*wpr + wtemp*wpi + wi;
	}
	mmax = istep;
    }
}



/* Round up to next higher power of 2 (return x if it's already a power of 2).*/
int pow2Roundup (int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

/*return pitch for an array of fourier samplings*/
float cepPitch(double* EX,int winSize2, int rate,int maxim,int minim)
{
	int j;
	int ms2 = (rate/maxim); 
	int ms20 = (rate/minim);
	int maxIndex;
	double maxValue = 0.0;
	for(j=1;j<winSize2 ;j+=2)
	{
		EX[j] = log(EX[j]*EX[j] + EX[j+1]*EX[j+1]+DBL_MIN) ;
		EX[j+1] = 0.0;
	}
	four1(EX,(winSize2-1)/2,-1);
	for(j=1; j<winSize2; j++)
	{
		EX[j] /= (winSize2-1)/2;
	}

	maxValue = 0.0;
	for(j=ms2;j<=ms20;j++)
	{
		if((EX[2*j+1]*EX[2*j+1] + EX[2*j+2]*EX[2*j+2])> maxValue)
		{
			maxValue = (EX[2*j+1]*EX[2*j+1] + EX[2*j+2]*EX[2*j+2]);
			maxIndex = j;
		}
	}
	return (rate/(float)maxIndex);
}

/*Get minimum maximum pitch given metadata*/
void GetMinMax(int metadata, int* minim,int* maxim)
{
	switch(metadata)
	{
		case 1:
		*minim =  100;
		*maxim =  180;
		break;
		case 2:
		*minim =  140;
		*maxim =  250;
		break;
		case 3:
		*minim =  120;
		*maxim =  215;
		break;
		case 4:
		*minim =  100;
		*maxim =  250;
		break;
		case 5:
		*minim =  60;
		*maxim =  420;
		break;
		case 6:
		*minim = 50;
		*maxim = 90;
		break;
		case 7:
		*minim = 70;
		*maxim = 125;
		break;
		case 8:
		*minim = 60;
		*maxim = 108;
		break;
		case 9:
		*minim = 50;
		*maxim = 250;
		break;
		case 10:
		*minim = 50;
		*maxim = 125;
		break;
	}
}

/*Return pitch given Low Energy frames using ENMF*/

float PitchENMF(double** EXLowEng,int height,int winSize,int rate,int minim, int maxim)
{
	double* W = (double *)malloc(winSize*sizeof(double));
	memset(W, 0,sizeof(double)*winSize);
	int i,j;
	/*Mean of magnitudes*/
	for(j=1;j<winSize;j+=2)
	{
		for(i=0;i<height;i++)
		{
			
			W[j] += sqrt(EXLowEng[i][j]*EXLowEng[i][j] + EXLowEng[i][j+1]*EXLowEng[i][j+1]);
		}
		W[j+1] = 0.0;
	}
	/*normalise*/
	for(j=1;j<winSize;j++)
	{
		W[j] /= 2*0.5*(winSize-1);
	}
	float pitch = cepPitch(W,winSize, rate, maxim, minim);
	free(W);
	return pitch;
}

/*Return pitch given Low Energy frames - Low Energy + Histogram method*/

int LowEnergyHist(double** EXLowEng,int winSize,int height,int rate, int maxim,int minim)
{
	int *histogram = (int*)malloc(600*sizeof(int));
	memset(histogram, 0,sizeof(int)*600);
	
	float pitch;
	int i;
	for(i=0;i<height;i++)
	{
		pitch = cepPitch(EXLowEng[i], winSize, rate, maxim, minim);
		histogram[(int)pitch] += 1;
	}
	int maxValue = 0;
	int maxIndex = 0;
	for(i=0;i<600;i++)
	{
		if(histogram[i]>maxValue)
		{
			maxValue = histogram[i];
			maxIndex = i;
		}
	}
	free(histogram);
	return maxIndex;
}


/*
GetTonicDrone
Inputs:
filename - Name of wav file
metadata - 1 male; 2 female; 3 Instrumental; 4 - not known; Default = 4
sec - No.of seconds of sampling to be taken
per - Percentage of low energy frames to be taken
algo - 1 for enmf, 2 for ceppitch
*/
int GetTonicDrone (char* filename, int metadata,int sec, int per, int algo)
{
	int minim, maxim;
	GetMinMax(metadata, &minim,&maxim);

	SNDFILE *sFile;
	SF_INFO wavInfo;
	int numItems;
	double *wavFile;
	int i,j,k;
	wavInfo.format = 0;
	/* Open the WAV file. */

	printf("Opening file\n");
	sFile = sf_open(filename,SFM_READ,&wavInfo);
	if (sFile == NULL)
	{
		printf("Failed to open the file.\n");
		exit(-1);
	}
	numItems = wavInfo.frames*wavInfo.channels;

	/* Allocate space for the data to be read, then read it. */

	printf("Reading file\n");
	wavFile = (double *) malloc(numItems*sizeof(double));
	int num = sf_read_double(sFile,wavFile,numItems);
	sf_close(sFile);
		
	/* Take the required duration and store it in ex */
	
	double *ex;
	int exSize, r;
	int dur =  (int)(numItems/wavInfo.samplerate) - sec;
	if (dur > 1)
	{
		srand(time(NULL));
		int r = (rand())%dur;
		//random number to determine starting point
		exSize = sec*(wavInfo.samplerate);
		ex = (double *) malloc(exSize*sizeof(double));
		for(i=0;i<exSize;i++)	ex[i] = wavFile[i+r*wavInfo.samplerate];
	}
	else
	{
		exSize = numItems;
		ex = (double *) malloc(exSize*sizeof(double)); 
		for(i=0;i<exSize;i++)	ex[i] = wavFile[i];
	}
	free(wavFile);
	/* Split into frames and store in EX */
	printf("Store required frames\n");
	int winSize = 1+ 2*(int)(2048*wavInfo.samplerate/44100) ;
	int winSize2 = 1+2*pow2Roundup((int)(2048*wavInfo.samplerate/44100));
	
	int temp = (winSize-1)/2 - (wavInfo.samplerate/100);
	
	int height = 2 + (exSize-winSize)*100/wavInfo.samplerate ;
	
	double **EX = malloc(height*sizeof(double *));
	for(i=0;i<height;i++) 
	{
		EX[i] = malloc((winSize2+1)*sizeof(double));
		memset(EX[i], 0,sizeof(double)*winSize2);
	}
	

	for(i=-temp ,j=0;i<exSize && j<height;j++)
	{
		for(k=1;k<winSize;k=k+2,i++)	//alternate numbers imaginary part
		{
			if(i<0) EX[j][k] = 0;
			else EX[j][k] = ex[i];
		}
		i = i- temp;
	}
	free(ex);

	printf("Calculation begins\n");
	/* Apply FFT, calculate energy using energy parsevals theorem, 
	store in first element of each frame */
	for(i=0;i<height;i++) 
	{
		four1(EX[i],(winSize2-1)/2,1);
		EX[i][0] = 0.0;
		for(j=1;j<winSize2;j++)	EX[i][0] += EX[i][j]*EX[i][j];
	}	

	/*Take frames with least energy*/
	int newHeight = (per*height/100.0);
	
	double **EXLowEng = (double **) malloc(newHeight*sizeof(double *));
	for(i=0;i<newHeight;i++) 
	{
		EXLowEng[i] = (double *) malloc((winSize2+1)*sizeof(double));
		memset(EXLowEng[i], 0,sizeof(double)*winSize2);
	}
	double prevLeast = 0.0, nowLeast;
	int newIndex;
	for(i=0;i<newHeight;i++)
	{
		nowLeast = DBL_MAX; //infinity
		for(j=0;j<height;j++)
		{
			if(EX[j][0]>prevLeast && EX[j][0]<nowLeast)
			//also leave out zero energy frames
			{
				newIndex = j;
				nowLeast = EX[j][0];
			}
		}
		prevLeast = EX[newIndex][0];
		for(j=1;j<winSize2;j++)
		{
			EXLowEng[i][j] = EX[newIndex][j];
		}
	}
	
	for(i=0;i<height;i++) 
	{
		free(EX[i]);
	}
	free(EX);
	
	/*ENMF or LowEnergyHist*/
	int pitch;
	if(algo == 1) pitch = PitchENMF(EXLowEng, newHeight, winSize2, wavInfo.samplerate, minim, maxim) ;
	
	else pitch = LowEnergyHist(EXLowEng, winSize2, newHeight, wavInfo.samplerate, maxim, minim);
	
	for(i=0;i<newHeight;i++) 
	{
		free(EXLowEng[i]);
	}
	free(EXLowEng);
	
	return pitch;
}

/*int main()
{
	printf("%d\n",GetTonicDrone ("uploads/rec.wav",1,90,10,1));
	printf("%d\n",GetTonicDrone ("3-seethapathe-131hz.wav",1,90,10,2));
}*/

