#include "globdefs.h"

#include "platform.h"

bool MCPlatformGetControlThemePropBool(MCPlatformControlType, MCPlatformControlPart, MCPlatformControlState, MCPlatformThemeProperty, bool&) { return false; }
bool MCPlatformGetControlThemePropInteger(MCPlatformControlType, MCPlatformControlPart, MCPlatformControlState, MCPlatformThemeProperty, int&) { return false; }
bool MCPlatformGetControlThemePropColor(MCPlatformControlType, MCPlatformControlPart, MCPlatformControlState, MCPlatformThemeProperty, MCColor&) { return false; }
bool MCPlatformGetControlThemePropFont(MCPlatformControlType, MCPlatformControlPart, MCPlatformControlState, MCPlatformThemeProperty, MCFontRef&) { return false; }
bool MCPlatformGetControlThemePropString(MCPlatformControlType, MCPlatformControlPart, MCPlatformControlState, MCPlatformThemeProperty, MCStringRef&) { return false; }

void MCPlatformSwitchFocusToView(MCPlatformWindowRef window, uint32_t id) { return; }

void MCPlatformSoundRecorderCreate(MCPlatformSoundRecorderRef& r_recorder) { return; }

void MCPlatformSoundRecorderRetain(MCPlatformSoundRecorderRef recorder) { return; }
void MCPlatformSoundRecorderRelease(MCPlatformSoundRecorderRef recorder) { return; }

// Return true if the recorder is recording.
bool MCPlatformSoundRecorderIsRecording(MCPlatformSoundRecorderRef recorder) { return false; }

// Return the current volume of the recorded input - if not recording, return 0.
double MCPlatformSoundRecorderGetLoudness(MCPlatformSoundRecorderRef recorder) { return 0.0; }

// Start sound recording to the given file. If the sound recorder is already recording then
// the existing recording should be cancelled (stop and delete output file).
bool MCPlatformSoundRecorderStart(MCPlatformSoundRecorderRef recorder, MCStringRef filename) { return false; }
// Stop the sound recording.
void MCPlatformSoundRecorderStop(MCPlatformSoundRecorderRef recorder) { return; }

void MCPlatformSoundRecorderPause(MCPlatformSoundRecorderRef recorder) { return; }
void MCPlatformSoundRecorderResume(MCPlatformSoundRecorderRef recorder) { return; }

// Call callback for each possible input device available - if the callback returns 'false' at any point
// enumeration is cancelled, and the false will be returned.
bool MCPlatformSoundRecorderListInputs(MCPlatformSoundRecorderRef recorder, MCPlatformSoundRecorderListInputsCallback callback, void *context) { return false; }
// Call callback for each possible compressor available - if the callback returns 'false' at any point
// enumeration is cancelled, and the false will be returned.
bool MCPlatformSoundRecorderListCompressors(MCPlatformSoundRecorderRef recorder, MCPlatformSoundRecorderListCompressorsCallback callback, void *context) { return false; }

// Get the current sound recording configuration. The caller is responsible for freeing 'extra_info'.
void MCPlatformSoundRecorderGetConfiguration(MCPlatformSoundRecorderRef recorder, MCPlatformSoundRecorderConfiguration& r_config) { return; }
// Set the current sound recording configuration.
void MCPlatformSoundRecorderSetConfiguration(MCPlatformSoundRecorderRef recorder, const MCPlatformSoundRecorderConfiguration& config) { return; }

// Popup a configuration dialog for the compressors. If the dialog is not cancelled the
// sound recorders config will have been updated.
void MCPlatformSoundRecorderBeginConfigurationDialog(MCPlatformSoundRecorderRef recorder) { return; }
// End the configuration dialog.
MCPlatformDialogResult MCPlatformSoundRecorderEndConfigurationDialog(MCPlatformSoundRecorderRef recorder) { return MCPlatformDialogResult(); }


void MCPlatformSoundCreateWithData(const void *data, size_t data_size, MCPlatformSoundRef& r_sound) { return; }

void MCPlatformSoundRetain(MCPlatformSoundRef sound) { return; }
void MCPlatformSoundRelease(MCPlatformSoundRef sound) { return; }

bool MCPlatformSoundIsPlaying(MCPlatformSoundRef sound) { return false; }

void MCPlatformSoundPlay(MCPlatformSoundRef sound) { return; }
void MCPlatformSoundPause(MCPlatformSoundRef sound) { return; }
void MCPlatformSoundResume(MCPlatformSoundRef sound) { return; }
void MCPlatformSoundStop(MCPlatformSoundRef sound) { return; }

