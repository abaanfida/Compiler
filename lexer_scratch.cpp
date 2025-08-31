#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
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
    lexer(const string& source)
    {
        src = source;
        pos = 0;
    }

    bool isEOF()
    {
        return pos >= src.size();
    }

    void skipWhitespace()
    {
        while (!isEOF() && isspace(src[pos]))
            pos++;
    }

    char peek()
    {
        return isEOF() ? '\0' : src[pos];
    }

    char advance()
    {
        return isEOF() ? '\0' : src[pos++];
    }

    token identifierOrKeyword()
    {
        int start = pos;
        while (!isEOF() && (isalnum(peek()) || peek() == '_'))
            advance();
        string val = src.substr(start, pos - start);

        if (val == "fn") return {T_FUNCTION, val};
        if (val == "int") return {T_INT, val};
        if (val == "float") return {T_FLOAT, val};
        if (val == "bool") return {T_BOOL, val};
        if (val == "string") return {T_STRING, val};
        if (val == "if") return {T_IF, val};
        if (val == "else") return {T_ELSE, val};
        if (val == "while") return {T_WHILE, val};
        if (val == "for") return {T_FOR, val};
        if (val == "return") return {T_RETURN, val};
        if (val == "true" || val == "false") return {T_BOOLLIT, val};

        return {T_IDENTIFIER, val};
    }

    token number()
    {
        int start = pos;

        while (!isEOF() && isdigit(peek()))
            advance();

        bool isFloat = false;
        if (!isEOF() && peek() == '.')
        {
            isFloat = true;
            advance();
            while (!isEOF() && isdigit(peek()))
                advance();
        }

        if (!isEOF() && (isalpha(peek()) || peek() == '_'))
        {
            int errStart = start;
            while (!isEOF() && (isalnum(peek()) || peek() == '_'))
                advance();
            string invalidVal = src.substr(errStart, pos - errStart);
            throw runtime_error("Invalid identifier: '" + invalidVal + "'");
        }


        string val = src.substr(start, pos - start);
        return isFloat ? token{T_FLOATLIT, val} : token{T_INTLIT, val};
    }


    token stringLiteral()
    {
        advance();
        int start = pos;
        while (!isEOF() && peek() != '"')
        {
            if (peek() == '\\') advance();
            advance();
        }
        if (isEOF()) throw runtime_error("Unterminated string literal");
        string val = src.substr(start, pos - start);
        advance();
        return {T_STRINGLIT, val};
    }

    token comment()
    {
        advance();
        if (peek() == '/')
        {
            while (!isEOF() && peek() != '\n')
                advance();
            return {T_COMMENT, ""};
        }
        else if (peek() == '*')
        {
            advance();
            while (!isEOF())
            {
                if (peek() == '*' && pos + 1 < src.size() && src[pos + 1] == '/')
                {
                    pos += 2;
                    return {T_COMMENT, ""};
                }
                advance();
            }
            throw runtime_error("Unterminated block comment");
        }
        return {T_DIV, "/"};
    }

    token getNextToken()
    {
        skipWhitespace();

        if (isEOF())
            return {T_EOF, ""};

        char c = peek();

        if (isalpha(c) || c == '_')
            return identifierOrKeyword();

        if (isdigit(c))
            return number();

        if (c == '"')
            return stringLiteral();

        if (c == '/')
            return comment();

        if (c == '=' && pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_EQUALSOP, "=="};
        }

        if (c == '!' && pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_NOTEQOP, "!="};
        }

        if (c == '<' && pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_LEQOP, "<="};
        }

        if (c == '>' && pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_GEQOP, ">="};
        }

        if (c == '&' && pos + 1 < src.size() && src[pos + 1] == '&')
        {
            pos += 2;
            return {T_AND, "&&"};
        }

        if (c == '|' && pos + 1 < src.size() && src[pos + 1] == '|')
        {
            pos += 2;
            return {T_OR, "||"};
        }

        if (c == '+')
        {
            if (pos + 1 < src.size() && src[pos + 1] == '+')
            {
                pos += 2;
                return {T_INCREMENT, "++"};
            }
            if (pos + 1 < src.size() && src[pos + 1] == '=')
            {
                pos += 2;
                return {T_PLUS_ASSIGN, "+="};
            }
            advance();
            return {T_PLUS, "+"};
        }
    
        if (c == '-')
        {
            if (pos + 1 < src.size() && src[pos + 1] == '-')
            {
                pos += 2;
                return {T_DECREMENT, "--"};
            }
            if (pos + 1 < src.size() && src[pos + 1] == '=')
            {
                pos += 2;
                return {T_MINUS_ASSIGN, "-="};
            }
            advance();
            return {T_MINUS, "-"};
        }
    
        if (c == '*')
        {
            if (pos + 1 < src.size() && src[pos + 1] == '=')
            {
                pos += 2;
                return {T_MUL_ASSIGN, "*="};
            }
            advance();
            return {T_MUL, "*"};
        }
    
        if (c == '/')
        {
            if (pos + 1 < src.size() && src[pos + 1] == '=')
            {
                pos += 2;
                return {T_DIV_ASSIGN, "/="};
            }
            
            return comment();
        }
    

        switch (c)
        {
            case '=': advance(); return {T_ASSIGNOP, "="};
            case '<': advance(); return {T_LESSOP, "<"};
            case '>': advance(); return {T_GREATOP, ">"};
            case '+': advance(); return {T_PLUS, "+"};
            case '-': advance(); return {T_MINUS, "-"};
            case '*': advance(); return {T_MUL, "*"};
            case '(': advance(); return {T_PARENL, "("};
            case ')': advance(); return {T_PARENR, ")"};
            case '{': advance(); return {T_BRACEL, "{"};
            case '}': advance(); return {T_BRACER, "}"};
            case '[': advance(); return {T_BRACKL, "["};
            case ']': advance(); return {T_BRACKR, "]"};
            case ',': advance(); return {T_COMMA, ","};
            case ';': advance(); return {T_SEMICOLON, ";"};
            case '"': advance(); return {T_QUOTES, "\""};
        }

        throw runtime_error("Unknown token starting at: " + string(1, c));
    }

};

