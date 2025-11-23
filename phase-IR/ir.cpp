#include "ir.h"
#include "parser.h"
#include <iomanip>

using namespace std;

string opcodeToString(IROpcode op) 
{
    switch(op) 
    {
        case IROpcode::ADD: return "ADD";
        case IROpcode::SUB: return "SUB";
        case IROpcode::MUL: return "MUL";
        case IROpcode::DIV: return "DIV";
        case IROpcode::NEG: return "NEG";
        case IROpcode::NOT: return "NOT";
        case IROpcode::EQ: return "EQ";
        case IROpcode::NE: return "NE";
        case IROpcode::LT: return "LT";
        case IROpcode::LE: return "LE";
        case IROpcode::GT: return "GT";
        case IROpcode::GE: return "GE";
        case IROpcode::AND: return "AND";
        case IROpcode::OR: return "OR";
        case IROpcode::ASSIGN: return "ASSIGN";
        case IROpcode::COPY: return "COPY";
        case IROpcode::LABEL: return "LABEL";
        case IROpcode::GOTO: return "GOTO";
        case IROpcode::IF_FALSE: return "IF_FALSE";
        case IROpcode::IF_TRUE: return "IF_TRUE";
        case IROpcode::PARAM: return "PARAM";
        case IROpcode::CALL: return "CALL";
        case IROpcode::RETURN: return "RETURN";
        case IROpcode::FUNC_BEGIN: return "FUNC_BEGIN";
        case IROpcode::FUNC_END: return "FUNC_END";
    }
    return "UNKNOWN";
}

void IRInstruction::print(ostream& os) const 
{
    string opStr = opcodeToString(op);
    
    switch(op) 
    {
        case IROpcode::LABEL:
            os << result << ":";
            break;
            
        case IROpcode::GOTO:
            os << "  GOTO " << result;
            break;
            
        case IROpcode::IF_FALSE:
            os << "  IF_FALSE " << arg1 << " GOTO " << result;
            break;
            
        case IROpcode::IF_TRUE:
            os << "  IF_TRUE " << arg1 << " GOTO " << result;
            break;
            
        case IROpcode::FUNC_BEGIN:
            os << "\nFUNCTION " << result << ":";
            break;
            
        case IROpcode::FUNC_END:
            os << "END_FUNCTION " << result;
            break;
            
        case IROpcode::PARAM:
            os << "  PARAM " << arg1;
            break;
            
        case IROpcode::CALL:
            if (!result.empty())
                os << "  " << result << " = CALL " << arg1 << ", " << arg2;
            else
                os << "  CALL " << arg1 << ", " << arg2;
            break;
            
        case IROpcode::RETURN:
            if (!arg1.empty())
                os << "  RETURN " << arg1;
            else
                os << "  RETURN";
            break;
            
        case IROpcode::NEG:
        case IROpcode::NOT:
            os << "  " << result << " = " << opStr << " " << arg1;
            break;
            
        case IROpcode::COPY:
            os << "  " << result << " = " << arg1;
            break;
            
        default:
            
            if (!arg2.empty())
                os << "  " << result << " = " << arg1 << " " << opStr << " " << arg2;
            else
                os << "  " << result << " = " << opStr << " " << arg1;
            break;
    }
}

string IRGenerator::newTemp() 
{
    return "t" + to_string(tempCounter++);
}

string IRGenerator::newLabel() 
{
    return "L" + to_string(labelCounter++);
}

void IRGenerator::emit(const IRInstruction& instr) 
{
    instructions.push_back(instr);
}

void IRGenerator::emit(IROpcode op, const string& result, 
                       const string& arg1, const string& arg2) 
{
    instructions.push_back(IRInstruction(op, result, arg1, arg2));
}

void IRGenerator::generate(shared_ptr<ProgramNode> program) 
{
    genProgram(program);
}

void IRGenerator::printIR(ostream& os) const 
{
    os << "\n=== THREE ADDRESS CODE (TAC) ===" << endl;
    for (const auto& instr : instructions) 
    {
        instr.print(os);
        os << endl;
    }
    os << "================================\n" << endl;
}

void IRGenerator::genProgram(shared_ptr<ProgramNode> node) 
{
    
    for (const auto& item : node->items) 
    {
        if (auto func = dynamic_pointer_cast<FunctionNode>(item)) 
        {
            vector<string> paramTypes;
            for (const auto& param : func->params) 
            {
                paramTypes.push_back(param.first);
            }
            scopeStack.addFunction(func->name, func->retType, paramTypes);
        }
    }
    
    
    for (const auto& item : node->items) 
    {
        if (auto func = dynamic_pointer_cast<FunctionNode>(item)) 
        {
            genFunction(func);
        } 
        else 
        {
            
            if (auto varDecl = dynamic_pointer_cast<VarDeclNode>(item)) 
            {
                genVarDecl(varDecl);
            }
        }
    }
}

