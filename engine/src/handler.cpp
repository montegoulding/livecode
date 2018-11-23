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

#include "prefix.h"

#include "globdefs.h"
#include "filedefs.h"
#include "objdefs.h"
#include "parsedef.h"

#include "scriptpt.h"

#include "debug.h"
#include "hndlrlst.h"
#include "handler.h"
#include "param.h"
#include "statemnt.h"
#include "literal.h"
#include "mcerror.h"
#include "newobj.h"
#include "object.h"
#include "util.h"
#include "dispatch.h"
#include "globals.h"
#include "cmds.h"
#include "license.h"
#include "redraw.h"
#include "type.h"

#include "exec.h"

////////////////////////////////////////////////////////////////////////////////

Boolean MCHandler::gotpass;

////////////////////////////////////////////////////////////////////////////////

MCHandler::MCHandler(uint1 htype, bool p_is_private, bool p_is_on)
    : hlist(nullptr),
      statements(nullptr),
      vars(nullptr),
      globals(nullptr),
      params(nullptr),
      vinfo(nullptr),
      pinfo(nullptr),
      cinfo(nullptr),
      nglobals(0),
      npassedparams(0),
      nparams(0),
      nvnames(0),
      npnames(0),
      nconstants(0),
      executing(0),
      firstline(0),
      lastline(0),
      fileindex(0),
      name(nullptr),
      prop(False),
      array(False),
      is_private(p_is_private ? True : False),
      is_on(p_is_on),
      type(htype),
      m_it(nullptr),
      non_lax(false),
      unnamed_variadic(true),
      operator_kind(kMCTypeOperatorKindUnknown),
      return_type(nullptr)
{
}

MCHandler::~MCHandler()
{
    if (operator_kind != kMCTypeOperatorKindUnknown)
    {
        MCTypeUndefineOperator(name, operator_kind);
    }
    
	MCStatement *stmp;
	while (statements != NULL)
	{
		stmp = statements;
		statements = statements->getnext();
		delete stmp;
	}

	for(uint32_t i = 0; i < nvnames; i++)
	{
		MCValueRelease(vinfo[i] . name);
		MCValueRelease(vinfo[i] . init);
	}
	delete[] vinfo; /* Allocated with new[] */

	for(uint32_t i = 0; i < npnames; i++)
    {
		MCValueRelease(pinfo[i] . name);
        if (pinfo[i].default_value != nullptr)
        {
            MCValueRelease(pinfo[i].default_value);
        }
    }
	delete[] pinfo; /* Allocated with new[] */

	delete[] globals; /* Allocated with new[] */

	for(uint32_t i = 0; i < nconstants; i++)
	{
		MCValueRelease(cinfo[i] . name);
		MCValueRelease(cinfo[i] . value);
	}
	delete[] cinfo; /* Allocated with new[] */
	
	// MW-2013-11-08: [[ RefactorIt ]] Delete the it varref.
	delete m_it;

	MCValueRelease(name);
}

/* The newparam() method parses a handler parameter. On entry, the sp will be
 * looking at a literal token.
 *
 * Handler parameters have the following syntax:
 *   <@literal> -> reference
 *   <literal> [ default <value> ] -> normal
 *   <literal> as <type> [ default <value> ] -> copy
 *   <literal> ... [ as <type> ] -> variadic
 *   ... -> allow unnamed parameters (accessed via param()).
 *
 * If there is an optional (has default clause), copy, variadic, or unnamed
 * parameter marker then the handler becomes 'non-lax' - this means that the
 * argument list must conform to the signature. i.e. missing parameters are
 * not allowed unless they are optional, and unnamed parameters are not allowed
 * unless the ... marker is present.
 *
 * Any default value expression is statically evaluated. It is an error to
 * specify a default value which cannot be statically evaluated in the
 * hander list static context.
 *
 * HANDLER_MUTABLE_PARAMS
 *
 * In the future the syntax might be extended to support a range of other
 * parameter modes:
 *   '@' '(' copy|ref|make|drop|modify|variadic ')' <literal> [ 'as' <literal> ] [ default <expression> ]
 *
 * A type clause is only allowed for copy, make, drop, modify and variadic modes (not ref)
 * A default clause is only allowed for copy modes
 * If a default expression is present, then it must be statically evaluatable.
 *
 */
