#ifndef IR_H
#define IR_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
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


enum class IROpcode 
{
    
    ADD, SUB, MUL, DIV,
    
    
    NEG, NOT,
    
    
    EQ, NE, LT, LE, GT, GE,
    
    
    AND, OR,
    
    
    ASSIGN, COPY,
    
    
    LABEL, GOTO, IF_FALSE, IF_TRUE,
    
    
    PARAM, CALL, RETURN,
    
    
    FUNC_BEGIN, FUNC_END
};


struct IRInstruction 
{
    IROpcode op;
    string result;      
    string arg1;        
    string arg2;        
    
    IRInstruction(IROpcode opcode, const string& res = "", 
                  const string& a1 = "", const string& a2 = "")
        : op(opcode), result(res), arg1(a1), arg2(a2) {}
    
    void print(ostream& os) const;
};

class IRGenerator 
{
private:
    vector<IRInstruction> instructions;
    ScopeStack scopeStack;
    
    int tempCounter;
    int labelCounter;
    
    string currentFunction;
    
    
    string newTemp();
    
    
    string newLabel();
    
    
    void emit(const IRInstruction& instr);
    void emit(IROpcode op, const string& result = "", 
              const string& arg1 = "", const string& arg2 = "");
    
public:
    IRGenerator() : tempCounter(0), labelCounter(0) {}
    
    void generate(shared_ptr<ProgramNode> program);
    void printIR(ostream& os) const;
    const vector<IRInstruction>& getInstructions() const { return instructions; }
    
private:
    
    void genProgram(shared_ptr<ProgramNode> node);
    void genFunction(shared_ptr<FunctionNode> node);
    void genBlock(shared_ptr<BlockNode> node);
    void genVarDecl(shared_ptr<VarDeclNode> node);
    void genReturn(shared_ptr<ReturnNode> node);
    void genIf(shared_ptr<IfNode> node);
    void genWhile(shared_ptr<WhileNode> node);
    void genExprStmt(shared_ptr<ExprStmtNode> node);
    
    
    string genExpression(AST node);
    string genBinaryOp(shared_ptr<BinaryOpNode> node);
    string genUnaryOp(shared_ptr<UnaryOpNode> node);
    string genLiteral(shared_ptr<LiteralNode> node);
    string genIdentifier(shared_ptr<IdentifierNode> node);
    string genCall(shared_ptr<CallNode> node);
    string genAssignment(shared_ptr<AssignmentNode> node);
};


string opcodeToString(IROpcode op);

#endif