#include <iostream>
#include <fstream>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
  // read file at argv[1]
  // lex
  // print tokens
  std::cout << "You have entered " << argc << " arguments:" << "\n";

  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << "\n";

  std::ifstream sourceFile(argv[1]);
  std::string sourceText;
  std::string sourceLine;

  while (std::getline(sourceFile, sourceLine)) {
    std::cout << sourceLine << "\n";
    sourceText += sourceLine + "\n";
  }

  std::cout << "\nLexing...\n";

  std::vector<Token> tokens = lex(sourceText);

  std::cout << "Lexed " << tokens.size() << " tokens:\n";

  for (Token token : tokens) {
    std::string colrow = "";
    colrow += std::to_string(token.row) + ":" + std::to_string(token.col);
    while (colrow.size() < 5) {
      colrow += " ";
    }
    std::string tokenKindName = "Token<";
    tokenKindName += KindName(token.kind);
    tokenKindName += ">";
    // Pad tokenKindName
    while (tokenKindName.size() < 24) {
      tokenKindName += " ";
    }

    std::cout << colrow << tokenKindName << " " << token.value << "\n";
  }

  std::cout << "\nParsing...\n";
  Module mod = parse(tokens);

  return 0;
}