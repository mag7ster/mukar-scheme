#pragma once

#include <string>
#include <variant>
#include <istream>

struct SymbolToken {
    std::string name;

    SymbolToken(const std::string& str) : name(str) {
    }

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int64_t value;

    ConstantToken(int64_t val) : value(val) {
    }

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

struct BooleanToken {
    bool value;

    BooleanToken(bool val) : value(val) {
    }

    bool operator==(const BooleanToken& other) const {
        return value == other.value;
    }
};

struct DummyToken {
    bool operator==(const DummyToken&) const {
        return true;
    }
};

using Token = std::variant<DummyToken, ConstantToken, BracketToken, SymbolToken, QuoteToken,
                           DotToken, BooleanToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in) : in_(in), cur_token_(DummyToken()) {
        Next();
    }

    bool IsEnd() {
        return std::get_if<DummyToken>(&cur_token_);
    }

    void Next();

    Token GetToken();

private:
    void DelSpaces();

    std::istream* in_;
    Token cur_token_;
};