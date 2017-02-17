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

#ifndef __MAC_PLATFORM_H__
#define __MAC_PLATFORM_H__

#include "platform.h"

#include <AppKit/NSSavePanel.h>

class MCMacPlatformCallbacks : public MCPlatformCallbacks
{
public:
	// TODO - handle platform-specific callback functions
	virtual NSString *MCStringConvertToAutoreleasedNSString(MCStringRef p_string_ref) = 0;
	virtual bool MCStringConvertToCFStringRef(MCStringRef p_string, CFStringRef& r_cfstring) = 0;
	virtual bool MCStringCreateWithCFString(CFStringRef p_cf_string, MCStringRef& r_string) = 0;
	virtual bool MCS_mac_elevation_bootstrap_main(int argc, char *argv[]) = 0;
};

class MCMacPlatformStubs
{
public:
	// Engine stubs
	Boolean MCS_exists(MCStringRef p_path, bool p_is_file);
	bool MCS_resolvepath(MCStringRef p_path, MCStringRef& r_resolved);
	bool MCStringsSplit(MCStringRef p_string, codepoint_t p_separator, MCStringRef*&r_strings, uindex_t& r_count);
	
	// module stubs
	bool MCModulesInitialize(void);
	void MCModulesFinalize(void);
	
	// libscript stubs
	bool MCScriptInitialize(void);
	void MCScriptFinalize(void);
	
	// libfoundation stubs
	bool MCInitialize(void);
	void MCFinalize(void);
	bool MCSInitialize(void);
	void MCSFinalize(void);
	
	template<typename T> bool MCMemoryAllocate(size_t p_size, T*& r_block)
	{
		void *t_block;
		if (MCMemoryAllocate(p_size, t_block))
		{
			r_block = static_cast<T *>(t_block);
			return true;
		}
		return false;
	}
	
	template<typename T> bool MCMemoryNew(T*& r_record)
	{
		void *t_record;
		if (MCMemoryNew(sizeof(T), t_record))
		{
			r_record = new (t_record) T;
			return true;
		}
		return false;
	}
	
	template<typename T> bool MCMemoryReallocate(T *p_block, size_t p_new_size, T*& r_new_block)
	{
		void *t_new_block;
		if (MCMemoryReallocate(p_block, p_new_size, t_new_block))
		{
			r_new_block = static_cast<T *>(t_new_block);
			return true;
		}
		return false;
	}
	
	template<typename T> bool MCMemoryNewArray(uindex_t p_count, T*& r_array)
	{
		void *t_array;
		if (MCMemoryNewArray(p_count, sizeof(T), t_array))
			return r_array = static_cast<T *>(t_array), true;
		return false;
	}
	
	template<typename T> inline bool MCMemoryResizeArray(uindex_t p_new_count, T*& x_array, uindex_t& x_count)
	{
		void *t_array;
		t_array = x_array;
		if (MCMemoryResizeArray(p_new_count, sizeof(T), t_array, x_count))
		{
			x_array = static_cast<T *>(t_array);
			return true;
		}
		return false;
	}
	
	template<typename T> void MCMemoryDeleteArray(T* p_array)
	{
		MCMemoryDeleteArray(static_cast<void *>(p_array));
	}
	
	bool MCMemoryAllocate(size_t size, void*& r_block);
	bool MCMemoryAllocateCopy(const void *block, size_t size, void*& r_new_block);
	bool MCMemoryReallocate(void *block, size_t new_size, void*& r_new_block);
	void MCMemoryDeallocate(void *block);
	bool MCMemoryNew(size_t size, void*& r_record);
	void MCMemoryDelete(void *p_record);
	bool MCMemoryNewArray(uindex_t count, size_t size, void*& r_array);
	bool MCMemoryResizeArray(uindex_t new_count, size_t size, void*& x_array, uindex_t& x_count);
	void MCMemoryDeleteArray(void *array);
	
	template<typename T> T MCValueRetain(T value)
	{
		return (T)MCValueRetain((MCValueRef)value);
	}
	
	MCValueRef MCValueRetain(MCValueRef p_value);
	void MCValueRelease(MCValueRef p_value);
	
