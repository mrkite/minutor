/*
Handy ToolTip class from EricForget here:
http://www.cocoadev.com/index.pl?ToolTip
*/

#import "ToolTipTextField.h"


@implementation ToolTipTextField

-(void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	[[NSColor colorWithCalibratedWhite:0.925 alpha:1.0] set];
	NSFrameRect(dirtyRect);
}

@end
