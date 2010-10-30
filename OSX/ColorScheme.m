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
#import "ColorScheme.h"
#include "../MinutorMap/blockInfo.h"

@interface ColorScheme : NSObject {
	NSString *name;
	unsigned int colors[256];
}
@property (retain) NSString *name;
@end
@implementation ColorScheme
@synthesize name;
// creating a new color scheme always copies the currently selected one
-(id)initWithName:(NSString *)theName
{
	if (self=[super init])
	{
		name=[theName retain];
		for (int i=0;i<numBlocks;i++)
		{
			uint32_t color=blocks[i].color;
			uint8_t r,g,b,a;
			r=color>>16;
			g=color>>8;
			b=color;
			double alpha=blocks[i].alpha;
			r/=alpha;
			g/=alpha;
			b/=alpha;
			a=alpha*255;
			color=(r<<24)|(g<<16)|(b<<8)|a;
			colors[i]=color;
		}
	}
	return self;
}
-(void)encodeWithCoder:(NSCoder *)encoder
{
	[encoder encodeObject:name forKey:@"name"];
	[encoder encodeArrayOfObjCType:"I" count:255 at:colors];
}
-(id)initWithCoder:(NSCoder *)decoder
{
	if (self=[super init])
	{
		name=[[decoder decodeObjectForKey:@"name"] retain];
		[decoder decodeArrayOfObjCType:"I" count:255 at:colors];
	}
	return self;

}
-(NSString *)getColorString:(int)idx
{
	return [NSString stringWithFormat:@"#%06x",(colors[idx]>>8)];
}
-(NSString *)getAlphaString:(int)idx
{
	return [NSString stringWithFormat:@"%d",colors[idx]&0xff];
}
-(void)setColor:(unsigned int)color forIndex:(int)idx
{
	colors[idx]&=0xff;
	colors[idx]|=color<<8;
}
-(void)setAlpha:(unsigned int)alpha forIndex:(int)idx
{
	colors[idx]&=~0xff;
	colors[idx]|=alpha&0xff;
}
-(unsigned int *)getColors
{
	return colors;
}
-(void)dealloc
{
	[name release];
	[super dealloc];
}
@end


@implementation ColorSchemes

-(void)readDefaults
{
	schemes=[[NSMutableArray array] retain];
	
	NSArray *s=[[NSUserDefaults standardUserDefaults] arrayForKey:@"colorSchemes"];
	if (s)
	{
		for (NSData *data in s)
		{
			ColorScheme *cs=(ColorScheme *)[NSKeyedUnarchiver unarchiveObjectWithData:data];
			[schemes addObject:cs];
			NSMenuItem *item=[[NSMenuItem alloc] initWithTitle:cs.name action:@selector(selectScheme:) keyEquivalent:@""];
			[menu insertItem:item atIndex:[schemes count]];
			[item release];
		}
	}
	
	//setup standard colorscheme
	standard=[[ColorScheme alloc] initWithName:@"Standard"];
	selected=0;
}

-(void)select:sender
{
	selected=[menu indexOfItem:sender];
	for (int i=0;i<[menu numberOfItems];i++)
	{
		[[menu itemAtIndex:i] setState:NSOffState];
	}
	[[menu itemAtIndex:selected] setState:NSOnState];
}
-(unsigned int *)current
{
	if (selected==0)
		return [standard getColors];
	ColorScheme *cs=[schemes objectAtIndex:selected-1];
	return [cs getColors];
}

-(void)saveDefaults
{
	NSUserDefaults *def=[NSUserDefaults standardUserDefaults];
	NSMutableArray *encoded=[[NSMutableArray alloc] init];
	for (ColorScheme *cs in schemes)
	{
		[encoded addObject:[NSKeyedArchiver archivedDataWithRootObject:cs]];
	}
	[def setObject:encoded forKey:@"colorSchemes"];
}

-(IBAction)editColorSchemes:sender
{
	[schemesWin makeKeyAndOrderFront:sender];
}

-(id)tableView:(NSTableView *)tv objectValueForTableColumn:(NSTableColumn *)tc row:(NSInteger)row
{
	if (tv==schemeList)
	{
		ColorScheme *cs=[schemes objectAtIndex:row];
		return cs.name;
	}
	if ([[tc identifier] isEqual:@"id"])
		return [NSString stringWithFormat:@"%d.",row+1];
	if ([[tc identifier] isEqual:@"name"])
		return [NSString stringWithCString:blocks[row].name encoding:NSASCIIStringEncoding];
	ColorScheme *cs=[schemes objectAtIndex:[schemeList selectedRow]];
	if ([[tc identifier] isEqual:@"color"])
		return [cs getColorString:row];
	if ([[tc identifier] isEqual:@"alpha"])
		return [cs getAlphaString:row];
	return nil;
}
-(void)tableView:(NSTableView *)tv setObjectValue:obj forTableColumn:(NSTableColumn *)tc row:(NSInteger)row
{
	if (tv==schemeList)
	{
		ColorScheme *cs=[schemes objectAtIndex:row];
		cs.name=obj;
		NSMenuItem *item=[menu itemAtIndex:1+row];
		[item setTitle:obj];
	}
	else
	{
		ColorScheme *cs=[schemes objectAtIndex:[schemeList selectedRow]];
		NSString *str=obj;
		NSUInteger range=[str rangeOfString:@"#"].location;
		if (range!=NSNotFound)
			str=[str substringFromIndex:range+1];

		str=[str stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
		if ([[tc identifier] isEqual:@"color"])
		{
			
			unsigned int color;
			NSScanner *scan=[NSScanner scannerWithString:str];
			if ([scan scanHexInt:&color])
				[cs setColor:color forIndex:row];
		}
		if ([[tc identifier] isEqual:@"alpha"])
		{
			id f=[[NSNumberFormatter alloc] init];
			NSNumber *num=[f numberFromString:str];
			[cs setAlpha:[num unsignedIntValue] forIndex:row];
			[f release];
		}
	}
	[self saveDefaults];
}
-(NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	if (tableView==schemeList)
		return [schemes count];
	else
	{
		return numBlocks;
	}
}

-(IBAction) addOrRemove:sender
{
	NSSegmentedControl *seg=sender;
	if ([seg selectedSegment]==0) //add
	{
		ColorScheme *cs=[[ColorScheme alloc] initWithName:@"New Color Scheme"];
		[schemes addObject:cs];
		NSMenuItem *item=[[NSMenuItem alloc] initWithTitle:@"New Color Scheme" action:@selector(selectScheme:) keyEquivalent:@""];
		[menu insertItem:item atIndex:[schemes count]];
		[item release];
		[schemeList reloadData];
		[self saveDefaults];
	}
	else
	{
		NSInteger num=[schemeList selectedRow];
		if (num==-1)
			return;
		[schemes removeObjectAtIndex:num];
		[menu removeItemAtIndex:num+1];
		[schemeList reloadData];
		[self saveDefaults];
	}
}

-(IBAction) edit:sender
{
	if ([schemeList selectedRow]==-1) return;
	[schemeWin makeKeyAndOrderFront:sender];
}

@end
