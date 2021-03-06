#include "prefix.h"

#include "core.h"
#include "globdefs.h"
#include "filedefs.h"
#include "objdefs.h"
#include "parsedef.h"
#include "mcio.h"

#include "uidc.h"
#include "util.h"
#include "objectstream.h"
#include "bitmapeffect.h"
#include "globals.h"
#include "execpt.h"
#include "paint.h"

#include "graphicscontext.h"
#include "graphics.h"
#include "graphics_util.h"
#include "font.h"

#include "pathprivate.h"

////////////////////////////////////////////////////////////////////////////////

static inline MCGBlendMode MCBitmapEffectBlendModeToMCGBlendMode(MCBitmapEffectBlendMode p_blend_mode)
{
	switch(p_blend_mode)
	{
		case kMCBitmapEffectBlendModeNormal:
			return kMCGBlendModeSourceOver;
		case kMCBitmapEffectBlendModeMultiply:
			return kMCGBlendModeMultiply;
		case kMCBitmapEffectBlendModeScreen:
			return kMCGBlendModeScreen;
		case kMCBitmapEffectBlendModeOverlay:
			return kMCGBlendModeOverlay;
		case kMCBitmapEffectBlendModeDarken:
			return kMCGBlendModeDarken;
		case kMCBitmapEffectBlendModeLighten:
			return kMCGBlendModeLighten;
		case kMCBitmapEffectBlendModeColorDodge:
			return kMCGBlendModeColorDodge;
		case kMCBitmapEffectBlendModeColorBurn:
			kMCGBlendModeColorBurn;
		case kMCBitmapEffectBlendModeHardLight:
			kMCGBlendModeHardLight;
		case kMCBitmapEffectBlendModeSoftLight:
			return kMCGBlendModeSoftLight;
		case kMCBitmapEffectBlendModeDifference:
			return kMCGBlendModeDifference;
		case kMCBitmapEffectBlendModeExclusion:
			return kMCGBlendModeExclusion;
		case kMCBitmapEffectBlendModeHue:
			return kMCGBlendModeHue;
		case kMCBitmapEffectBlendModeSaturation:
			return kMCGBlendModeSaturation;
		case kMCBitmapEffectBlendModeColor:
			return kMCGBlendModeColor;
		case kMCBitmapEffectBlendModeLuminosity:
			return kMCGBlendModeLuminosity;
	}
}

static void MCGraphicsContextAngleAndDistanceToXYOffset(int p_angle, int p_distance, MCGFloat &r_x_offset, MCGFloat &r_y_offset)
{
	r_x_offset = floor(0.5f + p_distance * cos(p_angle * M_PI / 180.0));
	r_y_offset = floor(0.5f + p_distance * sin(p_angle * M_PI / 180.0));
}

static inline MCGPoint extract_gpoint_from_legacy_path_ordinates(int32_t *&p_ordinates)
{
	MCGFloat x, y;
	x = (*p_ordinates++ >> 7) / 2.0f;
	y = (*p_ordinates++ >> 7) / 2.0f;
	return MCGPointMake(x, y);
}

