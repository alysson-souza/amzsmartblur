//
// AMZSmartBlur avisynth filter
// By Rodrigo Braz Monteiro (a.k.a. ArchMage ZeratuL) - 2005
//
// This file has been given to the public domain.
// You may freely modify and redistribute this source.
// While you're not required to do so, if you use this source
// for your own projects, it'd be appreciated if credit was given to me.
//

// Disable stupid boost warnings
#pragma warning(disable: 4511 4512 4251 4275)


// Includes
#include <windows.h>
#include "avisynth.h"
#include "smartblur.h"
#include "gaussian.h"
#include <math.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>


//
// Defines
//
#define MID(a,b,c) min(c,max(b,a))


//
// Gaussian list
//
GaussianMask Masks[256];
GaussianMask Masks_uv[256];


//
// AviSynth mutex
//
boost::mutex AviSynthMutex;


//
// Write edges
//
void WriteEdges(const UCHAR *src,UCHAR *dst,UINT w,UINT h,UINT pitch) {
	UCHAR *dst_y = dst;
	const UCHAR *src_y1 = src;
	const UCHAR *src_y2 = src + pitch;
	const UCHAR *src_y3 = src + 2*pitch;

	int gx,gy;

	// First row
	for (UINT x=0;x<w;x++) {
		*(dst_y+x) = 0;
	}
	dst_y += pitch+1;

	// Middle rows
	for (UINT y=1;y<h-1;y++) {
		// First pixel
		*(dst_y-1) = 0;

		// Middle pixels
		for (UINT x=1;x<w-1;x++) {
			gx = *(src_y1+x+2) + *(src_y3+x+2) - (*(src_y1+x) + *(src_y3+x)) + 2*(*(src_y2+x+2) - *(src_y2+x));
			gy = *(src_y1+x) + 2*(*(src_y1+x+1)) + *(src_y1+x+2) - (*(src_y3+x) + 2*(*(src_y3+x+1)) + *(src_y3+x+2));
			*(dst_y+x) = 255-MID(0,int(abs(gx) + abs(gy))*3-30,255);
			//else *(dst_y+x) = 255-MID(0,int(sqrt(float(gx*gx + gy*gy))/2),255);
		}

		// Last pixel
		*(dst_y+w-2) = 0;

		// Raise pitch
		dst_y += pitch;
		src_y1 += pitch;
		src_y2 += pitch;
		src_y3 += pitch;
	}

	// Last row
	for (UINT x=0;x<w;x++) {
		*(dst_y+x) = 0;
	}
}


//
// Gaussian blurs the mask
//
void GaussianCopy(const UCHAR *src,UCHAR *dst,int w,int h,UINT pitch,double rho) {
	UCHAR *dst_y = dst;
	const UCHAR *src_y = src;

	// Generate mask
	GaussianMask Mask;
	Mask.Generate(rho);

	UINT i,j;
	int posx,posy;
	float value;
	UINT pitchchange;
	UINT n = Mask.n;
	UINT mid = n/2;

	for (int y=0;y<h;y++) {
		for (int x=w;--x>=0;) {
			value = 0;
			// Convolute
			for (j=0;j<n;j++) {
				// Get Y
				posy = y+j-mid;
				if (posy < 0) posy = 0;
				if (posy >= h) posy = h-1;
				pitchchange = pitch*posy;

				for (i=0;i<n;i++) {
					// Get X
					posx = x+i-mid;
					if (posx < 0) posx = 0;
					if (posx >= w) posx = w-1;

					// Add value
					value += float(*(src_y+pitchchange+posx)) * Mask.mask[i][j];
				}
			}

			// Ponderate and sum
			*(dst_y+x) = MID(0,int(value / Mask.sum + 0.5),255);
		}

		// Raise pitch
		dst_y += pitch;
	}
}