string tokenTypeToString(tokenType type, const string& val = "")
{
    switch (type)
    {
    case T_FUNCTION: return "T_FUNCTION";
    case T_INT: return "T_INT";
    case T_FLOAT: return "T_FLOAT";
    case T_BOOL: return "T_BOOL";
    case T_STRING: return "T_STRING";
    case T_IF: return "T_IF";
    case T_ELSE: return "T_ELSE";
    case T_WHILE: return "T_WHILE";
    case T_FOR: return "T_FOR";
    case T_RETURN: return "T_RETURN";
    case T_IDENTIFIER: return "T_IDENTIFIER(\"" + val + "\")";
    case T_INTLIT: return "T_INTLIT(" + val + ")";
    case T_FLOATLIT: return "T_FLOATLIT(" + val + ")";
    case T_STRINGLIT: return "T_STRINGLIT(" + val + ")";
    case T_BOOLLIT: return "T_BOOLLIT(" + val + ")";
    case T_ASSIGNOP: return "T_ASSIGNOP";
    case T_EQUALSOP: return "T_EQUALSOP";
    case T_NOTEQOP: return "T_NOTEQOP";
    case T_LESSOP: return "T_LESSOP";
    case T_GREATOP: return "T_GREATOP";
    case T_LEQOP: return "T_LEQOP";
    case T_GEQOP: return "T_GEQOP";
    case T_AND: return "T_AND";
    case T_OR: return "T_OR";
    case T_PLUS: return "T_PLUS";
    case T_MINUS: return "T_MINUS";
    case T_MUL: return "T_MUL";
    case T_DIV: return "T_DIV";
    case T_PARENL: return "T_PARENL";
    case T_PARENR: return "T_PARENR";
    case T_BRACEL: return "T_BRACEL";
    case T_BRACER: return "T_BRACER";
    case T_BRACKL: return "T_BRACKL";
    case T_BRACKR: return "T_BRACKR";
    case T_COMMA: return "T_COMMA";
    case T_SEMICOLON: return "T_SEMICOLON";
    case T_QUOTES: return "T_QUOTES";
    case T_COMMENT: return "T_COMMENT";
    case T_INVALID: return "T_INVALID";
    case T_PLUS_ASSIGN: return "T_PLUS_ASSIGN";
    case T_MINUS_ASSIGN: return "T_MINUS_ASSIGN";
    case T_MUL_ASSIGN: return "T_MUL_ASSIGN";
    case T_DIV_ASSIGN: return "T_DIV_ASSIGN";
    case T_INCREMENT: return "T_INCREMENT";
    case T_DECREMENT: return "T_DECREMENT";
    case T_EOF: return "T_EOF";
    }
    return "UNKNOWN";
}

int main()
{
    string code = R"(
        fn int my_fn(int x, float y) {
            string my_str = "hmm\n";
            bool 1my_bool =1 x === 40;
            if (x != 0 && y >= 2.5) {
                y++;
                return x;
            }
        }
    )";

    lexer lexer(code);
    try
    {
        while (true)
        {
            token t = lexer.getNextToken();
            if (t.type == T_EOF) break;
            cout << tokenTypeToString(t.type, t.value) << endl;
        }
    }
    catch (exception& e)
    {
        cerr << "Lexer error: " << e.what() << endl;
    }
    return 0;
}
