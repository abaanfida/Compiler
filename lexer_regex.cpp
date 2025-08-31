#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>

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
    T_INVALID, T_EOF,T_UNTERMINATED_STRING
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

    vector<pair<tokenType, regex>> tokenPatterns =
{
    {T_FUNCTION, regex("^fn\\b")},
    {T_INT, regex("^int\\b")},
    {T_FLOAT, regex("^float\\b")},
    {T_BOOL, regex("^bool\\b")},
    {T_STRING, regex("^string\\b")},
    {T_IF, regex("^if\\b")},
    {T_ELSE, regex("^else\\b")},
    {T_WHILE, regex("^while\\b")},
    {T_FOR, regex("^for\\b")},
    {T_RETURN, regex("^return\\b")},
    {T_BOOLLIT, regex("^(true|false)\\b")},
    {T_FLOATLIT, regex("^[0-9]+\\.[0-9]+")},
    {T_INVALID, regex("^[0-9][a-zA-Z_][a-zA-Z0-9_]*")},
    {T_INTLIT, regex("^[0-9]+")},
    {T_STRINGLIT, regex("^\"([^\"\\\\]|\\\\.)*\"")},
    {T_UNTERMINATED_STRING, regex("^\"([^\"\\\\]|\\\\.)*$")},
    {T_IDENTIFIER, regex("^[a-zA-Z_][a-zA-Z0-9_]*")},
    {T_EQUALSOP, regex("^==")},
    {T_NOTEQOP, regex("^!=")},
    {T_LEQOP, regex("^<=")},
    {T_GEQOP, regex("^>=")},
    {T_AND, regex("^&&")},
    {T_OR, regex("^\\|\\|")},
    {T_PLUS_ASSIGN, regex("^\\+=")},
    {T_MINUS_ASSIGN, regex("^-=")},
    {T_MUL_ASSIGN, regex("^\\*=")},
    {T_DIV_ASSIGN, regex("^/=")},
    {T_INCREMENT, regex("^\\+\\+")},
    {T_DECREMENT, regex("^--")},
    {T_ASSIGNOP, regex("^=")},
    {T_LESSOP, regex("^<")},
    {T_GREATOP, regex("^>")},
    {T_PLUS, regex("^\\+")},
    {T_MINUS, regex("^-")},
    {T_MUL, regex("^\\*")},
    {T_DIV, regex("^/")},
    {T_PARENL, regex("^\\(")},
    {T_PARENR, regex("^\\)")},
    {T_BRACEL, regex("^\\{")},
    {T_BRACER, regex("^\\}")},
    {T_BRACKL, regex("^\\[")},
    {T_BRACKR, regex("^\\]")},
    {T_COMMA, regex("^,")},
    {T_SEMICOLON, regex("^;")},
    {T_QUOTES, regex("^\"")},
    {T_COMMENT, regex("^(//.*|/\\*[^*]*\\*+(?:[^/*][^*]*\\*+)*/)")},
};


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

    token getNextToken() 
    {
        skipWhitespace();
        if (isEOF())
            return { T_EOF, "" };

        string rest = src.substr(pos);
        smatch bestMatch;
        tokenType bestType = T_INVALID;
        size_t bestLength = 0;
    
        for (auto& [type, pattern] : tokenPatterns) 
        {
            smatch match;
            if (regex_search(rest, match, pattern))
            {
                string val = match.str();
                if (val.size() > bestLength)
                {
                    bestLength = val.size();
                    bestMatch = match;
                    bestType = type;
                }
            }
        }
    
        if (bestLength > 0) 
        {
            pos += bestLength;
            if (bestType == T_COMMENT) return getNextToken();
            if( bestType == T_UNTERMINATED_STRING)
                throw runtime_error("Unterminated string literal starting at: " + rest.substr(0, 10));
            if (bestType == T_INVALID) 
                throw runtime_error("Invalid identifier: " + rest.substr(0, 10));
            return { bestType, bestMatch.str() };
        }
    
        throw runtime_error("Unknown token starting at: " + rest.substr(0, 10));
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
    case T_UNTERMINATED_STRING: return "T_UNTERMINATED_STRING";
    case T_EOF: return "T_EOF";
    }
    return "UNKNOWN";
}

int main()
{
    string code = R"(
        fn int my_fn(int x, float y) {
            string my_str = "hmm\n";
            bool my_bool = x == 40;
            if (x != 0 && y >= 2.5) {
                y+=20;
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
