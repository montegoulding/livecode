/* Copyright (C) 2003-2015 LiveCode Ltd.

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

#ifndef __MC_HANDLER__
#define	__MC_HANDLER__

struct MCExecValue;

#ifndef __MC_TYPE__
#include "type.h"
#endif

// A single variable definition. If 'init' is nil then it means the var should
// be created as a uql.
struct MCHandlerVarInfo
{
	MCNameRef name;
	MCValueRef init;
};

enum MCHandlerParamKind
{
    /* Normal - the traditional parameter kind - not typed, defaultable */
    kMCHandlerParamKindNormal,
    /* Reference - the traditional reference kind - not typed, not defaulted */
    kMCHandlerParamKindReference,
    /* Copy - typeable, defaultable - copies incoming value (currently read-only) */
    kMCHandlerParamKindCopy,
    /* Variadic - typeable, not-defaultable, must be last - accumulates remaining
     * arguments into a sequence. (currently read-only) */
    kMCHandlerParamKindVariadic,
    
#ifdef HANDLER_MUTABLE_PARAMS
    /* Ref - not-typeable, not-defaultable - references incoming value */
    kMCHandlerParamKindRef,
    /* Make - typeable, not-defaultable - replaces referenced value on
     * successful handler exit. */
    kMCHandlerParamKindMake,
    /* Drop - typeable, not-defaultable - replaces referenced value with
     * undefined on successful handler exit. */
    kMCHandlerParamKindDrop,
    /* Modify - typeable, not-defaultable - takes referenced value, and returns
     * on successful handler exit. */
    kMCHandlerParamKindModify,
#endif
};

struct MCHandlerParamInfo
{
    MCHandlerParamKind kind : 4;
    bool default_value_not_converted;
    MCNameRef name;
    MCType *type;
    MCValueRef default_value;
};

struct MCHandlerConstantInfo
{
	MCNameRef name;
	MCValueRef value;
};

class MCHandler
{
	MCHandlerlist *hlist;
	MCStatement *statements;
	MCVariable **vars;
	MCVariable **globals;
	MCContainer **params;
	MCHandlerVarInfo *vinfo;
	MCHandlerParamInfo *pinfo;
    MCHandlerConstantInfo *cinfo;
    MCNameRef name;
    // MW-2013-11-08: [[ RefactorIt ]] The 'it' variable is now always defined
    //   and this varref is used by things that want to set it.
    MCVarref *m_it;    
    /* If type is HT_FUNCTION, then they can have an optional return type. */
    MCType *return_type;
	uint2 nglobals;
	uint2 npassedparams;
	uint2 nparams;
	uint2 nvnames;
	uint2 npnames;
	uint2 nconstants;
	uint2 executing;
	uint2 firstline;
	uint2 lastline;
	// MW-2011-06-22: [[ SERVER ]] This is the index of the file that this
	//   handler came from, it was loaded in server-script mode.
	uint2 fileindex;

    /* If type is HT_OPERATOR, then operator contains the kind of operator it
     * defines. */
    MCTypeOperatorKind operator_kind : 8;
    
    unsigned type : 4;
    bool prop : 1;
    bool array : 1;
    bool is_private : 1;
    bool is_on : 1;
    
    /* Non lax handlers check the argument list conforms to signature. */
    bool non_lax : 1;
    
    /* Unnamed variadic handlers allow arguments beyond the declared 
     * list. */
    bool unnamed_variadic : 1;
	
	static Boolean gotpass;
public:
	MCHandler(uint1 htype, bool p_is_private = false, bool p_is_on = false);
	~MCHandler();

	MCNameRef getname(void)
	{
		return name;
	}
	
	bool hasname(MCNameRef other_name)
	{
		return MCNameIsEqualToCaseless(name, other_name);
	}

	Parse_stat parse(MCScriptPoint &sp, Boolean isprop);
    Exec_stat exec(MCExecContext &, MCParameter *);
	
    MCVariable *getvar(uint2 index, Boolean isparam);
    MCContainer *getcontainer(uint2 index, Boolean isparam);
    
	integer_t getnparams(void);
    MCValueRef getparam(uindex_t p_index);
    bool getparamreadonly(uindex_t p_index);
	Parse_stat findvar(MCNameRef name, MCVarref **);
	Parse_stat newvar(MCNameRef name, MCValueRef init, MCVarref **);
	Parse_stat findconstant(MCNameRef name, MCExpression **);
	Parse_stat newconstant(MCNameRef name, MCValueRef value);
	void newglobal(MCNameRef name);
	bool getparamnames(MCListRef& r_list);
	bool getparamnames_as_properlist(MCProperListRef& r_list);
	bool getvariablenames(MCListRef& r_list);
    bool getvariablenames_as_properlist(MCProperListRef& r_list);
    bool getglobalnames(MCListRef& r_list);
    bool getglobalnames_as_properlist(MCProperListRef& r_list);
    bool getvarnames(bool p_all, MCListRef& r_list);
    bool getconstantnames_as_properlist(MCProperListRef& r_list);
    //Exec_stat eval(MCExecPoint &);
    uint4 linecount();

	// Used by the externals API, this method returns the current incarnation of
	// the 'it' variable in this handler - if any.
	MCVarref *getit(void);

	void clearpass()
	{
		gotpass = False;
	}
	Boolean getpass() const
	{
		return gotpass;
	}
	Handler_type gettype() const
	{
		return (Handler_type)type;
	}
	uint2 getstartline(void) const
	{
		return firstline;
	}
	uint2 getendline(void) const
	{
		return lastline;
	}
	void setfileindex(uint2 p_fileindex)
	{
		fileindex = p_fileindex;
	}
	uint2 getfileindex(void) const
	{
		return fileindex;
	}
	bool isprivate(void) const
	{
		return is_private == True;
	}
    
    bool ison(void) const
    {
        return is_on;
    }

    MCHandlerlist* gethandlerlist(void) const
    {
        return hlist;
    }
    
	void getvarlist(MCVariable**& r_vars, uint32_t& r_var_count)
	{
		r_vars = vars;
		r_var_count = nvnames;
	}
	
	void getgloballist(MCVariable**& r_vars, uint32_t& r_var_count)
	{
		r_vars = globals;
		r_var_count = nglobals;
	}
	
    MCHandlerlist *gethlist(void) const
    {
        return hlist;
    }
	void sethlist(MCHandlerlist *p_list)
	{
		hlist = p_list;
	}

	// OK-2010-01-14: [[Bug 6506]] - These two methods needed to support global watchedVariables
	uint2 getnglobals(void) const
	{
		return nglobals;
	}

	MCVariable *getglobal(uint2 p_index) const
	{
		return globals[p_index];
	}
    
    bool copysignature(MCArrayRef& r_array);

private:
	Parse_stat newparam(MCScriptPoint& sp);
    Exec_stat enter(MCExecContext& ctxt, MCParameter* params);
    Exec_stat enter_non_lax(MCExecContext& ctxt, MCParameter* params);
    Exec_stat leave(MCExecContext& ctxt, Exec_stat p_stat);
};
#endif
