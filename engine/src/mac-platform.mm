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

#include "mac-platform.h"

////////////////////////////////////////////////////////////////////////////////
// Engine stubs

Boolean MCMacPlatformStubs::MCS_exists(MCStringRef p_path, bool p_is_file)
{
	return GetCallbacks()->MCS_exists(p_path, p_is_file);
}

bool MCMacPlatformStubs::MCS_resolvepath(MCStringRef p_path, MCStringRef& r_resolved)
{
	return GetCallbacks()->MCS_resolvepath(p_path, r_resolved);
}

bool MCMacPlatformStubs::MCStringsSplit(MCStringRef p_string, codepoint_t p_separator, MCStringRef*&r_strings, uindex_t& r_count)
{
	return GetCallbacks()->MCStringsSplit(p_string, p_separator, r_strings, r_count);
}

bool MCMacPlatformStubs::MCS_pathtonative(MCStringRef p_livecode_path, MCStringRef& r_native_path)
{
    return GetCallbacks()->MCS_pathtonative(p_livecode_path, r_native_path);
}

////////////////////////////////////////////////////////////////////////////////
// module stubs

bool MCMacPlatformStubs::MCModulesInitialize(void)
{
	return GetCallbacks()->MCModulesInitialize();
}

void MCMacPlatformStubs::MCModulesFinalize(void)
{
	GetCallbacks()->MCModulesFinalize();
}

////////////////////////////////////////////////////////////////////////////////
// libscript stubs

bool MCMacPlatformStubs::MCScriptInitialize(void)
{
	return GetCallbacks()->MCScriptInitialize();
}

void MCMacPlatformStubs::MCScriptFinalize(void)
{
	GetCallbacks()->MCScriptFinalize();
}

////////////////////////////////////////////////////////////////////////////////
// libfoundation stubs

bool MCMacPlatformStubs::MCInitialize(void)
{
	return GetCallbacks()->MCInitialize();
}

void MCMacPlatformStubs::MCFinalize(void)
{
	GetCallbacks()->MCFinalize();
}

bool MCMacPlatformStubs::MCSInitialize(void)
{
	return GetCallbacks()->MCSInitialize();
}

void MCMacPlatformStubs::MCSFinalize(void)
{
	GetCallbacks()->MCSFinalize();
}

bool MCMacPlatformStubs::MCMemoryAllocate(size_t size, void*& r_block)
{
	return GetCallbacks()->MCMemoryAllocate(size, r_block);
}

bool MCMacPlatformStubs::MCMemoryAllocateCopy(const void *block, size_t size, void*& r_new_block)
{
	return GetCallbacks()->MCMemoryAllocateCopy(block, size, r_new_block);
}

bool MCMacPlatformStubs::MCMemoryReallocate(void *block, size_t new_size, void*& r_new_block)
{
	return GetCallbacks()->MCMemoryReallocate(block, new_size, r_new_block);
}

void MCMacPlatformStubs::MCMemoryDeallocate(void *block)
{
	GetCallbacks()->MCMemoryDeallocate(block);
}

bool MCMacPlatformStubs::MCMemoryNew(size_t size, void*& r_record)
{
	return GetCallbacks()->MCMemoryNew(size, r_record);
}

void MCMacPlatformStubs::MCMemoryDelete(void *p_record)
{
	GetCallbacks()->MCMemoryDelete(p_record);
}

bool MCMacPlatformStubs::MCMemoryNewArray(uindex_t count, size_t size, void*& r_array)
{
	return GetCallbacks()->MCMemoryNewArray(count, size, r_array);
}

bool MCMacPlatformStubs::MCMemoryResizeArray(uindex_t new_count, size_t size, void*& x_array, uindex_t& x_count)
{
	return GetCallbacks()->MCMemoryResizeArray(new_count, size, x_array, x_count);
}

void MCMacPlatformStubs::MCMemoryDeleteArray(void *p_array)
{
	GetCallbacks()->MCMemoryDeleteArray(p_array);
}

MCValueRef MCMacPlatformStubs::MCValueRetain(MCValueRef p_value)
{
	return GetCallbacks()->MCValueRetain(p_value);
}

