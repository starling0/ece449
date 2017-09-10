#include <iostream>
#include <fstream>
#include <string>

#include "evl_token.hpp"

#include "../lex/token_gen.hpp"
#include "../lex/store_token.hpp"

int main(int argc, char *argv[]){
    //throw error if no file is provided for lex analysis
    if (argc < 2){
        std::cerr << "You should provide a file name." << std::endl;
        return -1;
    }

    std::string evl_file = argv[1];
    std::vector<evl_token> tokens;
    if (!extract_tokens_from_file(evl_file, tokens)) {
        return -1;
    }
    display_tokens(tokens); // why we need it?
    if (!store_tokens_to_file(evl_file+".tokens", tokens)) {
        return -1;
    }
    return 0;
}