static MCGPathRef MCGPathFromMCPath(MCPath *p_path)
{
	MCGPathRef t_gpath;
	t_gpath = NULL;
		
	if (!MCGPathCreateMutable(t_gpath))
		return NULL;

	uindex_t t_command_cnt, t_point_cnt;
	p_path -> get_lengths(t_command_cnt, t_point_cnt);
	
	uint8_t *t_commands;
	t_commands = p_path -> get_commands();
		
	int32_t *t_ordinates;
	t_ordinates = p_path -> get_ordinates();
	
	for (uindex_t t_command_pos = 0; t_command_pos < t_command_cnt; t_command_pos++)
	{
		switch (t_commands[t_command_pos])
		{
			case PATH_COMMAND_END:
				break;
			case PATH_COMMAND_MOVE_TO:
				MCGPathMoveTo(t_gpath, extract_gpoint_from_legacy_path_ordinates(t_ordinates));
				break;
			case PATH_COMMAND_LINE_TO:
				MCGPathLineTo(t_gpath, extract_gpoint_from_legacy_path_ordinates(t_ordinates));
				break;
			case PATH_COMMAND_CUBIC_TO:
				MCGPathCubicTo(t_gpath, extract_gpoint_from_legacy_path_ordinates(t_ordinates), 
							   extract_gpoint_from_legacy_path_ordinates(t_ordinates), extract_gpoint_from_legacy_path_ordinates(t_ordinates));
				break;
			case PATH_COMMAND_QUADRATIC_TO:
				MCGPathQuadraticTo(t_gpath, extract_gpoint_from_legacy_path_ordinates(t_ordinates), extract_gpoint_from_legacy_path_ordinates(t_ordinates));
				break;
			case PATH_COMMAND_CLOSE:
				MCGPathCloseSubpath(t_gpath);
				break;				
		}
	}
	
	return t_gpath;
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::init(MCGContextRef p_context)
{
	m_gcontext = MCGContextRetain(p_context);
	m_pattern = nil;
	m_background = getblack();
	m_fill_style = FillSolid;
	
	m_line_width = 0;
	m_line_style = LineSolid;
	m_cap_style = CapButt;
	m_join_style = JoinBevel;

	m_miter_limit = 0.0;

	m_dash_phase = 0.0;
	m_dash_lengths = nil;
	m_dash_count = 0;
	
	MCGContextSetFlatness(p_context, 1 / 16.0f);
}

MCGraphicsContext::MCGraphicsContext(MCGContextRef p_context)
{
	init(p_context);
}

MCGraphicsContext::MCGraphicsContext(uint32_t p_width, uint32_t p_height, bool p_alpha)
{
	MCGContextRef t_context;
	/* UNCHECKED */ MCGContextCreate(p_width, p_height, p_alpha, t_context);

	init(t_context);
	MCGContextRelease(t_context);
}

MCGraphicsContext::MCGraphicsContext(uint32_t p_width, uint32_t p_height, uint32_t p_stride, void *p_pixels, bool p_alpha)
{
	MCGContextRef t_context;
	/* UNCHECKED */ MCGContextCreateWithPixels(p_width, p_height, p_stride, p_pixels, p_alpha, m_gcontext);

	init(t_context);
	MCGContextRelease(t_context);
}

MCGraphicsContext::~MCGraphicsContext()
{
	MCPatternRelease(m_pattern);
	MCGContextRelease(m_gcontext);
	MCMemoryDeleteArray(m_dash_lengths);
}

MCContextType MCGraphicsContext::gettype() const
{
	return CONTEXT_TYPE_SCREEN;
}

MCGContextRef MCGraphicsContext::getgcontextref(void) const
{
	return m_gcontext;
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::begin(bool p_group)
{
	MCGContextBegin(m_gcontext, p_group);
}

bool MCGraphicsContext::begin_with_effects(MCBitmapEffectsRef p_effects, const MCRectangle &p_shape)
{
	MCGBitmapEffects t_effects;
	t_effects . has_drop_shadow = false;
	t_effects . has_outer_glow = false;
	t_effects . has_inner_glow = false;
	t_effects . has_inner_shadow = false;

	if ((p_effects -> mask & kMCBitmapEffectTypeColorOverlayBit) != 0)
	{
		t_effects . has_color_overlay = true;
		t_effects . color_overlay . color = p_effects -> effects[kMCBitmapEffectTypeColorOverlay] . layer . color;
		t_effects . color_overlay . blend_mode = MCBitmapEffectBlendModeToMCGBlendMode((MCBitmapEffectBlendMode) p_effects -> effects[kMCBitmapEffectTypeColorOverlay] . layer . blend_mode);

	}
	else
		t_effects . has_color_overlay = false;
	
	if ((p_effects -> mask & kMCBitmapEffectTypeInnerGlowBit) != 0)
	{
		t_effects . has_inner_glow = true;
		t_effects . inner_glow . color = p_effects -> effects[kMCBitmapEffectTypeInnerGlow] . glow . color;
		t_effects . inner_glow . blend_mode = MCBitmapEffectBlendModeToMCGBlendMode((MCBitmapEffectBlendMode) p_effects -> effects[kMCBitmapEffectTypeInnerGlow] . glow . blend_mode);
		t_effects . inner_glow . size = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeInnerGlow] . glow . size;
		t_effects . inner_glow . spread = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeInnerGlow] . glow . spread / 255.0f;
		t_effects . inner_glow . inverted = p_effects -> effects[kMCBitmapEffectTypeInnerGlow] . glow . source == kMCBitmapEffectSourceEdge;
	}
	else
		t_effects . has_inner_glow = false;

	if ((p_effects -> mask & kMCBitmapEffectTypeInnerShadowBit) != 0)
	{
		t_effects . has_inner_shadow = true;
		t_effects . inner_shadow . color = p_effects -> effects[kMCBitmapEffectTypeInnerShadow] . shadow . color;
		t_effects . inner_shadow . blend_mode = MCBitmapEffectBlendModeToMCGBlendMode((MCBitmapEffectBlendMode) p_effects -> effects[kMCBitmapEffectTypeInnerShadow] . shadow . blend_mode);
		t_effects . inner_shadow . size = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeInnerShadow] . shadow . size;
		t_effects . inner_shadow . spread = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeInnerShadow] . shadow . spread / 255.0f;
		
		MCGFloat t_x_offset, t_y_offset;
		MCGraphicsContextAngleAndDistanceToXYOffset(p_effects -> effects[kMCBitmapEffectTypeInnerShadow] . shadow . angle, p_effects -> effects[kMCBitmapEffectTypeInnerShadow] . shadow . distance,
													t_x_offset, t_y_offset);
		t_effects . inner_shadow . x_offset = t_x_offset;
		t_effects . inner_shadow . y_offset = t_y_offset;
	}
	else
		t_effects . has_inner_shadow = false;	
	
	if ((p_effects -> mask & kMCBitmapEffectTypeOuterGlowBit) != 0)
	{
		t_effects . has_outer_glow = true;
		t_effects . outer_glow . color = p_effects -> effects[kMCBitmapEffectTypeOuterGlow] . glow . color;
		t_effects . outer_glow . blend_mode = MCBitmapEffectBlendModeToMCGBlendMode((MCBitmapEffectBlendMode) p_effects -> effects[kMCBitmapEffectTypeOuterGlow] . glow . blend_mode);
		t_effects . outer_glow . size = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeOuterGlow] . glow . size;
		t_effects . outer_glow . spread = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeOuterGlow] . glow . spread / 255.0f;
	}
	else
		t_effects . has_outer_glow = false;
	
	if ((p_effects -> mask & kMCBitmapEffectTypeDropShadowBit) != 0)
	{
		t_effects . has_drop_shadow = true;
		t_effects . drop_shadow . color = p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . color;
		t_effects . drop_shadow . blend_mode = MCBitmapEffectBlendModeToMCGBlendMode((MCBitmapEffectBlendMode) p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . blend_mode);
		t_effects . drop_shadow . size = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . size;
		t_effects . drop_shadow . spread = (MCGFloat) p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . spread / 255.0f;
		
		MCGFloat t_x_offset, t_y_offset;
		MCGraphicsContextAngleAndDistanceToXYOffset(p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . angle, p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . distance,
													t_x_offset, t_y_offset);
		t_effects . drop_shadow . x_offset = t_x_offset;
		t_effects . drop_shadow . y_offset = t_y_offset;
		t_effects . drop_shadow . knockout = p_effects -> effects[kMCBitmapEffectTypeDropShadow] . shadow . knockout;
	}
	else
		t_effects . has_drop_shadow = false;
	
	MCGContextBeginWithEffects(m_gcontext, MCGRectangleMake(p_shape . x, p_shape . y, p_shape . width, p_shape . height), t_effects);
	return true;
}

