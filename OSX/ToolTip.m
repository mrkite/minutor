/*
Handy ToolTip class from EricForget here:
http://www.cocoadev.com/index.pl?ToolTip
*/
#import "ToolTip.h"
#import "ToolTipTextField.h"

static ToolTip *sharedToolTip=nil;

@interface ToolTip (Private)

- (void) setString:(NSString *)string forEvent:(NSEvent *)theEvent;

@end

@implementation ToolTip

+(void) setString:(NSString *)string forEvent:(NSEvent *)theEvent
{
	if (sharedToolTip==nil)
		sharedToolTip=[[ToolTip alloc] init];
	[sharedToolTip setString:string forEvent:theEvent];
}
+(void) release
{
	[sharedToolTip release];
	sharedToolTip=nil;
}

-(id) init
{
	id retVal=[super init];
	if (retVal!=nil)
	{
		NSRect contentRect={{100,100},{10,20}};
		NSRect textFieldFrame={{0,0},{10,20}};
		window=[[NSWindow alloc]
				initWithContentRect:contentRect
				styleMask:NSBorderlessWindowMask
				backing:NSBackingStoreBuffered
				defer:YES];
		[window setOpaque:YES];
		[window setAlphaValue:0.80];
		[window	setBackgroundColor:[NSColor colorWithDeviceRed:1.0 green:0.96 blue:0.76 alpha:1.0]];
		[window setHasShadow:YES];
		[window setLevel:NSStatusWindowLevel];
		[window setReleasedWhenClosed:YES];
		[window orderFront:nil];
		
		textField=[[ToolTipTextField alloc] initWithFrame:textFieldFrame];
		[textField setEditable:NO];
		[textField setSelectable:NO];
		[textField setBezeled:NO];
		[textField setBordered:NO];
		[textField setDrawsBackground:NO];
		[textField setAlignment:NSCenterTextAlignment];
		[textField setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[textField setFont:[NSFont toolTipsFontOfSize:[NSFont systemFontSize]]];
		[[window contentView] addSubview:textField];
		
		[textField setStringValue:@" "];
		textAttributes=[[[textField attributedStringValue] attributesAtIndex:0 effectiveRange:nil] retain];
	}
	return retVal;
}
- (void) dealloc
{
	[window release];
	[textAttributes release];
	[super dealloc];
}

-(void) setString:(NSString *)string forEvent:(NSEvent *)theEvent
{
	NSSize size=[string sizeWithAttributes:textAttributes];
	NSPoint cursorScreenPosition=[[theEvent window] convertBaseToScreen:[theEvent locationInWindow]];
	[textField setStringValue:string];
	[window setFrameTopLeftPoint:NSMakePoint(cursorScreenPosition.x+10, cursorScreenPosition.y+28)];
	[window setContentSize:NSMakeSize(size.width+20,size.height+1)];
}


@end
