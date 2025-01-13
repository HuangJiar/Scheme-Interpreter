#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include "RE.hpp"
#include <sstream>
#include <iostream>
#include <map>


extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

void init(Assoc& e) {
    BindVariable(e, "+", Value(new Closure({"x", "y"}, Expr(new Plus(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "-", Value(new Closure({"x", "y"}, Expr(new Minus(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "*", Value(new Closure({"x", "y"}, Expr(new Mult(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, ">", Value(new Closure({"x", "y"}, Expr(new Greater(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "<", Value(new Closure({"x", "y"}, Expr(new Less(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "=", Value(new Closure({"x", "y"}, Expr(new Equal(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, ">=", Value(new Closure({"x", "y"}, Expr(new GreaterEq(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "<=", Value(new Closure({"x", "y"}, Expr(new LessEq(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "cons", Value(new Closure({"x", "y"}, Expr(new Cons(Expr(new Var("x")), Expr(new Var("y")))), e)));
    BindVariable(e, "car", Value(new Closure({"x"}, Expr(new Car(Expr(new Var("x")))), e)));
    BindVariable(e, "cdr", Value(new Closure({"x"}, Expr(new Cdr(Expr(new Var("x")))), e)));
    BindVariable(e, "not", Value(new Closure({"x"}, Expr(new Not(Expr(new Var("x")))), e)));
    BindVariable(e, "exit", Value(new Closure({}, Expr(new Exit()), e)));
    BindVariable(e, "void", Value(new Closure({}, Expr(new MakeVoid()), e)));
    BindVariable(e, "fixnum?", Value(new Closure({"x"}, Expr(new IsFixnum(Expr(new Var("x")))), e)));
    BindVariable(e, "boolean?", Value(new Closure({"x"}, Expr(new IsBoolean(Expr(new Var("x")))), e)));
    BindVariable(e, "null?", Value(new Closure({"x"}, Expr(new IsNull(Expr(new Var("x")))), e)));
    BindVariable(e, "pair?", Value(new Closure({"x"}, Expr(new IsPair(Expr(new Var("x")))), e)));
    BindVariable(e, "symbol?", Value(new Closure({"x"}, Expr(new IsSymbol(Expr(new Var("x")))), e)));
    BindVariable(e, "procedure?", Value(new Closure({"x"}, Expr(new IsProcedure(Expr(new Var("x")))), e)));
    BindVariable(e, "eq?", Value(new Closure({"x", "y"}, Expr(new IsEq(Expr(new Var("x")), Expr(new Var("y")))), e)));
    Assoc e1 = e;
    while (e1.get() != nullptr) {
        Closure *CloPtr = dynamic_cast<Closure*>(e1->v.get());
        if (CloPtr == nullptr)
            throw RuntimeError("Init failed");
        e1 = e1->next;
    }
}

void REPL()
{
    // read - evaluation - print loop
    Assoc global_env = empty();
    init(global_env);
    while (1)
    {
        #ifndef ONLINE_JUDGE
            std::cout << "scm> ";
        #endif
        Syntax stx = readSyntax(std :: cin); // read
        try
        {
            Expr expr = stx -> parse(global_env); // parse
            // stx -> show(std :: cout); // syntax print
            // std::cout<<std::endl;
            Value val = expr -> eval(global_env);
            if (val -> v_type == V_TERMINATE)
                break;
            val -> show(std :: cout); // value print
        }
        catch (const RuntimeError &RE)
        {
            // std :: cout << RE.message();
            std :: cout << "RuntimeError";//<<RE.message();
        }
        puts("");
    }
}


int main(int argc, char *argv[]) {
    initPrimitives();
    initReservedWords();
    REPL();
    return 0;
}