	uindex_t MCDataGetLength(MCDataRef p_data);
	bool MCDataCreateWithBytes(const byte_t *p_bytes, uindex_t p_byte_count, MCDataRef& r_data);
	const byte_t *MCDataGetBytePtr(MCDataRef p_data);
	
	MCStringRef MCSTR(const char *p_cstring);
	bool MCStringAppendFormat(MCStringRef string, const char *format, ...);
	bool MCStringAppendFormatV(MCStringRef string, const char *format, va_list args);
	bool MCStringConvertToUTF8(MCStringRef p_string, char*& r_utf8string, uindex_t& r_utf8_chars);
	bool MCStringCopy(MCStringRef self, MCStringRef& r_new_string);
	bool MCStringCopySubstring(MCStringRef self, MCRange p_range, MCStringRef& r_substring);
	bool MCStringCreateMutable(uindex_t p_initial_capacity, MCStringRef &r_string);
	bool MCStringCreateWithBytes(const byte_t *p_bytes, uindex_t p_byte_count, MCStringEncoding p_encoding, bool p_is_external_rep, MCStringRef& r_string);
	bool MCStringFirstIndexOfChar(MCStringRef self, codepoint_t p_needle, uindex_t p_after, MCStringOptions p_options, uindex_t& r_offset);
	uindex_t MCStringGetLength(MCStringRef self);
	bool MCStringIsEmpty(MCStringRef string);
	bool MCStringIsEqualToCString(MCStringRef p_string, const char *p_cstring, MCStringOptions p_options);
	bool MCStringLastIndexOfChar(MCStringRef self, codepoint_t p_needle, uindex_t p_before, MCStringOptions p_options, uindex_t& r_offset);
	bool MCStringSubstringIsEqualTo(MCStringRef self, MCRange p_sub, MCStringRef p_other, MCStringOptions p_options);
	
#if defined(DEBUG_LOG)
	void __MCAssert(const char *file, uint32_t line, const char *message);
	void __MCUnreachable(void);
#endif

	// Legacy
	void MCListPushBack(void*& x_list, void *p_element);
	void *MCListPopFront(void*& x_list);
	
	template<typename T> void MCListPushBack(T*& x_list, T *p_element)
	{
		void *t_list;
		t_list = x_list;
		MCListPushBack(t_list, p_element);
		x_list = static_cast<T *>(t_list);
	}
	
	template<typename T> T *MCListPopFront(T*& x_list)
	{
		void *t_list, *t_element;
		t_list = x_list;
		t_element = MCListPopFront(t_list);
		x_list = static_cast<T *>(t_list);
		return static_cast<T *>(t_element);
	}
	
	// libgraphics
	int32_t MCGImageGetWidth(MCGImageRef image);
	int32_t MCGImageGetHeight(MCGImageRef image);
	
	bool MCGRegionCreate(MCGRegionRef &r_region);
	void MCGRegionDestroy(MCGRegionRef p_region);
	MCGIntegerRectangle MCGRegionGetBounds(MCGRegionRef p_region);
	bool MCGRegionAddRect(MCGRegionRef p_region, const MCGIntegerRectangle &p_rect);
	bool MCGRegionIterate(MCGRegionRef p_region, MCGRegionIterateCallback p_callback, void *p_context);
	
	bool MCGContextCreateWithRaster(const MCGRaster& raster, MCGContextRef& r_context);
	void MCGContextRelease(MCGContextRef context);
	void MCGContextClipToRect(MCGContextRef context, MCGRectangle rect);
	void MCGContextClipToRegion(MCGContextRef self, MCGRegionRef p_region);
	void MCGContextTranslateCTM(MCGContextRef context, MCGFloat xoffset, MCGFloat yoffset);
	void MCGContextScaleCTM(MCGContextRef context, MCGFloat xscale, MCGFloat yscale);

	// TODO - handle platform-specific callback functions
	NSString *MCStringConvertToAutoreleasedNSString(MCStringRef p_string_ref);
	bool MCStringConvertToCFStringRef(MCStringRef p_string, CFStringRef& r_cfstring);
	bool MCStringCreateWithCFString(CFStringRef p_cf_string, MCStringRef& r_string);
	bool MCS_mac_elevation_bootstrap_main(int argc, char *argv[]);
	
	// get the callback object to use
	virtual MCMacPlatformCallbacks* GetCallbacks() = 0;
};

