#include "Def.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

bool IsDot(Syntax stx) {
    Identifier* IdPtr = dynamic_cast<Identifier*>(stx.get());
    return IdPtr != nullptr && IdPtr->s == ".";
}

void BindVariable(Assoc &e, const std::string &var, const Value &v) {
    e = Assoc(new AssocList(var, v, e));
}

void ChangeBind(Assoc &e, const std::string &s, const Value &v) {
    Assoc env = e;
    while (env.get() != nullptr) {
        if (env->x == s) {
            env->v = v;
            return;
        }
        env = env->next;
    }
    throw RuntimeError("Unknown error");
}



std::pair<bool, Value> FindVar(Assoc &env, const std::string &s) {
    Assoc e = env;
    while (e.get() != nullptr) {
        if (e->x == s) 
            return std::make_pair(true, e->v);
        e = e->next;
    }
    return std::make_pair(false, Value(nullptr));
}

Value Let::eval(Assoc &env) {
    Assoc env1 = env;
    for (auto [s, exp]:bind)
        BindVariable(env1, s, exp->eval(env));
    return body->eval(env1);
} // let expression

Value Lambda::eval(Assoc &env) {
    return Value(new Closure(x, e, env));
} // lambda expression

Value Apply::eval(Assoc &e) {
    // std::cout<<"Enter apply"<<std::endl;
    // PrintAssoc(e);
    Value Clo = rator->eval(e);
    Closure* CloPtr = dynamic_cast<Closure*>(Clo.get());
    if (CloPtr == nullptr)
        throw RuntimeError("Unknown error");
    // std::cout<<"parameters:";
    // for (auto s:CloPtr->parameters)
        // std::cout<<" "<<s;
    // std::cout<<std::endl;
    if (rand.size() != (CloPtr->parameters).size())
        throw RuntimeError("Number of variables doesn't match");
    // std::cout<<"Start binding(apply)"<<std::endl;
    Assoc e1 = CloPtr->env;
    // PrintAssoc(e1);
    for (int i = 0;i < rand.size();++i)
        BindVariable(e1, CloPtr->parameters[i], rand[i]->eval(e));
    // std::cout<<"End binding(apply)"<<std::endl;
    return CloPtr->e->eval(e1);
} // for function calling

Value Letrec::eval(Assoc &env) {
    // std::cout<<"Enter letrec"<<std::endl;
    Assoc env1 = env;
    for (auto [s, exp]:bind)
        BindVariable(env1, s, Value(nullptr));
    Assoc env2 = env1;
    for (auto [s, exp]:bind)
        BindVariable(env2, s, exp->eval(env1));
    for (auto [s, exp]:bind) {
        Value val = exp->eval(env2);
        ChangeBind(env2, s, val);
    }
    return body->eval(env2);
} // letrec expression

Value Var::eval(Assoc &e) {
    if (isdigit(x[0]))
        throw RuntimeError("Unrecognizable variable");
    for (auto ch:x)
        if (isspace(ch) || ch == '#' || ch == '\'' || ch == '\"' || ch == '`')
            throw RuntimeError("Unrecognizable variable");
    static Assoc p = empty();
    static bool flag;
    flag = false, p = e;
    while (!flag && p.get() != nullptr) {
        if (p->x == x)
            return p->v;
        p = p->next;
    }
    throw RuntimeError("Value of variable not found");
} // evaluation of variable

Value Fixnum::eval(Assoc &e) {return Value(new Integer(n));} // evaluation of a fixnum

Value If::eval(Assoc &e) {
    // std::cout<<"Enter if"<<std::endl;
    Value conval = cond->eval(e);
    Boolean* BoolPtr = dynamic_cast<Boolean*>(conval.get());
    if (BoolPtr != nullptr && !BoolPtr->b)
        return alter->eval(e);///
    return conseq->eval(e);///
} // if expression

Value True::eval(Assoc &e) {return Value(new Boolean(true));} // evaluation of #t