Parse_stat MCHandler::newparam(MCScriptPoint& sp)
{
    /* If there has already been a variadic parameter, then there can be no
     * more. */
    if ((non_lax && unnamed_variadic) ||
        (npnames > 0 && pinfo[npnames - 1].kind == kMCHandlerParamKindVariadic))
    {
        return sp.error(PE_PARAM_CANTVARIADIC);
    }
    
    /* If the name is '...' then we mark the handler as non_lax_variadic which
     * gives the same behavior as current handlers with extra parameters. */
    if (sp.current() == ST_DOTS)
    {
        unnamed_variadic = true;
        non_lax = true;
        return PS_NORMAL;
    }
    
    /* New style parameters are '@' followed by '('. */
    bool t_new_style = false;
    if (MCStringIsEqualToCString(sp.gettoken_stringref(), "@", kMCStringOptionCompareExact))
    {
        Symbol_type t_ttype;
        if (sp.next(t_ttype) == PS_NORMAL)
        {
            if (t_ttype == ST_LP)
            {
                t_new_style = true;
            }
            else
            {
                sp.backup();
            }
        }
    }
    
    MCNewAutoNameRef t_name;
    MCHandlerParamKind t_kind;
    MCType *t_type = nullptr;
    MCAutoValueRef t_default_value;
    if (t_new_style)
    {
        Symbol_type t_ttype;
        const LT *te;
        if (sp.next(t_ttype) != PS_NORMAL ||
            t_ttype != ST_ID ||
            sp.lookup(SP_SUGAR, te) == PS_NO_MATCH ||
            te->type != TT_PARAMETER)
        {
            return sp.error(PE_PARAM_BADMODE);
        }
        
        switch(te->which)
        {
        case PA_COPY:
            t_kind = kMCHandlerParamKindCopy;
            break;
        case PA_VARIADIC:
            t_kind = kMCHandlerParamKindVariadic;
            break;

#ifndef HANDLER_MUTABLE_PARAMS
        default:
            return sp.error(PE_PARAM_BADMODE);
            break;
#else
        case PA_REF:
            t_kind = kMCHandlerParamKindRef;
            break;
        case PA_MAKE:
            t_kind = kMCHandlerParamKindMake;
            break;
        case PA_DROP:
            t_kind = kMCHandlerParamKindDrop;
            break;
        case PA_MODIFY:
            t_kind = kMCHandlerParamKindModify;
            break;
        default:
            MCUnreachableReturn(PS_ERROR);
            break;
#endif
        }
        
        if (sp.next(t_ttype) != PS_NORMAL ||
            t_ttype != ST_RP)
        {
            return sp.error(PE_PARAM_NORP);
        }
        
        if (sp.next(t_ttype) != PS_NORMAL ||
            t_ttype != ST_ID)
        {
            return sp.error(PE_PARAM_BADNAME);
        }
        
        t_name = sp.gettoken_nameref();
    }
    else
    {
        MCStringRef t_token = sp.gettoken_stringref();
        MCAutoStringRef t_token_name;
        if (MCStringGetNativeCharAtIndex(t_token, 0) != '@')
        {
            t_kind = kMCHandlerParamKindNormal;
            t_token_name = t_token;
        }
        else
        {
            t_kind = kMCHandlerParamKindReference;
            if (!MCStringCopySubstring(t_token, MCRangeMakeMinMax(1, MCStringGetLength(t_token)), &t_token_name))
            {
                return sp.outofmemory();
            }
        }
        
        if (!MCNameCreate(*t_token_name, &t_name))
        {
            return sp.outofmemory();
        }
    }
    
    /* Parse the optional ... variadic marker */
    {
        Symbol_type t_ttype;
        if (sp.next(t_ttype) == PS_NORMAL)
        {
            if (t_ttype == ST_DOTS)
            {
                /* To have a variadic marker, the parameter kind must be normal
                 * or copy. */
                if (t_kind != kMCHandlerParamKindNormal &&
                    t_kind != kMCHandlerParamKindCopy)
                {
                    return sp.error(PE_PARAM_BADVARIADIC);
                }
                
                /* The parameter now becomes variadic */
                t_kind = kMCHandlerParamKindVariadic;
            }
            else
            {
                sp.backup();
            }
        }
    }
    
    /* Parse the optional 'as' clause */
    Symbol_type t_ttype;
    if (sp.skip_token(SP_FACTOR, TT_PREP, PT_AS) == PS_NORMAL)
    {
        /* If there is a type clause, then a normal parameter becomes a
         * copy parameter. */
        if (t_kind == kMCHandlerParamKindNormal)
        {
            t_kind = kMCHandlerParamKindCopy;
        }
        
        /* If there is a type clause, then the kind cannot be reference or
         * ref. */
        if (t_kind == kMCHandlerParamKindReference /* ||
             t_kind == kMCHandlerParamKindRef*/)
        {
            return sp.error(PE_PARAM_BADTYPE);
        }
        
        if (sp.next(t_ttype) != PS_NORMAL ||
            t_ttype != ST_ID)
        {
            return sp.error(PE_PARAM_BADTYPENAME);
        }
        
        if (!MCTypeDeclare(sp, sp.gettoken_nameref(), t_type))
        {
            return PS_ERROR;
        }
    }
    
    /* Parse the optional 'default' clause */
    bool t_default_value_converted = true;
    if (sp.skip_token(SP_COMMAND, TT_DEFAULT, TT_UNDEFINED) == PS_NORMAL)
    {
        /* If there is a default clause, then the parameter must be normal or
         * copy. */
        if (t_kind != kMCHandlerParamKindNormal &&
             t_kind != kMCHandlerParamKindCopy)
        {
            return sp.error(PE_PARAM_BADDEFAULT);
        }

        /* Parse the expression */
        MCAutoPointer<MCExpression> t_default_exp;
        if (sp.parseexp(False, False, &(&t_default_exp)) != PS_NORMAL)
        {
            return sp.error(PE_PARAM_NODEFAULTEXP);
        }
        
        /* Attempt to evaluate as static */
        if (!sp.staticevalexp(*t_default_exp, &t_default_value))
        {
            return sp.error(PE_PARAM_DEFAULTEXPNONCONST);
        }
        
        /* Now attempt to convert to type, if specified. */
        if (t_type != nullptr)
        {
            bool t_is_type;
            MCerrorlock++;
            t_is_type = MCTypeEvalAs(*sp.getstaticctxt(), t_type, t_default_value);
            MCerrorlock--;
            
            if (t_is_type)
            {            
                if (!t_default_value.MakeUnique())
                {
                    return sp.error(PE_OUTOFMEMORY);
                }
            }
            else
            {
                t_default_value_converted = false;
            }
        }
    }

	// OK-2010-01-11: [[Bug 7744]] - Check existing parsed parameters for duplicates.
	for (uint2 i = 0; i < npnames; i++)
	{
		if (MCNameIsEqualToCaseless(pinfo[i] . name, *t_name))
		{
            return sp.error(PE_HANDLER_DUPPARAM);
		}
	}

    /* If the parameter is not normal or reference, or there is a default value
     * then this is a non-lax handler */
    if (t_default_value.IsSet() ||
        (t_kind != kMCHandlerParamKindNormal &&
         t_kind != kMCHandlerParamKindReference))
    {
        non_lax = true;
        unnamed_variadic = false;
    }
    
    /* Push the new parameter onto the parameter list */
	MCU_realloc((char **)&pinfo, npnames, npnames + 1, sizeof(MCHandlerParamInfo));
    pinfo[npnames].kind = t_kind;
    pinfo[npnames].default_value_not_converted = !t_default_value_converted;
	pinfo[npnames].name = t_name.Take();
    pinfo[npnames].type = t_type;
    pinfo[npnames].default_value = t_default_value.Take();
	npnames++;

	return PS_NORMAL;
}

