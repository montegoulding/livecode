/* Copyright (C) 2015 LiveCode Ltd.
 
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

#include "prefix.h"

#include "globdefs.h"
#include "filedefs.h"
#include "objdefs.h"
#include "parsedef.h"


#include "util.h"
#include "mcerror.h"
#include "sellst.h"
#include "stack.h"
#include "card.h"
#include "image.h"
#include "widget.h"
#include "param.h"
#include "osspec.h"
#include "cmds.h"
#include "scriptpt.h"
#include "hndlrlst.h"
#include "debug.h"
#include "redraw.h"
#include "font.h"
#include "chunk.h"
#include "graphicscontext.h"
#include "objptr.h"

#include "globals.h"
#include "context.h"

#include "group.h"
#include "widget.h"
#include "native-layer-mac.h"

#include "platform-internal.h"
#include "graphics_util.h"


MCNativeLayerMac::MCNativeLayerMac(MCObject *p_object, MCPlatformNativeLayerContainerViewRef p_view) :
  m_view(p_view),
  m_cached(nil)
{
	m_object = p_object;
    
    MCPlatformNativeLayerContainerViewRetain(m_view);
}

MCNativeLayerMac::~MCNativeLayerMac()
{
    if (m_view != nil)
    {
        doDetach();
        MCPlatformNativeLayerContainerViewRelease(m_view);
    }
    if (m_cached != nil)
    {
        MCPlatformBitmapImageRepRelease(m_cached);
    }
}

void MCNativeLayerMac::doAttach()
{
    // Act as if there was a re-layer to put the widget in the right place
    // *** Can we assume open happens in back-to-front order? ***
    doRelayer();
    
    // Restore the visibility state of the widget (in case it changed due to a
    // tool change while on another card - we don't get a message then)
	
	doSetGeometry(m_rect);
	doSetVisible(ShouldShowLayer());
}

void MCNativeLayerMac::doDetach()
{
    // Remove the view from the stack's content view
    MCPlatformNativeLayerContainerViewRemove(m_view);
}

bool MCNativeLayerMac::doPaint(MCGContextRef p_context)
{
    // Get an image rep suitable for storing the cached bitmap
    if (m_cached == nil)
    {
        MCPlatformCreateBitmapImageRep(m_view, m_cached);
        MCPlatformBitmapImageRepRetain(m_cached);
    }
    
    MCGImageRef t_gimage;
    MCGRaster t_raster;
    MCPlatformBitmapImageRepCache(m_view, m_cached, t_raster);
    if (!MCGImageCreateWithRasterNoCopy(t_raster, t_gimage))
		return false;
    
    // Draw the image
    // FG-2014-10-10: a y offset of 1 is needed to keep things lined up, for some reason...
    MCGRectangle rect = {{0, 1}, {MCGFloat(t_raster.width), MCGFloat(t_raster.height)}};
    MCGContextDrawImage(p_context, t_gimage, rect, kMCGImageFilterNone);
    MCGImageRelease(t_gimage);
    
    return true;
}

void MCNativeLayerMac::doSetViewportGeometry(const MCRectangle &p_rect)
{
}

void MCNativeLayerMac::doSetGeometry(const MCRectangle &p_rect)
{
    MCPlatformNativeLayerContainerViewSetGeometry(m_view, p_rect, m_object->getstack()->getwindow(), m_object->getcard()->getrect().height);
    MCPlatformBitmapImageRepRelease(m_cached);
    m_cached = nil;
}

void MCNativeLayerMac::doSetVisible(bool p_visible)
{
    MCPlatformNativeLayerContainerViewSetVisible(m_view, p_visible);
   // [m_view setHidden:!p_visible];
}

void MCNativeLayerMac::doRelayer()
{
    // Find which native layer this should be inserted below
    MCObject *t_before;
    t_before = findNextLayerAbove(m_object);
	
	void *t_parent_view;
	t_parent_view = nil;
	
	if (!getParentView(t_parent_view))
		return;
	
    // Insert the widget in the correct place (but only if the card is current)
    if (isAttached() && m_object->getcard() == m_object->getstack()->getcurcard())
    {
        MCPlatformNativeLayerContainerViewRemove(m_view);
        if (t_before != nil)
        {
            // There is another native layer above this one
			void *t_before_view;
			/* UNCHECKED */ t_before->GetNativeView((void*&)t_before_view);
            MCPlatformNativeLayerContainerViewAddSubView(t_parent_view, m_view, t_before_view);
        }
        else
        {
            // This is the top-most native layer
            MCPlatformNativeLayerContainerViewAddSubView(t_parent_view, m_view, nil);
        }
        MCPlatformViewSetNeedsDisplay(t_parent_view);
    }
}

bool MCNativeLayerMac::getParentView(void *&r_view)
{
	if (m_object->getparent()->gettype() == CT_GROUP)
	{
		MCNativeLayer *t_container;
		t_container = nil;
		
		if (((MCGroup*)m_object->getparent())->getNativeContainerLayer(t_container))
            return t_container->GetNativeView((void*&)r_view);
	}
	else
	{
        MCPlatformWindowRef t_window;
        t_window = m_object->getstack()->getwindow();
        
        if (t_window != nil)
        {
            MCPlatformWindowContentView(t_window, r_view); // [t_window contentView]
            return true;
        }
	}

    return false;
}

bool MCNativeLayerMac::GetNativeView(MCPlatformNativeLayerContainerViewRef &r_view)
{
	r_view = m_view;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

MCNativeLayer* MCNativeLayer::CreateNativeLayer(MCObject *p_object, void *p_view)
{
	if (p_view == nil)
		return nil;
	
    return new MCNativeLayerMac(p_object, p_view);
}


bool MCNativeLayer::CreateNativeContainer(MCObject *p_object, void *&r_view)
{
    MCPlatformCreateNativeLayerContainerView(r_view);
    return true;
}

//////////

void MCNativeLayer::ReleaseNativeView(void * p_view)
{
	if (p_view == nil)
		return;
	
    MCPlatformNativeLayerContainerViewRelease(p_view);
}
