#include <iostream>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "scope_analyzer.h"
#include "type_checker.h"
#include "ir.h"
#include <fstream>

using namespace std;

int main() 
{
    ifstream file("text.txt");
    if (!file.is_open()) 
    {
        cerr << "Error: Could not open text.txt\n";
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string code = buffer.str();

    file.close();

    try 
    {
        
        Parser parser(code);
        auto ast = parser.parseProgram();
        
        cout << "AST:" << endl;
        ast->print(cout);
        
        
        ScopeAnalyzer scopeAnalyzer;
        scopeAnalyzer.analyze(ast);
        cout << "\nScope analysis passed" << endl;
        
        
        TypeChecker typeChecker(scopeAnalyzer.getScopeStack());
        typeChecker.check(ast);
        cout << "Type checking passed" << endl;
        
        
        IRGenerator irGen;
        irGen.generate(ast);
        cout << "\nIR generation passed" << endl;
        irGen.printIR(cout);
    } 
    catch (const runtime_error& e) 
    {
        cerr << "Lexer error: " << e.what() << endl;
        return 1;
    } 
    catch (const ParseError& e) 
    {
        cerr << "Parse error: " << e.message() << endl;
        return 1;
    } 
    catch (const ScopeException& e) 
    {
        cerr << e.what() << endl;
        return 1;
    } 
    catch (const TypeCheckException& e) 
    {
        cerr << e.what() << endl;
        return 1;
    }
    
    return 0;
}