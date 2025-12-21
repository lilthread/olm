#pragma once
#include <span>
#include <vector>
#include "memoryarena.h"
#include "lexer.h"
#include "ast.h"


class Parser final {
public:
  ArenaVec parseProgram();

  Parser(const std::vector<Token>& tokens, MemoryArena& arena):
  _tkns(tokens), _idx(0), _arena(arena){}

private:
  std::vector<Token> _tkns;
  std::size_t _idx;
  MemoryArena& _arena;
  std::vector<ASTNode*, ArenaAllocator<ASTNode*>> _ast {ArenaAllocator<ASTNode*>(_arena)};


  const Token& peek(int steps = 0) const;
  const Token& advance();
  bool match(TokenType type);
  const Token& expect(TokenType type, const std::string message);


  ASTNode* parseExpression();
  ASTNode* parseAdditiveExpression();
  ASTNode* parseMultDivExpr();
  ASTNode* parseUnaryExpression();
  ASTNode* parsePrimaryExpression();


  ASTNode* parseDeclaration();
  VariableDecl* parseVar();
  FunctionDecl* parseFunction();
  ClassDecl* parseClass();
  IfStatement* parseIf();
  WhileStatement* parseWhile();
  ASTNode* parseClassMember();

  ArenaVec parseBlock();
  ASTNode* parseStatement();
  Assignment* parseAssignment();
};

