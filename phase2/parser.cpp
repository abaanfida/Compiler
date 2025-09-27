#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include "lexer.h"
#include "parser_error.h"   

using namespace std;


struct ASTNode 
{
    virtual ~ASTNode() = default;
    virtual void print(ostream &os, int indent = 0) const = 0;
};

using AST = shared_ptr<ASTNode>;

static string indentStr(int n) 
{ 
    return string(n*2, ' '); 
}

struct ProgramNode : ASTNode 
{
    vector<AST> items;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Program\n";
        for (auto &it : items) it->print(os, indent+1);
    }
};

struct BlockNode : ASTNode 
{
    vector<AST> stmts;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Block\n";
        for (auto &s : stmts) s->print(os, indent+1);
    }
};

struct FunctionNode : ASTNode 
{
    string retType;
    string name;
    vector<pair<string,string>> params;
    shared_ptr<BlockNode> body;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Function " << name << " : " << retType << "\n";
        os << indentStr(indent+1) << "Params\n";
        for (auto &p: params)
            os << indentStr(indent+2) << p.first << " " << p.second << "\n";
        body->print(os, indent+1);
    }
};

struct VarDeclNode : ASTNode 
{
    string typeName;
    string name;
    AST init;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "VarDecl " << typeName << " " << name;
        if (init) 
        { 
            os << " =\n"; 
            init->print(os, indent+1); 
        }
        else 
        {
            os << "\n";
        }
    }
};

struct ReturnNode : ASTNode 
{
    AST expr;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Return\n";
        if (expr) expr->print(os, indent+1);
    }
};

struct IfNode : ASTNode 
{
    AST cond;
    shared_ptr<BlockNode> thenBlock;
    shared_ptr<BlockNode> elseBlock;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "If\n";
        os << indentStr(indent+1) << "Cond\n"; cond->print(os, indent+2);
        os << indentStr(indent+1) << "Then\n"; thenBlock->print(os, indent+2);
        if (elseBlock) 
        {
            os << indentStr(indent+1) << "Else\n"; elseBlock->print(os, indent+2);
        }
    }
};

struct WhileNode : ASTNode 
{
    AST cond;
    shared_ptr<BlockNode> body;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "While\n";
        os << indentStr(indent+1) << "Cond\n"; cond->print(os, indent+2);
        os << indentStr(indent+1) << "Body\n"; body->print(os, indent+2);
    }
};

struct ExprStmtNode : ASTNode 
{
    AST expr;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "ExprStmt\n";
        expr->print(os, indent+1);
    }
};

struct BinaryOpNode : ASTNode 
{
    string op;
    AST left, right;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "BinaryOp(" << op << ")\n";
        left->print(os, indent+1);
        right->print(os, indent+1);
    }
};

struct UnaryOpNode : ASTNode 
{
    string op;
    AST operand;
    bool postfix = false;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << (postfix ? "Postfix" : "Unary") << "Op(" << op << ")\n";
        operand->print(os, indent+1);
    }
};

struct LiteralNode : ASTNode 
{
    string kind;
    string value;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Literal " << kind << "(" << value << ")\n";
    }
};

struct IdentifierNode : ASTNode 
{
    string name;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Ident " << name << "\n";
    }
};

struct CallNode : ASTNode 
{
    AST callee;
    vector<AST> args;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Call\n";
        callee->print(os, indent+1);
        os << indentStr(indent+1) << "Args\n";
        for (auto &a: args) a->print(os, indent+2);
    }
};

struct AssignmentNode : ASTNode 
{
    AST left;
    string op;
    AST right;
    void print(ostream &os, int indent = 0) const override 
    {
        os << indentStr(indent) << "Assign(" << op << ")\n";
        left->print(os, indent+1);
        right->print(os, indent+1);
    }
};




class Parser 
{
    lexer lx;
    token cur;

public:
    Parser(const string &src): lx(src) 
    { 
        advance(); 
    }

    void advance() 
    {
        while (true) 
        {
            cur = lx.getNextToken();
            if (cur.type == T_COMMENT) continue;
            return;
        }
    }