Value False::eval(Assoc &e) {return Value(new Boolean(false));} // evaluation of #f

Value Begin::eval(Assoc &e) {
    Value ret(nullptr);
    if (es.size() == 0)
        throw RuntimeError("no expressions after \'begin\'");
    for (auto expr:es)
        ret = expr->eval(e);
    return ret;
} // begin expression

Value Quote::eval(Assoc &e) {
    // std::cout<<"Enter quote"<<std::endl;
    Number* NumPtr = dynamic_cast<Number*>(s.get());
    if (NumPtr != nullptr)
        return Value(new Integer(NumPtr->n));
    Identifier* IdPtr = dynamic_cast<Identifier*>(s.get());
    if (IdPtr != nullptr) {
        if (IdPtr->s == ".")
            throw RuntimeError("Unknown error with (.)");
        return Value(new Symbol(IdPtr->s));
    }
    TrueSyntax* TruePtr = dynamic_cast<TrueSyntax*>(s.get());
    if (TruePtr != nullptr)
        return Value(new Boolean(true));
    FalseSyntax* FalsePtr = dynamic_cast<FalseSyntax*>(s.get());
    if (FalsePtr != nullptr)
        return Value(new Boolean(false));
    List* LsPtr = dynamic_cast<List*>(s.get());
    std::vector<Syntax>& stxs = LsPtr->stxs;
    if (LsPtr == nullptr)
        throw RuntimeError("Unknown error while evaluating");
    if (stxs.size() == 0)
        return Value(new Null());
    if (stxs.size() == 1) 
        return Value(new Pair(Quote(stxs[0]).eval(e), Value(new Null())));
    if (stxs.size() == 2)
        return Value(new Pair(Quote(stxs[0]).eval(e), Value(new Pair(Quote(stxs[1]).eval(e), Value(new Null())))));
    Value *valptr;
    if (IsDot(stxs[stxs.size()-2]))
        valptr = new Value(new Pair(Quote(stxs[stxs.size()-3]).eval(e), Quote(stxs[stxs.size()-1]).eval(e)));
    else
        valptr = new Value(new Pair(
                    Quote(stxs[stxs.size()-3]).eval(e), 
                    Value(new Pair(
                        Quote(stxs[stxs.size()-2]).eval(e), 
                        Value (new Pair(
                            Quote(stxs[stxs.size()-1]).eval(e),
                            Value(new Null())
                        ))
                    ))
                ));
    for (int i = stxs.size()-4; ~i; --i)
        valptr = new Value(new Pair(Quote(stxs[i]).eval(e), *valptr));
    return *valptr;
} // quote expression

Value MakeVoid::eval(Assoc &e) {return Value(new Void());} // (void)

Value Exit::eval(Assoc &e) {return Value(new Terminate());} // (exit)

Value Binary::eval(Assoc &e) {return evalRator(rand1->eval(e), rand2->eval(e));} // evaluation of two-operators primitive

Value Unary::eval(Assoc &e) {return evalRator(rand->eval(e));} // evaluation of single-operator primitive

Value Mult::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Integer(IntPtr1->n * IntPtr2->n));
} // *

Value Plus::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Integer(IntPtr1->n + IntPtr2->n));
} // +

Value Minus::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Integer(IntPtr1->n - IntPtr2->n));
} // -

Value Less::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Boolean(IntPtr1->n < IntPtr2->n));
} // <

Value LessEq::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Boolean(IntPtr1->n <= IntPtr2->n));
} // <=

Value Equal::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Boolean(IntPtr1->n == IntPtr2->n));
} // =

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Boolean(IntPtr1->n >= IntPtr2->n));
} // >=

Value Greater::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 == nullptr || IntPtr2 == nullptr)
        throw RuntimeError("Calculationg Fault: Wrong date type.");
    return Value(new Boolean(IntPtr1->n > IntPtr2->n));
} // >

