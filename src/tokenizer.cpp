#include "tokenizer.h"

#include "error.h"

bool MaybeGoodChar(int c) {
    return std::isgraph(c) && c != '(' && c != ')' && c != '\'' && c != '.';
}

bool IsSymbol(const std::string& str) {
    return str.find('#') == std::string::npos && str.find('"') == std::string::npos;
}

void Tokenizer::Next() {
    DelSpaces();
    if (in_->eof()) {
        cur_token_ = DummyToken();
        return;
    }
    char c = in_->get();
    if (c == '(') {
        cur_token_ = BracketToken::OPEN;
    } else if (c == ')') {
        cur_token_ = BracketToken::CLOSE;
    } else if (c == '.') {
        cur_token_ = DotToken();
    } else if (c == '\'') {
        cur_token_ = QuoteToken();
    } else {
        std::string s;
        s += c;

        if (((c == '+' || c == '-') && isdigit(in_->peek())) || std::isdigit(c)) {
            while (std::isdigit(in_->peek())) {
                s += in_->get();
            }
            cur_token_ = ConstantToken(std::stoll(s));
        } else {

            while (MaybeGoodChar(in_->peek())) {
                s += in_->get();
            }
            if (s == "#t") {
                cur_token_ = BooleanToken(true);
            } else if (s == "#f") {
                cur_token_ = BooleanToken(false);
            } else if (IsSymbol(s)) {
                cur_token_ = SymbolToken(s);
            } else {
                throw SyntaxError("Illegal character " + s);
            }
        }
    }
}

Token Tokenizer::GetToken() {
    if (IsEnd()) {
        throw SyntaxError("No token, but expected");
    }
    return cur_token_;
}

void Tokenizer::DelSpaces() {
    while (std::isspace(in_->peek())) {
        in_->get();
    }
}