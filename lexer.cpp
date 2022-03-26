#include <iostream>
#include <fstream>
#include <vector>

#include "lexer.hpp"

std::vector<Token> lex(std::string source) {
  std::vector<Token> tokens;

  bool readingString = false;
  bool readingComment = false;
  bool readingNum = false;
  bool readingStringExpr = false;
  int row = 1;
  int pcol = 0;
  int col = 0;
  int i = 0;

  Token currentToken;

  auto startNewToken = [&](Kind kind, std::string value, bool immediate = false) {
    if (currentToken.value.size() > 0) {
      tokens.push_back(currentToken);
    }
    currentToken.kind = kind;
    currentToken.value = value;
    currentToken.row = row;
    currentToken.col = col;
    if (immediate) {
      tokens.push_back(currentToken);
      currentToken = {};
    }
  };

  auto pushback = [&]() {
    if (currentToken.value.size() > 0) {
      tokens.push_back(currentToken);
    }
    currentToken = {};
  };

  std::cout << "source length: " << source.size() << std::endl;
  while (i < source.size() - 1) {
    if (source[i] == '\n') {
      row++;
      pcol = i;
      col = 0;
      i++;
      readingString = false;
      readingComment = false;
      readingNum = false;
      readingStringExpr = false;
      if (currentToken.value.size() > 0) {
        tokens.push_back(currentToken);
        currentToken = {};
      }
    } else {
      col = i - pcol;
    }

    if (currentToken.row == -1) {
      currentToken.col = col;
      currentToken.row = row;
    }

    if (readingStringExpr && i > 0 && source[i - 1] != '\\' && source[i] == '}') {
      pushback();
      readingStringExpr = false;
      startNewToken(STRING_EXPR_END, "}", true);
      readingString = true;
      startNewToken(QUOTE_STRING, "");
      i++;
      continue;
    }

    if (((source[i] == '\t' || source[i] == ' ') && !readingComment && !readingString)) {
      i++;
      pushback();
      continue;
    }

    if (readingNum && (source[i] >= '0' && source[i] <= '9') || source[i] == '.') {
      currentToken.value += source[i];
      i++;
      continue;
    } else if (readingNum) {
      pushback();
      readingNum = false;
    }

    if ((i > 0 && source[i-1] == '\\' && readingString) || readingComment) {
      if (readingComment && source[i-1] == '/' && source[i] == ' ') {
        i++;
        continue;
      }
      currentToken.value += source[i];
      i++;
      continue;
    }

    if (source[i] == '"') {
      if (readingString) {
        pushback();
        readingString = false;
      } else {
        startNewToken(QUOTE_STRING, "");
        readingString = true;
      }
      i++;
      continue;
    }

    if (readingString && i > 0 && source[i-1] != '\\' && source[i] == '{') {
      pushback();
      readingString = false;
      readingStringExpr = true;
      startNewToken(STRING_EXPR_START, "{", true);
      i++;
    }

    if (readingString) {
      currentToken.value += source[i];
      i++;
      continue;
    }

    // TODO: very cursed, std::string() didnt work... fix thiss
    if (source[i] >= '0' && source[i] <= '9') {
      switch (source[i]) {
        case '0':
          startNewToken(NUM, "0");
          break;
        case '1':
          startNewToken(NUM, "1");
          break;
        case '2':
          startNewToken(NUM, "2");
          break;
        case '3':
          startNewToken(NUM, "3");
          break;
        case '4': 
          startNewToken(NUM, "4");
          break;
        case '5':
          startNewToken(NUM, "5");
          break;
        case '6':
          startNewToken(NUM, "6");
          break;
        case '7':
          startNewToken(NUM, "7");
          break;
        case '8':
          startNewToken(NUM, "8");
          break;
        case '9':
          startNewToken(NUM, "9");
          break;
      }
      readingNum = true;
      i++;
      continue;
    }

    if (source[i] == '(') {
      startNewToken(LPAREN, "(", true);
      i++;
      continue;
    }

    if (source[i] == ')') {
      startNewToken(RPAREN, ")", true);
      i++;
      continue;
    }

    if (source[i] == '{') {
      startNewToken(LBRACE, "{", true);
      i++;
      continue;
    }

    if (source[i] == '}') {
      startNewToken(RBRACE, "}", true);
      i++;
      continue;
    }

    if (source[i] == ',') {
      startNewToken(COMMA, ",", true);
      i++;
      continue;
    }

    if (i > 0 && source[i-1] == ':' && source[i] == '=') {
      startNewToken(ASSIGN, ":=", true);
      i++;
      continue;
    }

    if (source[i] == '-' && i+1 < source.size() && source[i+1] == '>') {
      startNewToken(ARROW, "->", true);
      i += 2;
      continue;
    }

    if (source[i] == '/' && i+1 < source.size() && source[i+1] == '/') {
      startNewToken(COMMENT, "");
      readingComment = true;
      i+=2;
      continue;
    }

    if (source[i] == '-' || source[i] == '+' || source[i] == '*' || source[i] == '/' || source[i] == '%') {
      startNewToken(OP, std::string(source[i], true));
      i++;
      continue;
    }

    if (i > 1 && (source[i-1] == '\n' || source[i-1] == ' ')) {
      std::string word = "";
      int k = 0;
      // AZaz_
      while (i+k < source.size() && (source[i+k] >= 'a' && source[i+k] <= 'z' || source[i+k] >= 'A' && source[i+k] <= 'Z' || source[i+k] == '_')) {
        word += source[i+k];
        k++;
      }
      bool found = false;
      if (word.size() > 0) {
        for (std::string keyword : keywords) {
          if (word == keyword) {
            startNewToken(KEYWORD, word, true);
            found = true;
            break;
          }
        }
      }
      if (!found && k > 0) {
        startNewToken(ID, word, true);
        i += k;
        continue;
      } else if (k > 0) {
        i += k;
        continue;
      }
      i++;
      continue;
    }

    if (source[i] != '\n' && source[i] != '\t' && source[i] != ' ') {
      currentToken.kind = ID;
      currentToken.value += source[i];
      i++;
      continue;
    }
    i++;
  }

  return tokens;
  
}

std::string KindName(Kind kind) {
  std::string tokenKindName = "";
  switch (kind) {
    case ID:
      tokenKindName += "ID";
      break;
    case KEYWORD:
      tokenKindName += "KEYWORD";
      break;
    case NUM:
      tokenKindName += "NUM";
      break;
    case LPAREN:
      tokenKindName += "LPAREN";
      break;
    case RPAREN:
      tokenKindName += "RPAREN";
      break;
    case LBRACE:
      tokenKindName += "LBRACE";
      break;
    case RBRACE:
      tokenKindName += "RBRACE";
      break;
    case QUOTE_STRING:
      tokenKindName += "QUOTE_STRING";
      break;
    case STRING_EXPR_START:
      tokenKindName += "STRING_EXPR_START";
      break;
    case STRING_EXPR_END:
      tokenKindName += "STRING_EXPR_END";
      break;
    case ASSIGN:
      tokenKindName += "ASSIGN";
      break;
    case ARROW:
      tokenKindName += "ARROW";
      break;
    case COMMA:
      tokenKindName += "COMMA";
      break;
    case COMMENT:
      tokenKindName += "COMMENT";
      break;
    case OP:
      tokenKindName += "OP";
      break;
    default:
      tokenKindName += "UNKNOWN";
      break;
  }
  return tokenKindName;
}