void MCGraphicsContext::end()
{
	MCGContextEnd(m_gcontext);
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::setprintmode(void)
{
}

bool MCGraphicsContext::changeopaque(bool p_value)
{
	return true;
}

void MCGraphicsContext::setclip(const MCRectangle& p_clip)
{
	//m_clip = p_clip;
	// IM-2013-09-03: [[ RefactorGraphics ]] use MCGContextSetClipToRect to replace current
	// clipping rect with p_clip (consistent with old MCContext::setclip functionality)
	MCGContextSetClipToRect(m_gcontext, MCRectangleToMCGRectangle(p_clip));
}

MCRectangle MCGraphicsContext::getclip(void) const
{
	return MCGRectangleGetIntegerBounds(MCGContextGetClipBounds(m_gcontext));
}

void MCGraphicsContext::clearclip(void)
{
}

void MCGraphicsContext::setorigin(int2 x, int2 y)
{
	MCGContextTranslateCTM(m_gcontext, -1.0f * x, -1.0f * y);
}

void MCGraphicsContext::clearorigin(void)
{
	MCGContextResetCTM(m_gcontext);
}

void MCGraphicsContext::setquality(uint1 p_quality)
{
	MCGContextSetShouldAntialias(m_gcontext, p_quality == QUALITY_SMOOTH);
}

void MCGraphicsContext::setfunction(uint1 p_function)
{
	m_function = p_function;
	MCGBlendMode t_blend_mode;
	t_blend_mode = kMCGBlendModeSourceOver;
	switch (p_function)
	{
		case GXblendClear:
			t_blend_mode = kMCGBlendModeClear;
			break;
		case GXblendSrcOver:
		case GXcopy:
			t_blend_mode = kMCGBlendModeSourceOver;
			break;
		case GXblendDstOver:
			t_blend_mode = kMCGBlendModeDestinationOver;
			break;
		case GXblendSrcIn:
			t_blend_mode = kMCGBlendModeSourceIn;
			break;
		case GXblendDstIn:
			t_blend_mode = kMCGBlendModeDestinationIn;
			break;
		case GXblendSrcOut:
			t_blend_mode = kMCGBlendModeSourceOut;
			break;
		case GXblendDstOut:
			t_blend_mode = kMCGBlendModeDestinationOut;
			break;
		case GXblendSrcAtop:
			t_blend_mode = kMCGBlendModeSourceAtop;
			break;
		case GXblendDstAtop:
			t_blend_mode = kMCGBlendModeDestinationAtop;
			break;
		case GXblendXor:
			t_blend_mode = kMCGBlendModeXor;
			break;
		case GXblendPlus:
			t_blend_mode = kMCGBlendModePlusLighter;
			break;
		case GXblendMultiply:
			t_blend_mode = kMCGBlendModeMultiply;
			break;
		case GXblendScreen:
			t_blend_mode = kMCGBlendModeScreen;
			break;
		case GXblendOverlay:
			t_blend_mode = kMCGBlendModeOverlay;
			break;
		case GXblendDarken:
			t_blend_mode = kMCGBlendModeDarken;
			break;
		case GXblendLighten:
			t_blend_mode = kMCGBlendModeLighten;
			break;
		case GXblendDodge:
			t_blend_mode = kMCGBlendModeColorDodge;
			break;
		case GXblendBurn:
			t_blend_mode = kMCGBlendModeColorBurn;
			break;
		case GXblendHardLight:
			t_blend_mode = kMCGBlendModeHardLight;
			break;
		case GXblendSoftLight:
			t_blend_mode = kMCGBlendModeSoftLight;
			break;
		case GXblendDifference:
			t_blend_mode = kMCGBlendModeDifference;
			break;
		case GXblendExclusion:
			t_blend_mode = kMCGBlendModeExclusion;
			break;		
			
		case GXclear:
			t_blend_mode = kMCGBlendModeLegacyClear;
			break;
		case GXand:
			t_blend_mode = kMCGBlendModeLegacyAnd;
			break;
		case GXandReverse:
			t_blend_mode = kMCGBlendModeLegacyAndReverse;
			break;
		case GXandInverted:
			t_blend_mode = kMCGBlendModeLegacyInverted;
			break;
		case GXnoop:
			t_blend_mode = kMCGBlendModeLegacyNoop;
			break;
		case GXxor:
			t_blend_mode = kMCGBlendModeLegacyXor;
			break;
		case GXor:
			t_blend_mode = kMCGBlendModeLegacyOr;
			break;
		case GXnor:
			t_blend_mode = kMCGBlendModeLegacyNor;
			break;
		case GXequiv:
			t_blend_mode = kMCGBlendModeLegacyEquiv;
			break;
		case GXinvert:
			t_blend_mode = kMCGBlendModeLegacyInvert;
			break;
		case GXorReverse:
			t_blend_mode = kMCGBlendModeLegacyOrReverse;
			break;
		case GXcopyInverted:
			t_blend_mode = kMCGBlendModeLegacyCopyInverted;
			break;
		case GXorInverted:
			t_blend_mode = kMCGBlendModeLegacyOrInverted;
			break;
		case GXnand:
			t_blend_mode = kMCGBlendModeLegacyNand;
			break;
		case GXset:
			t_blend_mode = kMCGBlendModeLegacySet;
			break;
		case GXblend:
			t_blend_mode = kMCGBlendModeLegacyBlend;
			break;
		case GXaddpin:
			t_blend_mode = kMCGBlendModeLegacyAddPin;
			break;
		case GXaddOver:
			t_blend_mode = kMCGBlendModeLegacyAddOver;
			break;
		case GXsubPin:
			t_blend_mode = kMCGBlendModeLegacySubPin;
			break;
		case GXtransparent:
			t_blend_mode = kMCGBlendModeLegacyTransparent;
			break;
		case GXaddMax:
			t_blend_mode = kMCGBlendModeLegacyAdMax;
			break;
		case GXsubOver:
			t_blend_mode = kMCGBlendModeLegacySubOver;
			break;
		case GXaddMin:
			t_blend_mode = kMCGBlendModeLegacyAdMin;
			break;
		case GXblendSrc:
			t_blend_mode = kMCGBlendModeLegacyBlendSource;
			break;
		case GXblendDst:
			t_blend_mode = kMCGBlendModeLegacyBlendDestination;
			break;
			
	}
	MCGContextSetBlendMode(m_gcontext, t_blend_mode);
}

uint1 MCGraphicsContext::getfunction(void)
{
	return m_function;
}

void MCGraphicsContext::setopacity(uint1 p_opacity)
{
	m_opacity = p_opacity;
	MCGContextSetOpacity(m_gcontext, p_opacity / 255.0f);
}

uint1 MCGraphicsContext::getopacity(void)
{
	/* TODO */
	return m_opacity;
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::setforeground(const MCColor& c)
{
	MCGContextSetFillRGBAColor(m_gcontext, (c . red >> 8) / 255.0f, (c . green >> 8) / 255.0f, (c . blue >> 8) / 255.0f, 1.0f);
	MCGContextSetStrokeRGBAColor(m_gcontext, (c . red >> 8) / 255.0f, (c . green >> 8) / 255.0f, (c . blue >> 8) / 255.0f, 1.0f);
}

void MCGraphicsContext::setbackground(const MCColor& c)
{
	m_background = c;
}

void MCGraphicsContext::setdashes(uint16_t p_offset, const uint8_t *p_dashes, uint16_t p_length)
{
	MCMemoryDeleteArray(m_dash_lengths);
	m_dash_lengths = nil;
	m_dash_count = 0;
	m_dash_phase = (MCGFloat)p_offset;
	
	if (p_length > 0)
	{
		m_dash_count = p_length;
		/* UNCHECKED */ MCMemoryNewArray(m_dash_count, m_dash_lengths);
		for (uint32_t i = 0; i < m_dash_count; i++)
		{
			// MM-2014-01-21: [[ Bug 11695 ]] Fudge a dash length of 0 to 0.01 - was used previoulsy as start/end cap.
			if (p_dashes[i] == 0)
				m_dash_lengths[i] = (MCGFloat) (p_dashes[i] + 0.01f);
			else
				m_dash_lengths[i] = (MCGFloat) p_dashes[i];
		}
	}
	
	if (m_line_style != LineSolid)
		MCGContextSetStrokeDashes(m_gcontext, m_dash_phase, m_dash_lengths, m_dash_count);
}

void MCGraphicsContext::setfillstyle(uint2 style, MCPatternRef p, int2 x, int2 y)
{
	MCPatternRelease(m_pattern);
	m_pattern = nil;
	
	// MM-2013-09-30: [[ Bug 11221 ]] Retain the fill style (and return in getfillstyle). Breaks backpatterns in fields otherwise.
	m_fill_style = style;

	// MW-2014-03-11: [[ Bug 11704 ]] Make sure we set both fill and stroke paints.
	if (style == FillTiled && p != NULL)
	{
		// IM-2013-08-14: [[ ResIndependence ]] apply pattern image scale to transform
		MCGFloat t_scale;
		t_scale = 1.0 / p->scale;
		
		MCGAffineTransform t_transform;
		t_transform = MCGAffineTransformMakeScale(t_scale, t_scale);
		t_transform = MCGAffineTransformTranslate(t_transform, x, y);

        // MM-2014-01-27: [[ UpdateImageFilters ]] Updated to use new libgraphics image filter types (was bilinear).
		MCGContextSetFillPattern(m_gcontext, p->image, t_transform, kMCGImageFilterMedium);
		MCGContextSetStrokePattern(m_gcontext, p->image, t_transform, kMCGImageFilterMedium);
		m_pattern = MCPatternRetain(p);
		m_pattern_x = x;
		m_pattern_y = y;
	}
	else if (style == FillStippled || style == FillOpaqueStippled)
	{
		MCGContextSetFillPaintStyle(m_gcontext, kMCGPaintStyleStippled);
		MCGContextSetStrokePaintStyle(m_gcontext, kMCGPaintStyleStippled);
	}
	else
	{
		MCGContextSetFillPaintStyle(m_gcontext, kMCGPaintStyleOpaque);
		MCGContextSetStrokePaintStyle(m_gcontext, kMCGPaintStyleOpaque);
	}
}

void MCGraphicsContext::getfillstyle(uint2& style, MCPatternRef& p, int2& x, int2& y)
{
	style = m_fill_style;
	p = m_pattern;
	x = m_pattern_x;
	y = m_pattern_y;
}

void MCGraphicsContext::setlineatts(uint2 linesize, uint2 linestyle, uint2 capstyle, uint2 joinstyle)
{
	// MM-2014-01-21: [[ Bug 11671 ]]
	if (capstyle & NoStartCap)
		capstyle = capstyle ^ NoStartCap;
	if (capstyle & NoEndCap)
		capstyle = capstyle ^ NoEndCap;	
	
	// IM-2013-09-03: [[ RefactorGraphics ]] track the current linewidth setting
	m_line_width = linesize;
	m_line_style = linestyle;
	m_cap_style = capstyle;
	m_join_style = joinstyle;
	
	MCGContextSetStrokeWidth(m_gcontext, (MCGFloat) linesize);
	
	switch (capstyle)
	{
		case CapButt:
			MCGContextSetStrokeCapStyle(m_gcontext, kMCGCapStyleButt);
			break;
		case CapRound:
			MCGContextSetStrokeCapStyle(m_gcontext, kMCGCapStyleRound);
			break;
		case CapProjecting:
			MCGContextSetStrokeCapStyle(m_gcontext, kMCGCapStyleSquare);
			break;
	}
	
	switch (linestyle)
	{
		case LineSolid:
			MCGContextSetStrokeDashes(m_gcontext, 0, nil, 0);
			break;

		case LineOnOffDash:
			MCGContextSetStrokeDashes(m_gcontext, m_dash_phase, m_dash_lengths, m_dash_count);
			break;

		case LineDoubleDash:
		{
			break;
		}
	}
	
	
	switch (joinstyle)
	{
		case JoinRound:
			MCGContextSetStrokeJoinStyle(m_gcontext, kMCGJoinStyleRound);
			break;
		case JoinMiter:
			MCGContextSetStrokeJoinStyle(m_gcontext, kMCGJoinStyleMiter);
			break;
		case JoinBevel:
			MCGContextSetStrokeJoinStyle(m_gcontext, kMCGJoinStyleBevel);
			break;
	}	
}

void MCGraphicsContext::getlineatts(uint2& linesize, uint2& linestyle, uint2& capstyle, uint2& joinstyle)
{
	linesize = m_line_width;
	linestyle = m_line_style;
	capstyle = m_cap_style;
	joinstyle = m_join_style;
}

void MCGraphicsContext::setmiterlimit(real8 p_limit)
{
	m_miter_limit = p_limit;

	MCGContextSetStrokeMiterLimit(m_gcontext, (MCGFloat) p_limit);
}

void MCGraphicsContext::getmiterlimit(real8 &r_limit)
{
	r_limit = m_miter_limit;
}

void MCGraphicsContext::setgradient(MCGradientFill *p_gradient)
{
	// MM-2013-10-32: [[ Bug 11367 ]] Added check to make sure ramp lenght isn't 0. Was causing issues with gradient inspector.
	if (p_gradient != NULL && p_gradient -> kind != kMCGradientKindNone && p_gradient -> ramp_length != 0)
	{	
		MCGGradientFunction t_function;
		t_function = kMCGGradientFunctionLinear;
		switch (p_gradient -> kind)
		{
			case kMCGradientKindLinear:
				t_function = kMCGGradientFunctionLinear;
				break;
			case kMCGradientKindRadial:
				t_function = kMCGGradientFunctionRadial;
				break;
			case kMCGradientKindConical:
				t_function = kMCGGradientFunctionSweep;
				break;
			case kMCGradientKindDiamond:
				t_function = kMCGLegacyGradientDiamond;
				break;
			case kMCGradientKindSpiral:
				t_function = kMCGLegacyGradientSpiral;
				break;
			case kMCGradientKindXY:
				t_function = kMCGLegacyGradientXY;
				break;
			case kMCGradientKindSqrtXY:
				t_function = kMCGLegacyGradientSqrtXY;
				break;
		}
		
        // MM-2014-01-27: [[ UpdateImageFilters ]] Updated to use new libgraphics image filter types.
		MCGImageFilter t_filter;
		t_filter = kMCGImageFilterNone;
		switch (p_gradient -> quality)
		{
			case kMCGradientQualityNormal:
				t_filter = kMCGImageFilterNone;
				break;
			case kMCGradientQualityGood:
				t_filter = kMCGImageFilterMedium;
				break;
		}
		
		MCGFloat *t_stops;
		/* UNCHECKED */ MCMemoryNewArray(p_gradient -> ramp_length, t_stops);
		MCGColor *t_colors;
		/* UNCHECKED */ MCMemoryNewArray(p_gradient -> ramp_length, t_colors);
		for (uint32_t i = 0; i < p_gradient -> ramp_length; i++)
		{
			t_stops[i] = (MCGFloat) p_gradient -> ramp[i] . offset / STOP_INT_MAX;
			t_colors[i] = p_gradient -> ramp[i] . color;
		}
						
		MCGAffineTransform t_transform;
		t_transform . a = p_gradient -> primary . x - p_gradient -> origin . x;
		t_transform . b = p_gradient -> primary . y - p_gradient -> origin . y;
		t_transform . c = p_gradient -> secondary . x - p_gradient -> origin . x;
		t_transform . d = p_gradient -> secondary . y - p_gradient -> origin . y;
		t_transform . tx = p_gradient -> origin . x;
		t_transform . ty = p_gradient -> origin . y;

		MCGContextSetFillGradient(m_gcontext, t_function, t_stops, t_colors, p_gradient -> ramp_length, p_gradient -> mirror, p_gradient -> wrap, p_gradient -> repeat, t_transform, t_filter);
		MCGContextSetStrokeGradient(m_gcontext, t_function, t_stops, t_colors, p_gradient -> ramp_length, p_gradient -> mirror, p_gradient -> wrap, p_gradient -> repeat, t_transform, t_filter);
		
		MCMemoryDeleteArray(t_stops);
		MCMemoryDeleteArray(t_colors);
	}
}

////////////////////////////////////////////////////////////////////////////////
// MM-2013-11-06: [[ Bug 11389 ]] Add legacy drawing functions for rounded rects and arcs.

static void add_legacy_move_to(MCGContextRef p_gcontext, int4 x, int4 y)
{
	MCGContextMoveTo(p_gcontext, MCGPointMake(x / 2.0f, y / 2.0f));
}

static void add_legacy_line_to(MCGContextRef p_gcontext, int4 x, int4 y)
{
	MCGContextLineTo(p_gcontext, MCGPointMake(x / 2.0f, y / 2.0f));
}

static void add_legacy_cubic_to(MCGContextRef p_gcontext, int4 ax, int4 ay, int4 bx, int4 by, int4 x, int4 y)
{
	MCGContextCubicTo(p_gcontext, MCGPointMake(ax / 2.0f, ay / 2.0f), MCGPointMake(bx / 2.0f, by / 2.0f), MCGPointMake(x / 2.0f, y / 2.0f));
}

static void add_legacy_arc_to(MCGContextRef p_gcontext, int4 cx, int4 cy, int4 hr, int4 vr, int4 s, int4 e, bool first)
{
	int4 hk, vk;
	
	if (e - s == 90)
	{
		hk = hr * 36195 / 65536;
		vk = vr * 36195 / 65536;
	}
	else
	{
		double h = tan(M_PI * (e - s) / 720.0);
		hk = int4(4.0 * h * hr / 3.0);
		vk = int4(4.0 * h * vr / 3.0);
	}
	
	double ca, sa;
	double cb, sb;
	
	ca = cos(M_PI * s / 180);
	sa = sin(M_PI * s / 180);
	
	cb = cos(M_PI * e / 180);
	sb = sin(M_PI * e / 180);
	
	if (first)
		add_legacy_move_to(p_gcontext, int4(cx + hr * ca), int4(cy - vr * sa));
	
	add_legacy_cubic_to(p_gcontext, int4(cx + hr * ca - hk * sa), int4(cy - vr * sa - vk * ca), int4(cx + hr * cb + hk * sb), int4(cy - vr * sb + vk * cb), int4(cx + hr * cb), int4(cy - vr * sb));	
}

static void add_legacy_rounded_rectangle(MCGContextRef p_gcontext, const MCGRectangle& p_rect, MCGFloat p_radius)
{
	int4 x, y;
	x = (int4)(p_rect . origin . x * 2);
	y = (int4)(p_rect . origin . y * 2);
	
	int4 aw, ah;
	aw = (int4)(p_rect . size . width * 2);
	ah = (int4)(p_rect . size . height * 2);
	
	int4 hr, vr;
	hr = MCU_min(aw / 2, (int4)(p_radius * 2));
	vr = MCU_min(ah / 2, (int4)(p_radius * 2));
	
	int4 h, v;
	h = aw - hr * 2;
	v = ah - vr * 2;
	
	int4 l, t, r, b;
	l = x + hr;
	t = y + vr;
	r = x + hr + h;
	b = y + vr + v;	
	
	int4 hk, vk;
	hk = signed(hr * 36195 / 65536);
	vk = signed(vr * 36195 / 65536);
	
	add_legacy_move_to(p_gcontext, r, t - vr);
	add_legacy_cubic_to(p_gcontext, r + hk, t - vr, r + hr, t - vk, r + hr, t);
	add_legacy_line_to(p_gcontext, r + hr, b);
	add_legacy_cubic_to(p_gcontext, r + hr, b + vk, r + hk, b + vr, r, b + vr);
	add_legacy_line_to(p_gcontext, l, b + vr);
	add_legacy_cubic_to(p_gcontext, l - hk, b + vr, l - hr, b + vk, l - hr, b);
	add_legacy_line_to(p_gcontext, l - hr, t);
	add_legacy_cubic_to(p_gcontext, l - hr, t - vk, l - hk, t - vr, l, t - vr);
	
	MCGContextCloseSubpath(p_gcontext);
}

static void add_legacy_arc(MCGContextRef p_gcontext, const MCGRectangle& p_rect, uint2 p_start, uint2 p_angle, bool adjust)
{
	if (p_rect . size . width == 0.0f || p_rect . size . height == 0.0f)
		return;
	
	if (p_angle == 0)
		return;
	
	if (p_angle > 360)
		p_angle = 360;
	
	bool closed = (p_angle == 360);
	
	p_start = p_start % 360;
	
	int4 cx, cy, hr, vr;
	cx = (int4)(p_rect . origin . x * 2 + p_rect . size . width);
	cy = (int4)(p_rect . origin . y * 2 + p_rect . size . height);
	hr = (int4)p_rect . size . width;
	vr = (int4)p_rect . size . height;
	
	if (adjust)
		hr -= 1, vr -= 1;
	
	bool t_first = true;
	int4 t_delta;
	
	while(p_angle > 0)
	{
		t_delta = MCU_min(90 - p_start % 90, p_angle);
		p_angle -= t_delta;
		add_legacy_arc_to(p_gcontext, cx, cy, hr, vr, p_start, p_start + t_delta, t_first);
		p_start += t_delta;
		t_first = false;
	}
	if (closed)
		MCGContextCloseSubpath(p_gcontext);
}

static void add_legacy_segment(MCGContextRef p_gcontext, const MCGRectangle& p_rect, uint2 p_start, uint2 p_angle, bool adjust)
{
	if (p_rect . size . width == 0.0f || p_rect . size . height == 0.0f)
		return;
	
	if (p_angle == 0)
		return;
		
	if (p_angle > 360)
		p_angle = 360;
	
	p_start = p_start % 360;
	
	int4 cx, cy, hr, vr;
	cx = (int4)(p_rect . origin . x * 2 + p_rect . size . width);
	cy = (int4)(p_rect . origin . y * 2 + p_rect . size . height);
	hr = (int4)p_rect . size . width;
	vr = (int4)p_rect . size . height;	
	
	if (adjust)
		hr -= 1, vr -= 1;
	
	bool t_first = true;
	int4 t_delta;
	
	while(p_angle > 0)
	{
		t_delta = MCU_min(90 - p_start % 90, p_angle);
		p_angle -= t_delta;
		add_legacy_arc_to(p_gcontext, cx, cy, hr, vr, p_start, p_start + t_delta, t_first);
		p_start += t_delta;
		t_first = false;
	}
	add_legacy_line_to(p_gcontext, cx, cy);
	MCGContextCloseSubpath(p_gcontext);
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::drawline(int2 x1, int2 y1, int2 x2, int2 y2)
{
	// MM-2013-11-14: [[ Bug 11457 ]] Adjust lines and polygons to make sure antialiased lines don't draw across pixels.
	MCGPoint t_start;
	t_start . x = (MCGFloat) x1 + 0.5f;
	t_start . y = (MCGFloat) y1 + 0.5f;
	
	MCGPoint t_finish;
	t_finish . x = (MCGFloat) x2 + 0.5f;
	t_finish . y = (MCGFloat) y2 + 0.5f;
	
	MCGContextBeginPath(m_gcontext);
	MCGContextAddLine(m_gcontext, t_start, t_finish);	
	MCGContextStroke(m_gcontext);
}

void MCGraphicsContext::drawlines(MCPoint *points, uint2 npoints, bool p_closed)
{
	// MM-2013-11-21: [[ Bug 11395 ]] When only 1 point is passed, assume we are drawing a dot.
	if (npoints == 1)
	{
		// MM-2013-11-25: [[ Bug 11395 ]] For consitency with previous versions, make sure the dot has square edges.
		MCGContextSave(m_gcontext);		
		MCGContextSetStrokeJoinStyle(m_gcontext, kMCGJoinStyleMiter);
		MCGContextSetStrokeCapStyle(m_gcontext, kMCGCapStyleButt);
		MCGContextSetStrokeMiterLimit(m_gcontext, 4.0f);
		
		MCGContextBeginPath(m_gcontext);
		MCGContextAddDot(m_gcontext, MCPointToMCGPoint(points[0], 0.5f));
		MCGContextStroke(m_gcontext);
		
		MCGContextRestore(m_gcontext);
	}
	else
	{	
		// MM-2013-11-14: [[ Bug 11457 ]] Adjust lines and polygons to make sure antialiased lines don't draw across pixels.	
		MCGPoint *t_points;
		/* UNCHECKED */ MCMemoryNewArray(npoints, t_points);
		for (uint32_t i = 0; i < npoints; i++)
			t_points[i] = MCPointToMCGPoint(points[i], 0.5f);
		
		MCGContextBeginPath(m_gcontext);
		if (p_closed)
			MCGContextAddPolygon(m_gcontext, t_points, npoints);
		else
			MCGContextAddPolyline(m_gcontext, t_points, npoints);	
		MCGContextStroke(m_gcontext);
		
		MCMemoryDeleteArray(t_points);
	}
}

void MCGraphicsContext::fillpolygon(MCPoint *points, uint2 npoints)
{
	// MM-2013-11-26: [[ Bug 11501 ]] Adjust lines and polygons to make sure antialiased lines don't draw across pixels.
	//  Here the adjust is 0.25 - not the same path as draw lines but appears to solve the issue where the fill interfers with the stroke.
	MCGPoint *t_points;
	/* UNCHECKED */ MCMemoryNewArray(npoints, t_points);
	for (uint32_t i = 0; i < npoints; i++)
		t_points[i] = MCPointToMCGPoint(points[i], 0.25f);
	
	MCGContextBeginPath(m_gcontext);
	MCGContextAddPolygon(m_gcontext, t_points, npoints);	
	MCGContextFill(m_gcontext);
	
	MCMemoryDeleteArray(t_points);	
}

static MCGRectangle MCGRectangleInset(const MCGRectangle &p_rect, MCGFloat p_inset)
{
	MCGRectangle t_rect;
	t_rect = MCGRectangleMake(p_rect.origin.x + p_inset, p_rect.origin.y + p_inset, p_rect.size.width - (p_inset * 2.0), p_rect.size.height - (p_inset * 2.0));
	
	if (t_rect.size.width < 0.0)
		t_rect.size.width = 0.0;
	if (t_rect.size.height < 0.0)
		t_rect.size.height = 0.0;
	
	return t_rect;
}

void MCGraphicsContext::drawarc(const MCRectangle& rect, uint2 start, uint2 angle, bool inside)
{	
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);	
	MCGFloat t_adjustment;
	if (m_line_width == 0.0f || inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);
	
	MCGContextBeginPath(m_gcontext);
	add_legacy_arc(m_gcontext, t_rect, start, angle, false);
	MCGContextStroke(m_gcontext);
}

void MCGraphicsContext::fillarc(const MCRectangle& rect, uint2 start, uint2 angle, bool inside)
{	
	// MM-2013-11-14: [[ Bug 11426 ]] Make sure we take account line width when filling arcs with a border.
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);	
	MCGFloat t_adjustment;
	if (inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);	
	
	MCGContextBeginPath(m_gcontext);
	// MM-2013-11-19: [[ Bug 11469 ]] Fill as a segment rather than an arc to make sure path is closed properly.
	add_legacy_segment(m_gcontext, t_rect, start, angle, false);
	MCGContextFill(m_gcontext);
}

void MCGraphicsContext::drawsegment(const MCRectangle& rect, uint2 start, uint2 angle, bool inside)
{	
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);	
	MCGFloat t_adjustment;
	if (m_line_width == 0.0f || inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);	
	
	MCGContextBeginPath(m_gcontext);
	add_legacy_segment(m_gcontext, t_rect, start, angle, false);
	MCGContextStroke(m_gcontext);
}