struct MCFileFilter;

class MCMacPlatform : public MCPlatformInterface, public MCMacPlatformStubs
{
public:
	int platform_main(int argc, char *argv[], char *envp[]);
	
	// System
	void BreakWait(void);
	bool WaitForEvent(double duration, bool blocking);
	bool GetAbortKeyPressed(void);
	bool GetMouseButtonState(uindex_t button);
	bool GetKeyState(MCPlatformKeyCode*& r_codes, uindex_t& r_code_count);
	MCPlatformModifiers GetModifiersState(void);
	bool GetMouseClick(uindex_t button, MCPoint& r_location);
	void GetMousePosition(MCPoint& r_location);
	void SetMousePosition(MCPoint location);
	void GrabPointer(MCPlatformWindowRef window);
	void UngrabPointer(void);
	void GetWindowAtPoint(MCPoint location, MCPlatformWindowRef& r_window);
	bool GetWindowWithId(uint32_t p_window_id, MCPlatformWindowRef& r_window);
	uint32_t GetEventTime(void);
	void FlushEvents(MCPlatformEventMask mask);
	void Beep(void);
	void GetSystemProperty(MCPlatformSystemProperty p_property, MCPlatformPropertyType p_type, void *r_value);
	void SetSystemProperty(MCPlatformSystemProperty p_property, MCPlatformPropertyType p_type, void *p_value);
	
	// Colorspace
	void CreateColorTransform(const MCColorSpaceInfo& info, MCPlatformColorTransformRef& r_transform);
	void RetainColorTransform(MCPlatformColorTransformRef transform);
	void ReleaseColorTransform(MCPlatformColorTransformRef transform);
	bool ApplyColorTransform(MCPlatformColorTransformRef transform, MCImageBitmap *image);
	
	// Pasteboard
	void DoDragDrop(MCPlatformWindowRef p_window, MCPlatformAllowedDragOperations p_allowed_operations, MCImageBitmap *p_image, const MCPoint *p_image_loc, MCPlatformDragOperation& r_operation);
	// Only used on mac desktop
	bool PasteboardConvertTIFFToPNG(MCDataRef p_in_data, MCDataRef& r_out_data);
	
	// Window
	void CreateWindow(MCPlatformWindowRef &r_window);
	
	// WindowMask
	//void WindowMaskCreate(int32_t width, int32_t height, int32_t stride, void *bits, MCPlatformWindowMaskRef& r_mask);
	void WindowMaskCreateWithAlphaAndRelease(int32_t width, int32_t height, int32_t stride, void *bits, MCPlatformWindowMaskRef& r_mask);
	void WindowMaskRetain(MCPlatformWindowMaskRef mask);
	void WindowMaskRelease(MCPlatformWindowMaskRef mask);
	
	// Dialog
	void BeginFolderDialog(MCPlatformWindowRef owner, MCStringRef p_title, MCStringRef p_message, MCStringRef p_initial);
	MCPlatformDialogResult EndFolderDialog(MCStringRef & r_selected_folder);
	void BeginFileDialog(MCPlatformFileDialogKind p_kind, MCPlatformWindowRef p_owner, MCStringRef p_title, MCStringRef p_prompt,  MCStringRef *p_types, uint4 p_type_count, MCStringRef p_initial);
	MCPlatformDialogResult EndFileDialog(MCPlatformFileDialogKind p_kind, MCStringRef& r_paths, MCStringRef& r_type);
	void BeginColorDialog(MCStringRef p_title, const MCColor& p_color);
	MCPlatformDialogResult EndColorDialog(MCColor& r_new_color);
	