Parse_stat MCHandler::parse(MCScriptPoint &sp, Boolean isprop)
{
    Parse_stat stat;
    const LT *te;
	Symbol_type t_type;
	
	firstline = sp.getline();
	hlist = sp.gethlist();
	prop = isprop;
    
    /* If this is not an operator handler, then we must parse the operator name
     * first - e.g. as, is, boolean, number, integer, string, data, array,
     * sequence. */
    MCTypeOperatorKind t_operator_kind = kMCTypeOperatorKindUnknown;
    MCNewAutoNameRef t_operator_name;
    if (type == HT_OPERATOR)
    {
        if (sp.next(t_type) != PS_NORMAL)
        {
            return sp.error(PE_HANDLER_NOOPERATOR);
        }
        
        if (t_type == ST_ID)
        {
            if (sp.lookup(SP_VALIDATION, te) != PS_NO_MATCH)
            {
                switch(te->which)
                {
                case IV_LOGICAL:
                    t_operator_kind = kMCTypeOperatorKindBoolean;
                    break;
                case IV_NUMBER:
                    t_operator_kind = kMCTypeOperatorKindNumber;
                    break;
                case IV_STRING:
                    t_operator_kind = kMCTypeOperatorKindString;
                    break;
                case IV_ARRAY:
                    t_operator_kind = kMCTypeOperatorKindArray;
                    break;
                default:
                    break;
                }
            }
            else if (sp.lookup(SP_SUGAR, te) != PS_NO_MATCH &&
                     te->which == SG_DATA)
            {
                t_operator_kind = kMCTypeOperatorKindData;
            }
            else if (sp.lookup(SP_FACTOR, te) != PS_NO_MATCH &&
                     te->type == TT_BINOP)
            {
                switch(te->which)
                {
                case O_AS_TYPE:
                    t_operator_kind = kMCTypeOperatorKindAsType;
                    break;
                case O_IS_TYPE:
                    t_operator_kind = kMCTypeOperatorKindIsType;
                    break;
                default:
                    break;
                }
            }
        }
        
        if (t_operator_kind == kMCTypeOperatorKindUnknown)
        {
            return sp.error(PE_HANDLER_BADOPERATOR);
        }
        
        t_operator_name = sp.gettoken_nameref();
    }
    
    /* Now parse the name */
    if (sp.next(t_type) != PS_NORMAL)
    {
        MCperror->add(PE_HANDLER_NONAME, sp);
        return PS_ERROR;
    }
    
    name = MCValueRetain(sp . gettoken_nameref());
    
    /* At this point we continue to use the static context of the handlerlist
     * as parameters might have default expressions. */
    
	// MW-2010-01-08: [[Bug 7792]] Check whether the handler name is a reserved function identifier
	if (t_type != ST_ID ||
			sp.lookup(SP_COMMAND, te) != PS_NO_MATCH ||
			(sp.lookup(SP_FACTOR, te) != PS_NO_MATCH &&
			te -> type == TT_FUNCTION))
	{
		MCperror->add(PE_HANDLER_BADNAME, sp);
		return PS_ERROR;
	}
    
	if (prop)
	{
		if (sp.next(t_type) == PS_NORMAL)
		{
			if (t_type == ST_LB)
			{
				if (sp.next(t_type) != PS_NORMAL || t_type != ST_ID)
				{
					MCperror->add(PE_HANDLER_BADPARAM, sp);
					return PS_ERROR;
				}

				if (newparam(sp) != PS_NORMAL)
					return PS_ERROR;

				if (sp.next(t_type) != PS_NORMAL || t_type != ST_RB)
				{
					MCperror->add(PE_HANDLER_BADPARAM, sp);
					return PS_ERROR;
				}
				array = True;
			}
			else
				sp.backup();
		}
	}

    bool t_needs_it;
    t_needs_it = true;
    
    /* If the type is HT_FUNCTION, and we encounter '(' then it is a non-lax
     * handler with return type */
    bool t_is_non_lax_function = false;
    if (sp.next(t_type) == PS_NORMAL)
    {
        if (t_type == ST_LP)
        {
            t_is_non_lax_function = true;
            
            /* Mark as non lax from the start */
            non_lax = true;
            unnamed_variadic = false;
        }
        else
        {
            sp.backup();
        }
    }
    
	while (sp.next(t_type) == PS_NORMAL)
	{
        if (t_is_non_lax_function &&
            t_type == ST_RP)
            break;
        
        /* Force presence of ',' in non-lax handlers */
        if (non_lax)
        {
            if (npnames > 0)
            {
                if (t_type != ST_SEP)
                {
                    return sp.error(PE_HANDLER_NOPARAMSEP);
                }
                
                if (sp.next(t_type) != PS_NORMAL)
                {
                    return sp.error(PE_HANDLER_BADPARAMEOL);
                }
            }
        }
        else
        {
            if (t_type == ST_SEP)
                continue;
        }
        
		const LT *t_te;
        MCExpression *newfact = NULL;
		if (t_type == ST_DOTS ||
            (t_type == ST_ID &&
                sp.lookup(SP_FACTOR, t_te) == PS_NO_MATCH &&
                sp.lookupconstant(&newfact) != PS_NORMAL))
        {
            if (newparam(sp) != PS_NORMAL)
            {
                return PS_ERROR;
            }
        }
        else
        {
			delete newfact;
            return sp.error(PE_HANDLER_BADPARAM);
		}
        
        // AL-2014-11-04: [[ Bug 13902 ]] Check if the param we just created was called 'it'.
        if (npnames > 0 &&
            MCNameIsEqualToCaseless(pinfo[npnames - 1] . name, MCN_it))
            t_needs_it = false;
    }
    
    /* If this is a non-lax function, then parse the return type */
    if (t_is_non_lax_function)
    {
        /* ')' 'as' <id> */
        if (t_type != ST_RP ||
            sp.skip_token(SP_FACTOR, TT_PREP, PT_AS) != PS_NORMAL ||
            sp.next(t_type) != PS_NORMAL ||
            t_type != ST_ID)
        {
            return sp.error(PE_HANDLER_BADRETURNTYPE);
        }
        
        if (!MCTypeDeclare(sp, sp.gettoken_nameref(), return_type))
        {
            return PS_ERROR;
        }
    }
    
    /* If this is an operator handler, check that the parameter list conforms to
     * requirements. */
    if (t_operator_kind != kMCTypeOperatorKindUnknown)
    {
        if (npnames != 1 ||
            pinfo[0].type != nullptr ||
            pinfo[0].kind != kMCHandlerParamKindNormal ||
            pinfo[0].default_value != nullptr)
        {
            return sp.error(PE_HANDLER_BADOPPARAMS);
        }
        
        non_lax = true;
        unnamed_variadic = false;
    }

    // AL-2014-11-04: [[ Bug 13902 ]] Only define it as a var if it wasn't one of the parameter names.
    if (t_needs_it)
        /* UNCHECKED */ newvar(MCN_it, kMCEmptyName, &m_it);
    
	if (sp.skip_eol() != PS_NORMAL)
	{
		MCperror->add(PE_HANDLER_BADPARAMEOL, sp);
		return PS_ERROR;
    }
    
    /* Now handler parsing is done, provide our own static context. */
    MCExecContext t_static_ctxt(hlist->getparent(), hlist, this);
    sp.setstaticctxt(&t_static_ctxt);
    
    sp.sethandler(this);
	MCStatement *curstatement = NULL;
	MCStatement *newstatement = NULL;
	while (True)
	{
		if ((stat = sp.next(t_type)) != PS_NORMAL)
		{
			if (stat == PS_EOL)
			{
				if (sp.skip_eol() != PS_NORMAL)
				{
					MCperror->add(PE_HANDLER_BADLINE, sp);
					return PS_ERROR;
				}
				else
					continue;
			}
			else
			{
				MCperror->add(PE_HANDLER_NOCOMMAND, sp);
				return PS_ERROR;
			}
		}
		if (t_type == ST_DATA)
			newstatement = new (nothrow) MCEcho;
		else if (sp.lookup(SP_COMMAND, te) != PS_NORMAL)
		{
			if (t_type != ST_ID)
			{
				MCperror->add(PE_HANDLER_NOCOMMAND, sp);
				return PS_ERROR;
			}
			newstatement = new (nothrow) MCComref(sp.gettoken_nameref());
		}
		else
		{
			switch (te->type)
			{
			case TT_STATEMENT:
				newstatement = MCN_new_statement(te->which);
				break;
            case TT_END:
                if ((stat = sp.next(t_type)) != PS_NORMAL)
                {
                    return sp.error(PE_HANDLER_NOEND);
                }
                    
                /* Operator handlers have the operator name before the type name
                 * after end. */
				if ((t_operator_kind != kMCTypeOperatorKindUnknown &&
                     (!MCNameIsEqualToCaseless(*t_operator_name, sp.gettoken_nameref())||
                      ((stat = sp.next(t_type)) != PS_NORMAL))) ||
                    !MCNameIsEqualToCaseless(name, sp.gettoken_nameref()))
				{
                    return sp.error(PE_HANDLER_BADEND);
				}
                    
				lastline = sp.getline();
				sp.skip_eol();
                goto done;
			default:
				MCperror->add(PE_HANDLER_NOTCOMMAND, sp);
				return PS_ERROR;
			}
		}
		if (newstatement->parse(sp) != PS_NORMAL)
		{
			MCperror->add(PE_HANDLER_BADCOMMAND, sp);
			delete newstatement;
			return PS_ERROR;
		}
		if (curstatement == NULL)
			statements = curstatement = newstatement;
		else
		{
			curstatement->setnext(newstatement);
			curstatement = newstatement;
		}
	}

done:
    /* If parsing succeeded, and this is an operator handler - declare it */
    if (t_operator_kind != kMCTypeOperatorKindUnknown)
    {
        if (!MCTypeDefineOperator(sp, name, t_operator_kind, this))
        {
            return PS_ERROR;
        }
    
        operator_kind = t_operator_kind;
    }
        
    return PS_NORMAL;
}

