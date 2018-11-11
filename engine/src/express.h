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

//
// MCExpression class declarations
//
#ifndef	EXPRESSION_H
#define	EXPRESSION_H

#define MAX_EXP 7

#ifndef __MC_EXEC__
#include "exec.h"
#endif

class MCExpressionAttrs
{
public:
    bool IsConstant(void) const
    {
        return (m_flags & kIsConstant) != 0;
    }
    
    bool HasType(void) const
    {
        return (m_flags & _kTypeMask) != 0;
    }
    
    bool IsBoolean(void) const
    {
        return _IsType(kIsBoolean);
    }
    
    bool IsString(void) const
    {
        return _IsType(kIsString);
    }
    
    bool IsNumber(void) const
    {
        return _IsType(kIsNumber);
    }
    
    bool IsData(void) const
    {
        return _IsType(kIsData);
    }
    
    bool IsArray(void) const
    {
        return _IsType(kIsArray);
    }
    
    MCExpressionAttrs SetIsConstant(void)
    {
        m_flags |= kIsConstant;
        return *this;
    }
    
    MCExpressionAttrs SetIsBoolean(void)
    {
        m_flags = (m_flags & ~_kTypeMask) | kIsBoolean;
        return *this;
    }
    
    MCExpressionAttrs SetIsString(void)
    {
        m_flags = (m_flags & ~_kTypeMask) | kIsString;
        return *this;
    }
    
    MCExpressionAttrs SetIsNumber(void)
    {
        m_flags = (m_flags & ~_kTypeMask) | kIsNumber;
        return *this;
    }
    
    MCExpressionAttrs SetIsData(void)
    {
        m_flags = (m_flags & ~_kTypeMask) | kIsData;
        return *this;
    }
    
    MCExpressionAttrs SetIsArray(void)
    {
        m_flags = (m_flags & ~_kTypeMask) | kIsArray;
        return *this;
    }
    
    template<typename T>
    MCExpressionAttrs SetType(void);
    
private:
    enum
    {
        kIsConstant = 1 << 0,
        kIsConstantString = 1 << 1,
        kIsConstantNumber = 1 << 2,
    };
    