	// Menu
	void CreateMenu(MCPlatformMenuRef& r_menu);
	void RetainMenu(MCPlatformMenuRef menu);
	void ReleaseMenu(MCPlatformMenuRef menu);
	void SetMenuTitle(MCPlatformMenuRef menu, MCStringRef title);
	void CountMenuItems(MCPlatformMenuRef menu, uindex_t& r_count);
	void AddMenuItem(MCPlatformMenuRef menu, uindex_t where);
	void AddMenuSeparatorItem(MCPlatformMenuRef menu, uindex_t where);
	void RemoveMenuItem(MCPlatformMenuRef menu, uindex_t where);
	void RemoveAllMenuItems(MCPlatformMenuRef menu);
	void GetMenuParent(MCPlatformMenuRef menu, MCPlatformMenuRef& r_parent, uindex_t& r_index);
	void GetMenuItemProperty(MCPlatformMenuRef menu, uindex_t index, MCPlatformMenuItemProperty property, MCPlatformPropertyType type, void *r_value);
	void SetMenuItemProperty(MCPlatformMenuRef menu, uindex_t index, MCPlatformMenuItemProperty property, MCPlatformPropertyType type, const void *value);
	bool PopUpMenu(MCPlatformMenuRef menu, MCPlatformWindowRef window, MCPoint location, uindex_t item);
	void SetIconMenu(MCPlatformMenuRef menu);
	void ShowMenubar(void);
	void HideMenubar(void);
	void SetMenubar(MCPlatformMenuRef menu);
	void GetMenubar(MCPlatformMenuRef &r_menu);
	
	// Cursor
	void CreateStandardCursor(MCPlatformStandardCursor standard_cusor, MCPlatformCursorRef& r_cursor);
	void CreateCustomCursor(MCImageBitmap *image, MCPoint hot_spot, MCPlatformCursorRef& r_cursor);
	void RetainCursor(MCPlatformCursorRef cursor);
	void ReleaseCursor(MCPlatformCursorRef cursor);
	void SetCursor(MCPlatformCursorRef cursor);
	void HideCursorUntilMouseMoves(void);
	
	// Print
	void BeginPrintSettingsDialog(MCPlatformWindowRef owner, void *session, void *settings, void *page_format);
	void BeginPageSetupDialog(MCPlatformWindowRef owner, void *session, void *settings, void *page_format);
	MCPlatformPrintDialogResult EndPrintDialog(void);
	
	// Player
	void CreatePlayer(bool p_dont_use_qt, MCPlatformPlayerRef &r_player);
	
	// Script Environment
	void ScriptEnvironmentCreate(MCStringRef p_language, MCPlatformScriptEnvironmentRef &r_environment);
	void ScriptEnvironmentRetain(MCPlatformScriptEnvironmentRef env);
	void ScriptEnvironmentRelease(MCPlatformScriptEnvironmentRef env);
	bool ScriptEnvironmentDefine(MCPlatformScriptEnvironmentRef env, const char *function, MCPlatformScriptEnvironmentCallback callback);
	void ScriptEnvironmentRun(MCPlatformScriptEnvironmentRef env, MCStringRef script, MCStringRef& r_result);
	void ScriptEnvironmentCall(MCPlatformScriptEnvironmentRef env, const char *method, const char **arguments, uindex_t argument_count, char*& r_result);

	//////////
	
	// System - internal
	bool InitializeAbortKey(void);
	void FinalizeAbortKey(void);
	
	// Dialog - internal
	bool folder_path_from_initial_path(MCStringRef p_path, MCStringRef &r_folderpath);
	bool FileFilterCreate(MCStringRef p_desc, MCFileFilter*& r_filter);
	void FileFilterDestroy(MCFileFilter *self);
	
	// Print - internal
	void BeginPrintDialog(MCPlatformWindowRef p_owner, void *p_session, void *p_settings, void *p_page_format, id p_dialog);
	
	// Menu - internal
	void SaveQuittingState();
	void PopQuittingState();
	bool IsInQuittingState(void);
	
	//////////
	
	MCMacPlatformCallbacks* GetCallbacks() { return m_callbacks; }
	
private:
	void BeginOpenSaveDialog(MCPlatformWindowRef p_owner, NSSavePanel *p_panel, MCStringRef p_folder, MCStringRef p_file);
	MCPlatformDialogResult EndOpenSaveDialog(void);

	MCMacPlatformCallbacks *m_callbacks;
};

////////////////////////////////////////////////////////////////////////////////

class MCMacPlatformAutoStringRefAsCFString
{
public:
	MCMacPlatformAutoStringRefAsCFString(MCMacPlatformStubs *p_stubs) :
	m_cfstring(nil), m_stubs(p_stubs)
	{}
	
	~MCMacPlatformAutoStringRefAsCFString(void)
	{
		Unlock();
	}
	
