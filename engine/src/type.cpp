#include "prefix.h"

#include "globdefs.h"
#include "parsedef.h"
#include "filedefs.h"
#include "mcio.h"

#include "uidc.h"
#include "scriptpt.h"

#include "chunk.h"
#include "type.h"
#include "mcerror.h"
#include "param.h"
#include "handler.h"
#include "hndlrlst.h"

#include "globals.h"
#include "exec.h"

/* TYPE CLASS */

/* MCType holds all information about a type.
 *
 * Types are identified by a name - called the typename.
 *
 * Concrete types are those which can be used as a tag on a value. Only strings,
 * numbers, arrays and data values may be tagged with a type. There is currently
 * a limit of 256 tags, and thus 256 concrete types. Tags for concrete types
 * are allocated whilst parsing the 'with type' operator, when the typename
 * appears on the right hand side.
 *
 * If a typename is never used in the 'with type' operator, it represents a
 * virtual type. Such types can be used to represent things such as type unions,
 * or type subsets.
 *
 */

class MCType
{
public:
    MCType(MCNameRef p_name)
      : m_name(p_name)
    {
    }
    
    bool IsConcrete(void) const
    {
        return m_tag != 0;
    }
    
    uindex_t GetTag(void) const
    {
        return m_tag;
    }
    
    MCNameRef GetName(void) const
    {
        return *m_name;
    }
    
    /* Check whether the given value is a member of the type. This operation
     * takes the MCExecValue. */
    bool Is(MCExecContext& ctxt, MCExecValue& d_value)
    {
        bool t_is;
        if (MCExecTypeIsValueRef(d_value.type) &&
            IsConcrete() &&
            MCValueGetTag(d_value.valueref_value) == GetTag())
        {
            /* If the type is already a valueref, this type is concrete and
             * the type already has the concrete tag, it is of this type. */
            t_is = true;
            
            MCExecTypeRelease(d_value);
        }
        else
        {
            MCHandler *t_operator = m_operators[kMCTypeOperatorKindIsType];
            if (t_operator != nullptr)
            {
                /* Is operators are executed in the context of the calling script's
                 * object. */
                MCExecContext t_inner_ctxt(ctxt.GetObject(), t_operator->gethlist(), t_operator);
                
                MCParameter t_param;
                t_param.give_exec_argument(d_value);
                if (t_operator->exec(t_inner_ctxt, &t_param) != ES_NORMAL)
                {
                    return false;
                }
                
                MCresult->take_exec_value(d_value);
                MCExecTypeConvertAndReleaseAlways(t_inner_ctxt, d_value.type, &d_value, kMCExecValueTypeBool, &t_is);
                if (ctxt.HasError())
                {
                    MCeerror->add(EE_TYPEOP_BADRETURNTYPE, t_operator->getstartline() - 1, 1);
                    
                    MCAutoStringRef t_id;
                    t_operator->gethlist()->getparent()->getstringprop(ctxt, 0, P_LONG_ID, False, &t_id);
                    MCeerror->add(EE_OBJECT_NAME, 0, 0, *t_id);
                    
                    return false;
                }
            }
            else
            {
                t_is = false;
            }
        }
                
        return t_is;
    }
    
    /* Attempt to convert the value to this type. This operation mutates
     * the value inplace. */
    bool As(MCExecContext& ctxt, MCExecValue& x_value)
    {
        /* If the value is already a valueref, this is a concrete type and
         * the tag of the valueref matches the tag of this type, then it
         * is already of the right type. */
        if (MCExecTypeIsValueRef(x_value.type) &&
            IsConcrete() &&
            MCValueGetTag(x_value.valueref_value) == GetTag())
        {
            return true;
        }
        
        MCHandler *t_operator = m_operators[kMCTypeOperatorKindAsType];
        
        if (t_operator != nullptr)
        {
            /* As operators are executed in the context of the calling script's
             * object. */
            MCExecContext t_inner_ctxt(ctxt.GetObject(), t_operator->gethlist(), t_operator);
            
            MCParameter t_param;
            t_param.give_exec_argument(x_value);
            if (t_operator->exec(t_inner_ctxt, &t_param) == ES_NORMAL)
            {
                MCresult->take_exec_value(x_value);
                return true;
            }
            
            t_param.take_exec_argument(x_value);
        }
        
        MCeerror->add(EE_TYPEERR_ERROR, 0, 0, *m_name);

        return false;
    }
    