/* Called on entry to a non-lax handler. */
Exec_stat MCHandler::enter_non_lax(MCExecContext& ctxt, MCParameter *p_params)
{
    MCAutoArrayRef t_trailing;
    
    /* Compute the number of passed parameters - used by the params function */
    npassedparams = 0;
    for (MCParameter *tptr = p_params; tptr != NULL; tptr = tptr->getnext())
    {
        if (!tptr->istrailing())
        {
            npassedparams++;
        }
        else
        {
            MCAutoValueRef t_trailing_value;
            if (!tptr->eval_argument(ctxt, &t_trailing_value) ||
                !ctxt.ConvertToArray(*t_trailing_value, &t_trailing))
            {
                MCeerror->add(EE_HANDLER_BADTRAILING, firstline - 1, 1, name);
                return ES_ERROR;
            }
            
            npassedparams += MCArrayGetCount(*t_trailing);
        }
    }
    
    /* If this is not an unnamed_variadic handler, then the number of needed
     * parameters is npnames, otherwise it is npassedparams. */
    uindex_t t_arg_count;
        
    /* If not unnamed_variadic and the number of passed arguments is greater
     * than the number of named parameters, then the last named parameter
     * must be variadic. */
    if (!unnamed_variadic)
    {
        if (npassedparams > npnames &&
            pinfo[npnames - 1].kind != kMCHandlerParamKindVariadic)
        {
            MCeerror->add(EE_HANDLER_TOOMANYARGS, firstline - 1, 1, name);
            return ES_ERROR;
        }
        
        t_arg_count = npnames;
    }
    else
    {
        t_arg_count = npassedparams;
    }
    
    /* Allocate the container argument array. */
    MCContainer **t_args;
    if (t_arg_count != 0)
    {
        t_args = new (nothrow) MCContainer *[t_arg_count];
    }
    else
    {
        t_args = nullptr;
    }
    
    /* Now process the parameter list into arguments. */
    Exec_errors t_error = EE_UNDEFINED;
    uindex_t t_trailing_arg = 0;
    if (p_params != nullptr &&
        p_params->istrailing())
    {
        p_params = nullptr;
        t_trailing_arg = 1;
    }
    uindex_t t_arg;
    for(t_arg = 0; t_arg < t_arg_count; t_arg++)
    {
        /* This holds the name of the argument */
        MCNameRef t_name;
        
        /* This holds the value of a non-reference argument */
        MCExecValue t_value;

        /* This holds the value of the container for a reference argument. */
        MCContainer *t_reference = nullptr;
        
        /* If true, then the default value of the parameter should be taken */
        bool t_take_default = false;
        
        /* What happens depends on whether the argument exists or not and
         * whether we are in the named parameters or not */
        if (p_params != nullptr || t_trailing_arg != 0)
        {
            if (t_arg < npnames)
            {
                /* Named parameter */
                
                switch(pinfo[t_arg].kind)
                {
                case kMCHandlerParamKindNormal:
                case kMCHandlerParamKindCopy:
                    if (t_trailing_arg == 0)
                    {
                        /* If the parameter has a default value, and there is no
                         * expression in the parameter, then take the default */
                        if (!p_params->hasexp() &&
                            pinfo[t_arg].default_value != nullptr)
                        {
                            t_take_default = true;
                            break;
                        }
                            
                        if (!p_params->eval_argument_ctxt(ctxt, t_value))
                        {
                            t_error = EE_HANDLER_BADPARAM;
                            break;
                        }
                    }
                    else
                    {
                        MCValueRef t_trailing_value;
                        if (!MCArrayFetchValueAtIndex(*t_trailing, t_trailing_arg, t_trailing_value))
                        {
                            t_error = EE_HANDLER_BADTRAILING;
                            break;
                        }
                        
                        MCExecTypeSetValueRef(t_value, MCValueRetain(t_trailing_value));
                    }
                    
                    if (pinfo[t_arg].type != nullptr &&
                        !MCTypeEvalAs(ctxt, pinfo[t_arg].type, t_value))
                    {
                        MCExecTypeRelease(t_value);
                        t_error = EE_HANDLER_BADPARAMTYPE;
                        break;
                    } 
                    break;
                case kMCHandlerParamKindReference:
                    if (t_trailing_arg == 0)
                    {
                        /* For reference parameters, evaluate the container */
                        t_reference = p_params->eval_argument_container();
                        if (t_reference == nullptr)
                        {
                            t_error = EE_HANDLER_BADPARAM;
                        }
                    }
                    else
                    {
                        /* Reference parameters can't be trailing currently. */
                        t_error = EE_HANDLER_BADPARAM;
                    }
                    break;
                case kMCHandlerParamKindVariadic:
                    /* For variadic parameters, evaluate the remaining arguments. */
                    {
                        MCAutoArrayRef t_arg_seq;
                        if (!MCArrayCreateMutable(&t_arg_seq))
                        {
                            t_error = EE_NO_MEMORY;
                            break;
                        }
                        
                        uindex_t t_arg_index = 1;
                        while(p_params != nullptr || t_trailing_arg != 0)
                        {
                            MCAutoValueRef t_arg_value;
                            if (p_params != nullptr)
                            {
                                if (!p_params->eval_argument(ctxt, &t_arg_value))
                                {
                                    t_error = EE_HANDLER_BADPARAM;
                                    break;
                                }
                            }
                            else
                            {
                                MCValueRef t_fetched_arg_value;
                                if (!MCArrayFetchValueAtIndex(*t_trailing, t_trailing_arg, t_fetched_arg_value))
                                {
                                    t_error = EE_HANDLER_BADTRAILING;
                                    break;
                                }
                                
                                t_arg_value = t_fetched_arg_value;
                            }
                            
                            if (pinfo[t_arg].type != nullptr &&
                                !MCTypeEvalAs(ctxt, pinfo[t_arg].type, t_arg_value))
                            {
                                t_error = EE_HANDLER_BADPARAM;
                                break;
                            }
                            
                            if (!MCArrayStoreValueAtIndex(*t_arg_seq,
                                                          t_arg_index,
                                                          *t_arg_value))
                            {
                                t_error = EE_NO_MEMORY;
                                break;
                            }
                            
                            t_arg_index += 1;
                            
                            if (t_trailing_arg == 0)
                            {
                                p_params = p_params->getnext();
                                
                                /* If the parameter list is empty, or it is trailing then move to
                                 * trailing */
                                if (p_params != nullptr && p_params->istrailing())
                                {
                                    p_params = nullptr;
                                    t_trailing_arg = 1;
                                }
                            }
                            else
                            {
                                t_trailing_arg += 1;
                                
                                if (MCArrayGetCount(*t_trailing) < t_trailing_arg)
                                {
                                    break;
                                }
                            }
                        }
                                
                        if (t_error == EE_UNDEFINED)
                        {
                            t_value.type = kMCExecValueTypeArrayRef;
                            t_value.arrayref_value = t_arg_seq.Take();
                        }
                    }
                    break;
                default:
                    MCUnreachableReturn(ES_ERROR);
                    break;
                }
                
                t_name = pinfo[t_arg].name;
            }
            else
            {
                /* Unnamed parameter */
                
                if (t_trailing_arg == 0)
                {
                    if (!p_params->eval_argument_ctxt(ctxt, t_value))
                    {
                        t_error = EE_HANDLER_BADPARAM;
                        break;
                    }
                }
                else
                {
                    MCValueRef t_trailing_value;
                    if (!MCArrayFetchValueAtIndex(*t_trailing, t_trailing_arg, t_trailing_value))
                    {
                        t_error = EE_HANDLER_BADTRAILING;
                        break;
                    }
                    
                    MCExecTypeSetValueRef(t_value, MCValueRetain(t_trailing_value));
                }
                
                t_name = kMCEmptyName;
            }
        }
        else
        {
            /* The parameter must be either optional or variadic. */
            switch(pinfo[t_arg].kind)
            {
            case kMCHandlerParamKindNormal:
            case kMCHandlerParamKindReference:
            case kMCHandlerParamKindCopy:
                /* If there is no default clause, then there are too few 
                 * arguments */
                if (pinfo[t_arg].default_value == nullptr)
                {
                    t_error = EE_HANDLER_TOOFEWARGS;
                    break;
                }
                    
                /* Otherwise, mark this argument as needing the default value. */
                t_take_default = true;
                /* In the optional case, the argument value is the default. */
                break;
            case kMCHandlerParamKindVariadic:
                /* In the variadic case, the argument value is the empty array. */
                t_value.type = kMCExecValueTypeArrayRef;
                t_value.valueref_value = MCValueRetain(kMCEmptyArray);
                break;
            default:
                MCUnreachableReturn(ES_ERROR);
                break;
            }
        }
        
        /* If there was an error, then stop processing */
        if (t_error != EE_UNDEFINED)
        {
            break;
        }
        
        /* Create the argument */
        if (t_reference == nullptr)
        {
            /* If the argument should take the default value, then do so */
            if (t_take_default)
            {
                MCAssert(pinfo[t_arg].default_value != nullptr);
                MCExecTypeSetValueRef(t_value, MCValueRetain(pinfo[t_arg].default_value));
                
                if (pinfo[t_arg].default_value_not_converted)
                {
                    if (!MCTypeEvalAs(ctxt, pinfo[t_arg].type, t_value))
                    {
                        t_error = EE_HANDLER_BADDEFAULTTYPE;
                        break;
                    }
                    
                    /* The computed value should probably be reassigned here
                     * to the pinfo array (technically default expressions must
                     * be deterministic - i.e. not based on state - rather than
                     * constant but we don't check quite that at the moment) */
                }
            }

            MCAutoPointer<MCVariable> t_arg_var;
            if (!MCVariable::createwithname(t_name, &t_arg_var))
            {
                t_error = EE_NO_MEMORY;
                break;
            }
            
            MCAutoPointer<MCContainer> t_arg_container =
                        new(nothrow) MCContainer(*t_arg_var);
            if (!t_arg_container)
            {
                t_error = EE_NO_MEMORY;
                break;
            }
            
            if (!t_arg_container->give_value(ctxt, t_value))
            {
                t_error = EE_NO_MEMORY;
                break;
            }
            
            t_arg_var.Release();
            
            t_args[t_arg] = t_arg_container.Release();
        }
        else
        {
            t_args[t_arg] = t_reference;
        }
        
        if (p_params != nullptr)
        {
            p_params = p_params->getnext();
            
            /* If the parameter list is empty, or it is trailing then move to
             * trailing */
            if (p_params != nullptr && p_params->istrailing())
            {
                p_params = nullptr;
                t_trailing_arg = 1;
            }
        }
        else if (t_trailing_arg != 0)
        {
            t_trailing_arg += 1;
        }
    }
    
    /* Handle any error and cleanup */
    if (t_error != EE_UNDEFINED)
    {
        if (t_arg < npnames)
        {
            MCeerror->add(t_error, firstline - 1, 1, pinfo[t_arg].name);
        }
        else
        {
            MCeerror->add(t_error, firstline - 1, 1, t_arg);
        }
            
        while(t_arg--)
        {
            if (t_arg >= npnames ||
                pinfo[t_arg].kind != kMCHandlerParamKindReference)
            {
                delete t_args[t_arg]->getvar();
                delete t_args[t_arg];
            }
        }
        delete[] t_args;
        
        return ES_ERROR;
    }
    
    /* Otherwise give the MCHandler the new param list. */
    params = t_args;
    nparams = t_arg_count;
    
    return ES_NORMAL;
}

