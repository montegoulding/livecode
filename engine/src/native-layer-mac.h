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


#ifndef __MC_NATIVE_LAYER_MAC__
#define __MC_NATIVE_LAYER_MAC__

#include "native-layer.h"

#include "platform.h"

class MCNativeLayerMac : public MCNativeLayer
{
public:
    
	virtual bool GetNativeView(void *&r_view);
	
    MCNativeLayerMac(MCObject *p_object, MCPlatformNativeLayerContainerViewRef p_view);
    ~MCNativeLayerMac();
    
private:
    
    MCPlatformNativeLayerContainerViewRef m_view;
    MCPlatformBitmapImageRepRef m_cached;

    // Returns the NSWindow* for the stack containing this widget
    void* getStackWindow();
	// Returns the nsview of the parent of this control
	bool getParentView(void *&r_view);

	// Performs the attach/detach operations
	virtual void doAttach();
	virtual void doDetach();
	
	virtual bool doPaint(MCGContextRef p_context);
	virtual void doSetGeometry(const MCRectangle &p_rect);
	virtual void doSetViewportGeometry(const MCRectangle &p_rect);
	virtual void doSetVisible(bool p_visible);
	
	// Performs a relayering operation
	virtual void doRelayer();
};

#endif // ifndef __MC_NATIVE_LAYER_MAC__
