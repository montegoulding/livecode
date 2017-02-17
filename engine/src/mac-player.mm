/* Copyright (C) 2016 LiveCode Ltd.
 
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

#include "mac-platform.h"

class MCAVFoundationPlayer;
class MCQTKitPlayer;

extern MCAVFoundationPlayer *MCAVFoundationPlayerCreate(MCMacPlatform *p_platform);
extern MCQTKitPlayer *MCQTKitPlayerCreate(MCMacPlatform *p_platform);
extern uint4 MCmajorosversion;
extern bool MCQTInit(void);

// PM-2015-06-16: [[ Bug 13820 ]] Take into account the *player* property dontuseqt
void MCMacPlatform::CreatePlayer(bool dontuseqt, MCPlatformPlayerRef& r_player)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_6
	// MW-2014-07-16: [[ QTSupport ]] If we manage to init QT (i.e. dontUseQT is false and
	//   QT is available) then use QTKit, else use AVFoundation if 10.8 and above.
	if (!MCQTInit() && MCmajorosversion >= 0x1080 && dontuseqt)
	{
		r_player = (MCPlatformPlayerRef)MCAVFoundationPlayerCreate(this);
	}
	else
#endif
		r_player = (MCPlatformPlayerRef)MCQTKitPlayerCreate(this);
}

