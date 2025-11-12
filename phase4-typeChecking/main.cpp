#include <iostream>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "scope_analyzer.h"
#include "type_checker.h"

using namespace std;

int main() 
{
    string code = R"(
    fn int factorial(int n) {
        if (n <= 1) {
            return 1;
        }
        bool x=true+10;
        return n * factorial(n - 1);
    }

    fn int main() {
        int x = 5;
        int result = factorial(x);
        return result;
    }
    )";

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