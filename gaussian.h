//
// AMZSmartBlur avisynth filter
// By Rodrigo Braz Monteiro (a.k.a. ArchMage ZeratuL) - 2005
//
// This file has been given to the public domain.
// You may freely modify and redistribute this source.
// While you're not required to do so, if you use this source
// for your own projects, it'd be appreciated if credit was given to me.
//

#pragma once

class GaussianMask {
public:
	float **mask;
	bool ready;
	int n;
	float sum;

	GaussianMask();
	~GaussianMask();
	void Generate(double rho);
};
