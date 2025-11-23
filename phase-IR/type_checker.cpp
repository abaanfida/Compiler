#include "type_checker.h"
#include "parser.h"
#include <memory>
#include <vector>
#include <iostream>

using namespace std;

TypeCheckException::TypeCheckException(TypeChkError type, const string& detail) 
    : errorType(type), details(detail) 
{
    ostringstream oss;
    oss << "Type Check Error: ";
    
    switch(type) 
    {
        case TypeChkError::ErroneousVarDecl:
            oss << "Erroneous variable declaration";
            break;
        case TypeChkError::FnCallParamCount:
            oss << "Function call parameter count mismatch";
            break;
        case TypeChkError::FnCallParamType:
            oss << "Function call parameter type mismatch";
            break;
        case TypeChkError::ErroneousReturnType:
            oss << "Return type mismatch";
            break;
        case TypeChkError::ExpressionTypeMismatch:
            oss << "Expression type mismatch";
            break;
        case TypeChkError::ExpectedBooleanExpression:
            oss << "Expected boolean expression";
            break;
        case TypeChkError::ErroneousBreak:
            oss << "Break statement outside loop";
            break;
        case TypeChkError::NonBooleanCondStmt:
            oss << "Non-boolean condition in control statement";
            break;
        case TypeChkError::EmptyExpression:
            oss << "Empty expression";
            break;
        case TypeChkError::AttemptedBoolOpOnNonBools:
            oss << "Boolean operation on non-boolean operands";
            break;
        case TypeChkError::AttemptedBitOpOnNonNumeric:
            oss << "Bitwise operation on non-numeric operands";
            break;
        case TypeChkError::AttemptedShiftOnNonInt:
            oss << "Shift operation on non-integer operands";
            break;
        case TypeChkError::AttemptedAddOpOnNonNumeric:
            oss << "Arithmetic operation on non-numeric operands";
            break;
        case TypeChkError::AttemptedExponentiationOfNonNumeric:
            oss << "Exponentiation of non-numeric operands";
            break;
        case TypeChkError::ReturnStmtNotFound:
            oss << "Missing return statement in non-void function";
            break;
    }
    
    if (!detail.empty()) 
    {
        oss << ": " << detail;
    }
    
    msg = oss.str();
}

bool TypeChecker::isNumericType(const string& type) 
{
    return type == "int" || type == "float";
}

bool TypeChecker::isIntegerType(const string& type) 
{
    return type == "int";
}

bool TypeChecker::isBooleanType(const string& type) 
{
    return type == "bool";
}

bool TypeChecker::areTypesCompatible(const string& type1, const string& type2) 
{
    if (type1 == type2) return true;
    
    
    if ((type1 == "int" && type2 == "float") || (type1 == "float" && type2 == "int"))
        return true;
    
    return false;
}

string TypeChecker::promoteTypes(const string& type1, const string& type2) 
{
    if (type1 == type2) return type1;
    
    
    if ((type1 == "int" && type2 == "float") || (type1 == "float" && type2 == "int"))
        return "float";
    
    return type1;
}

void TypeChecker::check(shared_ptr<ProgramNode> program) 
{
    checkProgram(program);
}

string TypeChecker::checkNode(AST node)
{
    if (!node) 
    {
        throw TypeCheckException(TypeChkError::EmptyExpression);
    }
    
    if (auto prog = dynamic_pointer_cast<ProgramNode>(node)) 
    {
        checkProgram(prog);
        return "void";
    } 
    else if (auto block = dynamic_pointer_cast<BlockNode>(node)) 
    {
        checkBlock(block);
        return "void";
    } 
    else if (auto func = dynamic_pointer_cast<FunctionNode>(node)) 
    {
        checkFunction(func);
        return func->retType;
    } 
    else if (auto varDecl = dynamic_pointer_cast<VarDeclNode>(node)) 
    {
        return checkVarDecl(varDecl);
    } 
    else if (auto ret = dynamic_pointer_cast<ReturnNode>(node)) 
    {
        checkReturn(ret);
        return "void";
    } 
    else if (auto ifNode = dynamic_pointer_cast<IfNode>(node)) 
    {
        checkIf(ifNode);
        return "void";
    } 
    else if (auto whileNode = dynamic_pointer_cast<WhileNode>(node)) 
    {
        checkWhile(whileNode);
        return "void";
    } 
    else if (auto exprStmt = dynamic_pointer_cast<ExprStmtNode>(node)) 
    {
        checkExprStmt(exprStmt);
        return "void";
    } 
    else if (auto binOp = dynamic_pointer_cast<BinaryOpNode>(node)) 
    {
        return checkBinaryOp(binOp);
    } 
    else if (auto unOp = dynamic_pointer_cast<UnaryOpNode>(node)) 
    {
        return checkUnaryOp(unOp);
    } 
    else if (auto lit = dynamic_pointer_cast<LiteralNode>(node)) 
    {
        return checkLiteral(lit);
    } 
    else if (auto id = dynamic_pointer_cast<IdentifierNode>(node)) 
    {
        return checkIdentifier(id);
    } 
    else if (auto call = dynamic_pointer_cast<CallNode>(node)) 
    {
        return checkCall(call);
    } 
    else if (auto assign = dynamic_pointer_cast<AssignmentNode>(node)) 
    {
        return checkAssignment(assign);
    }
    
    return "void";
}