void MCGraphicsContext::drawsegments(MCSegment *segments, uint2 nsegs)
{
	MCGContextBeginPath(m_gcontext);
	for (uint32_t i = 0; i < nsegs; i++)
	{
		MCGPoint t_point;
		t_point . x = (MCGFloat) segments[i] . x1;
		t_point . y = (MCGFloat) segments[i] . y1;
		MCGContextMoveTo(m_gcontext, t_point);		
		t_point . x = (MCGFloat) segments[i] . x2;
		t_point . y = (MCGFloat) segments[i] . y2;
		MCGContextLineTo(m_gcontext, t_point);		
	}
	MCGContextStroke(m_gcontext);
}

void MCGraphicsContext::drawrect(const MCRectangle& rect, bool inside)
{
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);
	
	MCGFloat t_adjustment;
	if (m_line_width == 0.0f || inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;
	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);
	
	MCGContextBeginPath(m_gcontext);
	MCGContextAddRectangle(m_gcontext, t_rect);
	MCGContextStroke(m_gcontext);
}

void MCGraphicsContext::fillrect(const MCRectangle& rect, bool inside)
{
	// MM-2013-11-14: [[ Bug 11426 ]] Make sure we take account line width when filling rects with a border.
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);	
	MCGFloat t_adjustment;
	if (inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);
	
	MCGContextBeginPath(m_gcontext);
	MCGContextAddRectangle(m_gcontext, t_rect);
	MCGContextFill(m_gcontext);
}

