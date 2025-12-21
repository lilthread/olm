#pragma once
#include "lexer.h"
#include "memoryarena.h"
#include <iterator>
#include <span>
#include <string_view>
#include <vector>
#include <string>
#include <array>

enum class ASTKind {
  ClassDecl,
  FunctionDecl,
  VariableDecl,
  IfStatement,
  Assignment,
  Return,
  Identifier,
  UnaryOp,
  BinaryOp,
  Literal,
  UNKNOWN
};


struct ASTNode {
  ASTKind kind;
  explicit ASTNode(ASTKind kind) : kind(kind) {}
};

using ArenaVec = std::vector<ASTNode*, ArenaAllocator<ASTNode*>>;
using Params = std::vector<std::string, ArenaAllocator<std::string>>;


struct ClassDecl : ASTNode {
  std::string id;
  ArenaVec members;

  ClassDecl(const std::string& id, ArenaVec& members)
  : ASTNode(ASTKind::ClassDecl), id(id), members(members){}
};

struct FunctionDecl : ASTNode {
  std::string id;
  Params params;
  ArenaVec body;

  FunctionDecl(const std::string& id, Params& params, ArenaVec& body)
  : ASTNode(ASTKind::FunctionDecl), id(id), params(params), body(body){}
};


struct Assignment : ASTNode {
  std::string id;
  ASTNode* expr;

  Assignment(std::string id, ASTNode* expr)
  : ASTNode(ASTKind::Assignment), id(id), expr(expr){}
};

struct Literal : ASTNode {
  Token token;
  Literal(Token token)
  : ASTNode(ASTKind::Literal), token(std::move(token)) {}
};

struct VariableDecl : ASTNode {
  bool is_const;
  std::string id;
  ASTNode* expr;
  VariableDecl(bool is_const, const std::string& id, ASTNode* expr):
    ASTNode(ASTKind::VariableDecl), is_const(is_const), id(id), expr(expr){}
};

struct UnaryOp : ASTNode {
  std::string op;
  ASTNode* operand;

  UnaryOp(std::string_view op, ASTNode* operand)
  : ASTNode(ASTKind::UnaryOp), op(std::move(op)), operand(operand) {}
};

struct BinaryOp : ASTNode {
  ASTNode* left;
  ASTNode* right;
  std::string op;

  BinaryOp(ASTNode* left, ASTNode* right, std::string& op)
  : ASTNode(ASTKind::BinaryOp), left(left), right(right), op(std::move(op)) {}
};

// TODO:

struct IfStatement : ASTNode {

};

struct WhileStatement : ASTNode {

};

struct ReturnStmt : ASTNode {
};

