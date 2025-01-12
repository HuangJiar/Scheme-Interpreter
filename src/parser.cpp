#ifndef PARSER 
#define PARSER

// parser of myscheme 

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std :: string;
using std :: vector; 
using std :: pair;

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

void PrintAssoc(Assoc &e) {
    Assoc f = e;
    std::cout<<"Print Assoclist:"<<e.get()<<std::endl;
    while (f.get() != nullptr) {
        std::cout<<f->x<<"  ";
        if (f->v.get() != nullptr)
            f->v->show(std::cout);
        else
            std::cout<<"null";
        std::cout<<std::endl;
        f = f->next;
    }
    std::cout<<"End print"<<std::endl;
}

Expr Syntax :: parse(Assoc &env) {return ptr->parse(env);}

Expr Number :: parse(Assoc &env) {return Expr(new Fixnum(n));}

Expr Identifier :: parse(Assoc &env) {
    if (s[0] == '.' || s[0] == '@' || isdigit(s[0]))
        throw RuntimeError("Invalid variable name");
    return Expr(new Var(s));
}

Expr TrueSyntax :: parse(Assoc &env) {return Expr(new True());}

Expr FalseSyntax :: parse(Assoc &env) {return Expr(new False());}

Expr List :: parse(Assoc &env) {
    if (!stxs.size())
        throw RuntimeError("List size=0");
    // std::cout<<"Start parsing list"<<std::endl;
    // PrintAssoc(env);
    Identifier* IdPtr1 = dynamic_cast<Identifier*>(stxs[0].get());
    if (IdPtr1 != nullptr) {
        // std::cout<<"with first identifier:"<<IdPtr1->s<<std::endl;
        auto [flag, find] = FindVar(env, IdPtr1->s);
        if (flag) {
            std::vector<Expr> vecexpr;
            for (int i = 1;i < stxs.size();++i)
                vecexpr.push_back(stxs[i].parse(env));
            return Expr(new Apply(Expr(new Var(IdPtr1->s)), vecexpr));
        }
        if (IdPtr1->s == "exit") {
            if (stxs.size() > 1)
                throw RuntimeError("exit with followings");
            return Expr(new Exit());
        }
        if (IdPtr1->s == "void") {
            if (stxs.size() > 1)
                throw RuntimeError("void with followings");
            return Expr(new MakeVoid());
        }
        if (IdPtr1->s == "#t")
            return Expr(new True());
        if (IdPtr1->s == "#f")
            return Expr(new False());
        if (IdPtr1->s == "+") {
            if (stxs.size() == 1)
                return Expr(new Fixnum(0));
            if (stxs.size() == 2)
                return Expr(new Plus(stxs[1]->parse(env), Expr(new Fixnum(0))));
            Expr ret = Expr(new Plus(stxs[1]->parse(env), stxs[2]->parse(env)));
            for (int i = 3;i < stxs.size();++i)
                ret = Expr(new Plus(ret, stxs[i]->parse(env)));
            return ret;
        }
        if (IdPtr1->s == "-") {
            if (stxs.size() == 1)
                throw RuntimeError("- with wrong number of variables");
            if (stxs.size() == 2)
                return Expr(new Minus(Expr(new Fixnum(0)), stxs[1]->parse(env)));
            Expr ret = Expr(new Minus(stxs[1]->parse(env), stxs[2]->parse(env)));
            for (int i = 3;i < stxs.size();++i)
                ret = Expr(new Minus(ret, stxs[i]->parse(env)));
            return ret;
        }
        if (IdPtr1->s == "*") {
            if (stxs.size() == 1)
                return Expr(new Fixnum(1));
            if (stxs.size() == 2)
                return Expr(new Mult(stxs[1]->parse(env), Expr(new Fixnum(1))));
            Expr ret = Expr(new Mult(stxs[1]->parse(env), stxs[2]->parse(env)));
            for (int i = 3;i < stxs.size();++i)
                ret = Expr(new Mult(ret, stxs[i]->parse(env)));
            return ret;
        }
        if (IdPtr1->s == "<") {
            if (stxs.size() != 3)
                throw RuntimeError("< with wrong number of variables");
            return Expr(new Less(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == ">") {
            if (stxs.size() != 3)
                throw RuntimeError("> with wrong number of variables");
            return Expr(new Greater(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == "<=") {
            if (stxs.size() != 3)
                throw RuntimeError("<= with wrong number of variables");
            return Expr(new LessEq(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == ">=") {
            if (stxs.size() != 3)
                throw RuntimeError(">= with wrong number of variables");
            return Expr(new GreaterEq(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == "=") {
            if (stxs.size() != 3)
                throw RuntimeError("= with wrong number of variables");
            return Expr(new Equal(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == "not") {
            if (stxs.size() != 2)
                throw RuntimeError("\'not\' with wrong number of variables");
            return Expr(new Not(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "fixnum?") {
            if (stxs.size() != 2)
                throw RuntimeError("\'fixnum?\' with wrong number of variables");
            return Expr(new IsFixnum(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "boolean?") {
            if (stxs.size() != 2)
                throw RuntimeError("\'boolean?\' with wrong number of variables");
            return Expr(new IsBoolean(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "null?") {
            if (stxs.size() != 2)
                throw RuntimeError("\'null?\' with wrong number of variables");
            return Expr(new IsNull(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "pair?") {
            if (stxs.size() != 2)
                throw RuntimeError("\'pair?\' with wrong number of variables");
            return Expr(new IsPair(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "symbol?") {
            if (stxs.size() != 2)
                throw RuntimeError("\'symbol?\' with wrong number of variables");
            return Expr(new IsSymbol(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "procedure?") {
            if (stxs.size() != 2)
                throw RuntimeError("\'procedure?\' with wrong number of variables");
            return Expr(new IsProcedure(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "eq?") {
            if (stxs.size() != 3)
                throw RuntimeError("\'eq?\' with wrong number of variables");
            return Expr(new IsEq(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == "quote") {
            if (stxs.size() != 2)
                throw RuntimeError("\'eq?\' with wrong number of variables");
            return Expr(new Quote(stxs[1]));
        }
        if (IdPtr1->s == "car") {
            if (stxs.size() != 2)
                throw RuntimeError("\'car\' with wrong number of variables");
            return Expr(new Car(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "cdr") {
            if (stxs.size() != 2)
                throw RuntimeError("\'cdr\' with wrong number of variables");
            return Expr(new Cdr(stxs[1]->parse(env)));
        }
        if (IdPtr1->s == "begin") {
            std::vector<Expr> e;
            for (int i = 1;i < stxs.size();++i)
                e.push_back(stxs[i]->parse(env));
            return Expr(new Begin(e));
        }
        if (IdPtr1->s == "if") {
            if (stxs.size() != 4)
                throw RuntimeError("\'if\' with wrong number of variables");
            return Expr(new If(stxs[1]->parse(env), stxs[2]->parse(env), stxs[3]->parse(env)));
        }
        if (IdPtr1->s == "cons") {
            if (stxs.size() != 3)
                throw RuntimeError("\'cons\' with wrong number of variables");
            return Expr(new Cons(stxs[1]->parse(env), stxs[2]->parse(env)));
        }
        if (IdPtr1->s == "lambda") {
            if (stxs.size() != 3)
                throw RuntimeError("\'lambda\' with wrong number of variables");
            List* LstPtr = dynamic_cast<List*>(stxs[1].get());
            if (LstPtr == nullptr)
                throw RuntimeError("\'lambda\' with unreconizable variables");
            std::vector<std::string> x;
            Assoc env1 = env;
            for (auto ptr:LstPtr->stxs) {
                static Identifier* IdPtr2;
                IdPtr2 = dynamic_cast<Identifier*>(ptr.get());
                if (IdPtr2 == nullptr)
                    throw RuntimeError("\'lambda\' with unreconizable variables");
                x.push_back(IdPtr2->s);
                BindVariable(env1, IdPtr2->s, Value(nullptr));
            }
            return Expr(new Lambda(x, stxs[2]->parse(env1)));
        }
        if (IdPtr1->s == "let") {
            // puts("Enter Let");
            // PrintAssoc(env);
            if (stxs.size() != 3)
                throw RuntimeError("\'let\' with wrong number of variables");
            List* LstPtr = dynamic_cast<List*>(stxs[1].get());
            if (LstPtr == nullptr)
                throw RuntimeError("\'let\' with unreconizable variables");
            std::vector<std::pair<std::string, Expr>> vecexp;
            Assoc env1 = env;
            for (auto stx:LstPtr->stxs) {
                List* LstPtr1 = dynamic_cast<List*>(stx.get());
                if ((LstPtr1->stxs).size() != 2)
                    throw RuntimeError("\'let\' with unreconizable variables");
                Identifier* IdPtr1 = dynamic_cast<Identifier*>(((LstPtr1->stxs)[0].get()));
                if (IdPtr1 == nullptr)
                    throw RuntimeError("\'let\' with unreconizable variables");
                vecexp.push_back(std::make_pair(IdPtr1->s, (LstPtr1->stxs)[1].parse(env)));
                BindVariable(env1, IdPtr1->s, Value(nullptr));
                // std::cout<<"Bind:"<<IdPtr1->s<<std::endl;
            }
            // PrintAssoc(env1);
            Expr rator = stxs[2].parse(env1);
            return Expr(new Let(vecexp, rator));
        }
        if (IdPtr1->s == "letrec") {
            if (stxs.size() != 3)
                throw RuntimeError("\'let\' with wrong number of variables");
            List* LstPtr = dynamic_cast<List*>(stxs[1].get());
            if (LstPtr == nullptr)
                throw RuntimeError("\'let\' with unreconizable variables");
            std::vector<std::pair<std::string, Expr>> vecexp;
            Assoc env1 = env;
            for (auto stx:LstPtr->stxs) {
                List* LstPtr1 = dynamic_cast<List*>(stx.get());
                if ((LstPtr1->stxs).size() != 2)
                    throw RuntimeError("\'let\' with unreconizable variables");
                Identifier* IdPtr1 = dynamic_cast<Identifier*>(((LstPtr1->stxs)[0].get()));
                if (IdPtr1 == nullptr)
                    throw RuntimeError("\'let\' with unreconizable variables");
                vecexp.push_back(std::make_pair(IdPtr1->s, (LstPtr1->stxs)[1].parse(env)));
                BindVariable(env1, IdPtr1->s, Value(nullptr));
            }
            Expr rator = stxs[2].parse(env1);
            return Expr(new Letrec(vecexp, rator));
        }
    }
    std::vector<Expr> vecexpr;
    Expr rator = stxs[0].parse(env);
    for (int i = 1;i < stxs.size();++i)
        vecexpr.push_back(stxs[i].parse(env));
    return Expr(new Apply(rator, vecexpr));
}

#endif