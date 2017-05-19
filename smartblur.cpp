//
// AMZSmartBlur avisynth filter
// By Rodrigo Braz Monteiro (a.k.a. ArchMage ZeratuL) - 2005
//
// This file has been given to the public domain.
// You may freely modify and redistribute this source.
// While you're not required to do so, if you use this source
// for your own projects, it'd be appreciated if credit was given to me.
//

#include <windows.h>
#include "avisynth.h"
#include "smartblur.h"


//
// Constructor
//
SmartBlur::SmartBlur(PClip _child, IScriptEnvironment* _env, double _exponent, double _maxrho, double _edgeblur, int _nthreads, bool _showedges) : GenericVideoFilter(_child) {
	exponent = _exponent;
	maxrho = _maxrho;
	env = _env;
	showedges = _showedges;
	nthreads = _nthreads;
	edgeBlur = _edgeblur;
}


//
// Destructor
//
SmartBlur::~SmartBlur() {
}


//
// Getframe
// Called by Avisynth for each frame
//
PVideoFrame __stdcall SmartBlur::GetFrame(int n, IScriptEnvironment* env) {
	if (!vi.IsYV12()) {
		env->ThrowError("AMZSmartBlur requires YV12 input.");
		return NULL;
	}
	return GetOutputFrame(n);
	//return child->GetFrame(n,env);
}
