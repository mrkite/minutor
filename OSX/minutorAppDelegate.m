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
#include "MinutorMap.h"

@interface minutorAppDelegate (private)
- (NSString *) worldToPath:(int)world;
@end

@implementation minutorAppDelegate


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	opts=0;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	int tag=[menuItem tag];
	if (tag!=0)
	{
		if (tag>100)
			return [mapViewer isVisible];
		NSString *root=[self worldToPath:tag];
		id files=[NSFileManager defaultManager];
		BOOL isDirectory;
		BOOL exists=[files fileExistsAtPath:root isDirectory:&isDirectory];
		if (exists && isDirectory)
			return YES;
		return NO;
	}
	return YES;
}

- (IBAction) openWorld:sender
{
	NSString *world=nil;
	int tag=[sender tag];
	if (tag==0)
	{
		id fileTypes=[NSArray arrayWithObject:@"dat"];
		id openDlg=[NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:YES];
		[openDlg setAllowedFileTypes:fileTypes];
		if ([openDlg runModal]==NSOKButton)
			world=[[openDlg filename] stringByDeletingLastPathComponent];
	}
	else
		world=[self worldToPath:tag];
	if (world!=nil)
		[mapViewer openWorld:world];
}
-(IBAction) jumpToSpawn:sender
{
	[mapViewer jumpToSpawn];
}
-(IBAction) jumpToPlayer:sender
{
	[mapViewer jumpToPlayer];
}
-(IBAction) toggleLighting:sender
{
	opts^=LIGHTING;
	[sender setState:((opts&LIGHTING)!=0)?NSOnState:NSOffState];
	[mapViewer setOptions:opts];
}
-(IBAction) toggleCaveMode:sender
{
	opts^=CAVEMODE;
	[sender setState:((opts&CAVEMODE)!=0)?NSOnState:NSOffState];
	[mapViewer setOptions:opts];
}
-(IBAction) toggleObscured:sender
{
	opts^=HIDEOBSCURED;
	[sender setState:((opts&HIDEOBSCURED)!=0)?NSOnState:NSOffState];
	[mapViewer setOptions:opts];
}
-(IBAction) toggleDepth:sender
{
	opts^=DEPTHSHADING;
	[sender setState:((opts&DEPTHSHADING)!=0)?NSOnState:NSOffState];
	[mapViewer setOptions:opts];
}

- (NSString *) worldToPath:(int)world
{
	NSArray *paths=NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *root=[paths objectAtIndex:0];
	root=[root stringByAppendingPathComponent:@"minecraft/saves"];
	root=[root stringByAppendingFormat:@"/World%d",world];
	return root;
}

@end