/* Called on entry to a handler to handle arguments appropriately. */
Exec_stat MCHandler::enter(MCExecContext& ctxt, MCParameter *p_params)
{
    /* Compute the number of passed parameters - used by the params function */
    npassedparams = 0;
    for (MCParameter *tptr = p_params; tptr != NULL; tptr = tptr->getnext())
    {
        /* Trailing arguments on lax handlers are not currently supported. */
        if (tptr->istrailing())
        {
            MCeerror->add(EE_HANDLER_BADTRAILING, firstline - 1, 1, name);
            return ES_ERROR;
        }
        
        npassedparams++;
    }
    
    /* The number of new parameters is the max of the defined params and passed
     * params. */
    uint2 newnparams = MCU_max(npassedparams, npnames);
    
    /* Allocate an array of containers for the total number of needed parameters. */
    MCContainer **newparams;
    if (newnparams == 0)
        newparams = NULL;
    else
        newparams = new (nothrow) MCContainer *[newnparams];
    
    /* Process the parameter list into the containers appropriately. */
    Boolean err = False;
    for (uint2 i = 0 ; i < newnparams ; i++)
    {
        if (p_params != NULL)
        {
            if (i < npnames && pinfo[i].kind == kMCHandlerParamKindReference)
            {
                if ((newparams[i] = p_params->eval_argument_container()) == NULL)
                {
                    err = True;
                    break;
                }
            }
            else
            {
                MCExecValue t_value;
                if (!p_params->eval_argument_ctxt(ctxt, t_value))
                {
                    err = True;
                    break;
                }
                
                MCVariable *t_new_var;
                /* UNCHECKED */ MCVariable::createwithname(i < npnames ? pinfo[i] . name : kMCEmptyName, t_new_var);
                /* UNCHECKED */ newparams[i] = new(nothrow) MCContainer(t_new_var);
                
                newparams[i]->give_value(ctxt, t_value);
            }
            
            // AL-2014-11-04: [[ Bug 13902 ]] If 'it' was this parameter's name then create the MCVarref as a
            //  param type, with this handler and param index, so that use of the get command syncs up correctly.
            if (i < npnames && MCNameIsEqualToCaseless(pinfo[i] . name, MCN_it))
                m_it = new (nothrow) MCVarref(this, i, True);
            
            p_params = p_params->getnext();
        }
        else
        {
            if (i < npnames && pinfo[i].kind == kMCHandlerParamKindReference)
            {
                err = True;
                break;
            }
            MCVariable *t_new_var;
            /* UNCHECKED */ MCVariable::createwithname(i < npnames ? pinfo[i] . name : kMCEmptyName, t_new_var);
            /* UNCHECKED */ newparams[i] = new(nothrow) MCContainer(t_new_var);
        }
    }
    
    /* If an error occurred, clean up and return an error. */
    if (err)
    {
        while (newnparams--)
        {
            // AL-2014-09-16: [[ Bug 13454 ]] Delete created variables before deleting containers to prevent memory leak
            if (newnparams >= npnames || pinfo[newnparams].kind != kMCHandlerParamKindReference)
            {
                delete newparams[newnparams] -> getvar();
                delete newparams[newnparams];
            }
        }
        delete newparams;
        MCeerror->add(EE_HANDLER_BADPARAM, firstline - 1, 1, name);
        return ES_ERROR;
    }
    
    /* Otherwise give the MCHandler the new param list. */
    params = newparams;
    nparams = newnparams;
    
    return ES_NORMAL;
}

