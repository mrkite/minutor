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

#import "Map.h"
#include "MinutorMap.h"

#define MINZOOM 1.0
#define MAXZOOM 10.0


@implementation Map

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		curX=0.0;
		curZ=0.0;
		curDepth=127;
		curScale=1.0;
		world=nil;
		curWidth=496;
		curHeight=400;
		moving=0;
		bits=malloc(curWidth*curHeight*4);
    }
    return self;
}

static id progressBar;
- (void)setProgress:(id)progress
{
	progressBar=progress;
}

void updateProgress(float progress)
{
	[progressBar setDoubleValue:progress*100.0];
	[progressBar displayIfNeeded];
}

- (void)drawRect:(NSRect)dirtyRect {
	if (world==nil) return;
	
	if (curWidth!=dirtyRect.size.width || curHeight!=dirtyRect.size.height)
	{
		curWidth=dirtyRect.size.width;
		curHeight=dirtyRect.size.height;
		bits=realloc(bits, curWidth*curHeight*4);
	}
	
	NSRect myRect = NSMakeRect(0,0,curWidth,curHeight);
	
	
	DrawMap([world UTF8String], curX, curZ, curDepth, curWidth, curHeight, curScale, bits, opts, updateProgress);
	NSBitmapImageRep *mapImage=[[NSBitmapImageRep alloc]
								initWithBitmapDataPlanes:&bits
								pixelsWide:curWidth
								pixelsHigh:curHeight
								bitsPerSample:8
								samplesPerPixel:4
								hasAlpha:YES
								isPlanar:NO
								colorSpaceName:NSCalibratedRGBColorSpace
								bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
								bytesPerRow:curWidth*4
								bitsPerPixel:32];
	
	[mapImage drawInRect: myRect];
	[mapImage release];
	updateProgress(0.0);
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}
- (void)mouseDragged:(NSEvent *)theEvent
{
	curX-=[theEvent deltaY]/curScale;
	curZ+=[theEvent deltaX]/curScale;
	[self setNeedsDisplay:YES];
}
- (void)mouseMoved:(NSEvent *)theEvent
{
	NSPoint loc=[self convertPoint:[theEvent locationInWindow] fromView:nil];
	loc.y=curHeight-loc.y;
	if (loc.y>=0 && loc.x>=0 && loc.y<curHeight && loc.x<curWidth)
	{
		int mx,mz;
		const char *label=IDBlock(loc.x, loc.y, curX, curZ,
							curWidth, curHeight, curScale, &mx, &mz);
		[status setStringValue:[NSString stringWithFormat:@"%d,%d %s",mx,mz,label]];
	}
}
- (void)scrollWheel:(NSEvent *)theEvent
{
	CGFloat dy=[theEvent deltaY];
	if (dy<0.0)
	{
		curScale+=dy/10.0;
		if (curScale<MINZOOM) curScale=MINZOOM;
		[self setNeedsDisplay:YES];
	}
	if (dy>0.0)
	{
		curScale+=dy/10.0;
		if (curScale>MAXZOOM) curScale=MAXZOOM;
		[self setNeedsDisplay:YES];
	}
}

- (void)keyDown:(NSEvent *)theEvent
{
	NSString *characters=[theEvent characters];
	unichar character=[characters characterAtIndex:0];
	BOOL changed=NO;
	
	switch (character)
	{
		case NSUpArrowFunctionKey:
		case 'w':
			moving|=1;
			break;
		case NSDownArrowFunctionKey:
		case 's':
			moving|=2;
			break;
		case NSLeftArrowFunctionKey:
		case 'a':
			moving|=4;
			break;
		case NSRightArrowFunctionKey:
		case 'd':
			moving|=8;
			break;
		case NSPageUpFunctionKey:
		case 'e':
			curScale+=0.5;
			if (curScale>MAXZOOM)
				curScale=MAXZOOM;
			changed=YES;
			break;
		case NSPageDownFunctionKey:
		case 'q':
			curScale-=0.5;
			if (curScale<MINZOOM)
				curScale=MINZOOM;
			changed=YES;
			break;
	}
	if (moving)
	{
		if (moving&1)
			curX-=10.0/curScale;
		if (moving&2)
			curX+=10.0/curScale;
		if (moving&4)
			curZ+=10.0/curScale;
		if (moving&8)
			curZ-=10.0/curScale;
		changed=YES;
	}
	if (changed)
		[self setNeedsDisplay:YES];
}
- (void)keyUp:(NSEvent *)theEvent
{
	NSString *characters=[theEvent characters];
	unichar character=[characters characterAtIndex:0];
	
	switch (character)
	{
		case NSUpArrowFunctionKey:
		case 'w':
			moving&=~1;
			break;
		case NSDownArrowFunctionKey:
		case 's':
			moving&=~2;
			break;
		case NSLeftArrowFunctionKey:
		case 'a':
			moving&=~4;
			break;
		case NSRightArrowFunctionKey:
		case 'd':
			moving&=~8;
			break;
	}
}

- (void)setX:(double)x andZ:(double)z
{
	if (opts&HELL)
	{
		x/=8.0;
		z/=8.0;
	}
	curX=x;
	curZ=z;
	[self setNeedsDisplay:YES];
}
- (void)setDepth:(int)depth
{
	curDepth=depth;
	[self setNeedsDisplay:YES];
}
- (void)setWorld:(NSString *)newWorld
{
	[world autorelease];
	world=[newWorld retain];
}
- (void)setStatus:(id)statusbar
{
	[status autorelease];
	status=[statusbar retain];
}
- (void)setOptions:(int)options
{
	if (((opts^options)&HELL)==HELL) //hell toggled
	{
		if (options&HELL)
		{
			curX/=8.0;
			curZ/=8.0;
		}
		else
		{
			curX*=8.0;
			curZ*=8.0;
		}
	}
	opts=options;
	[self setNeedsDisplay:YES];
}
-(void)setColorScheme:(unsigned int *)colors
{
	SetMapPalette(colors, 256);
	[self setNeedsDisplay:YES];
}

@end