	bool Lock(MCStringRef p_string)
	{
		return m_stubs->MCStringConvertToCFStringRef(p_string, m_cfstring);
	}
	
	void Unlock(void)
	{
		if (m_cfstring != nil)
			CFRelease(m_cfstring);
		m_cfstring = nil;
	}
	
	CFStringRef operator * (void)
	{
		return m_cfstring;
	}
	
private:
	CFStringRef m_cfstring;
	MCMacPlatformStubs *m_stubs;
};

class MCMacPlatformAutoStringRefAsUTF8String
{
public:
	MCMacPlatformAutoStringRefAsUTF8String(MCMacPlatformStubs *p_stubs)
	{
		m_utf8string = nil;
		m_size = 0;
		
		m_stubs = p_stubs;
	}
	
	~MCMacPlatformAutoStringRefAsUTF8String(void)
	{
		Unlock();
	}
	
	bool Lock(MCStringRef p_string)
	{
		return m_stubs->MCStringConvertToUTF8(p_string, m_utf8string, m_size);
	}
	
	void Unlock(void)
	{
		m_stubs->MCMemoryDeleteArray(m_utf8string);
		m_utf8string = nil;
	}
	
	const char *operator * (void) const
	{
		return m_utf8string;
	}
	
	uindex_t Size()
	{
		return m_size;
	}
	
private:
	char *m_utf8string;
	uindex_t m_size;
	
	MCMacPlatformStubs *m_stubs;
};

////////////////////////////////////////////////////////////////////////////////

template<typename T> class MCMacPlatformAutoValueRefBase : public MCMacPlatformStubs
{
public:
	
	inline MCMacPlatformAutoValueRefBase(MCMacPlatformStubs *p_stubs)
	: m_value(nil), m_stubs(p_stubs)
	{
	}
	
	inline MCMacPlatformAutoValueRefBase(const MCMacPlatformAutoValueRefBase& other)
	: m_value(nil)
	{
		m_stubs = other.m_stubs;
		Reset(other.m_value);
	}
	
	MCMacPlatformAutoValueRefBase(MCMacPlatformAutoValueRefBase&& other)
	: m_value(nil)
	{
		m_stubs = other.stubs;
		m_value = other.Take();
	}
	
	inline MCMacPlatformAutoValueRefBase(MCMacPlatformStubs *p_stubs, T p_value)
	: m_value(nil)
	{
		m_stubs = p_stubs;
		if (p_value)
			m_value = MCValueRetain(p_value);
	}
	
	inline ~MCMacPlatformAutoValueRefBase(void)
	{
		MCValueRelease(m_value);
	}
	
	inline T operator = (T value)
	{
		MCAssert(m_value == nil);
		m_value = (T)MCValueRetain(value);
		return value;
	}
	
	MCMacPlatformAutoValueRefBase& operator=(MCMacPlatformAutoValueRefBase&& other)
	{
		Give(other.Take());
		return *this;
	}
	
	inline T& operator & (void)
	{
		MCAssert(m_value == nil);
		return m_value;
	}
	
	inline T operator * (void) const
	{
		return m_value;
	}
	
	bool IsSet() const
	{
		return m_value != nil;
	}
	
	void Reset(T value = nil)
	{
		if (value == m_value)
			return;
		
		if (m_value)
			MCValueRelease(m_value);
		m_value = (value) ? (T)MCValueRetain(value) : NULL;
	}
	
	// The give method places the given value into the container without
	// retaining it - the auto container is considered to now own the value.
	inline void Give(T value)
	{
		if (m_value)
			MCValueRelease(m_value);
		m_value = value;
	}
	
	// The take method removes the value from the container passing ownership
	// to the caller.
	inline T Take(void)
	{
		T t_value;
		t_value = m_value;
		m_value = nil;
		return t_value;
	}
	
	MCMacPlatformCallbacks *GetCallbacks()
	{
		return m_stubs->GetCallbacks();
	}
	
protected:
	T m_value;
	
	// Return the contents of the auto pointer in a form for an in parameter.
	inline T In(void) const
	{
		return m_value;
	}
	
	// Return the contents of the auto pointer in a form for an out parameter.
	inline T& Out(void)
	{
		MCAssert(m_value == nil);
		return m_value;
	}
	
	// Return the contents of the auto pointer in a form for an inout parameter.
	inline T& InOut(void)
	{
		return m_value;
	}
	
private:
	MCMacPlatformAutoValueRefBase<T>& operator = (MCMacPlatformAutoValueRefBase<T>& x);
	
	MCMacPlatformStubs *m_stubs;
};

