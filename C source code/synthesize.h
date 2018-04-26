#include "getTonicDrone.h"

/*
Input - 
data - samplings array
n - No.of samplings
m - lpOrder

Output -
lpc - m lpc coefficients
*/
float vorbis_lpc_from_data(double *data,double *lpc,int n,int m);

double drand();   /* uniform distribution, (0..1] */

/*
Inputs - 
wav filename
corresponding pitch filename (frame Shift = sr/100)
Tonic to be changed to
lpOrder for lpc's
flag- 1 if given file is pitch file
	0 if cent file
*/
void synthesize (char* filename, char* pitchfile,float f0,int lpOrder,int flag);