Value IsEq::evalRator(const Value &rand1, const Value &rand2) {
    Integer* IntPtr1 = dynamic_cast<Integer*>(rand1.get()), *IntPtr2 = dynamic_cast<Integer*>(rand2.get());
    if (IntPtr1 != nullptr && IntPtr2 != nullptr)
        return Value(new Boolean(IntPtr1->n == IntPtr2->n));
    Boolean* BoolPtr1 = dynamic_cast<Boolean*>(rand1.get()), *BoolPtr2 = dynamic_cast<Boolean*>(rand2.get());
    if (BoolPtr1 != nullptr && BoolPtr2 != nullptr)
        return Value(new Boolean(BoolPtr1->b == BoolPtr2->b));
    Symbol* SymPtr1 = dynamic_cast<Symbol*>(rand1.get()), *SymPtr2 = dynamic_cast<Symbol*>(rand2.get());
    if (SymPtr1 != nullptr && SymPtr2 != nullptr)
        return Value(new Boolean(SymPtr1->s == SymPtr2->s));
    Null* NulPtr1 = dynamic_cast<Null*>(rand1.get()), *NulPtr2 = dynamic_cast<Null*>(rand2.get());
    if (NulPtr1 != nullptr && NulPtr2 != nullptr)
        return Value(new Boolean(true));
    Void* VoidPtr1 = dynamic_cast<Void*>(rand1.get()), *VoidPtr2 = dynamic_cast<Void*>(rand2.get());
    if (VoidPtr1 != nullptr && VoidPtr2 != nullptr)
        return Value(new Boolean(true));
    return Value(new Boolean(rand1.get() == rand2.get()));
} // eq?

Value Cons::evalRator(const Value &rand1, const Value &rand2) {return Value(new Pair(rand1, rand2));} // cons

Value IsBoolean::evalRator(const Value &rand) {
    Boolean* BoolPtr = dynamic_cast<Boolean*>(rand.get());
    return Value(new Boolean(BoolPtr != nullptr));
} // boolean?

Value IsFixnum::evalRator(const Value &rand) {
    Integer* IntPtr = dynamic_cast<Integer*>(rand.get());
    return Value(new Boolean(IntPtr != nullptr));
} // fixnum?

Value IsSymbol::evalRator(const Value &rand) {
    Symbol* SymPtr = dynamic_cast<Symbol*>(rand.get());
    return Value(new Boolean(SymPtr != nullptr));
} // symbol?

Value IsNull::evalRator(const Value &rand) {
    Null* NullPtr = dynamic_cast<Null*>(rand.get());
    return Value(new Boolean(NullPtr != nullptr));
} // null?

Value IsPair::evalRator(const Value &rand) {
    Pair* PairPtr = dynamic_cast<Pair*>(rand.get());
    return Value(new Boolean(PairPtr != nullptr));
} // pair?

Value IsProcedure::evalRator(const Value &rand) {
    Closure* CloPtr = dynamic_cast<Closure*>(rand.get());
    return Value(new Boolean(CloPtr != nullptr));
} // procedure?

Value Not::evalRator(const Value &rand) {
    Boolean* BoolPtr = dynamic_cast<Boolean*>(rand.get());
    if (BoolPtr == nullptr)
        return Value(new Boolean(false));
    return Value(new Boolean(!BoolPtr->b));
} // not

Value Car::evalRator(const Value &rand) {
    // Value val = rand->val();
    // Terminate* TerPtr = dynamic_cast<Terminate*>(val.get());
    // if (TerPtr != nullptr)
        // return Value(new Terminate());
    Pair* PairPtr = dynamic_cast<Pair*>(rand.get());
    if (PairPtr == nullptr)
        throw RuntimeError("Calculation fault: Wrong data type.");
    return PairPtr->car;
} // car

Value Cdr::evalRator(const Value &rand) {
    Pair* PairPtr = dynamic_cast<Pair*>(rand.get());
    if (PairPtr == nullptr)
        throw RuntimeError("Calculation fault: Wrong data type.");
    return PairPtr->cdr;
} // cdr
