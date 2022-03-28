#include <iostream>
#include <map>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

#define next()\
  i++;\
  continue;

// struct Token {
//   Kind kind;
//   std::string value;
//   int row = -1;
//   int col = -1;
// };

void _parseOp(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  std::cout << "Parsing op... " << tokens->at(*i).value << std::endl;
  OperatorNode* opnode = new OperatorNode {&tokens->at((*i))};
  std::cout << "opnode - " << tokens->at(*i).value << std::endl;
  if (nodes->size() > 0 && nodes->at(nodes->size()-1)->type() == Node::BaseType::EXPR) {
    opnode->left = nodes->at(nodes->size()-1);
    nodes->pop_back();
  } else {
    if (tokens->at(*i-1).kind == Kind::ID) {
      IdentifierNode* idnode = new IdentifierNode {&tokens->at(*i-1)};
      opnode->left = idnode;
    } else if (tokens->at(*i-1).kind == Kind::NUM) {
      NumericLiteralNode* numnode = new NumericLiteralNode {&tokens->at(*i-1)};
      opnode->left = numnode;
    } else if (tokens->at(*i-1).kind == Kind::QUOTE_STRING) {
      StringLiteralNode* strnode = new StringLiteralNode {&tokens->at(*i-1)};
      opnode->left = strnode;
    } else {
      throw errorAt(&tokens->at(*i-1), "Expected valid operator, found: " + tokens->at(*i-1).value + "[" + KindName(tokens->at(*i).kind) + "]");
    }
  }
  (*i)++;
  if ((*i) < tokens->size() && tokens->at(*i).kind == Kind::LPAREN) {
    std::vector<Node*> opchildren;
    parseExpr(tokens, i, module, &opchildren);
    if (opchildren.size() != 1) {
      throw errorAt(&tokens->at(*i), "Expected exactly one child for operator");
    }
    opnode->right = opchildren.at(0);
  } else {
    if (tokens->at(*i).kind == Kind::ID) {
      IdentifierNode* idnode = new IdentifierNode {&tokens->at(*i)};
      opnode->right = idnode;
    } else if (tokens->at(*i).kind == Kind::NUM) {
      NumericLiteralNode* numnode = new NumericLiteralNode {&tokens->at(*i)};
      opnode->right = numnode;
    } else if (tokens->at(*i).kind == Kind::QUOTE_STRING) {
      StringLiteralNode* strnode = new StringLiteralNode {&tokens->at(*i)};
      opnode->right = strnode;
    } else {
      throw errorAt(&tokens->at(*i), "Expected valid operator, found: " + tokens->at(*i).value + "[" + KindName(tokens->at(*i).kind) + "]");
    }
  }
  nodes->push_back(opnode);
}

void parseOp(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  // recursively parse stacked operators
  while (tokens->at(*i).kind == OP) {
    if (tokens->at(*i).value == "+" && *i + 1 < tokens->size()) {
      _parseOp(tokens, i, module, nodes);
      if (*i+1 < tokens->size() && tokens->at(*i+1).kind == OP) {
        (*i)++;
      }
    } else {
      break;
    }
  }
}

void parseNum(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  std::cout << "Parsing num... " << tokens->at(*i).value << std::endl;
  NumericLiteralNode* node = new NumericLiteralNode{&tokens->at(*i)};
  (*i)++;
  bool is_oped = tokens->at(*i).kind == OP;
  if (is_oped) {
    parseOp(tokens, i, module, nodes);
  } else {
    nodes->push_back(node);
  }
}

void parseString(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  std::cout << "Parsing string... " << tokens->at(*i).value << std::endl;
  StringLiteralNode* node = new StringLiteralNode{&tokens->at(*i)};
  (*i)++;
  while (tokens->at(*i).kind == QUOTE_STRING) {
    node->value = tokens->at(*i).value;
    (*i)++;
  }
  bool is_oped = tokens->at(*i).kind == OP;
  if (is_oped) {
    parseOp(tokens, i, module, nodes);
  } else {
    nodes->push_back(node);
  }
}

void parseId(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  IdentifierNode* node = new IdentifierNode{&tokens->at(*i)};
  (*i)++;
  bool is_oped = tokens->at(*i).kind == OP;
  if (is_oped) {
    parseOp(tokens, i, module, nodes);
  } else {
    nodes->push_back(node);
  }
}


