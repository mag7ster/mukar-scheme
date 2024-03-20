#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
#include <algorithm>
#include "object.h"
#include "error.h"
#include "scheme.h"

Object* Cell::Copy() {
    auto res = Heap::Instance()->Make<Cell>(nullptr, nullptr);
    if (this == first_) {
        res->GetFirst() = res;
    } else {
        res->GetFirst() = ::Copy(first_);
    }
    if (this == second_) {
        res->GetSecond() = res;
    } else {
        res->GetSecond() = ::Copy(second_);
    }
    return res;
}

Object*& NameSpace::Get(const std::string& name) {
    auto cur = this;
    while (cur != nullptr) {
        if (cur->data_.contains(name)) {
            return cur->data_[name];
        }
        cur = cur->upper_;
    }
    throw NameError(name + " not found");
}

void NameSpace::Set(const std::string& name, Object* obj) {
    data_[name] = obj;
}

template <typename T>
struct MaxFunctor {
    T operator()(const T& first, const T& second) {
        return std::max<T>(first, second);
    }
};

template <typename T>
struct MinFunctor {
    T operator()(const T& first, const T& second) {
        return std::min<T>(first, second);
    }
};

std::vector<Object*> ToVector(Object* obj) {
    std::vector<Object*> result;
    while (Is<Cell>(obj)) {
        auto cell = As<Cell>(obj);
        result.push_back(cell->GetFirst());
        obj = cell->GetSecond();
    }
    if (obj != nullptr) {
        throw RuntimeError("Must be proper list");
    }
    return result;
}

Object* FromVector(std::vector<Object*>& vec) {
    Object* right = nullptr;
    while (!vec.empty()) {
        auto temp = Heap::Instance()->Make<Cell>(vec.back(), right);
        right = temp;
        vec.pop_back();
    }
    return right;
}

void RequiresOnlyXArguments(std::vector<Object*>& a, size_t x) {
    if (a.size() != x) {
        throw RuntimeError("Requires only " + std::to_string(x) + " arguments, but got " +
                           std::to_string(a.size()));
    }
}

void RequiresOnlyXArgumentsS(std::vector<Object*>& a, size_t x) {
    if (a.size() != x) {
        throw SyntaxError("Requires only " + std::to_string(x) + " arguments, but got " +
                          std::to_string(a.size()));
    }
}

void RequiresOnlyLRArgumentsS(std::vector<Object*>& a, size_t l, size_t r) {
    if (a.size() < l || a.size() > r) {
        throw SyntaxError("Requires only from " + std::to_string(l) + " to " + std::to_string(r) +
                          " arguments, but got " + std::to_string(a.size()));
    }
}

void RequiresMinimumXArguments(std::vector<Object*>& a, size_t x) {
    if (a.size() < x) {
        throw RuntimeError("Requires minimum " + std::to_string(x) + " arguments, but got " +
                           std::to_string(a.size()));
    }
}

void RequiresMinimumXArgumentsS(std::vector<Object*>& a, size_t x) {
    if (a.size() < x) {
        throw SyntaxError("Requires minimum " + std::to_string(x) + " arguments, but got " +
                          std::to_string(a.size()));
    }
}

void RequiresValidIndex(std::vector<Object*>& a, size_t x) {
    if (a.size() <= x) {
        throw RuntimeError("Requires valid index");
    }
}

void CalcVector(std::vector<Object*>& a, NameSpace* scope) {
    for (auto& x : a) {
        x = Calc(x, scope);
    }
}

std::vector<Object*> CopyVector(std::vector<Object*>& a) {
    auto vec = a;
    for (auto& elem : vec) {
        elem = Copy(elem);
    }
    return vec;
}

std::vector<Object*> CopyAndCalcVector(std::vector<Object*>& a, NameSpace* scope) {
    auto vec = CopyVector(a);
    CalcVector(vec, scope);
    return vec;
}

Object* TrueObject() {
    return Heap::Instance()->Make<Boolean>(true);
}

Object* FalseObject() {
    return Heap::Instance()->Make<Boolean>(false);
}

Object* Condition(bool val) {
    if (val) {
        return TrueObject();
    } else {
        return FalseObject();
    }
}

Object* Quote::operator()(Object* obj, [[maybe_unused]] NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 1);
    return vec.front();
}

// List operations

Object* IsPair::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 1);
    CalcVector(vec, scope);
    return Condition(Is<Cell>(vec.front()));
}

Object* IsNull::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 1);
    CalcVector(vec, scope);
    return Condition(vec.front() == nullptr);
}

bool IsListHelper(Object* obj) {
    while (Is<Cell>(obj)) {
        auto cell = As<Cell>(obj);
        obj = cell->GetSecond();
    }
    return obj == nullptr;
}

Object* IsList::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 1);
    CalcVector(vec, scope);
    return Condition(IsListHelper(vec.front()));
}

