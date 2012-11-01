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

#import <Cocoa/Cocoa.h>
#import "ColorScheme.h"

@interface minutorAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	NSMutableArray *worldPaths;
	NSString *customWorld;
	IBOutlet id mapViewer;
	int opts;
	IBOutlet id colorSchemes;
	IBOutlet id worldMenu;
	IBOutlet id hellItem;
	IBOutlet id enderItem;
}
- (IBAction) openWorld: sender;
- (IBAction) jumpToSpawn: sender;
- (IBAction) jumpToPlayer: sender;
- (IBAction) toggleLighting: sender;
- (IBAction) toggleCaveMode: sender;
- (IBAction) toggleObscured: sender;
- (IBAction) toggleDepth: sender;
- (IBAction) toggleMob: sender;
- (IBAction) toggleSlime: sender;
- (IBAction) toggleHell: sender;
- (IBAction) toggleEnder: sender;
- (IBAction) selectScheme: sender;


@end
