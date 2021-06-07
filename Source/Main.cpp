#include "CelluarAutomata2D.h"
#include <iostream>
#include <string>

// s23b3    Conway's
// s45678b3 Coral

int main(u32 argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "ERROR: Use only 1 argument, for Example: \"s45678b3\"" << std::endl;
        return EXIT_FAILURE;
    }

    std::string rule_to_parse{argv[1]};
    auto index_of_b = rule_to_parse.find('b');
    if (index_of_b == std::string::npos)
    {
        std::cout << "Parameter Error:" << std::endl;
        std::cout << rule_to_parse << " should contain b" << std::endl;
        return EXIT_FAILURE;
    }

    auto index_of_s = rule_to_parse.find('s');
    if (index_of_s == std::string::npos)
    {
        std::cout << "Parameter Error:" << std::endl;
        std::cout << rule_to_parse << " should contain s" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Rulestring: " << rule_to_parse << " | " << "s at: " << index_of_s << "b at: " << index_of_b << std::endl;

    auto start_of_b = index_of_b + 1;
    auto start_of_s = index_of_s + 1;
    auto end_of_b = start_of_b;
    auto end_of_s = start_of_s;

    if (index_of_b < index_of_s)
    {
        end_of_b = index_of_s;
        end_of_s = rule_to_parse.length();
    }
    else
    {
        end_of_s = index_of_b;
        end_of_b = rule_to_parse.length();
    }

    std::cout << "start b: " << start_of_b << " end b: " <<  end_of_b << " length b: " << end_of_b-start_of_b << std::endl;
    std::cout << "start s: " << start_of_s << " end s: " <<  end_of_s << " length s: " << end_of_s-start_of_s << std::endl;
    //auto is_character_b = [](char ch) { return ch == 'b' || ch == 'B'; };
    //auto is_character_s = [](char ch) { return ch == 's' || ch == 'S'; };

    auto rules_for_b = rule_to_parse.substr(start_of_b, end_of_b-start_of_b);
    auto rules_for_s = rule_to_parse.substr(start_of_s, end_of_s-start_of_s);

    std::cout << "b rules: " << rules_for_b << std::endl;
    std::cout << "s rules: " << rules_for_s << std::endl;

    for (size_t index{}; index < rules_for_b.length(); index++)
    {        
        auto character = rules_for_b[index];
        if (!(std::isdigit(character)))
        {
            std::cout << "Parameter Error:" << std::endl;
            std::cout << rule_to_parse << std::endl;
            std::cout << std::string((index_of_b + index + 1), ' ') << "^--- This should be a digit" << std::endl;
            return EXIT_FAILURE;
        }
        Cells::BornRules.push_back((u32)(character - 48));                  

    }

    for (auto& character : rules_for_s)
        Cells::SurviveRules.push_back(std::stoi(std::string{character}));   // That?!

    std::cout << "b: ";
    for (auto& rule :Cells::BornRules)
        std::cout << rule;
    std::cout << std::endl;

    std::cout << "s: ";
    for (auto& rule :Cells::SurviveRules)
        std::cout << rule; 
    std::cout << std::endl;

    WinApp::Start();
}