    void expect(tokenType t, ParseError::Kind errKind) 
    {
        if (cur.type != t) 
        {
            if (cur.type == T_EOF) throw ParseError(ParseError::UnexpectedEOF, cur);
            throw ParseError(errKind, cur);
        }
        advance();
    }

    shared_ptr<ProgramNode> parseProgram() 
    {
        auto prog = make_shared<ProgramNode>();
        while (cur.type != T_EOF) 
        {
            if (cur.type == T_FUNCTION) prog->items.push_back(parseFunction());
            else prog->items.push_back(parseStatementOrDecl());
        }
        return prog;
    }

    string parseTypeName() 
    {
        if (cur.type == T_INT || cur.type == T_FLOAT || cur.type == T_BOOL || cur.type == T_STRING) 
        {
            string s = cur.value; advance(); return s;
        }
        throw ParseError(ParseError::ExpectedTypeToken, cur);
    }

    AST parseFunction() 
    {
        expect(T_FUNCTION, ParseError::FailedToFindToken);
        string ret = parseTypeName();
        if (cur.type != T_IDENTIFIER) throw ParseError(ParseError::ExpectedIdentifier, cur);
        string fname = cur.value; advance();
        expect(T_PARENL, ParseError::FailedToFindToken);
        vector<pair<string,string>> params;
        if (cur.type != T_PARENR) 
        {
            while (true) 
            {
                string ptype = parseTypeName();
                if (cur.type != T_IDENTIFIER) throw ParseError(ParseError::ExpectedIdentifier, cur);
                string pname = cur.value; advance();
                params.push_back({ptype,pname});
                if (cur.type == T_COMMA) 
                { 
                    advance(); 
                    continue; 
                }
                break;
            }
        }
        expect(T_PARENR, ParseError::FailedToFindToken);
        auto body = parseBlock();
        auto fn = make_shared<FunctionNode>();
        fn->retType = ret; fn->name = fname; fn->params = params; fn->body = body;
        return fn;
    }

    shared_ptr<BlockNode> parseBlock() 
    {
        expect(T_BRACEL, ParseError::FailedToFindToken);
        auto block = make_shared<BlockNode>();
        while (cur.type != T_BRACER && cur.type != T_EOF)
            block->stmts.push_back(parseStatementOrDecl());
        expect(T_BRACER, ParseError::FailedToFindToken);
        return block;
    }

    AST parseStatementOrDecl() 
    {
        if (cur.type == T_INT || cur.type == T_FLOAT || cur.type == T_BOOL || cur.type == T_STRING) 
        {
            string tname = parseTypeName();
            if (cur.type != T_IDENTIFIER) throw ParseError(ParseError::ExpectedIdentifier, cur);
            string name = cur.value; advance();
            AST init = nullptr;
            if (cur.type == T_ASSIGNOP) 
            { 
                advance(); 
                init = parseExpression(); 
            }
            expect(T_SEMICOLON, ParseError::FailedToFindToken);
            auto v = make_shared<VarDeclNode>();
            v->typeName = tname; v->name = name; v->init = init; return v;
        }
        if (cur.type == T_IF) return parseIf();
        if (cur.type == T_WHILE) return parseWhile();
        if (cur.type == T_RETURN) 
        {
            advance();
            AST expr = nullptr;
            if (cur.type != T_SEMICOLON) expr = parseExpression();
            expect(T_SEMICOLON, ParseError::FailedToFindToken);
            auto r = make_shared<ReturnNode>(); r->expr = expr; return r;
        }
        if (cur.type == T_BRACEL) return parseBlock();

        AST e = parseExpression();
        expect(T_SEMICOLON, ParseError::FailedToFindToken);
        auto es = make_shared<ExprStmtNode>(); es->expr = e; return es;
    }

