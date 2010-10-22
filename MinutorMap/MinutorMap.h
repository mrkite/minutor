/*
Copyright (c) 2010, Sean Kasun
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __MINUTOR_MAP_H__
#define __MINUTOR_MAP_H__

#ifndef WIN32
#define __declspec(a)
#define dllexport 0
#define __cdecl
#endif

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) void __cdecl DrawMap(const char *world,double cx,double cz,int y,int w,int h,double zoom,unsigned char *bits, int opts);
	__declspec(dllexport) const char * __cdecl IDBlock(int bx, int by, double cx, double cz, int w, int h, double zoom,int *ox,int *oz);
	__declspec(dllexport) void __cdecl CloseAll();
	__declspec(dllexport) void __cdecl GetSpawn(const char *world,int *x,int *y,int *z);
	__declspec(dllexport) void __cdecl GetPlayer(const char *world,int *px,int *py,int *pz);
#ifdef __cplusplus
}
#endif

#endif
