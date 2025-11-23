#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <sstream>
#include "scope_analyzer.h"

using namespace std;


struct ASTNode;
struct ProgramNode;
struct BlockNode;
struct FunctionNode;
struct VarDeclNode;
struct ReturnNode;
struct IfNode;
struct WhileNode;
struct ExprStmtNode;
struct BinaryOpNode;
struct UnaryOpNode;
struct LiteralNode;
struct IdentifierNode;
struct CallNode;
struct AssignmentNode;

using AST = shared_ptr<ASTNode>;

enum class TypeChkError 
{
    ErroneousVarDecl,
    FnCallParamCount,
    FnCallParamType,
    ErroneousReturnType,
    ExpressionTypeMismatch,
    ExpectedBooleanExpression,
    ErroneousBreak,
    NonBooleanCondStmt,
    EmptyExpression,
    AttemptedBoolOpOnNonBools,
    AttemptedBitOpOnNonNumeric,
    AttemptedShiftOnNonInt,
    AttemptedAddOpOnNonNumeric,
    AttemptedExponentiationOfNonNumeric,
    ReturnStmtNotFound,
};

class TypeCheckException : public exception 
{
    TypeChkError errorType;
    string msg;
    string details;
    
public:
    TypeCheckException(TypeChkError type, const string& detail = "");
    
    const char* what() const noexcept override 
    {
        return msg.c_str();
    }
    
    TypeChkError getErrorType() const { return errorType; }
    string getDetails() const { return details; }
};

class TypeChecker 
{
private:
    ScopeStack& scopeStack;
    string currentFunctionRetType;
    bool hasReturnStmt;
    
    
    bool isNumericType(const string& type);
    bool isIntegerType(const string& type);
    bool isBooleanType(const string& type);
    bool areTypesCompatible(const string& type1, const string& type2);
    string promoteTypes(const string& type1, const string& type2);
    
public:
    TypeChecker(ScopeStack& stack) : scopeStack(stack), hasReturnStmt(false) {}
    
    void check(shared_ptr<ProgramNode> program);
    
private:
    string checkNode(AST node);
    void checkProgram(shared_ptr<ProgramNode> node);
    void checkBlock(shared_ptr<BlockNode> node);
    void checkFunction(shared_ptr<FunctionNode> node);
    string checkVarDecl(shared_ptr<VarDeclNode> node);
    void checkReturn(shared_ptr<ReturnNode> node);
    void checkIf(shared_ptr<IfNode> node);
    void checkWhile(shared_ptr<WhileNode> node);
    void checkExprStmt(shared_ptr<ExprStmtNode> node);
    string checkBinaryOp(shared_ptr<BinaryOpNode> node);
    string checkUnaryOp(shared_ptr<UnaryOpNode> node);
    string checkLiteral(shared_ptr<LiteralNode> node);
    string checkIdentifier(shared_ptr<IdentifierNode> node);
    string checkCall(shared_ptr<CallNode> node);
    string checkAssignment(shared_ptr<AssignmentNode> node);
};

#endif