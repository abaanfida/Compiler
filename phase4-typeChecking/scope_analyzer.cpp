#include "scope_analyzer.h"
#include "parser.h"
#include <memory>
#include <vector>
#include <iostream>

using namespace std;

ScopeException::ScopeException(ScopeError type, const string& symbol) 
    : errorType(type), symbolName(symbol) 
{
    ostringstream oss;
    switch(type) 
    {
        case ScopeError::UndeclaredVariableAccessed:
            oss << "Scope Error: Undeclared variable accessed: '" << symbol << "'";
            break;
        case ScopeError::UndefinedFunctionCalled:
            oss << "Scope Error: Undefined function called: '" << symbol << "'";
            break;
        case ScopeError::VariableRedefinition:
            oss << "Scope Error: Variable redefinition in same scope: '" << symbol << "'";
            break;
        case ScopeError::FunctionPrototypeRedefinition:
            oss << "Scope Error: Function redefinition: '" << symbol << "'";
            break;
    }
    msg = oss.str();
}

ScopeStack::ScopeStack() : nextScopeId(0) 
{
    globalScope = make_shared<ScopeNode>(nextScopeId++);
    currentScope = globalScope;
}

void ScopeStack::enterScope() 
{
    currentScope = make_shared<ScopeNode>(nextScopeId++, currentScope);
}

void ScopeStack::exitScope() 
{
    if (currentScope->parent) 
    {
        currentScope = currentScope->parent;
    }
}

void ScopeStack::addSymbol(const string& name, const string& type, bool isFunction) 
{
    if (currentScope->symbols.find(name) != currentScope->symbols.end()) 
    {
        if (isFunction) 
        {
            throw ScopeException(ScopeError::FunctionPrototypeRedefinition, name);
        } 
        else 
        {
            throw ScopeException(ScopeError::VariableRedefinition, name);
        }
    }
    
    auto info = make_shared<SymbolInfo>(name, type, isFunction, currentScope->id);
    currentScope->symbols[name] = info;
}

void ScopeStack::addFunction(const string& name, const string& retType, const vector<string>& paramTypes) 
{
    if (globalScope->symbols.find(name) != globalScope->symbols.end())
    {
        throw ScopeException(ScopeError::FunctionPrototypeRedefinition, name);
    }
    
    auto info = make_shared<SymbolInfo>(name, retType, true, globalScope->id);
    info->paramTypes = paramTypes;
    globalScope->symbols[name] = info;
}

shared_ptr<SymbolInfo> ScopeStack::lookup(const string& name, bool functionLookup) 
{
    shared_ptr<ScopeNode> scope = currentScope;
    
    while (scope) 
    {
        auto it = scope->symbols.find(name);
        if (it != scope->symbols.end()) 
        {
            if (functionLookup && !it->second->isFunction) 
            {
                scope = scope->parent;
                continue;
            }
            
            if (!functionLookup && it->second->isFunction) 
            {
                scope = scope->parent;
                continue;
            }
            return it->second;
        }
        scope = scope->parent;
    }
    
    return nullptr;
}

shared_ptr<SymbolInfo> ScopeStack::requireSymbol(const string& name) 
{
    auto info = lookup(name, false);
    if (!info) 
    {
        throw ScopeException(ScopeError::UndeclaredVariableAccessed, name);
    }
    return info;
}

shared_ptr<SymbolInfo> ScopeStack::requireFunction(const string& name) 
{
    auto info = lookup(name, true);
    if (!info) 
    {
        throw ScopeException(ScopeError::UndefinedFunctionCalled, name);
    }
    return info;
}

void ScopeStack::printScopes(ostream& os) const 
{
    os << "\n=== SCOPE STACK ===" << endl;
    shared_ptr<ScopeNode> scope = currentScope;
    int depth = 0;
    
    while (scope)
    {
        os << "Scope " << scope->id << " (depth " << depth << ")";
        if (scope == globalScope) 
        {
            os << " [GLOBAL]";
        }
        os << ":" << endl;
        
        if (scope->symbols.empty()) 
        {
            os << "  (empty)" << endl;
        } 
        else 
        {
            for (const auto& pair : scope->symbols) 
            {
                os << "  " << pair.first << " : " << pair.second->type;
                if (pair.second->isFunction) 
                {
                    os << " (function, params: [";
                    for (size_t i = 0; i < pair.second->paramTypes.size(); i++) 
                    {
                        if (i > 0) os << ", ";
                        os << pair.second->paramTypes[i];
                    }
                    os << "])";
                }
                os << endl;
            }
        }
        scope = scope->parent;
        depth++;
    }
    os << "===================\n" << endl;
}

void ScopeAnalyzer::analyze(shared_ptr<ProgramNode> program) 
{
    analyzeProgram(program);
}

void ScopeAnalyzer::printScopes(ostream& os) const 
{
    scopeStack.printScopes(os);
}

