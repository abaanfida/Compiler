#pragma once
#include <string>
#include "lexer.h"
using namespace std;
struct ParseError 
{
    enum Kind {
        UnexpectedEOF,
        FailedToFindToken,
        ExpectedTypeToken,
        ExpectedIdentifier,
        UnexpectedToken,
        ExpectedFloatLit,
        ExpectedIntLit,
        ExpectedStringLit,
        ExpectedBoolLit,
        ExpectedExpr
    } kind;

    token offending;

    ParseError(Kind k, token t = {T_INVALID, ""})
        : kind(k), offending(t) {}

    string message() const
     {
        switch (kind) {
        case UnexpectedEOF: return "Unexpected end of file";
        case FailedToFindToken: return "Failed to find expected token";
        case ExpectedTypeToken: return "Expected type token";
        case ExpectedIdentifier: return "Expected identifier";
        case UnexpectedToken: return "Unexpected token: " + tokenTypeToString(offending.type, offending.value);
        case ExpectedFloatLit: return "Expected float literal";
        case ExpectedIntLit: return "Expected int literal";
        case ExpectedStringLit: return "Expected string literal";
        case ExpectedBoolLit: return "Expected bool literal";
        case ExpectedExpr: return "Expected expression";
        }
        return "Unknown parse error";
    }
};