//
// Gaussian blurs the luma, according to mask
//
void MaskedGaussianCopyLuma(const UCHAR *src,UCHAR *dst,const UCHAR *ref,int w,int h,UINT pitch,int relative) {
	UCHAR *dst_y = dst;
	const UCHAR *ref_y = ref;

	int i,j;
	int posx,posy;
	float value;
	UINT v;
	UINT pitchchange;
	int n,mid;

	for (int y=0;y<h;y++) {
		for (int x=w;--x>=0;) {
			v = *(ref_y+x);
			value = 0;
			n = Masks[v].n;
			mid = n/2;

			// Convolute
			posy = y-mid-1;
			for (j=n;--j>=0;) {
				// Get Y
				//posy = y+j-mid;
				++posy;
				if (relative == 0 && posy < 0) posy = 0;
				if (relative == 2 && posy >= h) posy = h-1;
				pitchchange = pitch*posy;

				posx = x-mid-1;
				for (i=n;--i>=0;) {
					// Get X
					//posx = x+i-mid;
					++posx;
					if (posx < 0) posx = 0;
					if (posx >= w) posx = w-1;

					// Add value
					value += float(*(src+pitchchange+posx)) * Masks[v].mask[i][j];
				}
			}

			// Ponderate and sum
			*(dst_y+x) = MID(0,int(value / Masks[v].sum + 0.5),255);
		}

		// Raise pitch
		dst_y += pitch;
		ref_y += pitch;
	}
}


//
// Gaussian blurs the chroma, according to mask
//
void MaskedGaussianCopyChroma(const UCHAR *src,UCHAR *dst,const UCHAR *ref,int w,int h,UINT pitch,UINT lumapitch,int relative) {
	UCHAR *dst_y = dst;
	const UCHAR *ref_y = ref;

	int i,j;
	int posx,posy;
	float value;
	UINT v;
	UINT pitchchange;
	int n,mid;

	for (int y=0;y<h;y++) {
		for (int x=w;--x>=0;) {
			v = *(ref_y+(2*x));
			value = 0;
			n = Masks_uv[v].n;
			mid = n/2;

			// Convolute
			posy = y-mid-1;
			for (j=n;--j>=0;) {
				// Get Y
				//posy = y+j-mid;
				++posy;
				if (relative == 0 && posy < 0) posy = 0;
				if (relative == 2 && posy >= h) posy = h-1;
				pitchchange = pitch*posy;

				// Prepare X
				posx = x-mid-1;

				for (i=n;--i>=0;) {
					// Get X
					//posx = x+i-mid;
					++posx;
					if (posx < 0) posx = 0;
					if (posx >= w) posx = w-1;

					// Add value
					value += *(src+pitchchange+posx) * Masks_uv[v].mask[i][j];
				}
			}

			// Ponderate and sum
			*(dst_y+x) = MID(0,int(value / Masks_uv[v].sum + 0.5),255);
		}

		// Raise pitch
		dst_y += pitch;
		ref_y += lumapitch*2;
	}
}


//
// Gives stuff for a thread to do
//
void ThreadProcess(PVideoFrame &src,const UCHAR *src_ptr[3],UCHAR *dst_ptr[3],const UCHAR *src_mask,UCHAR *mask,int nthreads,int thisthread,double edgeBlur) {
	// Create variables
	int w,h,pitch,w_uv,h_uv,pitch_uv;
	int this_h,this_h_uv,start_pitch,start_pitch_uv;
	UCHAR *dst_y,*dst_u,*dst_v;
	const UCHAR *src_y,*src_u,*src_v;

	// Gather Avisynth crap
	{
		boost::mutex::scoped_lock AVS(AviSynthMutex);
		w = src->GetRowSize(PLANAR_Y);
		h = src->GetHeight(PLANAR_Y);
		pitch = src->GetPitch(PLANAR_Y);
		w_uv = src->GetRowSize(PLANAR_U);
		h_uv = src->GetHeight(PLANAR_U);
		pitch_uv = src->GetPitch(PLANAR_U);
	}

	// Calculate helper values
	this_h = h / nthreads;
	this_h_uv = h_uv / nthreads;
	start_pitch = this_h * thisthread * pitch;
	start_pitch_uv = this_h_uv * thisthread * pitch_uv;

	// Get pointers
	dst_y = dst_ptr[0]+start_pitch;
	dst_u = dst_ptr[1]+start_pitch_uv;
	dst_v = dst_ptr[2]+start_pitch_uv;
	src_y = src_ptr[0]+start_pitch;
	src_u = src_ptr[1]+start_pitch_uv;
	src_v = src_ptr[2]+start_pitch_uv;

	// Gaussian copies the mask
	GaussianCopy(src_mask+start_pitch,mask+start_pitch,w,this_h,pitch,edgeBlur);

	// Determine relative position of this thread
	int relative = 1;
	if (thisthread == 0) relative = 0;
	else if (thisthread == nthreads-1) relative = 2;

	// Apply smart gaussian blur
	MaskedGaussianCopyLuma(src_y,dst_y,mask+start_pitch,w,this_h,pitch,relative);
	MaskedGaussianCopyChroma(src_u,dst_u,mask+start_pitch,w_uv,this_h_uv,pitch_uv,pitch,relative);
	MaskedGaussianCopyChroma(src_v,dst_v,mask+start_pitch,w_uv,this_h_uv,pitch_uv,pitch,relative);
}


