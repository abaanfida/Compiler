#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include "lexer.h"
#include "parser_error.h"

using namespace std;


struct ASTNode;
using AST = shared_ptr<ASTNode>;


struct ASTNode 
{
    virtual ~ASTNode() = default;
    virtual void print(ostream &os, int indent = 0) const = 0;
};

string indentStr(int n);

struct ProgramNode : ASTNode 
{
    vector<AST> items;
    void print(ostream &os, int indent = 0) const override;
};

struct BlockNode : ASTNode 
{
    vector<AST> stmts;
    void print(ostream &os, int indent = 0) const override;
};

struct FunctionNode : ASTNode 
{
    string retType;
    string name;
    vector<pair<string,string>> params;
    shared_ptr<BlockNode> body;
    void print(ostream &os, int indent = 0) const override;
};

struct VarDeclNode : ASTNode 
{
    string typeName;
    string name;
    AST init;
    void print(ostream &os, int indent = 0) const override;
};

struct ReturnNode : ASTNode 
{
    AST expr;
    void print(ostream &os, int indent = 0) const override;
};

struct IfNode : ASTNode 
{
    AST cond;
    shared_ptr<BlockNode> thenBlock;
    shared_ptr<BlockNode> elseBlock;
    void print(ostream &os, int indent = 0) const override;
};

struct WhileNode : ASTNode 
{
    AST cond;
    shared_ptr<BlockNode> body;
    void print(ostream &os, int indent = 0) const override;
};

struct ExprStmtNode : ASTNode 
{
    AST expr;
    void print(ostream &os, int indent = 0) const override;
};

struct BinaryOpNode : ASTNode 
{
    string op;
    AST left, right;
    void print(ostream &os, int indent = 0) const override;
};

struct UnaryOpNode : ASTNode 
{
    string op;
    AST operand;
    bool postfix = false;
    void print(ostream &os, int indent = 0) const override;
};

struct LiteralNode : ASTNode 
{
    string kind;
    string value;
    void print(ostream &os, int indent = 0) const override;
};

struct IdentifierNode : ASTNode 
{
    string name;
    void print(ostream &os, int indent = 0) const override;
};

struct CallNode : ASTNode 
{
    AST callee;
    vector<AST> args;
    void print(ostream &os, int indent = 0) const override;
};

struct AssignmentNode : ASTNode 
{
    AST left;
    string op;
    AST right;
    void print(ostream &os, int indent = 0) const override;
};


class Parser 
{
    lexer lx;
    token cur;
    vector<token> tokens;

public:
    Parser(const string &src);
    void advance();
    void expect(tokenType t, ParseError::Kind errKind);
    shared_ptr<ProgramNode> parseProgram();
    string parseTypeName();
    AST parseFunction();
    shared_ptr<BlockNode> parseBlock();
    AST parseStatementOrDecl();
    AST parseIf();
    AST parseWhile();
    AST parseExpression();
    AST parseAssignment();
    AST parseLogicalOr();
    AST parseLogicalAnd();
    AST parseEquality();
    AST parseRelational();
    AST parseAdditive();
    AST parseMultiplicative();
    AST parseUnary();
    AST parsePostfix();
    AST parsePrimary();
    void printTokens(ostream &os) const;
};

#endif 