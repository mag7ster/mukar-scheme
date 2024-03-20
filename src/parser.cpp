#include "parser.h"

#include <vector>

#include "error.h"
#include "object.h"

Object* Read(Tokenizer* tokenizer) {
    Heap* storage = Heap::Instance();
    auto cur_token = tokenizer->GetToken();
    tokenizer->Next();
    if (std::get_if<BracketToken>(&cur_token)) {
        if (std::get<BracketToken>(cur_token) == BracketToken::OPEN) {
            return ReadList(tokenizer);
        } else {
            throw SyntaxError("Close bracket unexpected");
        }
    } else if (std::get_if<QuoteToken>(&cur_token)) {
        auto first = storage->Make<Symbol>("quote");
        auto obj = Read(tokenizer);
        auto second = storage->Make<Cell>(obj, nullptr);
        return storage->Make<Cell>(first, second);
    } else if (std::get_if<DotToken>(&cur_token)) {
        throw SyntaxError("Dot unexpected");
    } else if (std::get_if<SymbolToken>(&cur_token)) {
        return storage->Make<Symbol>(std::get<SymbolToken>(cur_token).name);
    } else if (std::get_if<BooleanToken>(&cur_token)) {
        return storage->Make<Boolean>(std::get<BooleanToken>(cur_token).value);
    } else if (std::get_if<ConstantToken>(&cur_token)) {
        return storage->Make<Number>(std::get<ConstantToken>(cur_token).value);
    } else {
        throw SyntaxError("Undefined token type");
    }
}

Object* ReadList(Tokenizer* tokenizer) {
    Heap* storage = Heap::Instance();
    auto cur_token = tokenizer->GetToken();
    if (std::get_if<BracketToken>(&cur_token) &&
        std::get<BracketToken>(cur_token) == BracketToken::CLOSE) {
        tokenizer->Next();
        return nullptr;
    }
    bool is_pair = false;
    std::vector<Object*> list;
    list.push_back(Read(tokenizer));
    cur_token = tokenizer->GetToken();
    while (!(std::get_if<BracketToken>(&cur_token) &&
             std::get<BracketToken>(cur_token) == BracketToken::CLOSE)) {

        if (std::get_if<DotToken>(&cur_token)) {
            tokenizer->Next();
            is_pair = true;
            list.push_back(Read(tokenizer));
            cur_token = tokenizer->GetToken();
            break;
        }

        list.push_back(Read(tokenizer));
        cur_token = tokenizer->GetToken();
    }
    if ((!(std::get_if<BracketToken>(&cur_token) &&
           std::get<BracketToken>(cur_token) == BracketToken::CLOSE))) {

        throw SyntaxError("Close bracket expected");
    }
    tokenizer->Next();
    Object* right = nullptr;
    if (is_pair) {
        right = list.back();
        list.pop_back();
    }
    while (!list.empty()) {
        right = storage->Make<Cell>(list.back(), right);
        list.pop_back();
    }
    return right;
}