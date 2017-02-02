#include "platform.h"
#include "platform-internal.h"
#include "color.h"
#if defined(TARGET_PLATFORM_MACOS_X)
#include <Carbon/Carbon.h>
#endif

void MCPlatformHandleApplicationStartup(int p_argc, MCStringRef *p_argv, MCStringRef *p_envp, int& r_error_code, MCStringRef& r_error_message) { return; }
void MCPlatformHandleApplicationShutdown(int& r_exit_code) { return; }
void MCPlatformHandleApplicationShutdownRequest(bool& r_terminate) { return; }
void MCPlatformHandleApplicationSuspend(void) { return; }
void MCPlatformHandleApplicationResume(void) { return; }
void MCPlatformHandleApplicationRun(bool& r_continue) { return; }

void MCPlatformHandleScreenParametersChanged(void) { return; }

void MCPlatformHandleWindowCloseRequest(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowClose(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowCancel(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowIconify(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowUniconify(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowReshape(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowFocus(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowUnfocus(MCPlatformWindowRef window) { return; }
void MCPlatformHandleWindowRedraw(MCPlatformWindowRef window, MCPlatformSurfaceRef surface, MCGRegionRef dirty_rgn) { return; }
void MCPlatformHandleWindowConstrain(MCPlatformWindowRef window, MCPoint proposed_size, MCPoint& r_wanted_size) { return; }

void MCPlatformHandleModifiersChanged(MCPlatformModifiers modifiers) { return; }

void MCPlatformHandleRawKeyDown(MCPlatformWindowRef window, MCPlatformKeyCode key_code, codepoint_t mapped_codepoint, codepoint_t unmapped_codepoint) { return; }
void MCPlatformHandleKeyDown(MCPlatformWindowRef window, MCPlatformKeyCode key_code, codepoint_t mapped_codepoint, codepoint_t unmapped_codepoint) { return; }
void MCPlatformHandleKeyUp(MCPlatformWindowRef window, MCPlatformKeyCode key_code, codepoint_t mapped_codepoint, codepoint_t unmapped_codepoint) { return; }

void MCPlatformHandleTextInputQueryTextRanges(MCPlatformWindowRef window, MCRange& r_marked_range, MCRange& r_selected_range) { return; }
void MCPlatformHandleTextInputQueryTextIndex(MCPlatformWindowRef window, MCPoint location, uindex_t& r_index) { return; }
void MCPlatformHandleTextInputQueryTextRect(MCPlatformWindowRef window, MCRange range, MCRectangle& first_line_rect, MCRange& r_actual_range) { return; }
void MCPlatformHandleTextInputQueryText(MCPlatformWindowRef window, MCRange range, unichar_t*& r_chars, uindex_t& r_char_count, MCRange& r_actual_range) { return; }
void MCPlatformHandleTextInputInsertText(MCPlatformWindowRef window, unichar_t *chars, uindex_t char_count, MCRange replace_range, MCRange selection_range, bool mark) { return; }
void MCPlatformHandleTextInputAction(MCPlatformWindowRef window, MCPlatformTextInputAction action) { return; }

void MCPlatformHandleMouseEnter(MCPlatformWindowRef window) { return; }
void MCPlatformHandleMouseLeave(MCPlatformWindowRef window) { return; }
void MCPlatformHandleMouseDown(MCPlatformWindowRef window, uint32_t button, uint32_t count) { return; }
void MCPlatformHandleMouseUp(MCPlatformWindowRef window, uint32_t button, uint32_t count) { return; }
void MCPlatformHandleMouseDrag(MCPlatformWindowRef window, uint32_t button) { return; }
void MCPlatformHandleMouseRelease(MCPlatformWindowRef window, uint32_t button, bool was_menu) { return; }
void MCPlatformHandleMouseMove(MCPlatformWindowRef window, MCPoint location) { return; }
void MCPlatformHandleMouseScroll(MCPlatformWindowRef window, int dx, int dy) { return; }

void MCPlatformHandleDragEnter(MCPlatformWindowRef window, class MCRawClipboard* p_clipboard, MCPlatformDragOperation& r_operation) { return; }
void MCPlatformHandleDragMove(MCPlatformWindowRef window, MCPoint location, MCPlatformDragOperation& r_operation) { return; }
void MCPlatformHandleDragLeave(MCPlatformWindowRef window) { return; }
void MCPlatformHandleDragDrop(MCPlatformWindowRef window, bool& r_accepted) { return; }

void MCPlatformHandleMenuUpdate(MCPlatformMenuRef menu) { return; }
void MCPlatformHandleMenuSelect(MCPlatformMenuRef menu, uindex_t index) { return; }

void MCPlatformHandleViewFocusSwitched(MCPlatformWindowRef window, uint32_t id) { return; }

void MCPlatformHandlePlayerFrameChanged(MCPlatformPlayerRef player) { return; }
void MCPlatformHandlePlayerMarkerChanged(MCPlatformPlayerRef player, MCPlatformPlayerDuration time) { return; }
void MCPlatformHandlePlayerCurrentTimeChanged(MCPlatformPlayerRef player) { return; }
void MCPlatformHandlePlayerFinished(MCPlatformPlayerRef player) { return; }
void MCPlatformHandlePlayerBufferUpdated(MCPlatformPlayerRef player) { return; }

void MCPlatformHandleSoundFinished(MCPlatformSoundRef sound) { return; }

void* MCListPopFront(void*&) {return nullptr;}
void MCListPushBack(void*&, void*) {}
bool MCRegionCreate(MCRegionRef&) {return false;}
bool MCStringsSplit(MCStringRef p_string, codepoint_t p_separator, MCStringRef*&r_strings, uindex_t& r_count) {return false;}

void __MCAssert(const char *, unsigned int, char const *) {abort();}
bool MCGRegionCreate(MCGRegionRef&) {return false;}
void MCGRegionDestroy(MCGRegionRef) {}
void MCRegionDestroy(MCRegionRef) {}
bool MCRegionIsEmpty(MCRegionRef) {return false;}
bool MCS_resolvepath(MCStringRef, MCStringRef&) {return false;}
void __MCUnreachable(void) {abort();}
int32_t MCGImageGetWidth(MCGImageRef) {return 0;}
bool MCGRegionAddRect(MCGRegionRef, MCGIntegerRectangle const&) {return false;}
bool MCMemoryNewArray(unsigned int, unsigned long, void*&) { return false; }
void MCRegionSetEmpty(MCRegionRef) {}
bool MCS_pathtonative(MCStringRef, MCStringRef&) { return false;}
void MCGContextRelease(MCGContextRef);
bool MCGImageToCGImage(MCGImageRef, MCGIntegerRectangle const&, bool, CGImageRef&){return false;}


void MCFinalize() {return;}
bool MCS_exists(__MCString*, bool) {return false;}
bool MCInitialize() {return false;}
bool MCGRegionIterate(__MCGRegion*, bool (*)(void*, MCGIntegerRectangle const&), void*) {return false;}
void MCScriptFinalize() {}
void MCGContextRelease(__MCGContext*) {}
int32_t MCGImageGetHeight(__MCGImage*) {return 0;}
void MCImageFreeBitmap(MCImageBitmap*) {}
bool MCRegionAddRegion(__MCRegion*, __MCRegion*) {return false;}
void MCGContextScaleCTM(__MCGContext*, float, float) {return;}
bool MCGRasterToCGImage(MCGRaster const&, MCGIntegerRectangle const&, CGColorSpace*, bool, bool, CGImage*&) {return false;}
MCGIntegerRectangle MCGRegionGetBounds(__MCGRegion*) {return MCGIntegerRectangle();}
void MCImageBitmapClear(MCImageBitmap*) {return;}
bool MCScriptInitialize() {return false;}
void MCGRasterApplyAlpha(MCGRaster&, MCGRaster const&, MCGIntegerPoint const&) {return;}
bool MCImageBitmapCreate(unsigned int, unsigned int, MCImageBitmap*&) {return false;}
void MCMemoryDeleteArray(void*) {return;}
bool MCMemoryResizeArray(unsigned int, unsigned long, void*&, unsigned int&) {return false;}
bool MCRegionForEachRect(__MCRegion*, bool (*)(void*, MCRectangle const&), void*) {return false;}
bool MCRegionIncludeRect(__MCRegion*, MCRectangle const&) {return false;}
void MCGContextClipToRect(__MCGContext*, MCGRectangle) {return;}
void coretext_font_unload(__MCString*, bool) {return;}
MCGIntegerRectangle MCGRectangleGetBounds(MCGRectangle const&) {return MCGIntegerRectangle();}
void MCGContextClipToRegion(__MCGContext*, __MCGRegion*) {return;}
void MCGContextTranslateCTM(__MCGContext*, float, float) {return;}
bool MCImageBitmapToCGImage(MCImageBitmap*, bool, bool, CGImage*&) {return false;}
bool MCImageGetCGColorSpace(CGColorSpace*&) {return false;}
bool osx_cfstring_to_cstring(__CFString const*, bool, bool) {return false;}
bool MCGContextCreateWithRaster(MCGRaster const&, __MCGContext*&) {return false;}
void MCAppleEventHandlerDoOpenDoc(AEDesc const*, AEDesc*, long) {return;}
void MCAppleEventHandlerDoSpecial(AEDesc const*, AEDesc*, long) {return;}
CGBitmapInfo MCGPixelFormatToCGBitmapInfo(unsigned int, bool) {return 0;}
bool coretext_font_load_from_path(__MCString*, bool) {return false;}
void MCAppleEventHandlerDoAEAnswer(AEDesc const*, AEDesc*, long) {return;}
bool MCOSXCreateCGContextForBitmap(MCImageBitmap*, CGContext*&) {return false;}
bool MCColorTransformLinearRGBToXYZ(MCColorVector2 const&, MCColorVector2 const&, MCColorVector2 const&, MCColorVector2 const&, MCColorVector3&, MCColorMatrix3x3&) {return false;}
MCGIntegerRectangle MCGIntegerRectangleIntersection(MCGIntegerRectangle const&, MCGIntegerRectangle const&) {return MCGIntegerRectangle();}
int MCS_mac_elevation_bootstrap_main(int, char**) {return 0;}
bool MCStringConvertToAutoreleasedNSString(__MCString*) {return false;}
bool MCQTInit() {return false;}