/* Called on exit of a handler to handle arguments appropriately. */
Exec_stat MCHandler::leave(MCExecContext& ctxt, Exec_stat p_exec_stat)
{
    if (params != NULL)
    {
        uint2 i = nparams;
        // AL-2014-08-20: [[ ArrayElementRefParams ]] A container is always created for each parameter,
        //  so delete them all when the handler has finished executing
        while (i--)
        {
            // AL-2014-09-16: [[ Bug 13454 ]] Delete created variables before deleting containers to prevent memory leak
            if (i >= npnames || pinfo[i].kind != kMCHandlerParamKindReference)
            {
                delete params[i] -> getvar();
                delete params[i];
            }
        }
        delete[] params; /* Allocated with new[] */
    }
    
    /* If the handler has a return type - perform an implicit as type. */
    if (return_type != nullptr)
    {
        MCExecValue t_return_value;
        MCresult->take_exec_value(t_return_value);
        if (!MCTypeEvalAs(ctxt, return_type, t_return_value))
        {
            MCExecTypeRelease(t_return_value);
            ctxt.LegacyThrow(EE_HANDLER_BADRETURNTYPE);
            p_exec_stat = ES_ERROR;
        }
        else
        {
            MCresult->give_value(ctxt, t_return_value);
        }
    }
    
    return p_exec_stat;
}

Exec_stat MCHandler::exec(MCExecContext& ctxt, MCParameter *plist)
{
    /* If this is a non-array property handler then skip past the (empty valued)
     * index parameter. */
    if (prop && !array && plist != NULL)
        plist = plist->getnext();
    
    /* Setup the arguments */
	MCContainer **oldparams = params;
	uint2 oldnparams = nparams;
    if (non_lax)
    {
        if (enter_non_lax(ctxt, plist) != ES_NORMAL)
        {
            return ES_ERROR;
        }
    }
    else if (enter(ctxt, plist) != ES_NORMAL)
    {
        return ES_ERROR;
    }
    
    /* Setup a new copy of the local variables. */
	uint2 oldnconstants = nconstants;
    MCVariable **oldvars = vars;
    uint2 oldnvnames = nvnames;
	if (nvnames == 0)
		vars = NULL;
	else
	{
		vars = new (nothrow) MCVariable *[nvnames];
		uint2 i = nvnames;
		while (i--)
		{
			/* UNCHECKED */ MCVariable::createwithname(vinfo[i] . name, vars[i]);
            
			// A UQL is indicated by 'init' being nil.
			if (vinfo[i] . init != nil)
				vars[i] -> setvalueref(vinfo[i] . init);
			else
			{
				// At the moment UQL detection relies on the fact that the 'name'
				// and 'value' of a variable share the same base ptr as well as 'is_uql'
				// being set.
				vars[i] -> setvalueref(vinfo[i] . name);
				vars[i] -> setuql();
			}
		}
	}
    
    /* Mark the handler as executing. */
	executing++;
    
    /* Reset the result to empty */
	ctxt . SetTheResultToEmpty();
    
    /* Execute the handler's body */
	Exec_stat stat = ES_NORMAL;
	MCStatement *tspr = statements;
	if ((MCtrace || MCnbreakpoints) && tspr != NULL)
	{
		MCB_trace(ctxt, firstline, 0);
        
		// OK-2008-09-05: [[Bug 7115]] - Debugger doesn't stop if traceAbort is set following a breakpoint on the first line of a handler.
		if (MCexitall)
			tspr = NULL;
	}
	while (tspr != NULL)
	{
		if (MCtrace || MCnbreakpoints)
		{
			MCB_trace(ctxt, tspr->getline(), tspr->getpos());
			if (MCexitall)
				break;
		}
		ctxt.SetLineAndPos(tspr->getline(), tspr->getpos());
        
        tspr->exec_ctxt(ctxt);
		stat = ctxt . GetExecStat();
        
        MCActionsRunAll();
        
		switch(stat)
		{
            case ES_NORMAL:
                if (MCexitall)
                    tspr = NULL;
                else
                    tspr = tspr->getnext();
                break;
            case ES_EXIT_REPEAT:
            case ES_NEXT_REPEAT:
            case ES_ERROR:
                if ((MCtrace || MCnbreakpoints) && !MCtrylock && !MClockerrors)
                    do
                    {
                        MCB_error(ctxt, tspr->getline(), tspr->getpos(), EE_HANDLER_BADSTATEMENT);
                        ctxt . IgnoreLastError();
                        tspr->exec_ctxt(ctxt);
                    }
				while (MCtrace && (stat = ctxt . GetExecStat()) != ES_NORMAL);
                if (stat != ES_NORMAL)
                {
                    MCeerror->add(EE_HANDLER_BADSTATEMENT, tspr->getline(), tspr->getpos(), name);
                    if (MCexitall)
                        stat = ES_NORMAL;
                    else
                        stat = ES_ERROR;
                    tspr = NULL;
                }
                else
                    tspr = tspr->getnext();
                break;
            case ES_RETURN_HANDLER:
                tspr = NULL;
                break;
            case ES_EXIT_HANDLER:
            case ES_EXIT_SWITCH:
                tspr = NULL;
                stat = ES_NORMAL;
                break;
            default:
                tspr = NULL;
                break;
		}
	}
    
	// MW-2007-07-03: [[ Bug 4570 ]] - Exiting a handler except via return should
	//   clear the result.
	// MW-2007-09-17: [[ Bug 4570 ]] - REVERTING due to backwards-compatibility
	//   problems.
	if (stat == ES_RETURN_HANDLER)
		stat = ES_NORMAL;
    
	if (!MCexitall && (MCtrace || MCnbreakpoints))
		MCB_trace(ctxt, lastline, 0);
    
    /* Mark the handler as not executing */
	executing--;
    
    /* Teardown the local copy of variables. */
    if (vars != NULL)
    {
        while (nvnames--)
        {
            if (nvnames >= oldnvnames)
            {
                MCValueRelease(vinfo[nvnames] . name);
                MCValueRelease(vinfo[nvnames] . init);
            }
            delete vars[nvnames];
        }
        delete[] vars; /* Allocated with new[] */
    }
    vars = oldvars;
    nvnames = oldnvnames;
    nconstants = oldnconstants;
    
    /* Set a static var so MCObject::timer can distinguish pass from not handled. */
    if (stat == ES_PASS)
        gotpass = True;
    
    /* Teardown the arguments - this might change 'stat' if there is an error
     * whilst processing the argument list. */
    stat = leave(ctxt, stat);
	params = oldparams;
	nparams = oldnparams;
    
	return stat;
}

