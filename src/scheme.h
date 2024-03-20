#pragma once

#include <string>
#include "object.h"

#define SCHEME_FUZZING_2_PRINT_REQUESTS

Object* Copy(Object* object);

Object* Calc(Object* object, NameSpace* scope);

class Interpreter {
public:
    Interpreter() : global_namespace_(Heap::Instance()->Make<NameSpace>()) {

        global_namespace_->Set("quote", Heap::Instance()->Make<Quote>());
        global_namespace_->Set("pair?", Heap::Instance()->Make<IsPair>());
        global_namespace_->Set("null?", Heap::Instance()->Make<IsNull>());
        global_namespace_->Set("list?", Heap::Instance()->Make<IsList>());
        global_namespace_->Set("list", Heap::Instance()->Make<List>());
        global_namespace_->Set("cons", Heap::Instance()->Make<Cons>());
        global_namespace_->Set("car", Heap::Instance()->Make<Car>());
        global_namespace_->Set("cdr", Heap::Instance()->Make<Cdr>());
        global_namespace_->Set("list-ref", Heap::Instance()->Make<ListRef>());
        global_namespace_->Set("list-tail", Heap::Instance()->Make<ListTail>());
        global_namespace_->Set("=", Heap::Instance()->Make<EqualTo>());
        global_namespace_->Set(">", Heap::Instance()->Make<Greater>());
        global_namespace_->Set("<", Heap::Instance()->Make<Less>());
        global_namespace_->Set(">=", Heap::Instance()->Make<GreaterEqual>());
        global_namespace_->Set("<=", Heap::Instance()->Make<LessEqual>());
        global_namespace_->Set("+", Heap::Instance()->Make<Plus>());
        global_namespace_->Set("-", Heap::Instance()->Make<Minus>());
        global_namespace_->Set("*", Heap::Instance()->Make<Multiplies>());
        global_namespace_->Set("/", Heap::Instance()->Make<Divides>());
        global_namespace_->Set("max", Heap::Instance()->Make<Max>());
        global_namespace_->Set("min", Heap::Instance()->Make<Min>());
        global_namespace_->Set("abs", Heap::Instance()->Make<Abs>());
        global_namespace_->Set("boolean?", Heap::Instance()->Make<IsBoolean>());
        global_namespace_->Set("number?", Heap::Instance()->Make<IsNumber>());
        global_namespace_->Set("not", Heap::Instance()->Make<Not>());
        global_namespace_->Set("and", Heap::Instance()->Make<And>());
        global_namespace_->Set("or", Heap::Instance()->Make<Or>());

        global_namespace_->Set("define", Heap::Instance()->Make<Define>());
        global_namespace_->Set("symbol?", Heap::Instance()->Make<IsSymbol>());
        global_namespace_->Set("set!", Heap::Instance()->Make<Set>());
        global_namespace_->Set("set-car!", Heap::Instance()->Make<SetCar>());
        global_namespace_->Set("set-cdr!", Heap::Instance()->Make<SetCdr>());
        global_namespace_->Set("if", Heap::Instance()->Make<If>());
        global_namespace_->Set("lambda", Heap::Instance()->Make<CreateLambda>());
    }

    ~Interpreter() {
        Heap::Instance()->Clear();
    }

    std::string Run(const std::string& str);

    std::string GetString(Object* object);

private:
    NameSpace* global_namespace_;
};

template <typename T>
auto CalcAndGet(Object* obj, NameSpace* scope) {
    obj = Calc(obj, scope);
    RequireType<T>(obj);
    if constexpr (std::is_same_v<T, Number> || std::is_same_v<T, Boolean>) {
        return As<T>(obj)->GetValue();
    } else if constexpr (std::is_same_v<T, Symbol>) {
        return As<T>(obj)->GetName();
    } else if constexpr (std::is_same_v<T, Cell>) {
        return std::make_pair(As<T>(obj)->GetFirst(), As<T>(obj)->GetSecond());
    } else if constexpr (std::is_same_v<T, Functor>) {
        return As<T>(obj)->GetFunctorName();
    } else {
        throw RuntimeError("Type is unknown");
    }
}

template <typename T>
auto Get(Object* obj) {
    RequireType<T>(obj);
    if constexpr (std::is_same_v<T, Number> || std::is_same_v<T, Boolean>) {
        return As<T>(obj)->GetValue();
    } else if constexpr (std::is_same_v<T, Symbol>) {
        return As<T>(obj)->GetName();
    } else if constexpr (std::is_same_v<T, Cell>) {
        return std::make_pair(As<T>(obj)->GetFirst(), As<T>(obj)->GetSecond());
    } else if constexpr (std::is_same_v<T, Functor>) {
        return As<T>(obj)->GetFunctorName();
    } else {
        throw RuntimeError("Type is unknown");
    }
}