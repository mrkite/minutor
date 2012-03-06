/*
 Copyright (c) 2011, Sean Kasun
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

@implementation minutorAppDelegate


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	opts=0;
	[colorSchemes readDefaults];
	
	worldPaths=[[NSMutableArray alloc] initWithCapacity:10];
	NSArray *paths=NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *root=[paths objectAtIndex:0];
	root=[root stringByAppendingPathComponent:@"minecraft/saves"];
	
	id files=[NSFileManager defaultManager];
	id subs=[files contentsOfDirectoryAtPath:root error:NULL];
	for (NSString *f in subs)
	{
		NSString *path=[root stringByAppendingPathComponent:f];
		BOOL isDirectory;
		BOOL exists=[files fileExistsAtPath:path isDirectory:&isDirectory];
		if (exists && isDirectory)
		{
			[worldPaths addObject:path];
			NSMenuItem *item=[[NSMenuItem alloc] initWithTitle:f action:@selector(openWorld:) keyEquivalent:@""];
			[item setTag:[worldPaths count]];
			[worldMenu insertItem:item atIndex:[worldPaths count]-1];
			[item release];
		}
	}
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	int tag=[menuItem tag];
	if (tag!=0)
	{
		if (tag>100)
			return [mapViewer isVisible];
		id files=[NSFileManager defaultManager];
		BOOL isDirectory;
		BOOL exists=[files fileExistsAtPath:[worldPaths objectAtIndex:tag-1] isDirectory:&isDirectory];
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
		world=[worldPaths objectAtIndex:tag-1];
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
	[mapViewer setOpts:opts];
}
-(IBAction) toggleCaveMode:sender
{
	opts^=CAVEMODE;
	[sender setState:((opts&CAVEMODE)!=0)?NSOnState:NSOffState];
	[mapViewer setOpts:opts];
}
-(IBAction) toggleObscured:sender
{
	opts^=HIDEOBSCURED;
	[sender setState:((opts&HIDEOBSCURED)!=0)?NSOnState:NSOffState];
	[mapViewer setOpts:opts];
}
-(IBAction) toggleDepth:sender
{
	opts^=DEPTHSHADING;
	[sender setState:((opts&DEPTHSHADING)!=0)?NSOnState:NSOffState];
	[mapViewer setOpts:opts];
}
-(IBAction) toggleHell:sender
{
	opts^=HELL;
	opts&=~ENDER;
	[sender setState:((opts&HELL)!=0)?NSOnState:NSOffState];
	[enderItem setState:NSOffState];
	CloseAll();
	[mapViewer setOpts:opts];
}
-(IBAction) toggleEnder:sender
{
	opts^=ENDER;
	opts&=~HELL;
	[sender setState:((opts&ENDER)!=0)?NSOnState:NSOffState];
	[hellItem setState:NSOffState];
	CloseAll();
	[mapViewer setOpts:opts];
}

-(IBAction)selectScheme:sender
{
	[colorSchemes select:sender];
	[mapViewer setColorScheme:[colorSchemes current]];
}

@end