MCVariable *MCHandler::getvar(uint2 index, Boolean isparam)
{
    return isparam ? nil : vars[index];
}

MCContainer *MCHandler::getcontainer(uint2 index, Boolean isparam)
{
    return isparam ? params[index] : nil;
}

integer_t MCHandler::getnparams(void)
{
	return npassedparams;
}

// MW-2007-07-03: [[ Bug 3174 ]] - Non-declared parameters accessed via 'param' should
//   be considered to be both empty and 0 as they are akin to undeclared variables.
MCValueRef MCHandler::getparam(uindex_t p_index)
{
    if (p_index == 0)
        return name;
    else if (p_index > nparams)
        return kMCEmptyString;
    else
        return params[p_index - 1]->get_valueref();
}

bool MCHandler::getparamreadonly(uindex_t p_index)
{
    if (p_index >= npnames)
        return false;
    return pinfo[p_index].type != nullptr;
}

// MW-2013-11-08: [[ RefactorIt ]] Changed to return the 'm_it' varref we always have now.
MCVarref *MCHandler::getit(void)
{
	return m_it;
}

Parse_stat MCHandler::findvar(MCNameRef p_name, MCVarref **dptr)
{
	uint2 i;
	for (i = 0 ; i < nvnames ; i++)
		if (MCNameIsEqualToCaseless(p_name, vinfo[i] . name))
		{
			*dptr = new (nothrow) MCVarref(this, i, False);
			return PS_NORMAL;
		}

	for (i = 0 ; i < npnames ; i++)
		if (MCNameIsEqualToCaseless(p_name, pinfo[i] . name))
	{
			*dptr = new (nothrow) MCVarref(this, i, True);
			return PS_NORMAL;
		}

	for (i = 0 ; i < nglobals ; i++)
	{
		if (globals[i]->hasname(p_name))
		{
			*dptr = globals[i] -> newvarref();
			return PS_NORMAL;
		}
	}

	if (MCStringGetNativeCharAtIndex(MCNameGetString(p_name), 0) == '$')
	{
		MCVariable *t_global;
		/* UNCHECKED */ MCVariable::ensureglobal(p_name, t_global);
		*dptr = t_global -> newvarref();
		return PS_NORMAL;
	}

	// MW-2011-08-23: [[ UQL ]] Now search in the handler list but ignore any
    //   UQL vars that are there.
	return hlist->findvar(p_name, true, dptr);
}

Parse_stat MCHandler::newvar(MCNameRef p_name, MCValueRef p_init, MCVarref **r_ref)
{
	MCU_realloc((char **)&vinfo, nvnames, nvnames + 1, sizeof(MCHandlerVarInfo));
    vinfo[nvnames] . name = MCValueRetain(p_name);
	if (p_init != nil)
		/* UNCHECKED */ vinfo[nvnames] . init = MCValueRetain(p_init);
	else
		vinfo[nvnames] . init = nil;

	if (executing)
	{
		MCU_realloc((char **)&vars, nvnames, nvnames + 1, sizeof(MCVariable *));
		/* UNCHECKED */ MCVariable::createwithname(p_name, vars[nvnames]);

		if (p_init != nil)
			vars[nvnames] -> setvalueref(p_init);
		else
		{
			vars[nvnames] -> setvalueref(p_name);
			vars[nvnames] -> setuql();
		}
	}

	*r_ref = new (nothrow) MCVarref(this, nvnames++, False);

	return PS_NORMAL;
}

Parse_stat MCHandler::findconstant(MCNameRef p_name, MCExpression **dptr)
{
	uint2 i;
	for (i = 0 ; i < nconstants ; i++)
		if (MCNameIsEqualToCaseless(p_name, cinfo[i].name))
		{
			*dptr = new (nothrow) MCLiteral(cinfo[i].value);
			return PS_NORMAL;
		}
	return hlist->findconstant(p_name, dptr);
}

Parse_stat MCHandler::newconstant(MCNameRef p_name, MCValueRef p_value)
{
	MCU_realloc((char **)&cinfo, nconstants, nconstants + 1, sizeof(MCHandlerConstantInfo));
    cinfo[nconstants].name = MCValueRetain(p_name);
	cinfo[nconstants++].value = MCValueRetain(p_value);
	return PS_NORMAL;
}

bool MCHandler::getconstantnames_as_properlist(MCProperListRef& r_list)
{
    MCAutoProperListRef t_list;
    if (!MCProperListCreateMutable(&t_list))
        return false;
    
    for (uinteger_t i = 0; i < nconstants; i++)
        if (!MCProperListPushElementOntoBack(*t_list, cinfo[i].name))
            return false;
    
    if (!t_list.MakeImmutable())
    {
        return false;
    }
    
    r_list = t_list.Take();
    
    return true;
}


void MCHandler::newglobal(MCNameRef p_name)
{
	uint2 i;
	for (i = 0 ; i < nglobals ; i++)
		if (globals[i]->hasname(p_name))
			return;

	MCVariable *gptr;
	/* UNCHECKED */ MCVariable::ensureglobal(p_name, gptr);

	MCU_realloc((char **)&globals, nglobals, nglobals + 1, sizeof(MCVariable *));
	globals[nglobals++] = gptr;
}

bool MCHandler::getparamnames(MCListRef& r_list)
{
	MCAutoListRef t_list;
	if (!MCListCreateMutable(',', &t_list))
		return false;

	for (uinteger_t i = 0; i < npnames; i++)
		if (!MCListAppend(*t_list, pinfo[i].name))
			return false;

	return MCListCopy(*t_list, r_list);
}

bool MCHandler::getparamnames_as_properlist(MCProperListRef& r_list)
{
	MCAutoProperListRef t_list;
	if (!MCProperListCreateMutable(&t_list))
		return false;
	
	for (uinteger_t i = 0; i < npnames; i++)
		if (!MCProperListPushElementOntoBack(*t_list, pinfo[i].name))
			return false;
	
	if (!t_list.MakeImmutable())
	{
		return false;
	}
	
	r_list = t_list.Take();
	
	return true;
}

