#include <iostream>
#include <string>
#include "parser.h"
#include "scope_analyzer.h"

using namespace std;

void testScopeAnalysis(const string& code) 
{

    cout << "Code:" << endl;
    cout << code << endl;
    cout << "\nResult:" << endl;
    
    try 
    {
        Parser p(code);
        auto prog = p.parseProgram();
        
        ScopeAnalyzer analyzer;
        analyzer.analyze(prog);
        
        cout << "✓ Scope analysis PASSED" << endl;
        analyzer.printScopes(cout);
    }
    catch (const ScopeException& err) 
    {
        cout << "✗ " << err.what() << endl;
    } 
    catch (const ParseError& err) 
    {
        cout << "✗ Parse error: " << err.message() << endl;
    }
    catch (const exception& err) 
    {
        cout << "✗ Error: " << err.what() << endl;
    }
}

int main() 
{

    cout << "    SCOPE ANALYSIS SINGLE TEST" << endl;

    string code = R"(
        fn int test() {
            int x = undeclared_var + 5;
            return x;
        }
    )";

    testScopeAnalysis(code);

    cout << "        TEST COMPLETED" << endl;

    
    return 0;
}