void ScopeAnalyzer::analyzeNode(AST node)
{
    if (!node) return;
    
    if (auto prog = dynamic_pointer_cast<ProgramNode>(node)) 
    {
        analyzeProgram(prog);
    } else if (auto block = dynamic_pointer_cast<BlockNode>(node)) {
        analyzeBlock(block);
    } else if (auto func = dynamic_pointer_cast<FunctionNode>(node)) {
        analyzeFunction(func);
    } else if (auto varDecl = dynamic_pointer_cast<VarDeclNode>(node)) {
        analyzeVarDecl(varDecl);
    } else if (auto ret = dynamic_pointer_cast<ReturnNode>(node)) {
        analyzeReturn(ret);
    } else if (auto ifNode = dynamic_pointer_cast<IfNode>(node)) {
        analyzeIf(ifNode);
    } else if (auto whileNode = dynamic_pointer_cast<WhileNode>(node)) {
        analyzeWhile(whileNode);
    } else if (auto exprStmt = dynamic_pointer_cast<ExprStmtNode>(node)) {
        analyzeExprStmt(exprStmt);
    } else if (auto binOp = dynamic_pointer_cast<BinaryOpNode>(node)) {
        analyzeBinaryOp(binOp);
    } else if (auto unOp = dynamic_pointer_cast<UnaryOpNode>(node)) {
        analyzeUnaryOp(unOp);
    } else if (auto lit = dynamic_pointer_cast<LiteralNode>(node)) {
        analyzeLiteral(lit);
    } else if (auto id = dynamic_pointer_cast<IdentifierNode>(node)) {
        analyzeIdentifier(id);
    } else if (auto call = dynamic_pointer_cast<CallNode>(node)) {
        analyzeCall(call);
    } else if (auto assign = dynamic_pointer_cast<AssignmentNode>(node)) {
        analyzeAssignment(assign);
    }
}

void ScopeAnalyzer::analyzeProgram(shared_ptr<ProgramNode> node) 
{
    for (const auto& item : node->items) 
    {
        if (auto func = dynamic_pointer_cast<FunctionNode>(item)) 
        {
            vector<string> paramTypes;
            for (const auto& param : func->params) {
                paramTypes.push_back(param.first);  
            }
            scopeStack.addFunction(func->name, func->retType, paramTypes);
        }
    }
    
    for (const auto& item : node->items) 
    {
        analyzeNode(item);
    }
}

void ScopeAnalyzer::analyzeBlock(shared_ptr<BlockNode> node) 
{
    scopeStack.enterScope();
    
    for (const auto& stmt : node->stmts) 
    {
        analyzeNode(stmt);
    }
    
    scopeStack.exitScope();
}

void ScopeAnalyzer::analyzeFunction(shared_ptr<FunctionNode> node) 
{
    scopeStack.enterScope();
    
    for (const auto& param : node->params) 
    {
        scopeStack.addSymbol(param.second, param.first, false);  
    }
    
    for (const auto& stmt : node->body->stmts) 
    {
        analyzeNode(stmt);
    }
    
    scopeStack.exitScope();
}

void ScopeAnalyzer::analyzeVarDecl(shared_ptr<VarDeclNode> node) 
{
    if (node->init) 
    {
        analyzeNode(node->init);
    }
    
    scopeStack.addSymbol(node->name, node->typeName, false);
}

void ScopeAnalyzer::analyzeReturn(shared_ptr<ReturnNode> node) 
{
    if (node->expr) 
    {
        analyzeNode(node->expr);
    }
}

void ScopeAnalyzer::analyzeIf(shared_ptr<IfNode> node) 
{
    analyzeNode(node->cond);
    analyzeBlock(node->thenBlock);
    
    if (node->elseBlock)
    {
        analyzeBlock(node->elseBlock);
    }
}

void ScopeAnalyzer::analyzeWhile(shared_ptr<WhileNode> node) 
{
    analyzeNode(node->cond);
    analyzeBlock(node->body);
}

void ScopeAnalyzer::analyzeExprStmt(shared_ptr<ExprStmtNode> node) 
{
    analyzeNode(node->expr);
}

void ScopeAnalyzer::analyzeBinaryOp(shared_ptr<BinaryOpNode> node) 
{
    analyzeNode(node->left);
    analyzeNode(node->right);
}

void ScopeAnalyzer::analyzeUnaryOp(shared_ptr<UnaryOpNode> node) 
{
    analyzeNode(node->operand);
}

void ScopeAnalyzer::analyzeLiteral(shared_ptr<LiteralNode> node) 
{
    return;
}

void ScopeAnalyzer::analyzeIdentifier(shared_ptr<IdentifierNode> node) 
{
    scopeStack.requireSymbol(node->name);
}

void ScopeAnalyzer::analyzeCall(shared_ptr<CallNode> node)
{
    if (auto id = dynamic_pointer_cast<IdentifierNode>(node->callee)) 
    {
        scopeStack.requireFunction(id->name);
    }
    else 
    {
        analyzeNode(node->callee);
    }
    
    for (const auto& arg : node->args) 
    {
        analyzeNode(arg);
    }
}

void ScopeAnalyzer::analyzeAssignment(shared_ptr<AssignmentNode> node) 
{
    if (auto id = dynamic_pointer_cast<IdentifierNode>(node->left)) 
    {
        scopeStack.requireSymbol(id->name);
    }
    else 
    {
        analyzeNode(node->left);
    }
    
    analyzeNode(node->right);
}