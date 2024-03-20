#pragma once

#include "object.h"
#include "tokenizer.h"

Object* ReadList(Tokenizer* tokenizer);

Object* Read(Tokenizer* tokenizer);