//
// GetOutputFrame
//
PVideoFrame SmartBlur::GetOutputFrame(int n) {
	PVideoFrame dst_frame = env->NewVideoFrame(vi);
	PVideoFrame src_frame = child->GetFrame(n,env);

	UINT pitch,w,h,pitch_uv,w_uv,h_uv;
	pitch = src_frame->GetPitch(PLANAR_Y);
	w = src_frame->GetRowSize(PLANAR_Y);
	h = src_frame->GetHeight(PLANAR_Y);
	pitch_uv = src_frame->GetPitch(PLANAR_U);
	w_uv = src_frame->GetRowSize(PLANAR_U);
	h_uv = src_frame->GetHeight(PLANAR_U);

	// Generate masks
	for (int i=0;i<256;i++) {
		double level = maxrho * pow(double(i)/255.0,exponent);
		Masks[i].Generate(max(0.001,level));
		Masks_uv[i].Generate(max(0.001,level/2));
	}

	// Apply sobel edge detection to luma
	UCHAR *tmp = new UCHAR[pitch*h+1];
	WriteEdges(src_frame->GetReadPtr(PLANAR_Y),tmp,w,h,pitch);
	bool threaded = true;
	if (nthreads == 1) threaded = false;

	if (!showedges) {
		// Get pointers
		UCHAR *mask = new UCHAR[pitch*h+1];
		UCHAR *dst_ptr[3];
		const UCHAR *src_ptr[3];
		dst_ptr[0] = dst_frame->GetWritePtr(PLANAR_Y);
		dst_ptr[1] = dst_frame->GetWritePtr(PLANAR_U);
		dst_ptr[2] = dst_frame->GetWritePtr(PLANAR_V);
		src_ptr[0] = src_frame->GetReadPtr(PLANAR_Y);
		src_ptr[1] = src_frame->GetReadPtr(PLANAR_U);
		src_ptr[2] = src_frame->GetReadPtr(PLANAR_V);

		if (threaded) {
			// Init
			boost::thread **threads = new boost::thread* [nthreads];

			// Start threads
			for (int i=0;i<nthreads;i++) {
				threads[i] = new boost::thread(boost::bind(&ThreadProcess,src_frame,src_ptr,dst_ptr,tmp,mask,nthreads,i,edgeBlur));
			}

			// Join threads
			for (int i=0;i<nthreads;i++) {
				threads[i]->join();
				delete threads[i];
			}

			// Clears up
			delete [] threads;
		}

		else {
			ThreadProcess(src_frame,src_ptr,dst_ptr,tmp,mask,1,0,edgeBlur);
		}

		delete [] mask;
	}

	else {
		GaussianCopy(tmp,dst_frame->GetWritePtr(PLANAR_Y),w,h,pitch,edgeBlur);

		// Clear chroma
		UCHAR *dst_u = dst_frame->GetWritePtr(PLANAR_U);
		UCHAR *dst_v = dst_frame->GetWritePtr(PLANAR_V);
		for (UINT y=0;y<h_uv;y++) {
			for (UINT x=0;x<w_uv;x++) {
				*(dst_u+x) = 128;
				*(dst_v+x) = 128;
			}
			dst_u += pitch_uv;
			dst_v += pitch_uv;
		}
	}

	// Clean up
	delete [] tmp;

	return dst_frame;
}