Object* List::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    CalcVector(vec, scope);
    return FromVector(vec);
}

Object* Cons::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 2);
    CalcVector(vec, scope);
    auto temp = Heap::Instance()->Make<Cell>(vec[0], vec[1]);
    return temp;
}

Object* Car::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 1);
    CalcVector(vec, scope);
    RequireType<Cell>(vec.front());
    return As<Cell>(vec.front())->GetFirst();
}

Object* Cdr::operator()(Object* obj, NameSpace* scope) {
    auto vec = ToVector(obj);
    RequiresOnlyXArguments(vec, 1);
    CalcVector(vec, scope);
    RequireType<Cell>(vec.front());
    return As<Cell>(vec.front())->GetSecond();
}

Object* ListRef::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 2);
    CalcVector(args, scope);
    auto vec = ToVector(args.front());
    RequireType<Number>(args[1]);
    RequiresValidIndex(vec, As<Number>(args[1])->GetValue());
    return vec[As<Number>(args[1])->GetValue()];
}

Object* ListTail::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 2);
    CalcVector(args, scope);
    RequireType<Number>(args[1]);
    auto steps = As<Number>(args[1])->GetValue();
    auto cur = args[0];
    for (int64_t i = 0; i < steps; ++i) {
        RequireType<Cell>(cur);
        cur = As<Cell>(cur)->GetSecond();
    }
    return cur;
}

// Number operations

Object* IsNumber::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 1);
    CalcVector(args, scope);
    return Condition(Is<Number>(args.front()));
}

template <typename Functor>
bool CalcNumberListToBool(std::vector<Object*>& vec, NameSpace* scope) {
    if (vec.empty()) {
        return true;
    }
    static auto func = Functor();
    auto prev = CalcAndGet<Number>(vec.front(), scope);
    for (size_t i = 1; i < vec.size(); ++i) {
        auto cur = CalcAndGet<Number>(vec[i], scope);
        if (!func(prev, cur)) {
            return false;
        }
        prev = cur;
    }
    return true;
}

template <typename Functor>
int64_t CalcNumberListToInt(std::vector<Object*>& vec, NameSpace* scope, int64_t neutral) {
    static auto func = Functor();
    CalcVector(vec, scope);
    auto result = neutral;
    for (auto& elem : vec) {
        auto cur = Get<Number>(elem);
        result = func(result, cur);
    }
    return result;
}

template <typename Functor>
int64_t CalcIrrevNumberListToInt(std::vector<Object*>& vec, NameSpace* scope, int64_t neutral) {
    static auto func = Functor();
    CalcVector(vec, scope);
    if (vec.size() == 1) {
        return func(neutral, Get<Number>(vec.front()));
    }
    auto result = Get<Number>(vec.front());
    for (size_t i = 1; i < vec.size(); ++i) {
        auto cur = Get<Number>(vec[i]);
        result = func(result, cur);
    }
    return result;
}

Object* EqualTo::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    return Condition(CalcNumberListToBool<std::equal_to<int64_t>>(args, scope));
}

Object* Greater::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    return Condition(CalcNumberListToBool<std::greater<int64_t>>(args, scope));
}

Object* Less::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    return Condition(CalcNumberListToBool<std::less<int64_t>>(args, scope));
}

Object* GreaterEqual::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    return Condition(CalcNumberListToBool<std::greater_equal<int64_t>>(args, scope));
}

Object* LessEqual::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    return Condition(CalcNumberListToBool<std::less_equal<int64_t>>(args, scope));
}

Object* Plus::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    auto res = CalcNumberListToInt<std::plus<int64_t>>(args, scope, 0);
    return Heap::Instance()->Make<Number>(res);
}

Object* Minus::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresMinimumXArguments(args, 1);
    auto res = CalcIrrevNumberListToInt<std::minus<int64_t>>(args, scope, 0);
    return Heap::Instance()->Make<Number>(res);
}

Object* Multiplies::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    auto res = CalcNumberListToInt<std::multiplies<int64_t>>(args, scope, 1);
    return Heap::Instance()->Make<Number>(res);
}

Object* Divides::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresMinimumXArguments(args, 1);
    auto res = CalcIrrevNumberListToInt<std::divides<int64_t>>(args, scope, 1);
    return Heap::Instance()->Make<Number>(res);
}

Object* Max::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresMinimumXArguments(args, 1);
    auto res = CalcNumberListToInt<MaxFunctor<int64_t>>(args, scope, LLONG_MIN);
    return Heap::Instance()->Make<Number>(res);
}

Object* Min::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresMinimumXArguments(args, 1);
    auto res = CalcNumberListToInt<MinFunctor<int64_t>>(args, scope, LLONG_MAX);
    return Heap::Instance()->Make<Number>(res);
}

Object* Abs::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 1);
    auto res = std::abs(CalcAndGet<Number>(args.front(), scope));
    return Heap::Instance()->Make<Number>(res);
}

