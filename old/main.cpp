#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "visitors.hpp"
#include "simplifier.hpp"
// #include "token.hpp"
#include <fstream>

// g++ -Wall -std=c++20 -g -O0 -mconsole -o BIN/main  Mathly/main.cpp Mathly/ast.cpp Mathly/visitors.cpp Mathly/simplifier.cpp

int checkParserErrors(const Parser& p) {
    std::vector<std::string> errors = p.errors;

    if (errors.size() == 0)
        return 0;
    
    std::cerr << "\n\nparser has " << errors.size() << " error[s]" << std::endl;

    for (const auto& i : errors) {
        std::cerr << "\nparser error: " << i;
    }
    std::cerr << std::endl;

    return 1;

}

int main() {
    
    
    const std::string PROMPT = ">> ";

    bool running = true;
    std::string expr1;
    std::string expr2;

    while (running == true) {

        // // Open an output file
        // std::ofstream outputFile("output.txt");
        // if (!outputFile.is_open()) {
        //     std::cerr << "Failed to open file for writing." << std::endl;
        //     return 1;
        // }
        std::cout << "                    " << "\n";
        std::cout << "Expr 1: "<< "\n" << PROMPT;
        std::getline(std::cin, expr1);
        std::cout << "Expr 2: "<< "\n" << PROMPT;
        std::getline(std::cin, expr2);
        expr1.append("$");
        expr2.append("$");

        if (expr1 == "exit$" || expr2 == "exit$") {
            running = false;
            break;
        }   
    
        // if (expr1.back() != '$') {
        //     std::cerr << "End in '$'" << std::endl;
        //     continue;
        // }

        // if (expr1.empty()) {
        //     std::cout << "No expr1";
        //     running = false;
        //     break;
        // }

        Lexer lexer = Lexer(expr1);
        Parser parser = Parser(lexer);
        auto parsedExpr = parser.parseLoop();

        Lexer lexer2 = Lexer(expr2);
        Parser parser2 = Parser(lexer2);
        auto parsedExpr2 = parser2.parseLoop();

        std::cout << parsedExpr->String() << " = " << parsedExpr2->String() << "\n";

        int ec = checkParserErrors(parser);
        if (ec) {
            break;
        }
        
        ec = checkParserErrors(parser2);
        if (ec) {
            break;
        }

        SimplifyVisitor visitor;
        parsedExpr->is_root = true;
        visitor.setRoot(parsedExpr.get());

        std::string og1 = parsedExpr->String();

        parsedExpr->accept(visitor);
        auto simplified = visitor.getResult();


        SimplifyVisitor visitor2;
        parsedExpr2->is_root = true;
        visitor2.setRoot(parsedExpr2.get());

        std::string og2 = parsedExpr2->String();

        parsedExpr2->accept(visitor2);
        auto simplified2 = visitor2.getResult();
        // std::cout << "Simplification DONE!" << "\n";
        // std::cout << simplified->String()  << " = "<< simplified2->String() << "\n";
        auto s1 = simplified->String();
        auto s2 = simplified2->String();

        // simplified = visitor.expand_tree(simplified);
        
        // simplified2 = visitor2.expand_tree(simplified2);
        
        
        //===============================================
        
        // std::cout << simplified->String()  << " = "<< simplified2->String() << "\n";

        std::stringstream ss;

        ss << "Let's simplify:" << og1 << " = " << og2 << "\n\n";

        ss << "Start with the LHS: " << simplified->String() << "\n";

        int step_num = 1;


        for (const auto& step: visitor.steps) {
            ss << "Step " << step_num << ": " << step.rule << "\n"; 
            ss << step.before << "\n" ;
            ss << step.description << ".\n";
            ss <<  step.after << ".\n";
            step_num++;
        }
        // ss << "Giving us: " << visitor.root_steps[1] << ".\n";

        ss << "\nLHS: " << simplified->String() << ".\n";

        ss << "====" << "\n";
        ss <<"Next, let's solve the RHS." << "\n";

        step_num = 1;

        for (const auto& step: visitor2.steps) {
            ss << "Step " << step_num << ": " << step.rule << "\n"; 
            ss << step.before << "\n" ;
            ss << step.description << ".\n";
            ss <<  step.after << ".\n";
            step_num++;
        }
        ss << "RHS:" << simplified2->String() << ".\n";
        
        
        // auto simplified = automatic_simplify(std::move(parsedExpr));
        // auto simplified = automatic_simplify(std::move(parsedExpr));

        // #######################################################
        //##################################################
        std::cout << ss.str() << "\n";
        std::cout << "STEPS DONE" << "\n";
          
        


        

        


        while (simplified->String() != "x" && !(visitor.isZero(simplified.get()) && visitor.isZero(simplified2.get()))) {

            visitor.rearrange_left(simplified, simplified2);
            std::cout << simplified->String()  << " = "<< simplified2->String() << "\n";
    
            visitor.rearrange_right(simplified, simplified2);

            std::cout << simplified->String()  << " = "<< simplified2->String() << "\n";
        }


        // visitor.solve_x(simplified, simplified2);

        std::cout << simplified->String()  << " = "<< simplified2->String() << "\n";

        // outputFile.flush();

        // outputFile.close();
        // std::cout << "Tokens written to output.txt" << std::endl;
    }
     
    return 0; 
}
// g++ -Wall -std=c++20 -g -O0 -mconsole -o BIN/main Mathly/main.cpp Mathly/lexer.cpp