    uint32_t m_flags = 0;
};

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<MCBooleanRef>(void)
{
    return SetIsBoolean();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<double>(void)
{
    return SetIsNumber();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<integer_t>(void)
{
    return SetIsNumber();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<uinteger_t>(void)
{
    return SetIsNumber();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<MCNumberRef>(void)
{
    return SetIsNumber();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<MCStringRef>(void)
{
    return SetIsString();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<MCDataRef>(void)
{
    return SetIsData();
}

template<>
inline MCExpressionAttrs MCExpressionAttrs::SetType<MCArrayRef>(void)
{
    return SetIsArray();
}

class MCExpression
{
protected:
	uint2 line;
	uint2 pos;
	Factor_rank rank;
	MCExpression *root;
	MCExpression *left;
	MCExpression *right;

public:
	MCExpression();
	virtual ~MCExpression();

    virtual MCExpressionAttrs getattrs(void) const;
	
	virtual Parse_stat parse(MCScriptPoint &, Boolean the);

	// Evaluate the expression as its natural type basic type (note that
	// execvalue's cannot be set/enum/custom, they should all be resolved
	// to the appropriate basic type first!). This form should be used for
	// descendents of MCExpression which are an umbrella for many syntax forms
	// and thus have variant return type (such as MCProperty).

	virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
	
	// Evaluate the expression as a container, and place the reference to
	// the container's value in r_ref.
    // EP-less version of evaluation functions
    virtual bool evalcontainer(MCExecContext& ctxt, MCContainer& r_container);

	// Return the var-ref which lies at the root of this expression. 
	// A return value of NULL means that there is no root variable.
	// The purpose of this call is to analyze (after parsing) whether the
	// left and right hand side of an variable mutation command share the
	// same variable. It is designed to be used at parse-time, not exec-time.
	virtual MCVarref *getrootvarref(void);
	
	//////////
	
	template <typename T>
	void eval(MCExecContext& ctxt, T& r_value)
	{
		eval_typed(ctxt, MCExecValueTraits<T>::type_enum, &r_value);
    }
	
	// This method evaluates the the MCExpression as the specified type. The
	// value ptr should be a pointer to the appropriate native value to store
	// the result.
	void eval_typed(MCExecContext& ctxt, MCExecValueType return_type, void* return_value);
	
	//////////
    
    template<typename T>
    bool constant_eval(T& r_value)
    {
        return constant_eval_typed(MCExecValueTraits<T>::type_enum, &r_value);
    }
    
    bool constant_eval_typed(MCExecValueType return_type, void* return_value_ptr);
    
    //////////
	
	void setrank(Factor_rank newrank)
	{
		rank = newrank;
	}
	void setroot(MCExpression *newroot)
	{
		root = newroot;
	}
	void setleft(MCExpression *newleft)
	{
		left = newleft;
	}
	void setright(MCExpression *newright)
	{
		right = newright;
	}
	Factor_rank getrank()
	{
		return rank;
	}
	MCExpression *getroot()
	{
		return root;
	}
	MCExpression *getleft()
	{
		return left;
	}
	MCExpression *getright()
	{
		return right;
	}
	Parse_stat getexps(MCScriptPoint &sp, MCExpression *earray[], uint2 &ecount);
	void freeexps(MCExpression *earray[], uint2 ecount);
	Parse_stat get0params(MCScriptPoint &);
	Parse_stat get0or1param(MCScriptPoint &sp, MCExpression **exp, Boolean the);
	Parse_stat get1param(MCScriptPoint &, MCExpression **exp, Boolean the);
    Parse_stat get0or1or2params(MCScriptPoint &, MCExpression **e1,
                                MCExpression **e2, Boolean the);
	Parse_stat get1or2params(MCScriptPoint &, MCExpression **e1,
	                         MCExpression **e2, Boolean the);
	Parse_stat get2params(MCScriptPoint &, MCExpression **e1, MCExpression **e2);
	Parse_stat get2or3params(MCScriptPoint &, MCExpression **exp1,
	                         MCExpression **exp2, MCExpression **exp3);
	Parse_stat get3params(MCScriptPoint &, MCExpression **exp1,
	                      MCExpression **exp2, MCExpression **exp3);
	Parse_stat get4or5params(MCScriptPoint &, MCExpression **exp1,
	                         MCExpression **exp2, MCExpression **exp3,
	                         MCExpression **exp4, MCExpression **exp5);
	Parse_stat get6params(MCScriptPoint &, MCExpression **exp1,
	                      MCExpression **exp2, MCExpression **exp3,
	                      MCExpression **exp4, MCExpression **exp5,
	                      MCExpression **exp6);
	Parse_stat getvariableparams(MCScriptPoint &sp, uint32_t p_min_params, uint32_t p_param_count, ...);
	Parse_stat getparams(MCScriptPoint &spt, MCParameter **params);
	void initpoint(MCScriptPoint &);
	static bool compare_array_element(void *context, MCArrayRef array, MCNameRef key, MCValueRef value);
    
private:
    /* The single parameter is parsed to the 'single' argument of parseexp -
     * for 0 param fetches this is False, for others this is True. */
    Parse_stat gettheparam(MCScriptPoint& sp, Boolean single, MCExpression** exp);
};

class MCFuncref : public MCExpression
{
	MCNewAutoNameRef name;
    MCHandler *handler;
	MCParameter *params;
	bool resolved : 1;
    bool global_handler : 1;
public:
	MCFuncref(MCNameRef);
	virtual ~MCFuncref();
	virtual Parse_stat parse(MCScriptPoint &, Boolean the);
	void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};
#endif
