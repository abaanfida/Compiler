#pragma once
#include <string>
using namespace std;


enum tokenType 
{
    T_FUNCTION, T_INT, T_FLOAT, T_BOOL, T_STRING,
    T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
    T_IDENTIFIER, T_INTLIT, T_FLOATLIT, T_STRINGLIT, T_BOOLLIT,
    T_ASSIGNOP, T_EQUALSOP, T_NOTEQOP, T_LESSOP, T_GREATOP, T_LEQOP, T_GEQOP,
    T_AND, T_OR, T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_PLUS_ASSIGN, T_MINUS_ASSIGN, T_MUL_ASSIGN, T_DIV_ASSIGN,
    T_INCREMENT, T_DECREMENT,
    T_PARENL, T_PARENR, T_BRACEL, T_BRACER, T_BRACKL, T_BRACKR,
    T_COMMA, T_SEMICOLON, T_QUOTES,
    T_COMMENT,
    T_INVALID, T_EOF
};


struct token 
{
    tokenType type;
    string value;
};


class lexer 
{
    string src;
    int pos;
public:
    lexer(const std::string& source);
    bool isEOF();
    void skipWhitespace();
    char peek();
    char advance();
    token identifierOrKeyword();
    token number();
    token stringLiteral();
    token comment();
    token getNextToken();
};


std::string tokenTypeToString(tokenType type, const string& val = "");
