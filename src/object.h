#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "error.h"

class Heap;

class Object {
public:
    virtual ~Object() = default;

    virtual Object* Copy() = 0;

    friend class Heap;

    virtual void Mark() {
        used_ = true;
    }

    void UnMark() {
        used_ = false;
    }

    bool GetMark() {
        return used_;
    }

protected:
    bool used_ = false;
};

class Heap {
public:
    template <typename T, typename... Args>
        requires std::is_base_of_v<Object, T>
    T* Make(Args&&... args) {
        int index = data_.size();
        data_.emplace_back(nullptr);
        data_[index] = std::move(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
        return As<T>(data_[index].get());
    }

    static Heap* Instance() {
        static Heap instance;
        return &instance;
    }

    void Clear() {
        data_.clear();
    }

    size_t Size() {
        return data_.size();
    }

    void RemoveTrash(Object* start) {
        for (auto& e : data_) {
            e->UnMark();
        }

        start->Mark();

        for (size_t i = 0; i < data_.size(); ++i) {
            while (i < data_.size() && (data_[i] == nullptr || !data_[i]->GetMark())) {
                std::swap(data_[i], data_.back());
                data_.pop_back();
            }
        }
    }

private:
    std::vector<std::unique_ptr<Object>> data_;
};

class Number : public Object {
public:
    Number(int64_t val) : value_(val) {
    }

    int64_t& GetValue() {
        return value_;
    }

    int64_t GetValue() const {
        return value_;
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Number>(value_);
    }

private:
    int64_t value_;
};

class Boolean : public Object {
public:
    Boolean(bool val) : value_(val) {
    }

    bool& GetValue() {
        return value_;
    }

    bool GetValue() const {
        return value_;
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Boolean>(value_);
    }

private:
    bool value_;
};

class Symbol : public Object {
public:
    Symbol(const std::string& name) : name_(name) {
    }

    const std::string& GetName() const {
        return name_;
    }

    std::string& GetName() {
        return name_;
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Symbol>(name_);
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell(Object* first, Object* second) : first_(first), second_(second) {
    }

    Object*& GetFirst() {
        return first_;
    }

    Object*& GetSecond() {
        return second_;
    }

    Object* Copy() override;

    void Mark() override {
        used_ = true;
        if (first_ != nullptr && !first_->GetMark()) {
            first_->Mark();
        }
        if (second_ != nullptr && !second_->GetMark()) {
            second_->Mark();
        }
    }

private:
    Object *first_, *second_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

// template <class T>
// std::shared_ptr<T> As(Object* obj) {
//     return std::dynamic_pointer_cast<T>(obj);
// }

// template <class T>
// bool Is(Object* obj) {
//     return As<T>(obj) != nullptr;
// }

template <class T>
T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
}

template <class T>
bool Is(Object* obj) {
    return As<T>(obj) != nullptr;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void RequireType(Object* obj) {
    if (!Is<T>(obj)) {
        throw RuntimeError("Require different argument type");
    }
}

///////////////////////////////////////////////////////////////////////////////

class NameSpace : public Object {
public:
    NameSpace(NameSpace* upper = nullptr) : upper_(upper) {
    }

    Object*& Get(const std::string& name);

    void Set(const std::string& name, Object* obj);

    Object* Copy() override {
        auto res = Heap::Instance()->Make<NameSpace>(upper_);
        for (auto [key, value_] : data_) {
            res->Set(key, value_->Copy());
        }
        return res;
    }

    void Mark() override {
        used_ = true;
        if (upper_ != nullptr) {
            upper_->Mark();
        }
        for (auto [key, value] : data_) {
            if (value != nullptr && !value->GetMark()) {
                value->Mark();
            }
        }
    }

private:
    std::unordered_map<std::string, Object*> data_;
    NameSpace* upper_;
};

// Functors

class Functor : public Object {
public:
    virtual Object* operator()(Object* obj, NameSpace* scope) = 0;

    virtual std::string GetFunctorName() const = 0;
};

class Quote : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[quote]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Quote>();
    }
};

class IsPair : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[pair?]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<IsPair>();
    }
};

class IsNull : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[null?]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<IsNull>();
    }
};

class IsList : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[list?]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<IsList>();
    }
};

class List : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[list]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<List>();
    }
};

class Cons : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[cons]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Cons>();
    }
};

class Car : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[car]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Car>();
    }
};

class Cdr : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[cdr]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Cdr>();
    }
};

class ListRef : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[list-ref]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<ListRef>();
    }
};

class ListTail : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[list-tail]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<ListTail>();
    }
};

class IsNumber : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[number?]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<IsNumber>();
    }
};

class EqualTo : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[=]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<EqualTo>();
    }
};

class Greater : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[>]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Greater>();
    }
};

class Less : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[<]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Less>();
    }
};

class GreaterEqual : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[>=]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<GreaterEqual>();
    }
};

class LessEqual : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[<=]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<LessEqual>();
    }
};

class Plus : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[+]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Plus>();
    }
};

class Minus : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[-]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Minus>();
    }
};

class Multiplies : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[*]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Multiplies>();
    }
};

class Divides : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[/]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Divides>();
    }
};

class Max : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[max]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Max>();
    }
};

class Min : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[min]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Min>();
    }
};

class Abs : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[abs]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Abs>();
    }
};

class IsBoolean : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[boolean?]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<IsBoolean>();
    }
};

class Not : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[not]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Not>();
    }
};

class And : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[and]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<And>();
    }
};

class Or : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[or]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Or>();
    }
};

// Advanced

class Define : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[define]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Define>();
    }
};

class IsSymbol : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[symbol?]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<IsSymbol>();
    }
};

class Set : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[set!]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<Set>();
    }
};

class SetCar : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[set-car!]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<SetCar>();
    }
};

class SetCdr : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[set-cdr!]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<SetCdr>();
    }
};

class If : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[if]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<If>();
    }
};

class Lambda : public Functor {
public:
    Lambda() : arg_names_(), body_(), scope_(nullptr) {
    }

    Lambda(std::vector<Object*>& args, std::vector<Object*>& body, NameSpace* scope)
        : arg_names_(args), body_(body), scope_(Heap::Instance()->Make<NameSpace>(scope)) {
    }

    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[create-lambda]";
    }

    Object* Copy() override;

    void Mark() override {
        used_ = true;
        for (auto& e : arg_names_) {
            if (e != nullptr && !e->GetMark()) {
                e->Mark();
            }
        }
        for (auto& e : body_) {
            if (e != nullptr && !e->GetMark()) {
                e->Mark();
            }
        }
        if (scope_ != nullptr && !scope_->GetMark()) {
            scope_->Mark();
        }
    }

private:
    std::vector<Object*> arg_names_, body_;
    NameSpace* scope_;
};

class CreateLambda : public Functor {
public:
    Object* operator()(Object* obj, NameSpace* scope) override;

    std::string GetFunctorName() const override {
        return "[create-lambda]";
    }

    Object* Copy() override {
        return Heap::Instance()->Make<CreateLambda>();
    }
};