void MCMacPlatformStubs::MCValueRelease(MCValueRef p_value)
{
	GetCallbacks()->MCValueRelease(p_value);
}

uindex_t MCMacPlatformStubs::MCDataGetLength(MCDataRef p_data)
{
	return GetCallbacks()->MCDataGetLength(p_data);
}

bool MCMacPlatformStubs::MCDataCreateWithBytes(const byte_t *p_bytes, uindex_t p_byte_count, MCDataRef& r_data)
{
	return GetCallbacks()->MCDataCreateWithBytes(p_bytes, p_byte_count, r_data);
}

const byte_t *MCMacPlatformStubs::MCDataGetBytePtr(MCDataRef p_data)
{
	return GetCallbacks()->MCDataGetBytePtr(p_data);
}

MCStringRef MCMacPlatformStubs::MCSTR(const char *p_cstring)
{
	return GetCallbacks()->MCSTR(p_cstring);
}

bool MCMacPlatformStubs::MCStringAppendFormat(MCStringRef string, const char *format, ...)
{
	bool t_success;
	va_list t_args;
	va_start(t_args, format);
	t_success = MCStringAppendFormatV(string, format, t_args);
	va_end(t_args);
	return t_success;
}

bool MCMacPlatformStubs::MCStringAppendFormatV(MCStringRef string, const char *format, va_list args)
{
	return GetCallbacks()->MCStringAppendFormatV(string, format, args);
}

bool MCMacPlatformStubs::MCStringConvertToUTF8(MCStringRef p_string, char*& r_utf8string, uindex_t& r_utf8_chars)
{
	return GetCallbacks()->MCStringConvertToUTF8(p_string, r_utf8string, r_utf8_chars);
}

bool MCMacPlatformStubs::MCStringCopy(MCStringRef self, MCStringRef& r_new_string)
{
	return GetCallbacks()->MCStringCopy(self, r_new_string);
}

bool MCMacPlatformStubs::MCStringCopySubstring(MCStringRef self, MCRange p_range, MCStringRef& r_substring)
{
	return GetCallbacks()->MCStringCopySubstring(self, p_range, r_substring);
}

bool MCMacPlatformStubs::MCStringCreateMutable(uindex_t p_initial_capacity, MCStringRef &r_string)
{
	return GetCallbacks()->MCStringCreateMutable(p_initial_capacity, r_string);
}

bool MCMacPlatformStubs::MCStringCreateWithBytes(const byte_t *p_bytes, uindex_t p_byte_count, MCStringEncoding p_encoding, bool p_is_external_rep, MCStringRef& r_string)
{
	return GetCallbacks()->MCStringCreateWithBytes(p_bytes, p_byte_count, p_encoding, p_is_external_rep, r_string);
}

bool MCMacPlatformStubs::MCStringFirstIndexOfChar(MCStringRef self, codepoint_t p_needle, uindex_t p_after, MCStringOptions p_options, uindex_t& r_offset)
{
	return GetCallbacks()->MCStringFirstIndexOfChar(self, p_needle, p_after, p_options, r_offset);
}

uindex_t MCMacPlatformStubs::MCStringGetLength(MCStringRef self)
{
	return GetCallbacks()->MCStringGetLength(self);
}

bool MCMacPlatformStubs::MCStringIsEmpty(MCStringRef string)
{
	return GetCallbacks()->MCStringIsEmpty(string);
}

bool MCMacPlatformStubs::MCStringIsEqualToCString(MCStringRef p_string, const char *p_cstring, MCStringOptions p_options)
{
	return GetCallbacks()->MCStringIsEqualToCString(p_string, p_cstring, p_options);
}

bool MCMacPlatformStubs::MCStringLastIndexOfChar(MCStringRef self, codepoint_t p_needle, uindex_t p_before, MCStringOptions p_options, uindex_t& r_offset)
{
	return GetCallbacks()->MCStringLastIndexOfChar(self, p_needle, p_before, p_options, r_offset);
}

bool MCMacPlatformStubs::MCStringSubstringIsEqualTo(MCStringRef self, MCRange p_sub, MCStringRef p_other, MCStringOptions p_options)
{
	return GetCallbacks()->MCStringSubstringIsEqualTo(self, p_sub, p_other, p_options);
}