void TypeChecker::checkProgram(shared_ptr<ProgramNode> node) 
{
    
    for (const auto& item : node->items) 
    {
        checkNode(item);
    }
}

void TypeChecker::checkBlock(shared_ptr<BlockNode> node) 
{
    scopeStack.enterScope();
    
    for (const auto& stmt : node->stmts) 
    {
        checkNode(stmt);
    }
    
    scopeStack.exitScope();
}

void TypeChecker::checkFunction(shared_ptr<FunctionNode> node) 
{
    currentFunctionRetType = node->retType;
    hasReturnStmt = false;
    
    scopeStack.enterScope();
    
    for (const auto& param : node->params) 
    {
        scopeStack.addSymbol(param.second, param.first, false);
    }
    
    for (const auto& stmt : node->body->stmts) 
    {
        checkNode(stmt);
    }
    
    scopeStack.exitScope();
    
    
    if (node->retType != "void" && !hasReturnStmt) 
    {
        throw TypeCheckException(TypeChkError::ReturnStmtNotFound, 
            "Function '" + node->name + "' must return a value of type '" + node->retType + "'");
    }
}

string TypeChecker::checkVarDecl(shared_ptr<VarDeclNode> node) 
{
    if (node->init) 
    {
        string initType = checkNode(node->init);
        
        if (!areTypesCompatible(node->typeName, initType)) 
        {
            throw TypeCheckException(TypeChkError::ErroneousVarDecl,
                "Cannot initialize variable '" + node->name + "' of type '" + 
                node->typeName + "' with expression of type '" + initType + "'");
        }
    }
    
    scopeStack.addSymbol(node->name, node->typeName, false);
    return node->typeName;
}

void TypeChecker::checkReturn(shared_ptr<ReturnNode> node) 
{
    hasReturnStmt = true;
    
    if (node->expr) 
    {
        string exprType = checkNode(node->expr);
        
        if (currentFunctionRetType == "void") 
        {
            throw TypeCheckException(TypeChkError::ErroneousReturnType,
                "Cannot return a value from void function");
        }
        
        if (!areTypesCompatible(currentFunctionRetType, exprType)) 
        {
            throw TypeCheckException(TypeChkError::ErroneousReturnType,
                "Expected return type '" + currentFunctionRetType + 
                "' but got '" + exprType + "'");
        }
    } 
    else 
    {
        if (currentFunctionRetType != "void") 
        {
            throw TypeCheckException(TypeChkError::ErroneousReturnType,
                "Function must return value of type '" + currentFunctionRetType + "'");
        }
    }
}

void TypeChecker::checkIf(shared_ptr<IfNode> node) 
{
    string condType = checkNode(node->cond);
    
    if (!isBooleanType(condType)) 
    {
        throw TypeCheckException(TypeChkError::NonBooleanCondStmt,
            "If condition must be boolean, got '" + condType + "'");
    }
    
    checkBlock(node->thenBlock);
    
    if (node->elseBlock) 
    {
        checkBlock(node->elseBlock);
    }
}

void TypeChecker::checkWhile(shared_ptr<WhileNode> node) 
{
    string condType = checkNode(node->cond);
    
    if (!isBooleanType(condType)) 
    {
        throw TypeCheckException(TypeChkError::NonBooleanCondStmt,
            "While condition must be boolean, got '" + condType + "'");
    }
    
    checkBlock(node->body);
}

void TypeChecker::checkExprStmt(shared_ptr<ExprStmtNode> node) 
{
    checkNode(node->expr);
}

