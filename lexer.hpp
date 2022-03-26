enum Kind {
  ID,
  NUM,
  OP,
  ASSIGN,
  QUOTE_STRING,
  STRING_EXPR_START,
  STRING_EXPR_END,
  ARROW,
  COMMA,
  COMMENT,
  KEYWORD,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
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

std::vector<Token> lex(std::string source);
std::string KindName(Kind kind);