void IRGenerator::genFunction(shared_ptr<FunctionNode> node) 
{
    currentFunction = node->name;
    
    emit(IROpcode::FUNC_BEGIN, node->name);
    
    scopeStack.enterScope();
    
    
    for (const auto& param : node->params) 
    {
        scopeStack.addSymbol(param.second, param.first, false);
        
    }
    
    
    for (const auto& stmt : node->body->stmts) 
    {
        if (auto varDecl = dynamic_pointer_cast<VarDeclNode>(stmt)) 
        {
            genVarDecl(varDecl);
        } 
        else if (auto ret = dynamic_pointer_cast<ReturnNode>(stmt)) 
        {
            genReturn(ret);
        } 
        else if (auto ifNode = dynamic_pointer_cast<IfNode>(stmt)) 
        {
            genIf(ifNode);
        } 
        else if (auto whileNode = dynamic_pointer_cast<WhileNode>(stmt)) 
        {
            genWhile(whileNode);
        } 
        else if (auto exprStmt = dynamic_pointer_cast<ExprStmtNode>(stmt)) 
        {
            genExprStmt(exprStmt);
        } 
        else if (auto block = dynamic_pointer_cast<BlockNode>(stmt)) 
        {
            genBlock(block);
        }
    }
    
    scopeStack.exitScope();
    
    emit(IROpcode::FUNC_END, node->name);
}

void IRGenerator::genBlock(shared_ptr<BlockNode> node) 
{
    scopeStack.enterScope();
    
    for (const auto& stmt : node->stmts) 
    {
        if (auto varDecl = dynamic_pointer_cast<VarDeclNode>(stmt)) 
        {
            genVarDecl(varDecl);
        } 
        else if (auto ret = dynamic_pointer_cast<ReturnNode>(stmt)) 
        {
            genReturn(ret);
        } 
        else if (auto ifNode = dynamic_pointer_cast<IfNode>(stmt)) 
        {
            genIf(ifNode);
        } 
        else if (auto whileNode = dynamic_pointer_cast<WhileNode>(stmt)) 
        {
            genWhile(whileNode);
        } 
        else if (auto exprStmt = dynamic_pointer_cast<ExprStmtNode>(stmt)) 
        {
            genExprStmt(exprStmt);
        } 
        else if (auto block = dynamic_pointer_cast<BlockNode>(stmt)) 
        {
            genBlock(block);
        }
    }
    
    scopeStack.exitScope();
}

void IRGenerator::genVarDecl(shared_ptr<VarDeclNode> node) 
{
    if (node->init) 
    {
        string initValue = genExpression(node->init);
        emit(IROpcode::COPY, node->name, initValue);
    }
    
    
    scopeStack.addSymbol(node->name, node->typeName, false);
}

void IRGenerator::genReturn(shared_ptr<ReturnNode> node) 
{
    if (node->expr) 
    {
        string retValue = genExpression(node->expr);
        emit(IROpcode::RETURN, "", retValue);
    } 
    else 
    {
        emit(IROpcode::RETURN);
    }
}

void IRGenerator::genIf(shared_ptr<IfNode> node) 
{
    string condResult = genExpression(node->cond);
    
    string elseLabel = newLabel();
    string endLabel = newLabel();
    
    
    if (node->elseBlock) 
    {
        emit(IROpcode::IF_FALSE, elseLabel, condResult);
    } 
    else 
    {
        emit(IROpcode::IF_FALSE, endLabel, condResult);
    }
    
    
    genBlock(node->thenBlock);
    
    if (node->elseBlock) 
    {
        emit(IROpcode::GOTO, endLabel);
        emit(IROpcode::LABEL, elseLabel);
        genBlock(node->elseBlock);
    }
    
    emit(IROpcode::LABEL, endLabel);
}

void IRGenerator::genWhile(shared_ptr<WhileNode> node) 
{
    string startLabel = newLabel();
    string endLabel = newLabel();
    
    emit(IROpcode::LABEL, startLabel);
    
    string condResult = genExpression(node->cond);
    emit(IROpcode::IF_FALSE, endLabel, condResult);
    
    genBlock(node->body);
    
    emit(IROpcode::GOTO, startLabel);
    emit(IROpcode::LABEL, endLabel);
}

void IRGenerator::genExprStmt(shared_ptr<ExprStmtNode> node) 
{
    genExpression(node->expr);
}

