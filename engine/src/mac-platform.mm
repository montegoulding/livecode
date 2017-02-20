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

int32_t MCMacPlatformStubs::MCGImageGetWidth(MCGImageRef image)
{
	return GetCallbacks()->MCGImageGetWidth(image);
}

int32_t MCMacPlatformStubs::MCGImageGetHeight(MCGImageRef image)
{
	return GetCallbacks()->MCGImageGetHeight(image);
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

#endif