string TypeChecker::checkBinaryOp(shared_ptr<BinaryOpNode> node) 
{
    string leftType = checkNode(node->left);
    string rightType = checkNode(node->right);
    
    
    if (node->op == "&&" || node->op == "||") 
    {
        if (!isBooleanType(leftType) || !isBooleanType(rightType)) 
        {
            throw TypeCheckException(TypeChkError::AttemptedBoolOpOnNonBools,
                "Operator '" + node->op + "' requires boolean operands, got '" + 
                leftType + "' and '" + rightType + "'");
        }
        return "bool";
    }
    
    
    if (node->op == "==" || node->op == "!=" || 
        node->op == "<" || node->op == ">" || 
        node->op == "<=" || node->op == ">=") 
    {
        if (!areTypesCompatible(leftType, rightType)) 
        {
            throw TypeCheckException(TypeChkError::ExpressionTypeMismatch,
                "Cannot compare '" + leftType + "' with '" + rightType + "'");
        }
        return "bool";
    }
    
    
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/") 
    {
        if (!isNumericType(leftType) || !isNumericType(rightType)) 
        {
            throw TypeCheckException(TypeChkError::AttemptedAddOpOnNonNumeric,
                "Operator '" + node->op + "' requires numeric operands, got '" + 
                leftType + "' and '" + rightType + "'");
        }
        return promoteTypes(leftType, rightType);
    }
    
    return leftType;
}

string TypeChecker::checkUnaryOp(shared_ptr<UnaryOpNode> node) 
{
    string operandType = checkNode(node->operand);
    
    
    if (node->op == "-" || node->op == "+") 
    {
        if (!isNumericType(operandType)) 
        {
            throw TypeCheckException(TypeChkError::AttemptedAddOpOnNonNumeric,
                "Unary '" + node->op + "' requires numeric operand, got '" + operandType + "'");
        }
        return operandType;
    }
    
    
    if (node->op == "++" || node->op == "--") 
    {
        if (!isNumericType(operandType)) 
        {
            throw TypeCheckException(TypeChkError::AttemptedAddOpOnNonNumeric,
                "Operator '" + node->op + "' requires numeric operand, got '" + operandType + "'");
        }
        return operandType;
    }
    
    return operandType;
}

string TypeChecker::checkLiteral(shared_ptr<LiteralNode> node) 
{
    return node->kind;
}

string TypeChecker::checkIdentifier(shared_ptr<IdentifierNode> node) 
{
    auto symbol = scopeStack.lookup(node->name, false);
    if (!symbol) 
    {
        throw TypeCheckException(TypeChkError::ExpressionTypeMismatch,
            "Undefined variable '" + node->name + "'");
    }
    return symbol->type;
}

string TypeChecker::checkCall(shared_ptr<CallNode> node)
{
    auto idNode = dynamic_pointer_cast<IdentifierNode>(node->callee);
    if (!idNode) 
    {
        throw TypeCheckException(TypeChkError::ExpressionTypeMismatch,
            "Invalid function call");
    }
    
    auto funcSymbol = scopeStack.lookup(idNode->name, true);
    if (!funcSymbol) 
    {
        throw TypeCheckException(TypeChkError::ExpressionTypeMismatch,
            "Undefined function '" + idNode->name + "'");
    }
    
    
    if (node->args.size() != funcSymbol->paramTypes.size()) 
    {
        throw TypeCheckException(TypeChkError::FnCallParamCount,
            "Function '" + idNode->name + "' expects " + 
            to_string(funcSymbol->paramTypes.size()) + " parameters but got " + 
            to_string(node->args.size()));
    }
    
    
    for (int i = 0; i < node->args.size(); i++) 
    {
        string argType = checkNode(node->args[i]);
        string expectedType = funcSymbol->paramTypes[i];
        
        if (!areTypesCompatible(expectedType, argType)) 
        {
            throw TypeCheckException(TypeChkError::FnCallParamType,
                "Parameter " + to_string(i + 1) + " of function '" + idNode->name + 
                "' expects type '" + expectedType + "' but got '" + argType + "'");
        }
    }
    
    return funcSymbol->type;
}

string TypeChecker::checkAssignment(shared_ptr<AssignmentNode> node) 
{
    auto idNode = dynamic_pointer_cast<IdentifierNode>(node->left);
    if (!idNode) 
    {
        throw TypeCheckException(TypeChkError::ExpressionTypeMismatch,
            "Left side of assignment must be a variable");
    }
    
    string leftType = checkIdentifier(idNode);
    string rightType = checkNode(node->right);
    
    
    if (node->op != "=") 
    {
        if (!isNumericType(leftType) || !isNumericType(rightType)) 
        {
            throw TypeCheckException(TypeChkError::AttemptedAddOpOnNonNumeric,
                "Compound assignment '" + node->op + "' requires numeric operands");
        }
    } 
    else 
    {
        
        if (!areTypesCompatible(leftType, rightType)) 
        {
            throw TypeCheckException(TypeChkError::ExpressionTypeMismatch,
                "Cannot assign value of type '" + rightType + 
                "' to variable of type '" + leftType + "'");
        }
    }
    
    return leftType;
}