void parseFuncDef(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  FunctionNode* node = new FunctionNode {&tokens->at(*i)};
  ContextBlock* context = new ContextBlock {&node->children, FUNC};

  // Parse name
  if ((*i)+1 > tokens->size() || tokens->at((*i)+1).kind != Kind::ID) {
    throw errorAt(&tokens->at(*i), "Expected valid function name after 'fn', got '" + tokens->at((*i)+1).value + "'");
  }
  node->name = tokens->at((*i)+1).value;
  (*i) += 2;

  std::cout << "Parsing function " << node->name << std::endl;

  if ((*i) >= tokens->size() || (tokens->at(*i).kind != Kind::LPAREN && tokens->at(*i).kind != Kind::LBRACE && tokens->at(*i).kind != Kind::ARROW)) {
    throw errorAt(&tokens->at(*i), "Expected '(', '{', or '-> type' after function name");
  }
  
  // Parse params
  if ((*i) < tokens->size() && tokens->at(*i).kind == Kind::LPAREN) {
    std::cout << node->name << " has params. Parsing..." << std::endl;
    (*i)++;
    // example of params: "(a int)", "(a int, b str)"
    while ((*i)+2 < tokens->size() && tokens->at(*i).kind != Kind::RPAREN) {
      if (tokens->at(*i).kind != Kind::ID) {
        std::cout << tokens->at(*i).value << std::endl;
        throw errorAt(&tokens->at(*i), "Expected valid argument name");
      }
      // TODO: Check if type is valid
      if (tokens->at((*i)+1).kind != Kind::ID) {
        throw errorAt(&tokens->at(*i), "Expected valid argument type");
      }
      FunctionParameterNode* param = new FunctionParameterNode{&tokens->at(*i), &tokens->at((*i)+1)};
      node->params.push_back(param);
      std::cout << "Parsed param " << param->name << " of type " << param->typ << std::endl;
      (*i) += 2;
      if ((*i) < tokens->size() && tokens->at(*i).kind == Kind::COMMA) {
        (*i)++;
      }
    }
    (*i)++;
  } else {
    std::cout << node->name << " has no params." << std::endl;
  }

  // Parse return type.
  if ((*i) < tokens->size() && tokens->at(*i).kind == Kind::ARROW) {
    (*i)++;
    if ((*i) >= tokens->size() || tokens->at(*i).kind != Kind::ID) {
      throw errorAt(&tokens->at(*i), "Expected valid return type");
    }
    node->returnTyp = tokens->at(*i).value;
    std::cout << "Parsed return type " << node->returnTyp << std::endl;
    (*i)++;
  } else {
    std::cout << node->name << " has no return type. defaulting to void" << std::endl;
    node->returnTyp = "void";
  }

  nodes->push_back(node);
  module->contextStack.push_back(context);
  module->context = context;
}

void parseExpr(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  ExpressionNode* node = new ExpressionNode {&tokens->at(*i)};
  std::cout << "expr" << std::endl;
  while ((*i) < tokens->size() && tokens->at(*i).kind != Kind::NEWLINE) {
    if (module->funcCallDepth > 0 && tokens->at(*i).kind == Kind::COMMA) {
      std::cout << "comma" << std::endl;
      return;
    }
    std::cout << "exprkind: " << KindName(tokens->at(*i).kind) << std::endl;
    if (tokens->at(*i).kind == Kind::LPAREN) {
      (*i)++;
      parseExpr(tokens, i, module, &node->children);
      next();
    }
    if (tokens->at(*i).kind == Kind::QUOTE_STRING) {
      parseString(tokens, i, module, &node->children);
      next();
    }
    if (tokens->at(*i).kind == Kind::ID) {
      parseId(tokens, i, module, &node->children);
      next();
    }
    if (tokens->at(*i).kind == Kind::NUM) {
      parseNum(tokens, i, module, &node->children);
      next();
    }
    if (tokens->at(*i).kind == Kind::RPAREN) {
      (*i)++;
      break;
    }
    if ((*i) < tokens->size() && tokens->at(*i).kind == Kind::OP) {
      parseOp(tokens, i, module, &node->children);
      next();
    }
    (*i)++;
  }
  (*i)++;
  //if (node->children.size() > 0) {
    nodes->push_back(node);
  //}
}

void parseReturn(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  ReturnNode* node = new ReturnNode {&tokens->at(*i)};
  (*i)++;
  if (
    (*i) >= tokens->size() || (
      tokens->at(*i).kind != Kind::ID &&
      tokens->at(*i).kind != Kind::NUM &&
      tokens->at(*i).kind != Kind::QUOTE_STRING &&
      tokens->at(*i).kind != Kind::LPAREN
  )) {
    throw errorAt(&tokens->at(*i), "Expected valid return value");
  }

  std::vector<Node*> exprChildren;
  parseExpr(tokens, i, module, &exprChildren);
  if (exprChildren.size() != 1) {
    throw errorAt(&tokens->at(*i), "Expected single expression in return statement");
  }
  node->child = exprChildren[0];
  (*i)++;
  module->context->children->push_back(node);
}