void MCGraphicsContext::fillrects(MCRectangle *rects, uint2 nrects)
{
	MCGContextBeginPath(m_gcontext);
	for (uint2 i = 0; i < nrects; i++)
		MCGContextAddRectangle(m_gcontext, MCRectangleToMCGRectangle(rects[i]));
	MCGContextFill(m_gcontext);
}

void MCGraphicsContext::drawroundrect(const MCRectangle& rect, uint2 radius, bool inside)
{
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);
	
	MCGFloat t_adjustment;
	if (m_line_width == 0.0f || inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;
	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);
	
	MCGContextBeginPath(m_gcontext);	
	add_legacy_rounded_rectangle(m_gcontext, t_rect, radius * 0.5);			
	MCGContextStroke(m_gcontext);
}

void MCGraphicsContext::fillroundrect(const MCRectangle& rect, uint2 radius, bool inside)
{		
	// MM-2013-11-14: [[ Bug 11426 ]] Make sure we take account line width when filling rects with a border.
	MCGRectangle t_rect = MCRectangleToMCGRectangle(rect);	
	MCGFloat t_adjustment;
	if (inside)
		t_adjustment = (m_line_width == 0.0f ? 0.5f : m_line_width / 2.0f);
	else
		t_adjustment = 0.0f;	
	t_rect = MCGRectangleInset(t_rect, t_adjustment);
	
	MCGContextBeginPath(m_gcontext);
	add_legacy_rounded_rectangle(m_gcontext, t_rect, radius * 0.5);
	MCGContextFill(m_gcontext);
}