    AST parseIf() 
    {
        expect(T_IF, ParseError::FailedToFindToken);
        expect(T_PARENL, ParseError::FailedToFindToken);
        AST cond = parseExpression();
        expect(T_PARENR, ParseError::FailedToFindToken);
        auto thenB = parseBlock();
        shared_ptr<BlockNode> elseB = nullptr;
        if (cur.type == T_ELSE) 
        {
            advance();
            if (cur.type == T_BRACEL) elseB = parseBlock();
            else 
            {
                auto tmp = make_shared<BlockNode>();
                tmp->stmts.push_back(parseStatementOrDecl());
                elseB = tmp;
            }
        }
        auto n = make_shared<IfNode>(); n->cond = cond; n->thenBlock = thenB; n->elseBlock = elseB; return n;
    }

    AST parseWhile() 
    {
        expect(T_WHILE, ParseError::FailedToFindToken);
        expect(T_PARENL, ParseError::FailedToFindToken);
        AST cond = parseExpression();
        expect(T_PARENR, ParseError::FailedToFindToken);
        auto body = parseBlock();
        auto n = make_shared<WhileNode>(); n->cond = cond; n->body = body; return n;
    }

    AST parseExpression() 
    {
        if (cur.type == T_EOF) throw ParseError(ParseError::ExpectedExpr, cur);
        return parseAssignment();
    }

    AST parseAssignment() 
    {
        AST left = parseLogicalOr();
        if (cur.type == T_ASSIGNOP || cur.type == T_PLUS_ASSIGN || cur.type == T_MINUS_ASSIGN ||
            cur.type == T_MUL_ASSIGN || cur.type == T_DIV_ASSIGN) 
        {
            string op = cur.value.empty() ? tokenTypeToString(cur.type, cur.value) : cur.value;
            if (cur.type == T_ASSIGNOP) op = "=";
            else if (cur.type == T_PLUS_ASSIGN) op = "+=";
            else if (cur.type == T_MINUS_ASSIGN) op = "-=";
            else if (cur.type == T_MUL_ASSIGN) op = "*=";
            else if (cur.type == T_DIV_ASSIGN) op = "/=";
            advance();
            AST right = parseAssignment();
            auto an = make_shared<AssignmentNode>(); an->left = left; an->op = op; an->right = right; return an;
        }
        return left;
    }

    AST parseLogicalOr() 
    {
        AST node = parseLogicalAnd();
        while (cur.type == T_OR) 
        {
            string op = "||"; advance();
            AST rhs = parseLogicalAnd();
            auto bn = make_shared<BinaryOpNode>(); bn->op = op; bn->left = node; bn->right = rhs; node = bn;
        }
        return node;
    }

    AST parseLogicalAnd() 
    {
        AST node = parseEquality();
        while (cur.type == T_AND) 
        {
            string op = "&&"; advance();
            AST rhs = parseEquality();
            auto bn = make_shared<BinaryOpNode>(); bn->op = op; bn->left = node; bn->right = rhs; node = bn;
        }
        return node;
    }

    AST parseEquality() 
    {
        AST node = parseRelational();
        while (cur.type == T_EQUALSOP || cur.type == T_NOTEQOP) 
        {
            string op = cur.value; advance();
            AST rhs = parseRelational();
            auto bn = make_shared<BinaryOpNode>(); bn->op = op; bn->left = node; bn->right = rhs; node = bn;
        }
        return node;
    }

    AST parseRelational() 
    {
        AST node = parseAdditive();
        while (cur.type == T_LESSOP || cur.type == T_GREATOP || cur.type == T_LEQOP || cur.type == T_GEQOP) 
        {
            string op = cur.value; advance();
            AST rhs = parseAdditive();
            auto bn = make_shared<BinaryOpNode>(); bn->op = op; bn->left = node; bn->right = rhs; node = bn;
        }
        return node;
    }

    AST parseAdditive() 
    {
        AST node = parseMultiplicative();
        while (cur.type == T_PLUS || cur.type == T_MINUS) 
        {
            string op = cur.value; advance();
            AST rhs = parseMultiplicative();
            auto bn = make_shared<BinaryOpNode>(); bn->op = op; bn->left = node; bn->right = rhs; node = bn;
        }
        return node;
    }