bool ToBool(Object* obj) {
    if (Is<Boolean>(obj) && As<Boolean>(obj)->GetValue() == false) {
        return false;
    }
    return true;
}

Object* IsBoolean::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 1);
    CalcVector(args, scope);
    return Condition(Is<Boolean>(args.front()));
}

Object* Not::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 1);
    CalcVector(args, scope);
    auto res = ToBool(args.front());
    return Heap::Instance()->Make<Boolean>(!res);
}

Object* And::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    if (args.empty()) {
        return TrueObject();
    }
    for (auto& elem : args) {
        elem = Calc(elem, scope);
        if (!ToBool(elem)) {
            return FalseObject();
        }
    }
    return args.back();
}

Object* Or::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    size_t temp = 0;
    for (auto& elem : args) {
        ++temp;
        elem = Calc(elem, scope);
        if (ToBool(elem)) {
            if (temp == args.size()) {
                return elem;
            }
            return TrueObject();
        }
    }
    return FalseObject();
}

// Advanced

void DefineHelper(Object* name, Object* obj, NameSpace* scope) {
    scope->Set(Get<Symbol>(name), obj);
}

Object* CreateLambdaHelper(std::vector<Object*>& arg_names, std::vector<Object*>& body,
                           NameSpace* scope) {
    for (auto elem : arg_names) {
        if (!Is<Symbol>(elem)) {
            throw SyntaxError("Symbols expected");
        }
    }
    return Heap::Instance()->Make<Lambda>(arg_names, body, scope);
}

Object* Define::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresMinimumXArgumentsS(args, 2);
    if (Is<Symbol>(args.front())) {
        RequiresOnlyXArgumentsS(args, 2);
        DefineHelper(args.front(), ::Copy(Calc(args.back(), scope)), scope);
        return args.front();
    } else if (IsListHelper(args.front())) {

        auto arg_names = ToVector(args.front());
        auto name = arg_names.front();
        arg_names.erase(arg_names.begin());

        args.erase(args.begin());

        DefineHelper(name, CreateLambdaHelper(arg_names, args, scope), scope);

        return name;
        // throw std::runtime_error("Not implemented");
    } else {
        throw SyntaxError("Invalid arguments for define");
    }
}

Object* IsSymbol::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, 1);
    CalcVector(args, scope);
    return Condition(Is<Symbol>(args.front()));
}

Object* Set::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArgumentsS(args, 2);
    Object* prev = scope->Get(Get<Symbol>(args.front()));
    scope->Get(Get<Symbol>(args.front())) = ::Copy(Calc(args.back(), scope));
    return prev;
}

Object* SetCar::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArgumentsS(args, 2);

    CalcVector(args, scope);
    RequireType<Cell>(args[0]);
    auto prev = As<Cell>(args.front())->GetFirst();
    if (args.front() == args.back()) {
        As<Cell>(args[0])->GetFirst() = args.back();
    } else {
        As<Cell>(args[0])->GetFirst() = ::Copy(args.back());
    }
    return prev;
}

Object* SetCdr::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArgumentsS(args, 2);

    CalcVector(args, scope);
    RequireType<Cell>(args[0]);
    auto prev = As<Cell>(args.front())->GetSecond();
    if (args.front() == args.back()) {
        As<Cell>(args[0])->GetSecond() = args.back();
    } else {
        As<Cell>(args[0])->GetSecond() = ::Copy(args.back());
    }
    return prev;
}

Object* If::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyLRArgumentsS(args, 2, 3);
    args.front() = Calc(args.front(), scope);
    if (ToBool(args.front())) {
        return Calc(args[1], scope);
    } else if (args.size() == 3) {
        return Calc(args[2], scope);
    } else {
        return nullptr;
    }
}

Object* Lambda::operator()(Object* obj, NameSpace* scope) {
    auto args = ToVector(obj);
    RequiresOnlyXArguments(args, arg_names_.size());

    // args = CopyAndCalcVector(args, scope);
    CalcVector(args, scope);
    auto newscope = scope_;
    for (size_t i = 0; i < arg_names_.size(); ++i) {
        DefineHelper(arg_names_[i], args[i], newscope);
    }
    auto copy_body = CopyAndCalcVector(body_, newscope);
    return copy_body.back();
}

Object* Lambda::Copy() {
    auto res = Heap::Instance()->Make<Lambda>();
    res->arg_names_ = CopyVector(arg_names_);
    res->body_ = CopyVector(body_);
    res->scope_ = As<NameSpace>(scope_->Copy());
    return res;
}

Object* CreateLambda::operator()(Object* obj, NameSpace* scope) {
    auto body = ToVector(obj);
    RequiresMinimumXArgumentsS(body, 2);
    auto args = ToVector(body.front());
    body.erase(body.begin());
    RequiresMinimumXArgumentsS(body, 1);
    return CreateLambdaHelper(args, body, scope);
}