string IRGenerator::genExpression(AST node) 
{
    if (!node) return "";
    
    if (auto binOp = dynamic_pointer_cast<BinaryOpNode>(node)) 
    {
        return genBinaryOp(binOp);
    } 
    else if (auto unOp = dynamic_pointer_cast<UnaryOpNode>(node)) 
    {
        return genUnaryOp(unOp);
    } 
    else if (auto lit = dynamic_pointer_cast<LiteralNode>(node)) 
    {
        return genLiteral(lit);
    } 
    else if (auto id = dynamic_pointer_cast<IdentifierNode>(node)) 
    {
        return genIdentifier(id);
    } 
    else if (auto call = dynamic_pointer_cast<CallNode>(node)) 
    {
        return genCall(call);
    } 
    else if (auto assign = dynamic_pointer_cast<AssignmentNode>(node)) 
    {
        return genAssignment(assign);
    }
    
    return "";
}

string IRGenerator::genBinaryOp(shared_ptr<BinaryOpNode> node) 
{
    string left = genExpression(node->left);
    string right = genExpression(node->right);
    string result = newTemp();
    
    IROpcode op;
    if (node->op == "+") op = IROpcode::ADD;
    else if (node->op == "-") op = IROpcode::SUB;
    else if (node->op == "*") op = IROpcode::MUL;
    else if (node->op == "/") op = IROpcode::DIV;
    else if (node->op == "==") op = IROpcode::EQ;
    else if (node->op == "!=") op = IROpcode::NE;
    else if (node->op == "<") op = IROpcode::LT;
    else if (node->op == "<=") op = IROpcode::LE;
    else if (node->op == ">") op = IROpcode::GT;
    else if (node->op == ">=") op = IROpcode::GE;
    else if (node->op == "&&") op = IROpcode::AND;
    else if (node->op == "||") op = IROpcode::OR;
    else op = IROpcode::ADD; 
    
    emit(op, result, left, right);
    return result;
}

string IRGenerator::genUnaryOp(shared_ptr<UnaryOpNode> node) 
{
    string operand = genExpression(node->operand);
    
    if (node->op == "++" || node->op == "--") 
    {
        
        string one = "1";
        IROpcode op = (node->op == "++") ? IROpcode::ADD : IROpcode::SUB;
        
        if (node->postfix) 
        {
            
            string temp = newTemp();
            emit(IROpcode::COPY, temp, operand);
            string result = newTemp();
            emit(op, result, operand, one);
            emit(IROpcode::COPY, operand, result);
            return temp; 
        } 
        else 
        {
            
            string result = newTemp();
            emit(op, result, operand, one);
            emit(IROpcode::COPY, operand, result);
            return result;
        }
    } 
    else if (node->op == "-") 
    {
        string result = newTemp();
        emit(IROpcode::NEG, result, operand);
        return result;
    } 
    else if (node->op == "!") 
    {
        string result = newTemp();
        emit(IROpcode::NOT, result, operand);
        return result;
    }
    
    return operand;
}

string IRGenerator::genLiteral(shared_ptr<LiteralNode> node) 
{
    return node->value;
}

string IRGenerator::genIdentifier(shared_ptr<IdentifierNode> node) 
{
    return node->name;
}

string IRGenerator::genCall(shared_ptr<CallNode> node) 
{
    auto idNode = dynamic_pointer_cast<IdentifierNode>(node->callee);
    if (!idNode) return "";
    
    
    for (const auto& arg : node->args) 
    {
        string argValue = genExpression(arg);
        emit(IROpcode::PARAM, "", argValue);
    }
    
    
    string result = newTemp();
    string numArgs = to_string(node->args.size());
    emit(IROpcode::CALL, result, idNode->name, numArgs);
    
    return result;
}

string IRGenerator::genAssignment(shared_ptr<AssignmentNode> node) 
{
    auto idNode = dynamic_pointer_cast<IdentifierNode>(node->left);
    if (!idNode) return "";
    
    string rightValue = genExpression(node->right);
    
    if (node->op == "=") 
    {
        emit(IROpcode::COPY, idNode->name, rightValue);
    } 
    else 
    {
        
        IROpcode op;
        if (node->op == "+=") op = IROpcode::ADD;
        else if (node->op == "-=") op = IROpcode::SUB;
        else if (node->op == "*=") op = IROpcode::MUL;
        else if (node->op == "/=") op = IROpcode::DIV;
        else op = IROpcode::ADD;
        
        string result = newTemp();
        emit(op, result, idNode->name, rightValue);
        emit(IROpcode::COPY, idNode->name, result);
    }
    
    return idNode->name;
}