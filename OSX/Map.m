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
#import "ToolTip.h"
#include "MinutorMap.h"


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
		bits=malloc(curWidth*curHeight*4);
    }
    return self;
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
	
	
	DrawMap([world UTF8String], curX, curZ, curDepth, curWidth, curHeight, curScale, bits);
	
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
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}
- (void)rightMouseDown:(NSEvent *)theEvent
{
	// y=400-at.y
	NSPoint at=[self convertPoint:[theEvent locationInWindow] fromView:nil];
	if (at.y>curHeight || at.x>curWidth)
		return;
	int ofs=(curHeight-at.y)*curWidth*4+at.x*4;
	unsigned int color=(bits[ofs]<<16)|(bits[ofs+1]<<8)|bits[ofs+2];
	NSString *label=[NSString stringWithCString:IDBlock(color) encoding:[NSString defaultCStringEncoding]];
	[ToolTip setString:label forEvent:theEvent];
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
	[ToolTip release];
}
- (void)mouseDragged:(NSEvent *)theEvent
{
	curX-=[theEvent deltaX]/curScale;
	curZ-=[theEvent deltaY]/curScale;
	[self setNeedsDisplay:YES];
}
- (void)scrollWheel:(NSEvent *)theEvent
{
	CGFloat dy=[theEvent deltaY];
	if (dy<0.0)
	{
		curScale+=dy/10.0;
		if (curScale<1.0) curScale=1.0;
		[self setNeedsDisplay:YES];
	}
	if (dy>0.0)
	{
		curScale+=dy/10.0;
		if (curScale>5.0) curScale=5.0;
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
			curZ-=10.0/curScale;
			changed=YES;
			break;
		case NSDownArrowFunctionKey:
			curZ+=10.0/curScale;
			changed=YES;
			break;
		case NSLeftArrowFunctionKey:
			curX-=10.0/curScale;
			changed=YES;
			break;
		case NSRightArrowFunctionKey:
			curX+=10.0/curScale;
			changed=YES;
			break;
		case NSPageUpFunctionKey:
			curScale+=1.0;
			if (curScale>5.0)
				curScale=5.0;
			changed=YES;
			break;
		case NSPageDownFunctionKey:
			curScale-=1.0;
			if (curScale<1.0)
				curScale=1.0;
			changed=YES;
			break;
	}
	if (changed)
		[self setNeedsDisplay:YES];
}

- (void)setX:(double)x andZ:(double)z
{
	curX=x;
	curZ=z;
}
- (void)setDepth:(int)depth
{
	curDepth=depth;
	[self setNeedsDisplay:YES];
}
- (void)setWorld:(NSString *)newWorld
{
	world=[newWorld retain];
}

@end