//////////

NSString *MCMacPlatformStubs::MCStringConvertToAutoreleasedNSString(MCStringRef p_string_ref)
{
	return GetCallbacks()->MCStringConvertToAutoreleasedNSString(p_string_ref);
}

bool MCMacPlatformStubs::MCStringConvertToCFStringRef(MCStringRef p_string, CFStringRef& r_cfstring)
{
	return GetCallbacks()->MCStringConvertToCFStringRef(p_string, r_cfstring);
}

bool MCMacPlatformStubs::MCStringCreateWithCFString(CFStringRef p_cf_string, MCStringRef& r_string)
{
	return GetCallbacks()->MCStringCreateWithCFString(p_cf_string, r_string);
}

bool MCMacPlatformStubs::MCS_mac_elevation_bootstrap_main(int argc, char *argv[])
{
	return GetCallbacks()->MCS_mac_elevation_bootstrap_main(argc, argv);
}

// Legacy

void MCMacPlatformStubs::MCListPushBack(void*& x_list, void *p_element)
{
	GetCallbacks()->MCListPushBack(x_list, p_element);
}

void *MCMacPlatformStubs::MCListPopFront(void*& x_list)
{
	return GetCallbacks()->MCListPopFront(x_list);
}

#if defined(DEBUG_LOG)
void MCMacPlatformStubs::__MCAssert(const char *file, uint32_t line, const char *message)
{
	GetCallbacks()->__MCAssert(file, line, message);
}

void MCMacPlatformStubs::__MCUnreachable(void)
{
	GetCallbacks()->__MCUnreachable();
}

////////////////////////////////////////////////////////////////////////////////
// libgraphics stubs

bool MCMacPlatformStubs::MCImageGetCGColorSpace(CGColorSpaceRef &r_colorspace)
{
    return GetCallbacks()->MCImageGetCGColorSpace(r_colorspace);
}

int32_t MCMacPlatformStubs::MCGImageGetWidth(MCGImageRef image)
{
	return GetCallbacks()->MCGImageGetWidth(image);
}

int32_t MCMacPlatformStubs::MCGImageGetHeight(MCGImageRef image)
{
	return GetCallbacks()->MCGImageGetHeight(image);
}

bool MCMacPlatformStubs::MCGImageToCGImage(MCGImageRef p_src, const MCGIntegerRectangle &p_src_rect, bool p_invert, CGImageRef &r_image)
{
    return GetCallbacks()->MCGImageToCGImage(p_src, p_src_rect, p_invert, r_image);
}

bool MCMacPlatformStubs::MCGRasterToCGImage(const MCGRaster &p_raster, const MCGIntegerRectangle &p_src_rect, CGColorSpaceRef p_colorspace, bool p_copy, bool p_invert, CGImageRef &r_image)
{
    return GetCallbacks()->MCGRasterToCGImage(p_raster, p_src_rect, p_colorspace, p_copy, p_invert, r_image);
}

bool MCMacPlatformStubs::MCGRegionCreate(MCGRegionRef &r_region)
{
	return GetCallbacks()->MCGRegionCreate(r_region);
}

void MCMacPlatformStubs::MCGRegionDestroy(MCGRegionRef p_region)
{
	GetCallbacks()->MCGRegionDestroy(p_region);
}

MCGIntegerRectangle MCMacPlatformStubs::MCGRegionGetBounds(MCGRegionRef p_region)
{
	return GetCallbacks()->MCGRegionGetBounds(p_region);
}

bool MCMacPlatformStubs::MCGRegionAddRect(MCGRegionRef p_region, const MCGIntegerRectangle &p_rect)
{
	return GetCallbacks()->MCGRegionAddRect(p_region, p_rect);
}

bool MCMacPlatformStubs::MCGRegionIterate(MCGRegionRef p_region, MCGRegionIterateCallback p_callback, void *p_context)
{
	return GetCallbacks()->MCGRegionIterate(p_region, p_callback, p_context);
}

bool MCMacPlatformStubs::MCGContextCreateWithRaster(const MCGRaster& raster, MCGContextRef& r_context)
{
	return GetCallbacks()->MCGContextCreateWithRaster(raster, r_context);
}

