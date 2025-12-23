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
  If,
  While,
  Assignment,
  Return,
  UnaryOp,
  BinaryOp,
  Literal,
  UNKNOWN
};


struct ASTNode {
  ASTKind kind;
  explicit ASTNode(ASTKind kind) : kind(kind) {}
};

//using ArenaVec = std::vector<ASTNode*, ArenaAllocator<ASTNode*>>;
//using Params = std::vector<std::string, ArenaAllocator<std::string>>;

using StmtSlice   = Slice<ASTNode*>;
using ParamSlice  = Slice<std::string>;


struct ClassDecl : ASTNode {
  std::string id;
  StmtSlice members;

  ClassDecl(std::string id, StmtSlice members)
  : ASTNode(ASTKind::ClassDecl), id(std::move(id)), members(members){}
};

struct FunctionDecl : ASTNode {
  std::string id;
  ParamSlice params;
  StmtSlice body;

  FunctionDecl(std::string id, ParamSlice params, StmtSlice body)
  : ASTNode(ASTKind::FunctionDecl), id(std::move(id)), params(params), body(body){}
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

struct IfStatement : ASTNode {
  ASTNode* condition;
  StmtSlice then_body;
  enum class next { None, If, Else} next;
  union {
    IfStatement* else_if;
    StmtSlice else_block;
  };

  IfStatement(ASTNode* condition, StmtSlice then_body)
  : ASTNode(ASTKind::If), condition(condition), then_body(then_body), next(next::None) {}
};
struct WhileStatement : ASTNode {
  ASTNode* condition;
  StmtSlice body;
  WhileStatement(ASTNode* condition, StmtSlice body)
  : ASTNode(ASTKind::While), condition(condition), body(body){}
};

struct ReturnStatement : ASTNode {
  ASTNode* expr;
  ReturnStatement(ASTNode* expr): ASTNode(ASTKind::Return), expr(expr){}
};

