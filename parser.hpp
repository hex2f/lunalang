#include <map>
#include <vector>

class Node {
public:
  enum class BaseType {
    NODE, EXPR, FUNC, FUNC_CALL, FUNCPAR, STRING_LITERAL, NUMERIC_LITERAL, ID, ASSIGN
  };
  int col;
  int row;
  Node(Token* token) {
    this->col = token->col;
    this->row = token->row;
  }
  virtual std::string toString(int* depth)  {
    std::string result = std::string((*depth)*4, ' ');
    result += "Node{}";
    return result;
  }
  virtual BaseType type() {
    return Node::BaseType::NODE;
  }
};

class StringLiteralNode: public Node {
public:
  std::string value;
  StringLiteralNode(Token* token) : Node(token) {
    this->value = token->value;
  }
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "StringLiteralNode{\""+this->value+"\"}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::STRING_LITERAL;
  }
};

class NumericLiteralNode: public Node {
public:
  std::string value;
  NumericLiteralNode(Token* token) : Node(token) {
    this->value = token->value;
  }
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "NumericLiteralNode{\""+this->value+"\"}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::NUMERIC_LITERAL;
  }
};

class IdentifierNode: public Node {
public:
  std::string value;
  IdentifierNode(Token* token) : Node(token) {
    this->value = token->value;
  }
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "IdentifierNode{"+this->value+"}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::ID;
  }
};

class AssignmentNode: public Node {
public:
  Node* left;
  Node* right;
  AssignmentNode(Token* token) : Node(token) {}
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "AssignmentNode{\n";
    (*depth)++;
    result += left->toString(depth) + '\n';
    result += right->toString(depth) + '\n';
    (*depth)--;
    result += std::string((*depth)*4, ' ') + "}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::ASSIGN;
  }
};


class ExpressionNode: public Node {
public:
  std::vector<Node*> children;
  ExpressionNode(Token* token) : Node(token) {}
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "ExpressionNode{\n";
    (*depth)++;
    std::cout << "ExpressionNode children size: " << this->children.size() << std::endl;
    for (auto child : this->children) {
      result += child->toString(depth) + "\n";
    }
    (*depth)--;
    result += std::string((*depth)*4, ' ') + "}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::EXPR;
  }
};

class FunctionParameterNode: public Node {
public:
  std::string name;
  std::string typ;
  FunctionParameterNode(Token* nameToken, Token* typToken) : Node(nameToken) {
    this->name = nameToken->value;
    this->typ = typToken->value;
  }
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "FunctionParameterNode{"+this->typ+" "+this->name+"}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::FUNCPAR;
  }
};

class FunctionNode: public Node {
public:
  std::string name;
  std::string returnTyp;
  std::vector<FunctionParameterNode*> params;
  std::vector<Node*> children;
  FunctionNode(Token* token) : Node(token) {}
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "FunctionNode(\n";
    (*depth)++;
    result += std::string((*depth)*4, ' ') + "params{";
    (*depth)++;
    for (auto param : this->params) {
      result += "\n" + param->toString(depth);
    }
    (*depth)--;
    if (this->params.size() > 0) {
      result += "\n";
      result += std::string((*depth)*4, ' ') + "}\n";
    } else {
      result += "}\n";
    }
    result += std::string((*depth)*4, ' ') + "name{"+this->name+"}\n";
    result += std::string((*depth)*4, ' ') + "returnTyp{"+this->returnTyp+"}\n";
    (*depth)--;
    result += std::string((*depth)*4, ' ') + ") {\n";
    (*depth)++;
    for (auto child : this->children) {
      result += child->toString(depth) + "\n";
    }
    (*depth)--;
    result += std::string((*depth)*4, ' ') + "}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::FUNC;
  }
};

class FunctionCallNode: public Node {
public:
  std::string name;
  FunctionNode* function;
  std::vector<Node*> arguments;
  FunctionCallNode(Token* token) : Node(token) {}
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "FunctionCallNode{\n";
    (*depth)++;
    result += std::string((*depth)*4, ' ') + "name{"+this->name+"}\n";
    // result += std::string((*depth)*4, ' ') + "function{"+this->function->name+"}\n";
    result += std::string((*depth)*4, ' ') + "arguments{";
    (*depth)++;
    for (auto arg : this->arguments) {
      result += "\n" + arg->toString(depth);
    }
    (*depth)--;
    if (this->arguments.size() > 0) {
      result += "\n";
      result += std::string((*depth)*4, ' ') + "}\n";
    } else {
      result += "}\n";
    }
    (*depth)--;
    result += std::string((*depth)*4, ' ') + "}";
    return result;
  }
};

class ReturnNode: public Node {
public:
  Node* child = NULL;
  ReturnNode(Token* token) : Node(token) {}
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    if (child != NULL) {
      std::cout << "ReturnNode["<< child << "]" << std::endl;
      result += "ReturnNode{\n";
      (*depth)++;
      result += child->toString(depth);
      (*depth)--;
      result += "\n" + std::string((*depth)*4, ' ') + "}";
    } else {
      result += "ReturnNode{}";
    }
    return result;
  }
  BaseType type() override {
    return Node::BaseType::EXPR;
  }
};

class OperatorNode: public Node {
public:
  Operator op;
  std::string opStr;
  Node* left;
  Node* right;
  OperatorNode(Token* token) : Node(token) {
    this->opStr = token->value;
    if (token->value == "+") { this->op = Operator::ADD; }
    else if (token->value == "-") { this->op = Operator::SUB; }
    else if (token->value == "*") { this->op = Operator::MUL; }
    else if (token->value == "/") { this->op = Operator::DIV; }
    else if (token->value == "%") { this->op = Operator::MOD; }
    else if (token->value == ">>") { this->op = Operator::BSR; }
    else if (token->value == "<<") { this->op = Operator::BSL; }
    else { throw errorAt(token, "expected a valid operator. Found: \"" + token->value + "\" ["+KindName(token->kind)+"]"); }
  }
  std::string toString(int* depth) override  {
    std::string result = std::string((*depth)*4, ' ');
    result += "OperatorNode{\n";
    (*depth)++;
    result += this->left->toString(depth) + "\n";
    result += std::string((*depth)*4, ' ') + "op{"+this->opStr+"}\n";
    result += this->right->toString(depth);
    (*depth)--;
    result += "\n" + std::string((*depth)*4, ' ') + "}";
    return result;
  }
  BaseType type() override {
    return Node::BaseType::EXPR;
  }
};

enum ContextType {
  MODULE,
  FUNC,
  BLOCK
};

struct ContextBlock {
public:
  std::vector<Node*> *children;
  ContextType typ;

  ContextBlock(std::vector<Node*> *children, ContextType typ) {
    this->children = children;
    this->typ = typ;
  }
};

struct Module {
public:
  std::string name;
  std::vector<Token> tokens;
  std::vector<Node*> children;
  // context stack
  std::vector<ContextBlock*> contextStack;
  int funcCallDepth = 0;
  ContextBlock* context;
  Module() {
    this->context = new ContextBlock(&this->children, ContextType::MODULE);
    this->contextStack.push_back(this->context);
  }
};

Module parse(std::vector<Token> tokens);
void parseExpr(std::vector<Token> *tokens, int *i, Module *module, std::vector<Node*> *nodes);