    AST parseMultiplicative() 
    {
        AST node = parseUnary();
        while (cur.type == T_MUL || cur.type == T_DIV) 
        {
            string op = cur.value; advance();
            AST rhs = parseUnary();
            auto bn = make_shared<BinaryOpNode>(); bn->op = op; bn->left = node; bn->right = rhs; node = bn;
        }
        return node;
    }

    AST parseUnary() 
    {
        if (cur.type == T_PLUS || cur.type == T_MINUS) 
        {
            string op = cur.value; advance();
            AST operand = parseUnary();
            auto un = make_shared<UnaryOpNode>(); un->op = op; un->operand = operand; un->postfix = false; return un;
        }
        if (cur.type == T_INCREMENT || cur.type == T_DECREMENT) 
        {
            string op = cur.value; advance();
            AST operand = parseUnary();
            auto un = make_shared<UnaryOpNode>(); un->op = op; un->operand = operand; un->postfix = false; return un;
        }
        return parsePostfix();
    }

    AST parsePostfix() 
    {
        AST node = parsePrimary();
        while (true) 
        {
            if (cur.type == T_PARENL) 
            {
                advance();
                vector<AST> args;
                if (cur.type != T_PARENR) 
                {
                    while (true) 
                    {
                        args.push_back(parseExpression());
                        if (cur.type == T_COMMA) 
                        { 
                            advance(); 
                            continue; 
                        }
                        break;
                    }
                }
                expect(T_PARENR, ParseError::FailedToFindToken);
                auto cn = make_shared<CallNode>(); cn->callee = node; cn->args = args; node = cn;
                continue;
            }
            if (cur.type == T_INCREMENT || cur.type == T_DECREMENT) 
            {
                string op = cur.value; advance();
                auto un = make_shared<UnaryOpNode>(); un->op = op; un->operand = node; un->postfix = true; node = un;
                continue;
            }
            break;
        }
        return node;
    }

    AST parsePrimary() 
    {
        if (cur.type == T_IDENTIFIER) 
        {
            auto id = make_shared<IdentifierNode>(); id->name = cur.value; advance(); return id;
        }
        if (cur.type == T_INTLIT) 
        {
            auto lit = make_shared<LiteralNode>(); lit->kind = "int"; lit->value = cur.value; advance(); return lit;
        }
        if (cur.type == T_FLOATLIT) 
        {
            auto lit = make_shared<LiteralNode>(); lit->kind = "float"; lit->value = cur.value; advance(); return lit;
        }
        if (cur.type == T_STRINGLIT) 
        {
            auto lit = make_shared<LiteralNode>(); lit->kind = "string"; lit->value = cur.value; advance(); return lit;
        }
        if (cur.type == T_BOOLLIT) 
        {
            auto lit = make_shared<LiteralNode>(); lit->kind = "bool"; lit->value = cur.value; advance(); return lit;
        }
        if (cur.type == T_PARENL) 
        {
            advance();
            AST e = parseExpression();
            expect(T_PARENR, ParseError::FailedToFindToken);
            return e;
        }

        
        switch (cur.type) 
        {
        case T_INT: throw ParseError(ParseError::ExpectedIntLit, cur);
        case T_FLOAT: throw ParseError(ParseError::ExpectedFloatLit, cur);
        case T_STRING: throw ParseError(ParseError::ExpectedStringLit, cur);
        case T_BOOL: throw ParseError(ParseError::ExpectedBoolLit, cur);
        case T_EOF: throw ParseError(ParseError::UnexpectedEOF, cur);
        default: throw ParseError(ParseError::UnexpectedToken, cur);
        }
    }
};




int main() 
{
    string code = R"(
        fn int my_fn(int x, float y) {
            string my_str = "hmm\n";
            int my_bool = true;   
            if (x != 0 && y >= 2.5) {
                return x
            } else {
                return "oops"
            }
        }
    )";

    try 
    {
        Parser p(code);
        auto prog = p.parseProgram();
        prog->print(cout, 0);
    } 
    catch (const ParseError &err) 
    {
        cerr << "Parse error: " << err.message() << endl;
    }
    return 0;
}