    /** CONCRETE **/
    
    bool To(MCExecContext& ctxt, MCTypeOperatorKind p_kind, MCValueRef p_value, MCValueRef& r_value)
    {
        MCHandler *t_operator = m_operators[p_kind];
        
        if (t_operator != nullptr)
        {
            /* To operators are executed in the context of the calling script's
             * object. */
            MCExecContext t_inner_ctxt(ctxt.GetObject(), t_operator->gethlist(), t_operator);
            
            MCParameter t_param;
            t_param.setvalueref_argument(p_value);
            if (t_operator->exec(t_inner_ctxt, &t_param) != ES_NORMAL)
            {
                return false;
            }
            
            MCAutoValueRef t_out_value;
            if (!MCresult->copyasvalueref(&t_out_value))
            {
                return false;
            }
            
            bool t_valid;
            switch(MCValueGetTypeCode(*t_out_value))
            {
            case kMCValueTypeCodeBoolean:
                t_valid = p_kind == kMCTypeOperatorKindBoolean;
                break;
            case kMCValueTypeCodeName:
            case kMCValueTypeCodeString:
                t_valid = p_kind == kMCTypeOperatorKindString;
                break;
            case kMCValueTypeCodeNumber:
                t_valid = p_kind == kMCTypeOperatorKindNumber;
                break;
            case kMCValueTypeCodeData:
                t_valid = p_kind == kMCTypeOperatorKindData;
                break;
            case kMCValueTypeCodeArray:
                t_valid = p_kind == kMCTypeOperatorKindArray;
                break;
            default:
                t_valid = false;
                break;
            }
            
            if (!t_valid)
            {
                MCeerror->add(EE_TYPEOP_BADRETURNTYPE, t_operator->getstartline() - 1, 1);
                
                MCAutoStringRef t_id;
                t_operator->gethlist()->getparent()->getstringprop(ctxt, 0, P_LONG_ID, False, &t_id);
                MCeerror->add(EE_OBJECT_NAME, 0, 0, *t_id);
                
                return false;
            }
            
            r_value = t_out_value.Take();
            
            return true;
        }
        
        return false;
    }
    
    /* Check whether the given value is this type. This operation takes the
     * MCExecValue. */
    bool Has(MCExecValue& d_value)
    {
        bool t_has;
        
        /* Only valuerefs can have a concrete type. */
        if (MCExecTypeIsValueRef(d_value.type))
        {
            /* The valueref is of this type if this type is concrete and the
             * tag of the value matches. */
            t_has = IsConcrete() &&
                MCValueGetTag(d_value.valueref_value) == GetTag();
        }
        else
        {
            t_has = false;
        }
        
        MCExecTypeRelease(d_value);
        
        return t_has;
    }
    
    bool MakeConcrete(MCScriptPoint& sp)
    {
        if (m_tag != 0)
        {
            return true;
        }
        
        uindex_t t_tag = MCProperListGetLength(s_concrete_types);
        if (t_tag == 255)
        {
            MCperror->add(PE_TYPEOP_NOTAGSLEFT, sp);
            return PS_ERROR;
        }

        MCNumberRef t_type_ptr;
        MCNumberCreateWithAlignedPointer(this, t_type_ptr);
        if (!MCProperListPushElementOntoBack(s_concrete_types, t_type_ptr))
        {
            MCperror->add(PE_OUTOFMEMORY, sp);
            return PS_ERROR;
        }
        
        m_tag = t_tag + 1;
        
        return true;
    }
    