void MCPlatformSoundSetProperty(MCPlatformSoundRef sound, MCPlatformSoundProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformSoundGetProperty(MCPlatformSoundRef sound, MCPlatformSoundProperty property, MCPlatformPropertyType type, void *value) { return; }


// SN-2014-07-23: [[ Bug 12907 ]]
//  Update as well MCSreenDC::createscriptenvironment (and callees)
void MCPlatformScriptEnvironmentCreate(MCStringRef language, MCPlatformScriptEnvironmentRef& r_env) { return; }
void MCPlatformScriptEnvironmentRetain(MCPlatformScriptEnvironmentRef env) { return; }
void MCPlatformScriptEnvironmentRelease(MCPlatformScriptEnvironmentRef env) { return; }
bool MCPlatformScriptEnvironmentDefine(MCPlatformScriptEnvironmentRef env, const char *function, MCPlatformScriptEnvironmentCallback callback) { return false; }
void MCPlatformScriptEnvironmentRun(MCPlatformScriptEnvironmentRef env, MCStringRef script, MCStringRef& r_result) { return; }
void MCPlatformScriptEnvironmentCall(MCPlatformScriptEnvironmentRef env, const char *method, const char **arguments, uindex_t argument_count, char*& r_result) { return; }

void MCPlatformCreatePlayer(bool dontuseqt, MCPlatformPlayerRef& r_player) { return; }

void MCPlatformPlayerRetain(MCPlatformPlayerRef player) { return; }
void MCPlatformPlayerRelease(MCPlatformPlayerRef player) { return; }

void *MCPlatformPlayerGetNativeView(MCPlatformPlayerRef player) { return nullptr; }
bool MCPlatformPlayerSetNativeParentView(MCPlatformPlayerRef p_player, void *p_parent_view) { return false; }

bool MCPlatformPlayerIsPlaying(MCPlatformPlayerRef player) { return false; }

void MCPlatformStepPlayer(MCPlatformPlayerRef player, int amount) { return; }
void MCPlatformStartPlayer(MCPlatformPlayerRef player, double rate) { return; }
//void MCPlatformFastPlayer(MCPlatformPlayerRef player, Boolean forward) { return; }
//void MCPlatformFastForwardPlayer(MCPlatformPlayerRef player) { return; }
//void MCPlatformFastBackPlayer(MCPlatformPlayerRef player) { return; }
void MCPlatformStopPlayer(MCPlatformPlayerRef player) { return; }

bool MCPlatformLockPlayerBitmap(MCPlatformPlayerRef player, const MCGIntegerSize &p_size, MCImageBitmap*& r_bitmap) { return false; }
void MCPlatformUnlockPlayerBitmap(MCPlatformPlayerRef player, MCImageBitmap *bitmap) { return; }

void MCPlatformSetPlayerProperty(MCPlatformPlayerRef player, MCPlatformPlayerProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformGetPlayerProperty(MCPlatformPlayerRef player, MCPlatformPlayerProperty property, MCPlatformPropertyType type, void *value) { return; }

void MCPlatformCountPlayerTracks(MCPlatformPlayerRef player, uindex_t& r_track_count) { return; }
void MCPlatformGetPlayerTrackProperty(MCPlatformPlayerRef player, uindex_t index, MCPlatformPlayerTrackProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformSetPlayerTrackProperty(MCPlatformPlayerRef player, uindex_t index, MCPlatformPlayerTrackProperty property, MCPlatformPropertyType type, void *value) { return; }
bool MCPlatformFindPlayerTrackWithId(MCPlatformPlayerRef player, uint32_t id, uindex_t& r_index) { return false; }

void MCPlatformCountPlayerNodes(MCPlatformPlayerRef player, uindex_t& r_node_count) { return; }
void MCPlatformGetPlayerNodeProperty(MCPlatformPlayerRef player, uindex_t index, MCPlatformPlayerNodeProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformSetPlayerNodeProperty(MCPlatformPlayerRef player, uindex_t index, MCPlatformPlayerNodeProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformFindPlayerNodeWithId(MCPlatformPlayerRef player, uint32_t id, uindex_t& r_index) { return; }

void MCPlatformCountPlayerHotSpots(MCPlatformPlayerRef player, uindex_t& r_node_count) { return; }
void MCPlatformGetPlayerHotSpotProperty(MCPlatformPlayerRef player, uindex_t index, MCPlatformPlayerHotSpotProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformSetPlayerHotSpotProperty(MCPlatformPlayerRef player, uindex_t index, MCPlatformPlayerHotSpotProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformFindPlayerHotSpotWithId(MCPlatformPlayerRef player, uint32_t id, uindex_t& r_index) { return; }


void MCPlatformBeginFolderDialog(MCPlatformWindowRef owner, MCStringRef p_title, MCStringRef p_message, MCStringRef p_initial) { return; }
MCPlatformDialogResult MCPlatformEndFolderDialog(MCStringRef & r_selected_folder) { return MCPlatformDialogResult(); }


void MCPlatformBeginFileDialog(MCPlatformFileDialogKind p_kind, MCPlatformWindowRef p_owner, MCStringRef p_title, MCStringRef p_prompt,  MCStringRef *p_types, uint4 p_type_count, MCStringRef p_initial) { return; }
MCPlatformDialogResult MCPlatformEndFileDialog(MCPlatformFileDialogKind p_kind, MCStringRef& r_paths, MCStringRef& r_type) { return MCPlatformDialogResult(); }

void MCPlatformBeginColorDialog(MCStringRef p_title, const MCColor& p_color) { return; }
MCPlatformDialogResult MCPlatformEndColorDialog(MCColor& r_new_color) { return MCPlatformDialogResult(); }

void MCPlatformBeginPrintSettingsDialog(MCPlatformWindowRef owner, void *session, void *settings, void *page_format) { return; }
void MCPlatformBeginPageSetupDialog(MCPlatformWindowRef owner, void *session, void *settings, void *page_format) { return; }
MCPlatformPrintDialogResult MCPlatformEndPrintDialog(void) { return MCPlatformPrintDialogResult(); }

// MM-2014-07-31: [[ ThreadedRendering ]] Updated to match new platform surface API.
bool MCPlatformSurfaceLockGraphics(MCPlatformSurfaceRef surface, MCGIntegerRectangle region, MCGContextRef& r_context, MCGRaster& r_raster) { return false; }
void MCPlatformSurfaceUnlockGraphics(MCPlatformSurfaceRef surface, MCGIntegerRectangle region, MCGContextRef p_context, MCGRaster& p_raster) { return; }

// MM-2014-07-31: [[ ThreadedRendering ]] Updated to match new platform surface API.
// IM-2014-08-26: [[ Bug 13261 ]] Return the actual locked area covered by the pixels
bool MCPlatformSurfaceLockPixels(MCPlatformSurfaceRef surface, MCGIntegerRectangle region, MCGRaster& r_raster, MCGIntegerRectangle &r_locked_area) { return false; }
void MCPlatformSurfaceUnlockPixels(MCPlatformSurfaceRef surface, MCGIntegerRectangle region, MCGRaster& p_raster) { return; }

bool MCPlatformSurfaceLockSystemContext(MCPlatformSurfaceRef surface, void*& r_context) { return false; }
void MCPlatformSurfaceUnlockSystemContext(MCPlatformSurfaceRef surface) { return; }

MCGFloat MCPlatformSurfaceGetBackingScaleFactor(MCPlatformSurfaceRef surface) { return 0.0; }

bool MCPlatformSurfaceComposite(MCPlatformSurfaceRef surface, MCGRectangle dst_rect, MCGImageRef src_image, MCGRectangle src_rect, MCGFloat alpha, MCGBlendMode blend) { return false; }

void MCPlatformConfigureBackdrop(MCPlatformWindowRef backdrop_window) { return; }

void MCPlatformConfigureTextInputInWindow(MCPlatformWindowRef window, bool activate) { return; }
void MCPlatformResetTextInputInWindow(MCPlatformWindowRef window) { return; }

void MCPlatformSetWindowProperty(MCPlatformWindowRef window, MCPlatformWindowProperty property, MCPlatformPropertyType type, const void *value) { return; }
void MCPlatformGetWindowProperty(MCPlatformWindowRef window, MCPlatformWindowProperty property, MCPlatformPropertyType type, void *value) { return; }

void MCPlatformSetWindowBoolProperty(MCPlatformWindowRef window, MCPlatformWindowProperty property, bool value) { return; }
void MCPlatformSetWindowFloatProperty(MCPlatformWindowRef window, MCPlatformWindowProperty property, float value) { return; }

void MCPlatformMapPointFromWindowToScreen(MCPlatformWindowRef window, MCPoint window_point, MCPoint& r_screen_point) { return; }
void MCPlatformMapPointFromScreenToWindow(MCPlatformWindowRef window, MCPoint screen_point, MCPoint& r_window_point) { return; }

void MCPlatformCreateWindow(MCPlatformWindowRef& r_window) { return; }
void MCPlatformRetainWindow(MCPlatformWindowRef window) { return; }
void MCPlatformReleaseWindow(MCPlatformWindowRef window) { return; }

void MCPlatformInvalidateWindow(MCPlatformWindowRef window, MCRegionRef region) { return; }
void MCPlatformUpdateWindow(MCPlatformWindowRef window) { return; }

void MCPlatformShowWindow(MCPlatformWindowRef window) { return; }
void MCPlatformShowWindowAsSheet(MCPlatformWindowRef window, MCPlatformWindowRef parent_window) { return; }
void MCPlatformShowWindowAsDrawer(MCPlatformWindowRef window, MCPlatformWindowRef parent_window, MCPlatformWindowEdge edge) { return; }
void MCPlatformHideWindow(MCPlatformWindowRef window) { return; }
void MCPlatformFocusWindow(MCPlatformWindowRef window) { return; }
void MCPlatformRaiseWindow(MCPlatformWindowRef window) { return; }
void MCPlatformIconifyWindow(MCPlatformWindowRef window) { return; }
void MCPlatformUniconifyWindow(MCPlatformWindowRef window) { return; }

bool MCPlatformIsWindowVisible(MCPlatformWindowRef window) { return false; }

void MCPlatformSetWindowContentRect(MCPlatformWindowRef window, MCRectangle content_rect) { return; }
void MCPlatformGetWindowContentRect(MCPlatformWindowRef window, MCRectangle& r_content_rect) { return; }

void MCPlatformSetWindowFrameRect(MCPlatformWindowRef window, MCRectangle frame_rect) { return; }
void MCPlatformGetWindowFrameRect(MCPlatformWindowRef window, MCRectangle& r_frame_rect) { return; }


void MCPlatformDoDragDrop(MCPlatformWindowRef window, MCPlatformAllowedDragOperations allowed_operations, MCImageBitmap *image, const MCPoint *image_loc, MCPlatformDragOperation& r_operation) { return; }

void MCPlatformCreateStandardCursor(MCPlatformStandardCursor standard_cusor, MCPlatformCursorRef& r_cursor) { return; }
void MCPlatformCreateCustomCursor(MCImageBitmap *image, MCPoint hot_spot, MCPlatformCursorRef& r_cursor) { return; }
void MCPlatformRetainCursor(MCPlatformCursorRef cursor) { return; }
void MCPlatformReleaseCursor(MCPlatformCursorRef cursor) { return; }

void MCPlatformSetCursor(MCPlatformCursorRef cursor) { return; }
void MCPlatformHideCursorUntilMouseMoves(void) { return; }

void MCPlatformShowMenubar(void) { return; }
void MCPlatformHideMenubar(void) { return; }

void MCPlatformSetMenubar(MCPlatformMenuRef menu) { return; }
void MCPlatformGetMenubar(MCPlatformMenuRef menu) { return; }


void MCPlatformCreateMenu(MCPlatformMenuRef& r_menu) { return; }
void MCPlatformRetainMenu(MCPlatformMenuRef menu) { return; }
void MCPlatformReleaseMenu(MCPlatformMenuRef menu) { return; }

void MCPlatformSetMenuTitle(MCPlatformMenuRef menu, MCStringRef title) { return; }

void MCPlatformCountMenuItems(MCPlatformMenuRef menu, uindex_t& r_count) { return; }

void MCPlatformAddMenuItem(MCPlatformMenuRef menu, uindex_t where) { return; }
void MCPlatformAddMenuSeparatorItem(MCPlatformMenuRef menu, uindex_t where) { return; }
void MCPlatformRemoveMenuItem(MCPlatformMenuRef menu, uindex_t where) { return; }
void MCPlatformRemoveAllMenuItems(MCPlatformMenuRef menu) { return; }

void MCPlatformGetMenuParent(MCPlatformMenuRef menu, MCPlatformMenuRef& r_parent, uindex_t& r_index) { return; }

void MCPlatformGetMenuItemProperty(MCPlatformMenuRef menu, uindex_t index, MCPlatformMenuItemProperty property, MCPlatformPropertyType type, void *r_value) { return; }
void MCPlatformSetMenuItemProperty(MCPlatformMenuRef menu, uindex_t index, MCPlatformMenuItemProperty property, MCPlatformPropertyType type, const void *value) { return; }

//////////

bool MCPlatformPopUpMenu(MCPlatformMenuRef menu, MCPlatformWindowRef window, MCPoint location, uindex_t item) { return false; }

//////////

void MCPlatformSetIconMenu(MCPlatformMenuRef menu) { return; }

//////////

void MCPlatformWindowMaskCreate(int32_t width, int32_t height, int32_t stride, void *bits, MCPlatformWindowMaskRef& r_mask) { return; }
void MCPlatformWindowMaskCreateWithAlphaAndRelease(int32_t width, int32_t height, int32_t stride, void *bits, MCPlatformWindowMaskRef& r_mask) { return; }
void MCPlatformWindowMaskRetain(MCPlatformWindowMaskRef mask) { return; }
void MCPlatformWindowMaskRelease(MCPlatformWindowMaskRef mask) { return; }

void MCPlatformCreateColorTransform(const MCColorSpaceInfo& info, MCPlatformColorTransformRef& r_transform) { return; }
void MCPlatformRetainColorTransform(MCPlatformColorTransformRef transform) { return; }
void MCPlatformReleaseColorTransform(MCPlatformColorTransformRef transform) { return; }

bool MCPlatformApplyColorTransform(MCPlatformColorTransformRef transform, MCImageBitmap *image) { return false; }

bool MCPlatformLoadFont(MCStringRef p_path, bool globally, MCPlatformLoadedFontRef& r_loaded_font) { return false; }
bool MCPlatformUnloadFont(MCStringRef p_path, bool globally, MCPlatformLoadedFontRef loaded_font) { return false; }


void MCPlatformGetScreenCount(uindex_t& r_count) { return; }
void MCPlatformGetScreenPixelScale(uindex_t index, float& r_scale) { return; }
void MCPlatformGetScreenViewport(uindex_t index, MCRectangle& r_viewport) { return; }
void MCPlatformGetScreenWorkarea(uindex_t index, MCRectangle& r_workarea) { return; }

void MCPlatformScreenSnapshotOfUserArea(MCPoint *p_size, MCImageBitmap*& r_bitmap) { return; }
void MCPlatformScreenSnapshot(MCRectangle area, MCPoint *p_size, MCImageBitmap*& r_bitmap) { return; }
void MCPlatformScreenSnapshotOfWindow(uint32_t window_id, MCPoint *p_size, MCImageBitmap*& r_bitmap) { return; }
void MCPlatformScreenSnapshotOfWindowArea(uint32_t window_id, MCRectangle p_area, MCPoint *p_size, MCImageBitmap*& r_bitmap) { return; }

// Break the current WaitForEvent which is progress.
void MCPlatformBreakWait(void) { return; }

// Wait for any event for at most duration seconds. If blocking is true then
// no events which cause a dispatch should be processed. If an event is processed
// during duration, true is returned { return; } otherwise false is.
bool MCPlatformWaitForEvent(double duration, bool blocking) { return false; }

// Return true if the abort key has been pressed since the last check.
bool MCPlatformGetAbortKeyPressed(void) { return false; }

// Get the current (right now!) state of the mouse button.
bool MCPlatformGetMouseButtonState(uindex_t button) { return false; }

// Returns an array of all the currently pressed keys.
bool MCPlatformGetKeyState(MCPlatformKeyCode*& r_codes, uindex_t& r_code_count) { return false; }

// Get the current (right now!) state of the modifier keys.
MCPlatformModifiers MCPlatformGetModifiersState(void) { return 0; }

// Peek into the event queue and pull out a mouse click event (down then up)
// for the given button. If button is 0, then any button click will do.
bool MCPlatformGetMouseClick(uindex_t button, MCPoint& r_location) { return  false; }

// Get the position of the mouse in global coords.
void MCPlatformGetMousePosition(MCPoint& r_location) { return; }
// Set the position of the mouse in global coords.
void MCPlatformSetMousePosition(MCPoint location) { return; }

// Make the given window grab the pointer (if possible).
void MCPlatformGrabPointer(MCPlatformWindowRef window) { return; }

// Release the pointer from a grab.
void MCPlatformUngrabPointer(void) { return; }

// Get the window (that we know about) at the given co-ords.
void MCPlatformGetWindowAtPoint(MCPoint location, MCPlatformWindowRef& r_window) { return; }

bool MCPlatformGetWindowWithId(uint32_t p_window_id, MCPlatformWindowRef& r_window) { return false; }

// Return the 'time' of the last event.
uint32_t MCPlatformGetEventTime(void) { return 0; }

// Flush events of the specified types in mask.
void MCPlatformFlushEvents(MCPlatformEventMask mask) { return; }

// Produce a system beep.
void MCPlatformBeep(void) { return; }

void MCPlatformGetSystemProperty(MCPlatformSystemProperty property, MCPlatformPropertyType type, void *value) { return; }
void MCPlatformSetSystemProperty(MCPlatformSystemProperty property, MCPlatformPropertyType type, void *value) { return; }

// Clipboard
MCPlatformClipboardRef MCPlatformPasteboardSystem(void) { return NULL; }
MCPlatformClipboardRef MCPlatformPasteboardWithUniqueName(void){ return NULL; }
MCPlatformClipboardRef MCPlatformPasteboardDrag(void){ return NULL; }
void MCPlatformPasteboardRetain(MCPlatformClipboardRef p_pasteboard) { return; }
void MCPlatformPasteboardRelease(MCPlatformClipboardRef p_pasteboard) { return; }
uindex_t MCPlatformPasteboardChangeCount(MCPlatformClipboardRef p_pasteboard){ return 0; }
uindex_t MCPlatformPasteboardClearContents(MCPlatformClipboardRef p_pasteboard){ return 0; }
bool MCPlatformPasteboardWriteItems(MCPlatformClipboardRef p_pasteboard, CFMutableArrayRef p_items){ return true; }
CFMutableArrayRef MCPlatformPasteboardItemsMutableCopy(MCPlatformClipboardRef p_pasteboard){ return NULL; }
MCPlatformClipboardItemRef MCPlatformPasteboardCreateItemRef(void){ return NULL; }
void MCPlatformPasteboardItemRetain(MCPlatformClipboardItemRef p_item) { return; }
void MCPlatformPasteboardItemRelease(MCPlatformClipboardItemRef p_item) { return; }
bool MCPlatformPasteboardItemIsNSPasteboardItem(MCPlatformClipboardItemRef p_item){ return true; }
bool MCPlatformPasteboardItemAddRepresentation(MCPlatformClipboardItemRef p_item, CFStringRef p_type, CFDataRef p_data){ return true; }
CFArrayRef MCPlatformPasteboardItemRepresentationTypes(MCPlatformClipboardItemRef m_item){ return NULL; }
CFStringRef MCPlatformCFStringFromClass(void * p_class){ return NULL; }
CFDataRef MCPlatformPasteboardItemDataForType(MCPlatformClipboardItemRef p_item, CFStringRef p_type){ return NULL; }

// Native layer

void MCPlatformNativeLayerContainerViewRetain(MCPlatformNativeLayerContainerViewRef p_view) { return; }
void MCPlatformNativeLayerContainerViewRelease(MCPlatformNativeLayerContainerViewRef p_view) { return; }
void MCPlatformBitmapImageRepRelease(MCPlatformNativeLayerContainerViewRef p_view) { return; }
void MCPlatformNativeLayerContainerViewRemove(MCPlatformNativeLayerContainerViewRef p_view) { return; }
void MCPlatformCreateBitmapImageRep(MCPlatformNativeLayerContainerViewRef p_view, MCPlatformBitmapImageRepRef &r_rep) { return; }
void MCPlatformBitmapImageRepRetain(MCPlatformBitmapImageRepRef p_cached) { return; }
void MCPlatformBitmapImageRepCache(MCPlatformNativeLayerContainerViewRef p_view, MCPlatformBitmapImageRepRef p_cached, MCGRaster &r_raster) { return; }
void MCPlatformNativeLayerContainerViewSetGeometry(MCPlatformNativeLayerContainerViewRef p_view, MCRectangle p_rect, MCPlatformWindowRef p_window, int32_t p_gp_height) { return; }
void MCPlatformNativeLayerContainerViewSetVisible(MCPlatformNativeLayerContainerViewRef p_view, bool p_visible) { return; }
void MCPlatformNativeLayerContainerViewAddSubView(void * t_parent_view,MCPlatformNativeLayerContainerViewRef m_view, MCPlatformNativeLayerContainerViewRef t_before_view) { return; }
void MCPlatformWindowContentView(MCPlatformWindowRef p_window, void *&r_view) { return; }
void MCPlatformCreateNativeLayerContainerView(MCPlatformNativeLayerContainerViewRef & r_view) { return; }
void MCPlatformViewSetNeedsDisplay(void * p_view) { return; }