void MCMacPlatformStubs::MCGContextRelease(MCGContextRef context)
{
	GetCallbacks()->MCGContextRelease(context);
}

void MCMacPlatformStubs::MCGContextClipToRect(MCGContextRef context, MCGRectangle rect)
{
	GetCallbacks()->MCGContextClipToRect(context, rect);
}

void MCMacPlatformStubs::MCGContextClipToRegion(MCGContextRef self, MCGRegionRef p_region)
{
	GetCallbacks()->MCGContextClipToRegion(self, p_region);
}

void MCMacPlatformStubs::MCGContextTranslateCTM(MCGContextRef context, MCGFloat xoffset, MCGFloat yoffset)
{
	GetCallbacks()->MCGContextTranslateCTM(context, xoffset, yoffset);
}

void MCMacPlatformStubs::MCGContextScaleCTM(MCGContextRef context, MCGFloat xscale, MCGFloat yscale)
{
	GetCallbacks()->MCGContextScaleCTM(context, xscale, yscale);
}

void MCMacPlatformStubs::MCImageFreeBitmap(MCImageBitmap *p_bitmap)
{
    GetCallbacks()->MCImageFreeBitmap(p_bitmap);
}

void MCMacPlatformStubs::MCImageBitmapClear(MCImageBitmap *p_bitmap)
{
    GetCallbacks()->MCImageBitmapClear(p_bitmap);
}

bool MCMacPlatformStubs::MCImageBitmapCreate(uindex_t p_width, uindex_t p_height, MCImageBitmap *&r_bitmap)
{
    return GetCallbacks()->MCImageBitmapCreate(p_width, p_height, r_bitmap);
}

void MCMacPlatformStubs::MCGRasterApplyAlpha(MCGRaster &x_raster, const MCGRaster &p_alpha, const MCGIntegerPoint &p_offset)
{
    GetCallbacks()->MCGRasterApplyAlpha(x_raster, p_alpha, p_offset);
}

bool MCMacPlatformStubs::MCRegionForEachRect(MCRegionRef p_region, MCRegionForEachRectCallback p_callback, void *p_context)
{
    return GetCallbacks()->MCRegionForEachRect(p_region, p_callback, p_context);
}

bool MCMacPlatformStubs::MCImageBitmapToCGImage(MCImageBitmap *p_bitmap, bool p_copy, bool p_invert, CGImageRef &r_image)
{
    return GetCallbacks()->MCImageBitmapToCGImage(p_bitmap, p_copy, p_invert, r_image);
}

OSErr MCMacPlatformStubs::MCAppleEventHandlerDoOpenDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long refCon)
{
    return GetCallbacks()->MCAppleEventHandlerDoOpenDoc(theAppleEvent, reply, refCon);
}

OSErr MCMacPlatformStubs::MCAppleEventHandlerDoAEAnswer(const AppleEvent *theAppleEvent, AppleEvent *reply, long refCon)
{
    return GetCallbacks()->MCAppleEventHandlerDoAEAnswer(theAppleEvent, reply, refCon);
}

OSErr MCMacPlatformStubs::MCAppleEventHandlerDoSpecial(const AppleEvent *theAppleEvent, AppleEvent *reply, long refCon)
{
    return GetCallbacks()->MCAppleEventHandlerDoSpecial(theAppleEvent, reply, refCon);
}

CGBitmapInfo MCMacPlatformStubs::MCGPixelFormatToCGBitmapInfo(uint32_t p_pixel_format, bool p_alpha)
{
    return GetCallbacks()->MCGPixelFormatToCGBitmapInfo(p_pixel_format, p_alpha);
}

bool MCMacPlatformStubs::MCOSXCreateCGContextForBitmap(MCImageBitmap *p_bitmap, CGContextRef &r_context)
{
    return GetCallbacks()->MCOSXCreateCGContextForBitmap(p_bitmap, r_context);
}

bool MCMacPlatformStubs::MCColorTransformLinearRGBToXYZ(const MCColorVector2 &p_white, const MCColorVector2 &p_red, const MCColorVector2 &p_green, const MCColorVector2 &p_blue,
                                    MCColorVector3 &r_white, MCColorMatrix3x3 &r_matrix)
{
    return GetCallbacks()->MCColorTransformLinearRGBToXYZ(p_white, p_red, p_green, p_blue, r_white, r_matrix);
}