    bool DefineOperator(MCScriptPoint& sp, MCTypeOperatorKind p_kind, MCHandler *p_handler)
    {
        if (m_operators[p_kind] != nullptr)
        {
            MCperror->add(PE_TYPEOP_ALREADYDEFINED, sp);
            return false;
        }
        
        m_operators[p_kind] = p_handler;
        
        return true;
    }
    
    void UndefineOperator(MCTypeOperatorKind p_kind)
    {
        m_operators[p_kind] = nullptr;
    }

    static bool Initialize(void)
    {
        if (!MCArrayCreateMutable(s_types))
        {
            return false;
        }
        
        if (!MCProperListCreateMutable(s_concrete_types))
        {
            return false;
        }
        
        return true;
    }
    
    static void Finalize(void)
    {
        MCNameRef t_key;
        MCValueRef t_value;
        uintptr_t t_types_iterator = 0;
        while(MCArrayIterate(s_types, t_types_iterator, t_key, t_value))
        {
            MCType *t_type = (MCType*)MCNumberFetchAsAlignedPointer((MCNumberRef)t_value);
            delete t_type;
        }
        MCValueRelease(s_types);
        MCValueRelease(s_concrete_types);
    }
    
    static bool Declare(MCScriptPoint& sp, MCNameRef p_name, MCType*& r_type)
    {
        MCType *t_type = Lookup(p_name);
        if (t_type == nullptr)
        {
            MCAutoPointer<MCType> t_new_type = new (nothrow) MCType(p_name);
            
            MCNumberRef t_type_as_number;
            if (*t_new_type == nullptr ||
                !MCNumberCreateWithAlignedPointer(*t_new_type, t_type_as_number) ||
                !MCArrayStoreValue(s_types, false, p_name, t_type_as_number))
            {
                MCperror->add(PE_OUTOFMEMORY, sp);
                return PS_ERROR;
                
            }
            
            t_type = t_new_type.Release();
        }
        
        r_type = t_type;
        
        return true;
    }

    static MCType *Lookup(MCNameRef p_name)
    {
        MCNumberRef t_type;
        if (!MCArrayFetchValue(s_types, false, p_name, (MCValueRef&)t_type))
        {
            return nullptr;
        }
        return (MCType *)MCNumberFetchAsAlignedPointer(t_type);
    }
    
    static MCType *ConcreteLookup(uindex_t p_tag)
    {
        MCAssert(p_tag <= MCProperListGetLength(s_concrete_types));
        
        if (p_tag == 0)
        {
            return nullptr;
        }
        
        MCNumberRef t_type = (MCNumberRef)MCProperListFetchElementAtIndex(s_concrete_types, p_tag - 1);
        
        return (MCType *)MCNumberFetchAsAlignedPointer(t_type);
    }
    
private:
    MCNewAutoNameRef m_name;
    uindex_t m_tag = 0;
    
    MCHandler *m_operators[kMCTypeOperatorKind_Count] = {};
    
    /* A mapping from typename to MCType* */
    static MCArrayRef s_types;
    /* A mapping from concrete type tag to MCType* */
    static MCProperListRef s_concrete_types;
};

/* GLOBAL STATE */

MCArrayRef MCType::s_types = nullptr;
MCProperListRef MCType::s_concrete_types = nullptr;

/* 'X _as_type_ T' - attempts to convert a value to the specified type
 * 'Y _is_type_ T' - returns true if tY can be converted to a value of type T
 * 'X _has_type_ T' - returns true if the current type of X is T
 * 'X _with_type_ T' - returns X, but tagged with type T
 * '_type_of_(X)' - return ths current type of X
 * '_strict_type_of_(X)' - returns the storage/strict type of X
 * '_type_error_()' - ???
 */

/* TYPE OPERATOR BASE CLASS */

