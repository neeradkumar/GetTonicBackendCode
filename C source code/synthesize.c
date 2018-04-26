#include "synthesize.h"

float vorbis_lpc_from_data(double *data,double *lpc,int n,int m)
{
	double *aut= malloc(sizeof(double)*(m+1));
	double error;
	double epsilon;
	int i,j;
	
	/*hamming window*/
	for(i=0;i<n;i++)
	{
		data[i] = data[i]*(0.54+0.46*cos(PI*(2*i-n)/(n-1)));
	}
	
	/* autocorrelation, p+1 lag coefficients */
	j=m+1;
	while(j--)
	{
		double d=0;
		for(i=j;i<n;i++) d+=data[i]*data[i-j];
		aut[j]=d;
	}

	/* Generate lpc coefficients from autocorr values */

	/* set our noise floor to about -100dB */
	error=aut[0] * (1. + 1e-10);
	epsilon=1e-9*aut[0]+1e-10;

	for(i=0;i<m;i++)
	{
		double r= -aut[i+1];
		if(error<epsilon)
		{
			memset(lpc+i,0,(m-i)*sizeof(*lpc));
			break;
		}

	/* Sum up this iteration's reflection coefficient*/

		for(j=0;j<i;j++) r-=lpc[j]*aut[i-j];
		r/=error;

	/* Update LPC coefficients and total error */

		lpc[i]=r;
		for(j=0;j<i/2;j++)
		{
			double tmp=lpc[j];
			lpc[j]+=r*lpc[i-1-j];
			lpc[i-1-j]+=r*tmp;
		}
		if(i&1)lpc[j]+=lpc[j]*r;
		error*=1.-r*r;
	}
	free(aut);
	
	return error;
}

double drand()   /* uniform distribution, (0..1] */
{
  return (rand()+1.0)/(RAND_MAX+1.0);
}

/*
Inputs - 
wav filename
corresponding pitch filename (frame Shift = sr/100)
Tonic to be changed to
lpOrder for lpc's
flag- 1 if given file is pitch file
	0 if cent file
	
Output - 
*/
void synthesize (char* filename, char* pitchfile,float f0,int lpOrder,int flag)
{
	SNDFILE *sFile;
	SF_INFO wavInfo;
	int numItems;
	double *wavFile;
	int i,j,k,tonic;
	wavInfo.format = 0;
	if(flag==1) tonic = GetTonicDrone(filename,4,90,10,1);
	/* Open the WAV file. */

	sFile = sf_open(filename,SFM_READ,&wavInfo);
	if (sFile == NULL)
	{
		printf("Failed to open the file.\n");
		exit(-1);
	}
	numItems = wavInfo.frames*wavInfo.channels;
	printf("numItems %d\n",numItems);
	/* Allocate space for the data to be read, then read it. */

	wavFile = malloc(numItems*sizeof(double));
	
	int num = sf_read_double(sFile,wavFile,numItems);
	sf_close(sFile);
	int sr = wavInfo.samplerate;
	int frameSize = sr/40, frameShift = sr/100;
	
	/*Compute energy (eng[]) and pitch (pitch[]) (new) of each frame*/
	int numFrames=(int)(numItems/frameShift)+1;
	int frameShift2 = Pow2Roundup(frameShift);
	double* energy = malloc(numFrames*sizeof(double));
	double* energy1 = malloc(numFrames*sizeof(double));
	double* pitch = malloc(numFrames*sizeof(double));
	double* EX = malloc((2*frameShift2+1)*sizeof(double));
	double sum=0;
	
	FILE * fp;
	char * line = NULL;
	size_t len = 8;
	ssize_t read;
	fp = fopen(pitchfile, "r");
	if (fp == NULL) exit(EXIT_FAILURE);
	
	printf("nuFrames %d \n",numFrames);
	printf("frameShift %d \n",frameShift);
	/*Calculate energy and pitch*/
	for(i=0,k=0;i<numItems-frameShift && k<numFrames ;i+=frameShift,k++)
	{
		energy[k]=0;
		for(j=0;j<frameShift;j++)
		{
			energy[k] += wavFile[i+j]*wavFile[i+j];	
		}
		energy[k] = sqrt(energy[k]/frameShift);
		//replace with a pitch algorithm, should give zero for unvoiced
		read = getline(&line, &len, fp);
		
		if(flag==1) pitch[k] = f0*atof(line)/tonic;
		else pitch[k] = f0*pow(2,(atof(line)/1200));
	}
	if(line) free(line);
	close(fp);
	free(EX);
	/*create impulse*/
	int ind, hop, frSize, frame=0;
	int z=-1,nonz=0;	
	float t1 = 0.004, t2 = 0.002;
	double* impulseSig = malloc(numItems*sizeof(double));
	while  (frame < numItems)
	{
		ind = (int)(frame/frameShift);
		//remove below sentence for uniform energy
		//energy[ind] =0.7;
		if (pitch[ind] == 0)
		{
			for(i=frame;i<frameShift*(ind+1);i++) impulseSig[i] = sqrt(energy[ind])*drand() ;
			frame=frameShift*(ind+1);
			
		}
		else
		{
			
			hop=(int)(sr/pitch[ind]);
			frame=frame+hop;
			if((frame+(t1+t2)*sr) > numItems) break;
			for(i=0;i<t1*sr;i++)
			{
			  	impulseSig[frame+i] = sqrt(energy[ind])*0.5*(1-cos(PI*i/(t1*sr)));
			}
			for(i=(int)(t1*sr)+1;i<(t1+t2)*sr;i++)
			{
				impulseSig[frame+i] = sqrt(energy[ind])*cos(0.5*PI*((double)i/sr - t1)/t2);
			}	
		}	
	}

	/*lpSyn*/
	double lpc[lpOrder], Err;
	double max = 0.0;
	double* dataFrame = malloc(frameSize*sizeof(double));
	for (i=0;i<numItems-frameSize;i+=frameShift)
	{
		for(j=0;j<frameSize;j++) dataFrame[j] = wavFile[i+j];
		for(j=0;j<lpOrder;j++) lpc[j]=0.0;		
		vorbis_lpc_from_data(dataFrame,lpc,frameSize,lpOrder);
		for(j=0;j<frameShift;j++)
		{
			Err = 0.0;
			for(k=0;k<lpOrder;k++)
			{
				if( (i+j-k-1) < 0) break;
				Err = Err+ (lpc[k])*(wavFile[i+j-k-1]);
			}
			wavFile[i+j] = impulseSig[i+j]-Err;
			if(wavFile[i+j]>max) max = wavFile[i+j];
		}	
	}
	for(i=0;i<numItems;i++) wavFile[i]/= 1.02*max;
	
	free(impulseSig);
	/*write to wav*/
	char dest[50],src[10];
	strcpy(dest,filename);
	strcpy(src,"_out.wav");

	char *lastdot = strrchr (dest, '.');
	if (lastdot != NULL) *lastdot = '\0';
	strcat(dest,src);

	SNDFILE *outfile;
	if (! (outfile = sf_open (dest, SFM_WRITE, &wavInfo))) 
	{
		printf ("Not able to open output file %s \n", dest) ;
		return;
	};
	sf_write_double (outfile, wavFile , numItems) ;
	sf_close (outfile) ;
	free(wavFile);

}
int main()
{
	synthesize ("RK_Varali3_NF02RK.wav","RK_Varali3_NF02RK.wav.spl.mel.441f0.cent",140,12,0);
}