bool MCHandler::getvariablenames(MCListRef& r_list)
{
	MCAutoListRef t_list;
	if (!MCListCreateMutable(',', &t_list))
		return false;

	for (uinteger_t i = 0; i < nvnames; i++)
		if (!MCListAppend(*t_list, vinfo[i].name))
			return false;

	return MCListCopy(*t_list, r_list);
}

bool MCHandler::getvariablenames_as_properlist(MCProperListRef& r_list)
{
    MCAutoProperListRef t_list;
    if (!MCProperListCreateMutable(&t_list))
        return false;
    
    for (uinteger_t i = 0; i < nvnames; i++)
        if (!MCProperListPushElementOntoBack(*t_list, vinfo[i].name))
            return false;
    
    if (!t_list.MakeImmutable())
    {
        return false;
    }
    
    r_list = t_list.Take();
    
    return true;
}

bool MCHandler::getglobalnames(MCListRef& r_list)
{
	MCAutoListRef t_list;
	if (!MCListCreateMutable(',', &t_list))
		return false;

	// OK-2008-06-25: <Bug where the variableNames property would return duplicate global names>
	for (uint2 i = 0 ; i < nglobals ; i++)
		if (!MCListAppend(*t_list, globals[i]->getname()))
			return false;

	for (uint2 i = 0; i < hlist -> getnglobals(); i++)
	{
		MCNameRef t_global_name = hlist->getglobal(i)->getname();
		bool t_already_appended;
		t_already_appended = false;
		for (uint2 j = 0; (!t_already_appended) && j < nglobals; j++)
			t_already_appended = globals[j] -> hasname(t_global_name);

		if (!t_already_appended)
			if (!MCListAppend(*t_list, t_global_name))
				return false;
	}

	return MCListCopy(*t_list, r_list);
}

bool MCHandler::getglobalnames_as_properlist(MCProperListRef& r_list)
{
    MCAutoProperListRef t_list;
    if (!MCProperListCreateMutable(&t_list))
        return false;
    
    for (uinteger_t i = 0; i < nglobals; i++)
        if (!MCProperListPushElementOntoBack(*t_list, globals[i]->getname()))
            return false;
    
    if (!t_list.MakeImmutable())
    {
        return false;
    }
    
    r_list = t_list.Take();
    
    return true;
}


bool MCHandler::getvarnames(bool p_all, MCListRef& r_list)
{
	MCAutoListRef t_list;
	if (!MCListCreateMutable('\n', &t_list))
		return false;

	MCAutoListRef t_param_list, t_variable_list, t_script_variable_list;
	if (!(getparamnames(&t_param_list) &&
		MCListAppend(*t_list, *t_param_list)))
		return false;

	if (!(getvariablenames(&t_variable_list) &&
		MCListAppend(*t_list, *t_variable_list)))
		return false;

	if (!(hlist->getlocalnames(&t_script_variable_list) &&
		MCListAppend(*t_list, *t_script_variable_list)))
		return false;

	if (p_all)
	{
		MCAutoListRef t_global_list;
		if (!(getglobalnames(&t_global_list) &&
			MCListAppend(*t_list, *t_global_list)))
			return false;
	}

	return MCListCopy(*t_list, r_list);
}

uint4 MCHandler::linecount()
{
	uint4 count = 0;
	MCStatement *stmp = statements;
	while (stmp != NULL)
	{
		count += stmp->linecount();
		stmp = stmp->getnext();
	}
	return count;
}

/* The signature of a handler is an array with the following keys:
 *   - kind: one of
 *       event
 *       command / function
 *       get / set
 *       before / after
 *       is type / as type
 *       boolean / number / string / data / array
 *   - name: the name of the handler
 *   - scope: public or private
 *   - parameters: a sequence of parameter arrays
 *   - type: the (function) return type
 * 
 * A parameter array has keys:
 *   - kind: one of normal, reference, strict in, strict out, variadic
 *   - name: the name of the parameter, maybe empty if it is an unnamed variadic
 *   - type: (only present if typed) the type of the handler
 *   - default: (only present if optional) the default value
 */
bool MCHandler::copysignature(MCArrayRef& r_signature)
{
    static const char *kHandlerKindNames[] =
    {
        "",
        "command",
        "function",
        "get",
        "set",
        "before",
        "after"
    };
    
    static const char *kOperatorKindNames[] =
    {
        "is type",
        "as type",
        "boolean",
        "number",
        "string",
        "data",
        "array"
    };
    
    const char *t_kind;
    if (is_on)
        t_kind = "event";
    else if (type == HT_OPERATOR)
        t_kind = kOperatorKindNames[operator_kind];
    else
        t_kind = kHandlerKindNames[type];
    
    const char *t_scope;
    t_scope = is_private ? "private" : "public";
    
    MCAutoArrayRef t_parameters;
    if (!MCArrayCreateMutable(&t_parameters))
    {
        return false;
    }
    
    for(uindex_t i = 0; i < npnames; i++)
    {
        static const char *kParameterKindNames[] =
        {
            "normal",
            "reference",
            "strict in",
            "variadic"
        };
        
        MCAutoArrayRef t_parameter;
        if (!MCArrayCreateMutable(&t_parameter) ||
            !MCArrayStoreValue(*t_parameter, true, MCNAME("kind"), MCNAME(kParameterKindNames[pinfo[i].kind])) ||
            !MCArrayStoreValue(*t_parameter, true, MCNAME("name"), pinfo[i].name) ||
            (pinfo[i].type != nullptr &&
                !MCArrayStoreValue(*t_parameter, true, MCNAME("type"), MCTypeGetName(pinfo[i].type))) ||
            (pinfo[i].default_value != nullptr &&
                !MCArrayStoreValue(*t_parameter, true, MCNAME("default"), pinfo[i].default_value)) ||
            !t_parameter.MakeImmutable() ||
            !MCArrayStoreValueAtIndex(*t_parameters, i + 1, *t_parameter))
        {
            return false;
        }
    }
    
    if (unnamed_variadic)
    {
        MCAutoArrayRef t_parameter;
        if (!MCArrayCreateMutable(&t_parameter) ||
            !MCArrayStoreValue(*t_parameter, true, MCNAME("kind"), MCNAME("variadic")) ||
            !t_parameter.MakeImmutable() ||
            !MCArrayStoreValueAtIndex(*t_parameters, npnames + 1, *t_parameter))
        {
            return false;
        }
    }
    
    MCAutoArrayRef t_signature;
    if (!MCArrayCreateMutable(&t_signature) ||
        !MCArrayStoreValue(*t_signature, true, MCNAME("kind"), MCNAME(t_kind)) ||
        !MCArrayStoreValue(*t_signature, true, MCNAME("name"), name) ||
        !MCArrayStoreValue(*t_signature, true, MCNAME("scope"), MCNAME(t_scope)) ||
        !t_parameters.MakeImmutable() ||
        !MCArrayStoreValue(*t_signature, true, MCNAME("parameters"), *t_parameters) ||
        (return_type != nullptr &&
            !MCArrayStoreValue(*t_signature, true, MCNAME("type"), MCTypeGetName(return_type))) ||
        !t_signature.MakeImmutable())
    {
        return false;
    }
    
    r_signature = t_signature.Take();
    
    return true;
}
    
////////////////////////////////////////////////////////////////////////////////
