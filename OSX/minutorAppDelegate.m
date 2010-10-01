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

#import "minutorAppDelegate.h"
#import "MapViewer.h"

@implementation minutorAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	//minecraft saves are inside application support
	customWorld=nil;
	NSArray *paths=NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *root=[paths objectAtIndex:0];
	root=[root stringByAppendingPathComponent:@"minecraft/saves"];
	
	id files=[NSFileManager defaultManager];
	[worlds removeAllItems];
	for (int i=0;i<5;i++)
	{
		[worlds addItemWithTitle:[NSString stringWithFormat:@"World %d", i+1,nil]];
		[[worlds itemAtIndex:i] setTag:-1];
	}
	BOOL isDirectory;
	BOOL exists=[files fileExistsAtPath:root isDirectory:&isDirectory];
	worldPaths=[[NSMutableArray alloc] initWithCapacity:5];
	if (exists && isDirectory)
	{
		NSError *err;
		NSArray *fs=[files contentsOfDirectoryAtPath:root error:&err];
		int count=[fs count];
		for (int i=0;i<count;i++)
		{
			int num=-1;
			NSString *f=[fs objectAtIndex:i];
			if ([f isEqual:@"World1"]) num=0;
			if ([f isEqual:@"World2"]) num=1;
			if ([f isEqual:@"World3"]) num=2;
			if ([f isEqual:@"World4"]) num=3;
			if ([f isEqual:@"World5"]) num=4;
			if (num!=-1)
			{
				[[worlds itemAtIndex:num] setTag:[worldPaths count]];
				[worldPaths addObject:[root stringByAppendingPathComponent:f]];
			}
		}
	}
	[worlds selectItemAtIndex:0];
	
}

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>) anItem
{
	int tag=[anItem tag];
	if (tag==-1) return NO;
	return YES;
}

- (IBAction) customClicked: sender
{
	NSArray *fileTypes=[NSArray arrayWithObject:@"dat"];
	NSOpenPanel *openDlg=[NSOpenPanel openPanel];
	[openDlg setCanChooseFiles:YES];
	[openDlg setAllowedFileTypes:fileTypes];
	if ([openDlg runModal]==NSOKButton)
	{
		if (customWorld!=nil)
			[customWorld release];
		customWorld=[[[openDlg filename] stringByDeletingLastPathComponent] retain];
		[worldChoice selectCellAtRow:1 column:0];
		[customLabel setStringValue:customWorld];
	}
}

- (IBAction) viewClicked: sender
{
	if ([worldChoice selectedRow]==0)
	{
		int tag=[[worlds selectedItem] tag];
		if (tag!=-1)
		{
			[mapViewer openWorld:[worldPaths objectAtIndex:tag]];
			[window orderOut:self];
		}
	}
	else if (customWorld!=nil)
	{
		[mapViewer openWorld:customWorld];
		[window orderOut:self];
	}
}
- (IBAction) standardSelection: sender
{
	[worldChoice selectCellAtRow:0 column:0];
}

@end
