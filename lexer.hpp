enum Kind {
  ID,
  NUM,
  OP,
  ASSIGN,
  QUOTE_STRING,
  ARROW,
  COMMA,
  COMMENT,
  KEYWORD,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  NEWLINE
};

enum Operator {
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  BSL, // Bit shift left
  BSR  // Bit shift right
};

enum Comp {
  EQ,
  GT,
  LT,
  GTE,
  LTE
};

const std::string keywords[3] = {
  "fn", "return"
};

struct Token {
  Kind kind;
  std::string value;
  int row = -1;
  int col = -1;
};

enum Keyword {
  FN,
  RETURN,
};

Keyword getKeyword(Token *token);

std::vector<Token> lex(std::string source);
std::string KindName(Kind kind);

std::runtime_error errorAt(Token *token, std::string message);