MCExpressionAttrs MCTypeOperator::getattrs(void) const
{
    return right->getattrs();
}

Parse_stat MCTypeOperator::parse(MCScriptPoint& sp, Boolean the)
{
    initpoint(sp);
    
    Symbol_type type;
    if (sp.next(type) != PS_NORMAL)
    {
        MCperror->add(PE_TYPEOP_NONAME, sp);
        return PS_ERROR;
    }
    
    if (type != ST_ID)
    {
        MCperror->add(PE_TYPEOP_BADNAME, sp);
        return PS_ERROR;
    }
    
    if (!MCType::Declare(sp, sp.gettoken_nameref(), m_type))
    {
        return PS_ERROR;
    }
    
    return PS_BREAK;
}

/* TYPE FUNCTION BASE CLASS */

MCExpressionAttrs MCTypeFunction::getattrs(void) const
{
    return (*m_operand)->getattrs();
}

Parse_stat MCTypeFunction::parse(MCScriptPoint& sp, Boolean the)
{
    if (get1param(sp, &(&m_operand), False) != PS_NORMAL)
    {
        MCperror->add(PE_TYPEFUNC_BADPARAM, sp);
        return PS_ERROR;
    }
    return PS_NORMAL;
}

/* TYPE ERROR FUNCTION */

MCExpressionAttrs MCTypeError::getattrs(void) const
{
    /* The type error function isn't constant as it has side-effects - notably
     * throwing an error (in the future it could be changed to return a unique
     * 'type error' value which the type conversion logic would pick up. */
    return {};
}

Parse_stat MCTypeError::parse(MCScriptPoint& sp, Boolean the)
{
    if (get0or1param(sp, &(&m_hint), False) != PS_NORMAL)
    {
        MCperror->add(PE_TYPEFUNC_BADPARAM, sp);
        return PS_ERROR;
    }
    return PS_NORMAL;
}

void MCTypeError::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value)
{
    MCAutoStringRef t_hint;
    if (*m_hint != nullptr)
    {
        if (!ctxt.EvalExprAsStringRef(*m_hint, EE_TYPEERR_BADHINT, &t_hint))
        {
            return;
        }
    }
    else
    {
        t_hint = kMCEmptyString;
    }
    
    /* We don't use the MCExecContext Throw here as we want position info */
    MCeerror->add(EE_TYPEERR_ERROR, line, pos, *t_hint);
    ctxt.Throw();
}

/* AS TYPE OPERATOR */

MCExpressionAttrs MCAsType::getattrs(void) const
{
    /* TODO: _as_type_ is only constant if the as operator for the type is -
     * however we need them to evaluate for constants so we assume static
     * eval for now. */
    return right->getattrs();
}

void MCAsType::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCExecValue t_value;
    if (!ctxt.EvaluateExpression(right, EE_TYPEFUNC_BADOPERAND, r_exec_value))
    {
        return;
    }
    
    if (!m_type->As(ctxt, r_exec_value))
    {
        ctxt.Throw();
        MCExecTypeRelease(r_exec_value);
    }
}

/* IS TYPE OPERATOR */

MCExpressionAttrs MCIsType::getattrs(void) const
{
    /* _is_type_ is only constant if the as operator for the type is - we can't
     * detect that yet! */
    return {};
}

void MCIsType::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCExecValue t_value;
    if (!ctxt.EvaluateExpression(right, EE_TYPEFUNC_BADOPERAND, t_value))
    {
        return;
    }
    
    r_exec_value.type = kMCExecValueTypeBool;
    r_exec_value.bool_value = m_type->Is(ctxt, t_value);
}

/* WITH TYPE OPERATOR */

Parse_stat MCWithType::parse(MCScriptPoint& sp, Boolean the)
{
    Parse_stat t_stat = MCTypeOperator::parse(sp, the);
    if (t_stat == PS_ERROR)
    {
        return PS_ERROR;
    }
    
    if (!m_type->MakeConcrete(sp))
        return PS_ERROR;
    
    return t_stat;
}
    
