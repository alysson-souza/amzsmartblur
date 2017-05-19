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

// Class definition
class SmartBlur: public GenericVideoFilter {
private:
	IScriptEnvironment* env;
	PVideoFrame GetOutputFrame(int n);
	double maxrho;
	double exponent;
	double edgeBlur;
	bool showedges;
	int nthreads;

public:
	SmartBlur(PClip _child, IScriptEnvironment* env, double _exponent, double _maxrho, double _edgeblur, int _nthreads, bool _showedges);
	~SmartBlur();

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};
