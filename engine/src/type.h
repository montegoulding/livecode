#ifndef	TYPE_H
#define	TYPE_H

#include "exec.h"
#include "executionerrors.h"

#include "express.h"

/* TYPE CLASS */

class MCHandler;
class MCType;

enum MCTypeOperatorKind
{
    kMCTypeOperatorKindIsType,
    kMCTypeOperatorKindAsType,
    kMCTypeOperatorKindBoolean,
    kMCTypeOperatorKindNumber,
    kMCTypeOperatorKindString,
    kMCTypeOperatorKindData,
    kMCTypeOperatorKindArray,
    
    kMCTypeOperatorKind_Count,
    
    kMCTypeOperatorKindUnknown,
};

MCNameRef MCTypeGetName(MCType *p_type);

bool MCTypeEvalAs(MCExecContext& ctxt, MCType *p_type, MCExecValue& x_value);
bool MCTypeEvalAs(MCExecContext& ctxt, MCType *p_type, MCAutoValueRef& x_value);

bool MCTypeEvalTo(MCExecContext& ctxt, uindex_t p_tag, MCTypeOperatorKind p_kind, MCValueRef p_value, MCValueRef& r_value);

bool MCTypeDeclare(MCScriptPoint& sp, MCNameRef p_name, MCType*& r_type);
bool MCTypeDefineOperator(MCScriptPoint& sp, MCNameRef p_type, MCTypeOperatorKind p_kind, MCHandler* p_handler);
void MCTypeUndefineOperator(MCNameRef p_type, MCTypeOperatorKind p_kind);

/* TYPE OPERATOR BASE CLASS
 *
 * Covers type related syntax of the form:
 *   X <type-operator> <typename>
 */
class MCTypeOperator : public MCExpression
{
public:
    MCTypeOperator(void)
    {
        rank = FR_TYPE;
    }
    
    virtual MCExpressionAttrs getattrs(void) const;
    virtual Parse_stat parse(MCScriptPoint& sp, Boolean the);
    
protected:
    MCType *m_type;
};

/* TYPE FUNCTION BASE CLASS
 *
 * Covers type related syntax of the form:
 *   <type-function>(X)
 */

class MCTypeFunction : public MCExpression
{
public:
    virtual MCExpressionAttrs getattrs(void) const;
    virtual Parse_stat parse(MCScriptPoint& sp, Boolean the);
    
protected:
    MCAutoPointer<MCExpression> m_operand;
};

/* TYPE ERROR FUNCTION */

class MCTypeError : public MCExpression
{
public:
    virtual MCExpressionAttrs getattrs(void) const;
    virtual Parse_stat parse(MCScriptPoint& sp, Boolean the);
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
    
private:
    MCAutoPointer<MCExpression> m_hint;
};

/* TYPE OF FUNCTION */

class MCTypeOf : public MCTypeFunction
{
public:
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};

/* STRICT TYPE OF FUNCTION */

class MCStrictTypeOf : public MCTypeFunction
{
public:
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};

/* AS STRICT TYPE OPERATOR */

class MCAsStrictType : public MCExpression
{
public:
    MCAsStrictType(MCExecValueType p_type)
        : m_type(p_type)
    {
        rank = FR_TYPE;
    }
    
    virtual MCExpressionAttrs getattrs(void) const;
    virtual Parse_stat parse(MCScriptPoint& sp, Boolean the);
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
    
protected:
    MCExecValueType m_type;
};

/* AS TYPE OPERATOR */

class MCAsType : public MCTypeOperator
{
public:
    virtual MCExpressionAttrs getattrs(void) const;
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};

/* IS TYPE OPERATOR */

class MCIsType : public MCTypeOperator
{
public:
    virtual MCExpressionAttrs getattrs(void) const;
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};

/* HAS TYPE OPERATOR */

class MCHasType : public MCTypeOperator
{
public:
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};

/* WITH TYPE OPERATOR */

class MCWithType : public MCTypeOperator
{
public:
    virtual Parse_stat parse(MCScriptPoint& sp, Boolean the);
    virtual void eval_ctxt(MCExecContext& ctxt, MCExecValue& r_value);
};

extern bool MCTypeInitialize(void);
extern void MCTypeFinalize(void);

#endif