////////////////////////////////////////////////////////////////////////////////
// MM-2014-01-20: [[ Bug 11673 ]] Implement draw and fill path - there are still some cases where MCPath is being used.

void MCGraphicsContext::drawpath(MCPath *path)
{
	MCGPathRef t_path;
	t_path = MCGPathFromMCPath(path);
	
	if (t_path != NULL)
	{
		MCGContextAddPath(m_gcontext, t_path);
		MCGContextStroke(m_gcontext);
	}
}

void MCGraphicsContext::fillpath(MCPath *path, bool p_evenodd)
{
	MCGPathRef t_path;
	t_path = MCGPathFromMCPath(path);
	
	if (t_path != NULL)
	{
		MCGContextSave(m_gcontext);
		if (p_evenodd)
			MCGContextSetFillRule(m_gcontext, kMCGFillRuleEvenOdd);
		MCGContextAddPath(m_gcontext, t_path);
		MCGContextFill(m_gcontext);	
		MCGContextRestore(m_gcontext);
	}
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::drawpict(uint1 *data, uint4 length, bool embed, const MCRectangle& drect, const MCRectangle& crect)
{
}

void MCGraphicsContext::draweps(real8 sx, real8 sy, int2 angle, real8 xscale, real8 yscale, int2 tx, int2 ty,
								const char *prolog, const char *psprolog, uint4 psprologlength, const char *ps, uint4 length,
								const char *fontname, uint2 fontsize, uint2 fontstyle, MCFontStruct *font, const MCRectangle& trect)
{
}

void MCGraphicsContext::drawimage(const MCImageDescriptor& p_image, int2 sx, int2 sy, uint2 sw, uint2 sh, int2 dx, int2 dy)
{
	MCImageBitmap *t_bits = p_image.bitmap;

	MCGRaster t_raster;
	t_raster . width = t_bits -> width;
	t_raster . height = t_bits -> height;
	t_raster . stride = t_bits -> stride;
	t_raster . pixels = t_bits -> data;
	t_raster . format = t_bits -> has_transparency ? kMCGRasterFormat_ARGB : kMCGRasterFormat_xRGB;

	MCGRectangle t_clip;
	t_clip . origin . x = dx;
	t_clip . origin . y = dy;
	t_clip . size . width = sw;
	t_clip . size . height = sh;

	MCGRectangle t_dest;
	t_dest.origin.x = dx - sx;
	t_dest.origin.y = dy - sy;
	t_dest.size.width = t_raster.width;
	t_dest.size.height = t_raster.height;

	MCGContextSave(m_gcontext);
	MCGContextClipToRect(m_gcontext, t_clip);
	
	// MM-2013-10-03: [[ Bug ]] Make sure we apply the images transform before taking into account it's scale factor.
	if (p_image.has_transform)
	{
		MCGAffineTransform t_transform = MCGAffineTransformMakeTranslation(-t_dest.origin.x, -t_dest.origin.y);
		t_transform = MCGAffineTransformConcat(p_image.transform, t_transform);
		t_transform = MCGAffineTransformTranslate(t_transform, t_dest.origin.x, t_dest.origin.y);
		
		MCGContextConcatCTM(m_gcontext, t_transform);
	}	

	// IM-2013-07-19: [[ ResIndependence ]] if image has a scale factor then we need to scale the context before drawing
	if (p_image.scale_factor != 0.0 && p_image.scale_factor != 1.0)
	{
		MCGContextTranslateCTM(m_gcontext, t_dest.origin.x, t_dest.origin.y);
		MCGContextScaleCTM(m_gcontext, 1.0 / p_image.scale_factor, 1.0 / p_image.scale_factor);
		MCGContextTranslateCTM(m_gcontext, -t_dest.origin.x, -t_dest.origin.y);
	}

	MCGContextDrawPixels(m_gcontext, t_raster, t_dest, p_image.filter);
	MCGContextRestore(m_gcontext);
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::drawtheme(MCThemeDrawType p_type, MCThemeDrawInfo* p_info)
{
	MCThemeDraw(m_gcontext, p_type, p_info);
}

MCRegionRef MCGraphicsContext::computemaskregion(void)
{
	return NULL;
}

void MCGraphicsContext::clear(void)
{
}

void MCGraphicsContext::clear(const MCRectangle* rect)
{
}

void MCGraphicsContext::applywindowshape(MCWindowShape *p_mask, uint4 p_u_width, uint4 p_u_height)
{
}

////////////////////////////////////////////////////////////////////////////////

void MCGraphicsContext::drawlink(const char *link, const MCRectangle& region)
{
}


void MCGraphicsContext::drawtext(int2 x, int2 y, const char *s, uint2 length, MCFontRef p_font, Boolean image, bool p_is_unicode)
{
	// MW-2013-10-29: [[ Bug 11338 ]] If 'image' is true, then render the background
	//   rect.
	if (image)
	{
		// MM-2014-04-16: [[ Bug 11964 ]] Pass through the transform of the context to make sure we measure the width of scaled text correctly.
		int32_t t_width;
		t_width = MCFontMeasureText(p_font, s, length, p_is_unicode, MCGContextGetDeviceTransform(m_gcontext));
		
		MCGContextSave(m_gcontext);
		setforeground(m_background);
		fillrect(MCU_make_rect(x, y - MCFontGetAscent(p_font), t_width, MCFontGetAscent(p_font) + MCFontGetDescent(p_font)), false);
		MCGContextRestore(m_gcontext);
	}
		
	MCFontDrawText(m_gcontext, x, y, s, length, p_font, p_is_unicode);
}	

////////////////////////////////////////////////////////////////////////////////

uint2 MCGraphicsContext::getdepth(void) const
{
	return 32;
}

const MCColor& MCGraphicsContext::getblack(void) const
{
	return MCscreen->black_pixel;
}

const MCColor& MCGraphicsContext::getwhite(void) const
{
	return MCscreen->white_pixel;
}

const MCColor& MCGraphicsContext::getgray(void) const
{
	return MCscreen->gray_pixel;
}

const MCColor& MCGraphicsContext::getbg(void) const
{
	return MCscreen->background_pixel;
}

////////////////////////////////////////////////////////////////////////////////