template<typename T, bool (*MutableCopyAndRelease)(T, T&), bool (*ImmutableCopyAndRelease)(T, T&)>
class MCMacPlatformAutoMutableValueRefBase :
public MCMacPlatformAutoValueRefBase<T>
{
public:
	inline MCMacPlatformAutoMutableValueRefBase(MCMacPlatformStubs *p_stubs) :
	MCMacPlatformAutoValueRefBase<T>(p_stubs)
	{
	}
	
	inline explicit MCMacPlatformAutoMutableValueRefBase(MCMacPlatformStubs *p_stubs, T p_value) :
	MCMacPlatformAutoValueRefBase<T>(p_stubs, p_value)
	{
	}
	
	inline T operator = (T value)
	{
		return MCMacPlatformAutoValueRefBase<T>::operator =(value);
	}
	
	inline bool MakeMutable(void)
	{
		return MutableCopyAndRelease(MCMacPlatformAutoValueRefBase<T>::m_value, MCMacPlatformAutoValueRefBase<T>::m_value);
	}
	
	inline bool MakeImmutable(void)
	{
		return ImmutableCopyAndRelease(MCMacPlatformAutoValueRefBase<T>::m_value, MCMacPlatformAutoValueRefBase<T>::m_value);
	}
	
private:
	MCMacPlatformAutoMutableValueRefBase<T, MutableCopyAndRelease, ImmutableCopyAndRelease>& operator = (MCAutoMutableValueRefBase<T, MutableCopyAndRelease, ImmutableCopyAndRelease>& x);
};

typedef MCMacPlatformAutoValueRefBase<MCValueRef> MCMacPlatformAutoValueRef;
typedef MCMacPlatformAutoValueRefBase<MCNumberRef> MCMacPlatformAutoNumberRef;
typedef MCMacPlatformAutoMutableValueRefBase<MCStringRef, MCStringMutableCopyAndRelease, MCStringCopyAndRelease> MCMacPlatformAutoStringRef;
typedef MCMacPlatformAutoMutableValueRefBase<MCArrayRef, MCArrayMutableCopyAndRelease, MCArrayCopyAndRelease> MCMacPlatformAutoArrayRef;
typedef MCMacPlatformAutoValueRefBase<MCListRef> MCMacPlatformAutoListRef;
typedef MCMacPlatformAutoValueRefBase<MCBooleanRef> MCMacPlatformAutoBooleanRef;
typedef MCMacPlatformAutoMutableValueRefBase<MCSetRef, MCSetMutableCopyAndRelease, MCSetCopyAndRelease> MCMacPlatformAutoSetRef;
typedef MCMacPlatformAutoValueRefBase<MCNameRef> MCMacPlatformNewAutoNameRef;
typedef MCMacPlatformAutoMutableValueRefBase<MCDataRef, MCDataMutableCopyAndRelease, MCDataCopyAndRelease> MCMacPlatformAutoDataRef;
typedef MCMacPlatformAutoValueRefBase<MCTypeInfoRef> MCMacPlatformAutoTypeInfoRef;
typedef MCMacPlatformAutoMutableValueRefBase<MCRecordRef, MCRecordMutableCopyAndRelease, MCRecordCopyAndRelease> MCMacPlatformAutoRecordRef;
typedef MCMacPlatformAutoValueRefBase<MCErrorRef> MCMacPlatformAutoErrorRef;
typedef MCMacPlatformAutoMutableValueRefBase<MCProperListRef, MCProperListMutableCopyAndRelease, MCProperListCopyAndRelease> MCMacPlatformAutoProperListRef;

////////////////////////////////////////////////////////////////////////////////