void parseKeyword (std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  Keyword keyword = getKeyword(&tokens->at(*i));
  switch (keyword) {
    case Keyword::FN:
      parseFuncDef(tokens, i, module, nodes);
      break;
    case Keyword::RETURN:
      parseReturn(tokens, i, module, nodes);
      break;
    default:
      throw errorAt(&tokens->at(*i), "Expected valid keyword");
      break;
  }
}

void parseAssign (std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  AssignmentNode* node = new AssignmentNode {&tokens->at(*i)};
  if (tokens->at(*i).kind != Kind::ID) {
    throw errorAt(&tokens->at(*i), "Expected valid assignment target");
  }
  IdentifierNode* target = new IdentifierNode {&tokens->at(*i)};
  node->left = target;
  (*i)++;
  if (tokens->at(*i).kind != Kind::ASSIGN) {
    throw errorAt(&tokens->at(*i), "Expected '=' after assignment target");
  }
  (*i)++;
  if (tokens->at(*i).kind != Kind::ID &&
      tokens->at(*i).kind != Kind::NUM &&
      tokens->at(*i).kind != Kind::QUOTE_STRING &&
      tokens->at(*i).kind != Kind::LPAREN) {
    throw errorAt(&tokens->at(*i), "Expected valid assignment value");
  }
  std::vector<Node*> exprChildren;
  parseExpr(tokens, i, module, &exprChildren);
  if (exprChildren.size() != 1) {
    throw errorAt(&tokens->at(*i), "Expected single expression in assignment");
  }
  node->right = exprChildren[0];
  nodes->push_back(node);
}

void parseCall(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes) {
  module->funcCallDepth++;
  FunctionCallNode* node = new FunctionCallNode {&tokens->at(*i)};
  if (tokens->at(*i).kind != Kind::ID) {
    throw errorAt(&tokens->at(*i), "Expected valid function call target, found: " + KindName(tokens->at(*i).kind));
  }
  IdentifierNode* target = new IdentifierNode {&tokens->at(*i)};
  node->name = target->value;
  (*i)++;
  if (tokens->at(*i).kind != Kind::LPAREN) {
    throw errorAt(&tokens->at(*i), "Expected '(' after function call target");
  }
  (*i)++;
  while ((*i) < tokens->size() && tokens->at(*i).kind != Kind::RPAREN) {
    std::vector<Node*> exprChildren;
    parseExpr(tokens, i, module, &exprChildren);
    // workaround for comma-separated function calls, i am stupid
    if (exprChildren.size() == 1) {
      node->arguments.push_back(exprChildren[0]);
    }

    if ((*i) < tokens->size() && tokens->at(*i).kind == Kind::RPAREN) {
      break;
    }
    
    (*i)++;
  }

  if (tokens->at(*i).kind != Kind::RPAREN) {
    throw errorAt(&tokens->at(*i), "Expected ')' after function call");
  }

  (*i)++;
  nodes->push_back(node);
  module->funcCallDepth--;
}

std::string trace(Module *module) {
  std::string result = "Module{\n";
  int depth = 1;
  for (const auto node : module->children) {
    result += node->toString(&depth) + '\n';
  }
  return result + "}";
}

Module parse(std::vector<Token> tokens) {
  Module module;
  module.tokens = tokens;
  int i = 0;
  while (i < module.tokens.size()) {
    switch (module.tokens.at(i).kind) {
      case Kind::KEYWORD:
        parseKeyword(&module.tokens, &i, &module, module.context->children);
        break;
      case Kind::RBRACE: {
        if (module.contextStack.size() == 0) {
          throw errorAt(&module.tokens.at(i), "Unexpected '}'");
        }
        std::cout << "Leaving context typ[" << module.context->typ << "]" << std::endl;
        module.contextStack.pop_back();
        auto context = module.contextStack.at(module.contextStack.size() - 1);
        module.context = context;
        break;
      }
      case Kind::ID: {
        if (module.tokens.at(i + 1).kind == Kind::ASSIGN) {
          std::cout << "assign" << std::endl;
          parseAssign(&module.tokens, &i, &module, module.context->children);
          break;
        }
        if (module.tokens.at(i + 1).kind == Kind::LPAREN) {
          std::cout << "call" << std::endl;
          parseCall(&module.tokens, &i, &module, module.context->children);
          break;
        }
        break;
      }
      default:
        break;
    }
    i++;
  }
  std::cout << trace(&module) << std::endl;
  return module;
}
