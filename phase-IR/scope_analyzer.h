#ifndef SCOPE_ANALYZER_H
#define SCOPE_ANALYZER_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <sstream>

using namespace std;

// Forward declarations - actual definitions are in parser.h
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

enum class ScopeError 
{
    UndeclaredVariableAccessed,
    UndefinedFunctionCalled,
    VariableRedefinition,
    FunctionPrototypeRedefinition,
};

struct SymbolInfo 
{
    string name;
    string type;           
    bool isFunction;
    vector<string> paramTypes;  
    int scopeLevel;
    
    SymbolInfo(const string& n, const string& t, bool isFunc = false, int level = 0)
        : name(n), type(t), isFunction(isFunc), scopeLevel(level) {}
};

class ScopeException : public exception 
{
    ScopeError errorType;
    string symbolName;
    string msg;
    
public:
    ScopeException(ScopeError type, const string& symbol);
    
    const char* what() const noexcept override 
    {
        return msg.c_str();
    }
    
    ScopeError getErrorType() const { return errorType; }
    string getSymbolName() const { return symbolName; }
};

class ScopeStack 
{
private:
    struct ScopeNode 
    {
        int id;
        shared_ptr<ScopeNode> parent;
        unordered_map<string, shared_ptr<SymbolInfo>> symbols;
        
        ScopeNode(int scopeId, shared_ptr<ScopeNode> par = nullptr)
            : id(scopeId), parent(par) {}
    };
    
    shared_ptr<ScopeNode> currentScope;
    shared_ptr<ScopeNode> globalScope;
    int nextScopeId;
    
public:
    ScopeStack();
    
    void enterScope();
    void exitScope();
    void addSymbol(const string& name, const string& type, bool isFunction = false);
    void addFunction(const string& name, const string& retType, const vector<string>& paramTypes);
    shared_ptr<SymbolInfo> lookup(const string& name, bool functionLookup = false);
    shared_ptr<SymbolInfo> requireSymbol(const string& name);
    shared_ptr<SymbolInfo> requireFunction(const string& name);
    void printScopes(ostream& os) const;
};

class ScopeAnalyzer 
{
private:
    ScopeStack scopeStack;
    
public:
    ScopeAnalyzer() {}
    
    void analyze(shared_ptr<ProgramNode> program);
    void printScopes(ostream& os) const;
    ScopeStack& getScopeStack() { return scopeStack; }
private:
    void analyzeNode(AST node);
    void analyzeProgram(shared_ptr<ProgramNode> node);
    void analyzeBlock(shared_ptr<BlockNode> node);
    void analyzeFunction(shared_ptr<FunctionNode> node);
    void analyzeVarDecl(shared_ptr<VarDeclNode> node);
    void analyzeReturn(shared_ptr<ReturnNode> node);
    void analyzeIf(shared_ptr<IfNode> node);
    void analyzeWhile(shared_ptr<WhileNode> node);
    void analyzeExprStmt(shared_ptr<ExprStmtNode> node);
    void analyzeBinaryOp(shared_ptr<BinaryOpNode> node);
    void analyzeUnaryOp(shared_ptr<UnaryOpNode> node);
    void analyzeLiteral(shared_ptr<LiteralNode> node);
    void analyzeIdentifier(shared_ptr<IdentifierNode> node);
    void analyzeCall(shared_ptr<CallNode> node);
    void analyzeAssignment(shared_ptr<AssignmentNode> node);
};

#endif