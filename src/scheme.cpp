#include <sstream>
#include <string>
#include "error.h"
#include "object.h"
#include "parser.h"
#include "scheme.h"

Object* Copy(Object* object) {
    if (object == nullptr) {
        return nullptr;
    }
    return object->Copy();
}

Object* Calc(Object* object, NameSpace* scope) {
    if (object == nullptr) {
        RuntimeError("cant calc this cell");
    } else if (Is<Number>(object)) {
        return As<Number>(object);
    } else if (Is<Boolean>(object)) {
        return As<Boolean>(object);
    } else if (Is<Symbol>(object)) {
        auto res = scope->Get(As<Symbol>(object)->GetName());
        return res;
    } else if (Is<Cell>(object)) {
        auto func = Calc(As<Cell>(object)->GetFirst(), scope);
        auto args = (As<Cell>(object))->GetSecond();
        if (!Is<Functor>(func)) {
            throw RuntimeError("cant calc this cell");
        }
        func = func->Copy();
        return (*As<Functor>(func))(args, scope);
    }
    throw RuntimeError("Unknown object");
}

std::string Interpreter::Run(const std::string& str) {
    std::stringstream stream;
    stream << str;
    Tokenizer tokenizer(&stream);
    auto object = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Unexpected tokens");
    }
    std::string answer = GetString(Calc(object, global_namespace_));
    Heap::Instance()->RemoveTrash(global_namespace_);
    return answer;
}

std::string Interpreter::GetString(Object* object) {
    if (object == nullptr) {
        return "()";
    } else if (Is<Number>(object)) {
        return std::to_string(As<Number>(object)->GetValue());
    } else if (Is<Boolean>(object)) {
        if (As<Boolean>(object)->GetValue()) {
            return "#t";
        } else {
            return "#f";
        }
    } else if (Is<Symbol>(object)) {
        return As<Symbol>(object)->GetName();
    } else if (Is<Cell>(object)) {
        std::string res = "(";

        do {
            Cell* cell = As<Cell>(object);
            if (cell->GetFirst() == object) {
                res += "{selfref} ";
            } else {
                res += Interpreter::GetString(cell->GetFirst()) + " ";
            }

            object = cell->GetSecond();

        } while (Is<Cell>(object) && As<Cell>(object)->GetSecond() != object);

        if (object != nullptr) {
            if (Is<Cell>(object) && As<Cell>(object)->GetSecond() == object) {
                res += ". {selfref} ";
            } else {
                res += ". " + Interpreter::GetString(object) + " ";
            }
        }
        res.back() = ')';
        return res;
    } else if (Is<Functor>(object)) {
        return As<Functor>(object)->GetFunctorName();
    } else {
        throw RuntimeError("Unknown object");
    }
}
