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


// This is the function that created the filter, when the filter has been called.
AVSValue __cdecl Create_SmartBlur(AVSValue args, void* user_data, IScriptEnvironment* env) {
	return new SmartBlur(args[0].AsClip(), env, args[1].AsFloat(6.0), args[2].AsFloat(2.5), args[3].AsFloat(3.0), args[4].AsInt(1), args[5].AsBool(false));  
}


// The following function is the function that actually registers the filter in AviSynth
// It is called automatically, when the plugin is loaded to see which functions this filter contains.
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
    env->AddFunction("AMZSmartBlur", "c[exponent]f[strength]f[edgeblur]f[threads]i[showedges]b", Create_SmartBlur, 0);
    return "`Spatiotemporal SmartBlur' by Rodrigo Braz Monteiro (ArchMage ZeratuL)";
}