template<typename T> class MCMacPlatformAutoValueRefArrayBase : public MCMacPlatformStubs
{
public:
	MCMacPlatformAutoValueRefArrayBase(MCMacPlatformStubs *p_stubs)
	{
		m_values = nil;
		m_value_count = 0;
		
		m_stubs = p_stubs;
	}
	
	~MCMacPlatformAutoValueRefArrayBase(void)
	{
		Reset();
	}
	
	MCMacPlatformCallbacks *GetCallbacks()
	{
		return m_stubs->GetCallbacks();
	}
	
	//////////
	
	bool New(uindex_t p_size)
	{
		MCAssert(m_values == nil);
		return MCMemoryNewArray(p_size, m_values, m_value_count);
	}
	
	//////////
	
	void Give(T* p_array, uindex_t p_size)
	{
		MCAssert(m_values == nil);
		m_values = p_array;
		m_value_count = p_size;
	}
	
	void Take(T*& r_array, uindex_t& r_count)
	{
		r_array = m_values;
		r_count = m_value_count;
		
		m_values = nil;
		m_value_count = 0;
	}
	
	/* Take the contents of the array as an immutable MCProperList.
	 * The contents of the array will no longer be accessible. */
	bool TakeAsProperList (MCProperListRef& r_list)
	{
		MCProperListRef t_list;
		if (!MCProperListCreateAndRelease ((MCValueRef *) m_values,
										   m_value_count,
										   t_list))
			return false;
		
		m_values = nil;
		m_value_count = 0;
		
		r_list = t_list;
		return true;
	}
	
	/* Reset the managed array by releasing all the values in the
	 * array and the underlying array storage. */
	void Reset()
	{
		if (m_values != nil)
		{
			for (uindex_t i = 0; i < m_value_count; i++)
				MCValueRelease(m_values[i]);
			MCMemoryDeleteArray(m_values);
		}
	}
	
	//////////
	
	T* Ptr()
	{
		return m_values;
	}
	
	uindex_t Size()
	{
		return m_value_count;
	}
	
	T*& PtrRef()
	{
		MCAssert(m_values == nil);
		return m_values;
	}
	
	uindex_t& CountRef()
	{
		MCAssert(m_value_count == 0);
		return m_value_count;
	}
	
	//////////
	
	bool Resize(uindex_t p_new_size)
	{
		return MCMemoryResizeArray(p_new_size, m_values, m_value_count);
	}
	
	bool Extend(uindex_t p_new_size)
	{
		MCAssert(p_new_size >= m_value_count);
		return Resize(p_new_size);
	}
	
	void Shrink(uindex_t p_new_size)
	{
		MCAssert(p_new_size <= m_value_count);
		Resize(p_new_size);
	}
	
	//////////
	
	bool Append(MCAutoValueRefArrayBase<T> &p_array)
	{
		uindex_t t_index = Count();
		if (!Extend(t_index + p_array.Count()))
			return false;
		
		for (uindex_t i = 0; i < p_array.m_value_count; i++)
			m_values[t_index + i] = MCValueRetain(p_array.m_values[i]);
		
		return true;
	}
	
	bool Push(T p_value)
	{
		if (!Extend(m_value_count + 1))
			return false;
		m_values[m_value_count - 1] = MCValueRetain(p_value);
		return true;
	}
	
	//////////
	
	uindex_t Count(void)
	{
		return m_value_count;
	}
	
	//////////
	
	T* operator * (void) const
	{
		return m_values;
	}
	
	T& operator [] (const uindex_t p_index)
	{
		MCAssert(m_values != nil);
		return m_values[p_index];
	}
	
	MCSpan<T> Span() const
	{
		return MCMakeSpan(m_values, m_value_count);
	}
	
private:
	T* m_values;
	uindex_t m_value_count;
	
	MCMacPlatformStubs *m_stubs;
};

typedef MCMacPlatformAutoValueRefArrayBase<MCValueRef> MCMacPlatformAutoValueRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCNumberRef> MCMacPlatformAutoNumberRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCStringRef> MCMacPlatformAutoStringRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCDataRef> MCMacPlatformAutoDataRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCArrayRef> MCMacPlatformAutoArrayRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCListRef> MCMacPlatformAutoListRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCBooleanRef> MCMacPlatformAutoBooleanRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCSetRef> MCMacPlatformAutoSetRefArray;
typedef MCMacPlatformAutoValueRefArrayBase<MCNameRef> MCMacPlatformAutoNameRefArray;

////////////////////////////////////////////////////////////////////////////////

#endif //__MAC_PLATFORM_H__