void MCWithType::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCValueRef t_value;
    if (!ctxt.EvalExprAsValueRef(right, EE_TYPEOP_BADOPERAND, t_value))
    {
        return;
    }
    
    if (!MCValueCopyWithTagAndRelease(t_value, m_type->GetTag(), t_value))
    {
        MCValueRelease(t_value);
        
        /* We don't use the MCExecContext Throw here as we want position info */
        MCeerror->add(EE_WITHTYPE_CANNOTTAG, line, pos);
        ctxt.Throw();
        return;
    }
    
    MCExecTypeSetValueRef(r_exec_value, t_value);
}

/* HAS TYPE OPERATOR */

void MCHasType::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCExecValue t_value;
    if (!ctxt.EvaluateExpression(right, EE_TYPEFUNC_BADOPERAND, t_value))
    {
        return;
    }
        
    r_exec_value.type = kMCExecValueTypeBool;
    r_exec_value.bool_value = m_type->Has(t_value);
}

/* TYPE OF OPERATOR */

void MCTypeOf::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCExecValue t_value;
    if (!ctxt.EvaluateExpression(*m_operand, EE_TYPEFUNC_BADOPERAND, t_value))
    {
        return;
    }
    
    uindex_t t_tag;
    if (MCExecTypeIsValueRef(t_value.type))
    {
        t_tag = MCValueGetTag(t_value.valueref_value);
    }
    else
    {
        t_tag = 0;
    }
    
    MCExecTypeRelease(t_value);
    
    MCType *t_type = MCType::ConcreteLookup(t_tag);
    r_exec_value.type = kMCExecValueTypeNameRef;
    r_exec_value.nameref_value = MCValueRetain(t_type != nullptr ? t_type->GetName() : kMCEmptyName);
}

/* STRICT TYPE OF FUNCTION */

/* Returns the strict (storage) type of the operand.
 *
 * Currently this returns one of:
 *   - nothing
 *   - boolean
 *   - number
 *   - string
 *   - data
 *   - array
 *   - unknown
 *
 * Which correspond to the 'script visible' types. However, this list
 * may change in the future providing greater fidelity. */
void MCStrictTypeOf::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCExecValue t_value;
    if (!ctxt.EvaluateExpression(*m_operand, EE_TYPEFUNC_BADOPERAND, t_value))
    {
        return;
    }
    
    if (t_value.type == kMCExecValueTypeValueRef)
    {
        switch(MCValueGetTypeCode(t_value.valueref_value))
        {
        case kMCValueTypeCodeNull:
            t_value.type = kMCExecValueTypeNone;
            break;
        case kMCValueTypeCodeBoolean:
            t_value.type = kMCExecValueTypeBooleanRef;
            break;
        case kMCValueTypeCodeNumber:
            t_value.type = kMCExecValueTypeNumberRef;
            break;
        case kMCValueTypeCodeName:
            t_value.type = kMCExecValueTypeNameRef;
            break;
        case kMCValueTypeCodeString:
            t_value.type = kMCExecValueTypeStringRef;
            break;
        case kMCValueTypeCodeData:
            t_value.type = kMCExecValueTypeDataRef;
            break;
        case kMCValueTypeCodeArray:
            t_value.type = kMCExecValueTypeArrayRef;
            break;
        case kMCValueTypeCodeList:
        case kMCValueTypeCodeSet:
        case kMCValueTypeCodeProperList:
        case kMCValueTypeCodeCustom:
        case kMCValueTypeCodeRecord:
        case kMCValueTypeCodeHandler:
        case kMCValueTypeCodeTypeInfo:
        case kMCValueTypeCodeError:
        case kMCValueTypeCodeForeignValue:
            t_value.type = kMCExecValueTypeValueRef;
            break;
        }
    }

    MCNameRef t_name;
    switch(t_value.type)
    {
    case kMCExecValueTypeNone:
        t_name = MCN_nothing;
        break;
    case kMCExecValueTypeValueRef:
        t_name = MCN_unknown;
        break;
    case kMCExecValueTypeBooleanRef:
    case kMCExecValueTypeBool:
        t_name = MCN_boolean;
        break;
    case kMCExecValueTypeStringRef:
    case kMCExecValueTypeNameRef:
    case kMCExecValueTypeChar:
    case kMCExecValueTypePoint:
    case kMCExecValueTypeColor:
    case kMCExecValueTypeRectangle:
        t_name = MCN_string;
        break;
    case kMCExecValueTypeDataRef:
        t_name = MCN_data;
        break;
    case kMCExecValueTypeArrayRef:
        t_name = MCN_array;
        break;
    case kMCExecValueTypeNumberRef:
    case kMCExecValueTypeUInt:
    case kMCExecValueTypeInt:
    case kMCExecValueTypeDouble:
    case kMCExecValueTypeFloat:
        t_name = MCN_number;
        break;
    }
    
    MCExecTypeRelease(t_value);
    
    r_exec_value.type = kMCExecValueTypeNameRef;
    r_exec_value.nameref_value = MCValueRetain(t_name);
}

