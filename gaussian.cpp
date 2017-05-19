//
// AMZSmartBlur avisynth filter
// By Rodrigo Braz Monteiro (a.k.a. ArchMage ZeratuL) - 2005
//
// This file has been given to the public domain.
// You may freely modify and redistribute this source.
// While you're not required to do so, if you use this source
// for your own projects, it'd be appreciated if credit was given to me.
//

#define _USE_MATH_DEFINES

#include "gaussian.h"
#include <math.h>


//
// Constructor
//
GaussianMask::GaussianMask () {
	ready = false;
}


//
// Destructor
//
GaussianMask::~GaussianMask () {
	if (ready) {
		for (int i=0;i<n;i++) delete [] mask[i];
		delete [] mask;
	}
	ready = false;
}


//
// Generates a mask
//
void GaussianMask::Generate(double rho) {
	if (ready) return;
	n = (int(rho+1)*2)+1;
	//n = (int(2*rho)*2)-1;
	if (n < 3) n = 3;

	// Allocate
	mask = new float*[n];
	for (int i=0;i<n;i++) mask[i] = new float[n];
	double **temp = new double*[n];
	for (int i=0;i<n;i++) temp[i] = new double[n];

	// Generate
	ready = true;
	double r;
	int mid = n/2;
	double max = 0;
	for (int i=0;i<n;i++) {
		for (int j=0;j<n;j++) {
			r = sqrt(double((i-mid)*(i-mid) + (j-mid)*(j-mid)));
			temp[i][j] = exp(-pow(r/rho,2)/2) / sqrt(2*rho*M_PI);
			if (temp[i][j] > max) max = temp[i][j];
		}
	}

	// Normalize
	sum = 0;
	double normal = 32/max;
	for (int i=0;i<n;i++) {
		for (int j=0;j<n;j++) {
			mask[i][j] = (float) (temp[i][j]*normal);
			sum += mask[i][j];
		}
	}

	// Clean up temp
	for (int i=0;i<n;i++) delete [] temp[i];
	delete [] temp;
}