bool MCMacPlatformStubs::MCPlatformInitializeColorTransform()
{
    return GetCallbacks()->MCPlatformInitializeColorTransform();
}

void MCMacPlatformStubs::MCPlatformFinalizeColorTransform()
{
    return GetCallbacks()->MCPlatformFinalizeColorTransform();
}


// Globals
bool MCMacPlatformStubs::GetGlobalProperty(MCPlatformGlobalProperty p_property, MCPlatformPropertyType p_type, void *r_value)
{
    return GetCallbacks()->GetGlobalProperty(p_property, p_type, r_value);
}

char * MCMacPlatformStubs::osx_cfstring_to_cstring(CFStringRef p_string, bool p_release, bool p_utf8)
{
    return GetCallbacks()->osx_cfstring_to_cstring(p_string, p_release, p_utf8);
}

// menus

bool MCMacPlatformStubs::MCPlatformInitializeMenu()
{
    return GetCallbacks()->MCPlatformInitializeMenu();
}

void MCMacPlatformStubs::MCPlatformFinalizeMenu()
{
    return GetCallbacks()->MCPlatformFinalizeMenu();
}

// events
#if defined(FEATURE_PLATFORM_APPLICATION)

void MCMacPlatformStubs::MCPlatformCallbackSendApplicationStartup(int p_argc, MCStringRef *p_argv, MCStringRef *p_envp, int& r_error_code, MCStringRef & r_error_message)
{
    GetCallbacks()->MCPlatformCallbackSendApplicationStartup( p_argc, p_argv, p_envp, r_error_code, r_error_message);
}

void MCMacPlatformStubs::MCPlatformCallbackSendApplicationShutdown(int& r_exit_code)
{
    GetCallbacks()->MCPlatformCallbackSendApplicationShutdown(r_exit_code);
}

void MCMacPlatformStubs::MCPlatformCallbackSendApplicationShutdownRequest(bool& r_terminate)
{
    GetCallbacks()->MCPlatformCallbackSendApplicationShutdownRequest(r_terminate);
}

void MCMacPlatformStubs::MCPlatformCallbackSendApplicationRun(bool& r_continue)
{
    GetCallbacks()->MCPlatformCallbackSendApplicationRun(r_continue);
}

void MCMacPlatformStubs::MCPlatformCallbackSendApplicationSuspend(void)
{
    GetCallbacks()->MCPlatformCallbackSendApplicationSuspend();
}

void MCMacPlatformStubs::MCPlatformCallbackSendApplicationResume(void)
{
    GetCallbacks()->MCPlatformCallbackSendApplicationResume();
}

#endif // FEATURE_PLATFORM_APPLICATION

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendScreenParametersChanged(void)
{
    GetCallbacks()->MCPlatformCallbackSendScreenParametersChanged();
}

//////////

#if defined(FEATURE_PLATFORM_WINDOW)