/* AS STRICT TYPE OPERATOR */

MCExpressionAttrs MCAsStrictType::getattrs(void) const
{
    return right->getattrs();
}

Parse_stat MCAsStrictType::parse(MCScriptPoint& sp, Boolean the)
{
    initpoint(sp);

    return PS_BREAK;
}

void MCAsStrictType::eval_ctxt(MCExecContext& ctxt, MCExecValue& r_exec_value)
{
    MCExecValue t_value;
    if (!ctxt.EvaluateExpression(right, EE_TYPEFUNC_BADOPERAND, t_value))
    {
        return;
    }
    
    r_exec_value.type = m_type;
    MCExecTypeConvertAndReleaseAlways(ctxt, t_value.type, &t_value, m_type, &r_exec_value);
}

/* GLOBAL METHODS */

MCNameRef MCTypeGetName(MCType *p_type)
{
    return p_type->GetName();
}

bool MCTypeEvalAs(MCExecContext& ctxt, MCType *p_type, MCExecValue& x_value)
{
    return p_type->As(ctxt, x_value);
}

bool MCTypeEvalAs(MCExecContext& ctxt, MCType *p_type, MCAutoValueRef& x_value)
{
    MCExecValue t_value;
    MCExecTypeSetValueRef(t_value, x_value.Take());
    
    bool t_error = p_type->As(ctxt, t_value);
    
    x_value.Give(t_value.valueref_value);
    
    return t_error;
}

bool MCTypeEvalTo(MCExecContext& ctxt, uindex_t p_tag, MCTypeOperatorKind p_kind, MCValueRef p_value, MCValueRef& r_value)
{
    return MCType::ConcreteLookup(p_tag)->To(ctxt, p_kind, p_value, r_value);
}

bool MCTypeDeclare(MCScriptPoint& sp, MCNameRef p_typename, MCType*& r_type)
{
    return MCType::Declare(sp, p_typename, r_type);
}

bool MCTypeDefineOperator(MCScriptPoint& sp, MCNameRef p_typename, MCTypeOperatorKind p_kind, MCHandler *p_handler)
{
    MCType *t_type;
    if (!MCType::Declare(sp, p_typename, t_type))
    {
        return false;
    }
    
    return t_type->DefineOperator(sp, p_kind, p_handler);
}

void MCTypeUndefineOperator(MCNameRef p_name, MCTypeOperatorKind p_kind)
{
    MCType *t_type = MCType::Lookup(p_name);
    if (t_type != nullptr)
    {
        t_type->UndefineOperator(p_kind);
    }
}

bool MCTypeInitialize(void)
{
    return MCType::Initialize();
}

void MCTypeFinalize(void)
{
    MCType::Finalize();
}
