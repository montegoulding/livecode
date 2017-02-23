/* Copyright (C) 2017 LiveCode Ltd.
 
 This file is part of LiveCode.
 
 LiveCode is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License v3 as published by the Free
 Software Foundation.
 
 LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

#import "platform.h"
#include "platform-internal.h"

#include "mac-platform.h"
#include "mac-internal.h"

#import <AppKit/NSView.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSTextInputClient.h>
#import <AppKit/NSImage.h>

//////////

// IM-2015-12-16: [[ NativeLayer ]] Keep the coordinate system of group contents the same as
//                the top-level window view by keeping its bounds the same as its frame.
//                This allows us to place contents in terms of window coords without having to
//                adjust for the location of the group container.
@interface com_runrev_livecode_MCContainerView: NSView

- (void)setFrameOrigin:(NSPoint)newOrigin;
- (void)setFrameSize:(NSSize)newSize;

@end

@compatibility_alias MCContainerView com_runrev_livecode_MCContainerView;

@implementation com_runrev_livecode_MCContainerView

- (void)setFrameOrigin:(NSPoint)newOrigin
{
    [super setFrameOrigin:newOrigin];
    [self setBoundsOrigin:newOrigin];
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    [self setBoundsSize:newSize];
}

@end

void MCMacPlatform::NativeLayerContainerViewRetain(MCPlatformNativeLayerContainerViewRef p_view)
{
    [(NSView *)p_view retain];
}

void MCMacPlatform::NativeLayerContainerViewRelease(MCPlatformNativeLayerContainerViewRef p_view)
{
    [(NSView *)p_view release];
}

void MCMacPlatform::BitmapImageRepRelease(MCPlatformBitmapImageRepRef p_rep)
{
    [(NSBitmapImageRep *)p_rep release];
}

void MCMacPlatform::BitmapImageRepRetain(MCPlatformBitmapImageRepRef p_rep)
{
    [(NSBitmapImageRep *)p_rep retain];
}

void MCMacPlatform::NativeLayerContainerViewRemove(MCPlatformNativeLayerContainerViewRef p_view)
{
    [(NSView *)p_view removeFromSuperview];
}

void MCMacPlatform::CreateBitmapImageRep(MCPlatformNativeLayerContainerViewRef p_view, MCPlatformBitmapImageRepRef &r_rep)
{
    r_rep = [(NSView *)p_view bitmapImageRepForCachingDisplayInRect:[(NSView *)p_view bounds]];
}

void MCMacPlatform::BitmapImageRepCache(MCPlatformNativeLayerContainerViewRef p_view, MCPlatformBitmapImageRepRef p_rep, MCGRaster &r_raster)
{
    // Draw the widget
    bzero([(NSBitmapImageRep*)p_rep bitmapData], [(NSBitmapImageRep*)p_rep bytesPerRow] * [(NSBitmapImageRep*)p_rep pixelsHigh]);
    [(NSView*)p_view cacheDisplayInRect:[(NSView*)p_view bounds] toBitmapImageRep:(NSBitmapImageRep*) p_rep];

    // Turn the NSBitmapImageRep into something we can actually draw
    MCGRaster t_raster;
    t_raster.format = kMCGRasterFormat_ARGB;
    t_raster.width = [(NSBitmapImageRep*)p_rep pixelsWide];
    t_raster.height = [(NSBitmapImageRep*)p_rep pixelsHigh];
    t_raster.stride = [(NSBitmapImageRep*)p_rep bytesPerRow];
    t_raster.pixels = [(NSBitmapImageRep*)p_rep bitmapData];
    
    r_raster = t_raster;
}

void MCMacPlatform::NativeLayerContainerViewSetGeometry(MCPlatformNativeLayerContainerViewRef p_view, MCRectangle p_rect, MCPlatformWindowRef p_window, int32_t p_gp_height)
{
    int32_t t_gp_height;

    NSWindow *t_window = ((MCMacPlatformWindow*)p_window)->GetHandle();
    if (t_window != nil)
        t_gp_height = (int32_t)[[t_window contentView] frame].size.height;
    else
        t_gp_height = p_gp_height;

    NSRect t_rect;
    t_rect = NSMakeRect(p_rect.x, t_gp_height - (p_rect.y + p_rect.height), p_rect.width, p_rect.height);

    [(NSView *)p_view setFrame:t_rect];
    [(NSView *)p_view setNeedsDisplay:YES];
}

void MCMacPlatform::NativeLayerContainerViewSetVisible(MCPlatformNativeLayerContainerViewRef p_view, bool p_visible)
{
    [(NSView *)p_view setHidden:!p_visible];
}

void MCMacPlatform::NativeLayerContainerViewAddSubView(void * p_parent_view, MCPlatformNativeLayerContainerViewRef p_view, void * p_before_view)
{
    if (p_before_view)
        [(NSView *)p_parent_view addSubview:(NSView *)p_view positioned:NSWindowBelow relativeTo:(NSView *)p_before_view];
    else
        [(NSView *)p_parent_view addSubview:(NSView *)p_view];
        
    [(NSView *)p_parent_view setNeedsDisplay:YES];
}

void MCMacPlatform::WindowContentView(MCPlatformWindowRef p_window, void *&r_view)
{
    NSWindow *t_window = ((MCMacPlatformWindow*)p_window)->GetHandle();
    if (t_window)
        r_view = [t_window contentView];
    else
        r_view = nil;
}

void MCMacPlatform::CreateNativeLayerContainerView(void *&r_view)
{
    NSView *t_view;
    t_view = [[[ MCContainerView alloc] init] autorelease];

    if (t_view != nil)
        [t_view setAutoresizesSubviews:NO];
    
    r_view = t_view;
}