void MCMacPlatformStubs::MCPlatformCallbackSendWindowCloseRequest(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowCloseRequest(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowClose(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowClose(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowCancel(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowCancel(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowReshape(MCPlatformWindowRef p_window, MCRectangle p_new_content)
{
    GetCallbacks()->MCPlatformCallbackSendWindowReshape(p_window, p_new_content);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowConstrain(MCPlatformWindowRef p_window, MCPoint p_proposed_size, MCPoint& r_wanted_size)
{
    GetCallbacks()->MCPlatformCallbackSendWindowConstrain( p_window, p_proposed_size, r_wanted_size);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowRedraw(MCPlatformWindowRef p_window, MCPlatformSurfaceRef p_surface, MCGRegionRef p_dirty_rgn)
{
    GetCallbacks()->MCPlatformCallbackSendWindowRedraw(p_window, p_surface, p_dirty_rgn);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowIconify(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowIconify(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowUniconify(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowUniconify(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowFocus(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowFocus( p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendWindowUnfocus(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendWindowUnfocus(p_window);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendModifiersChanged(MCPlatformModifiers p_modifiers)
{
    GetCallbacks()->MCPlatformCallbackSendModifiersChanged(p_modifiers);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendMouseEnter(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendMouseEnter(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseLeave(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendMouseLeave( p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseDown(MCPlatformWindowRef p_window, uint32_t p_button, uint32_t p_count)
{
    GetCallbacks()->MCPlatformCallbackSendMouseDown( p_window, p_button, p_count);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseUp(MCPlatformWindowRef p_window, uint32_t p_button, uint32_t p_count)
{
    GetCallbacks()->MCPlatformCallbackSendMouseUp( p_window, p_button, p_count);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseDrag(MCPlatformWindowRef p_window, uint32_t p_button)
{
    GetCallbacks()->MCPlatformCallbackSendMouseDrag( p_window, p_button);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseRelease(MCPlatformWindowRef p_window, uint32_t p_button, bool p_was_menu)
{
    GetCallbacks()->MCPlatformCallbackSendMouseRelease( p_window, p_button, p_was_menu);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseMove(MCPlatformWindowRef p_window, MCPoint p_location)
{
    GetCallbacks()->MCPlatformCallbackSendMouseMove( p_window, p_location);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMouseScroll(MCPlatformWindowRef p_window, int dx, int dy)
{
    GetCallbacks()->MCPlatformCallbackSendMouseScroll( p_window, dx, dy);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendDragEnter(MCPlatformWindowRef p_window, MCPlatformClipboardRef p_dragboard, MCPlatformDragOperation& r_operation)
{
    GetCallbacks()->MCPlatformCallbackSendDragEnter( p_window, p_dragboard, r_operation);
}

void MCMacPlatformStubs::MCPlatformCallbackSendDragLeave(MCPlatformWindowRef p_window)
{
    GetCallbacks()->MCPlatformCallbackSendDragLeave(p_window);
}

void MCMacPlatformStubs::MCPlatformCallbackSendDragMove(MCPlatformWindowRef p_window, MCPoint p_location, MCPlatformDragOperation& r_operation)
{
    GetCallbacks()->MCPlatformCallbackSendDragMove(p_window, p_location, r_operation);
}

void MCMacPlatformStubs::MCPlatformCallbackSendDragDrop(MCPlatformWindowRef p_window, bool& r_accepted)
{
    GetCallbacks()->MCPlatformCallbackSendDragDrop( p_window, r_accepted);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendRawKeyDown(MCPlatformWindowRef p_window, MCPlatformKeyCode p_key_code, codepoint_t p_mapped_codepoint, codepoint_t p_unmapped_codepoint)
{
    GetCallbacks()->MCPlatformCallbackSendRawKeyDown( p_window, p_key_code, p_mapped_codepoint, p_unmapped_codepoint);
}

void MCMacPlatformStubs::MCPlatformCallbackSendKeyDown(MCPlatformWindowRef p_window, MCPlatformKeyCode p_key_code, codepoint_t p_mapped_codepoint, codepoint_t p_unmapped_codepoint)
{
    GetCallbacks()->MCPlatformCallbackSendKeyDown( p_window, p_key_code, p_mapped_codepoint, p_unmapped_codepoint);
}

void MCMacPlatformStubs::MCPlatformCallbackSendKeyUp(MCPlatformWindowRef p_window, MCPlatformKeyCode p_key_code, codepoint_t p_mapped_codepoint, codepoint_t p_unmapped_codepoint)
{
    GetCallbacks()->MCPlatformCallbackSendKeyUp( p_window, p_key_code, p_mapped_codepoint, p_unmapped_codepoint);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendTextInputQueryTextRanges(MCPlatformWindowRef p_window, MCRange& r_marked_range, MCRange& r_selected_range)
{
    GetCallbacks()->MCPlatformCallbackSendTextInputQueryTextRanges( p_window, r_marked_range, r_selected_range);
}

void MCMacPlatformStubs::MCPlatformCallbackSendTextInputQueryTextIndex(MCPlatformWindowRef p_window, MCPoint p_location, uindex_t& r_index)
{
    GetCallbacks()->MCPlatformCallbackSendTextInputQueryTextIndex( p_window, p_location, r_index);
}

void MCMacPlatformStubs::MCPlatformCallbackSendTextInputQueryTextRect(MCPlatformWindowRef p_window, MCRange p_range, MCRectangle& r_first_line_rect, MCRange& r_actual_range)
{
    GetCallbacks()->MCPlatformCallbackSendTextInputQueryTextRect( p_window, p_range, r_first_line_rect, r_actual_range);
}

void MCMacPlatformStubs::MCPlatformCallbackSendTextInputQueryText(MCPlatformWindowRef p_window, MCRange p_range, unichar_t*& r_chars, uindex_t& r_char_count, MCRange& r_actual_range)
{
    GetCallbacks()->MCPlatformCallbackSendTextInputQueryText( p_window, p_range, r_chars, r_char_count, r_actual_range);
}

void MCMacPlatformStubs::MCPlatformCallbackSendTextInputInsertText(MCPlatformWindowRef p_window, unichar_t *p_chars, uindex_t p_char_count, MCRange p_replace_range, MCRange p_selection_range, bool p_mark)
{
    GetCallbacks()->MCPlatformCallbackSendTextInputInsertText(p_window, p_chars, p_char_count, p_replace_range, p_selection_range, p_mark);
}

void MCMacPlatformStubs::MCPlatformCallbackSendTextInputAction(MCPlatformWindowRef p_window, MCPlatformTextInputAction p_action)
{
    GetCallbacks()->MCPlatformCallbackSendTextInputAction(p_window, p_action);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendMenuUpdate(MCPlatformMenuRef p_menu)
{
    GetCallbacks()->MCPlatformCallbackSendMenuUpdate(p_menu);
}

void MCMacPlatformStubs::MCPlatformCallbackSendMenuSelect(MCPlatformMenuRef p_menu, uindex_t p_index)
{
    GetCallbacks()->MCPlatformCallbackSendMenuSelect(p_menu, p_index);
}

//////////

void MCMacPlatformStubs::MCPlatformCallbackSendViewFocusSwitched(MCPlatformWindowRef p_window, uint32_t p_view_id)
{
    GetCallbacks()->MCPlatformCallbackSendViewFocusSwitched(p_window, p_view_id);
}

#endif // FEATURE_PLATFORM_WINDOW

//////////

#if defined(FEATURE_PLATFORM_PLAYER)

void MCMacPlatformStubs::MCPlatformCallbackSendPlayerFrameChanged(MCPlatformPlayerRef p_player)
{
    GetCallbacks()->MCPlatformCallbackSendPlayerFrameChanged(p_player);
}

void MCMacPlatformStubs::MCPlatformCallbackSendPlayerMarkerChanged(MCPlatformPlayerRef p_player, MCPlatformPlayerDuration p_time)
{
    GetCallbacks()->MCPlatformCallbackSendPlayerMarkerChanged(p_player, p_time);
}

void MCMacPlatformStubs::MCPlatformCallbackSendPlayerCurrentTimeChanged(MCPlatformPlayerRef p_player)
{
    GetCallbacks()->MCPlatformCallbackSendPlayerCurrentTimeChanged(p_player);
}

void MCMacPlatformStubs::MCPlatformCallbackSendPlayerFinished(MCPlatformPlayerRef p_player)
{
    GetCallbacks()->MCPlatformCallbackSendPlayerFinished(p_player);
}

void MCMacPlatformStubs::MCPlatformCallbackSendPlayerBufferUpdated(MCPlatformPlayerRef p_player)
{
    GetCallbacks()->MCPlatformCallbackSendPlayerBufferUpdated(p_player);
}

#endif // FEATURE_PLATFORM_PLAYER

//////////

#if defined(FEATURE_PLATFORM_AUDIO)

void MCMacPlatformStubs::MCPlatformCallbackSendSoundFinished(MCPlatformSoundRef p_sound)
{
    GetCallbacks()->MCPlatformCallbackSendSoundFinished(p_sound);
}

#endif // FEATURE_PLATFORM_AUDIO

// Fonts

bool MCMacPlatformStubs::MCFontCreateWithHandle(MCSysFontHandle p_handle, MCNameRef p_name, MCFontRef& r_font)
{
    return GetCallbacks()->MCFontCreateWithHandle(p_handle, p_name, r_font);
}

#endif
