#pragma once
#include <variant>
#include <string>
#include <memory>
#include <vector>
#include "tokens.h"

enum class NodeType {
  CLASSDECL, FUNCTIONDECL, VARIABLEDECL, ASSIGNMENT, LITERAL,
  UNARYOP, BINARYOP, WHILESTATEMENT, IFSTATEMENT,
  RETURNSTATEMENT, FUNCTIONCALL, ARRAYDECL, UNKNOWN
};

struct IAST {
  IAST(NodeType type = NodeType::UNKNOWN): node_type(type){}
  virtual ~IAST() = default;
  const NodeType node_type{};
};

using ExprPtr    = std::unique_ptr<IAST>;
using StmtsPtr   = std::vector<ExprPtr>;
using ExprsPtr   = StmtsPtr;
using ParamSlice = std::vector<std::string>;

auto debug_see_nodetype(const IAST* node, int indent) noexcept -> void;
static auto debug_see_nodetype(const StmtsPtr& stmts, int indent = 0) noexcept -> void {
  for (const auto& stmt : stmts)
    debug_see_nodetype(stmt.get(), indent);
}

template <NodeType Knodetype>
struct NodeImpl : IAST {
  NodeImpl(): IAST(Knodetype) {}
};


// Nodes
struct ClassDecl final : NodeImpl<NodeType::CLASSDECL> {
  std::string id{};
  StmtsPtr members{};
  ClassDecl(std::string id, StmtsPtr members) : id(std::move(id)), members(std::move(members)){}
};

struct FunctionDecl final : NodeImpl<NodeType::FUNCTIONDECL> {
  std::string id{};
  ParamSlice params{};
  StmtsPtr body{};

  FunctionDecl(std::string id, ParamSlice params, StmtsPtr body)
  : id(std::move(id)), params(params), body(std::move(body)){}
};

struct Assignment final : NodeImpl<NodeType::ASSIGNMENT> {
  std::string id{};
  ExprPtr expr{};

  Assignment(std::string id, ExprPtr expr)
  : id(std::move(id)), expr(std::move(expr)){}
};

struct Literal final : NodeImpl<NodeType::LITERAL> {
  Token token;
  Literal(Token token)
  : token(token) {}
};

struct VariableDecl final : NodeImpl<NodeType::VARIABLEDECL> {
  bool is_const{};
  std::string id{};
  ExprPtr expr{};
  VariableDecl(bool is_const, const std::string& id, ExprPtr expr):
    is_const(is_const), id(id), expr(std::move(expr)){}
};

struct UnaryOp final : NodeImpl<NodeType::UNARYOP> {
  ExprPtr operand{};
  TokenType op = TokenType::ILEGAL;

  UnaryOp(TokenType op, ExprPtr operand)
  : op(op), operand(std::move(operand)) {}
};

struct BinaryOp final : NodeImpl<NodeType::BINARYOP> {
  ExprPtr left;
  ExprPtr right;
  Token op;
  BinaryOp(ExprPtr left, ExprPtr right, Token op)
  : left(std::move(left)), right(std::move(right)), op(std::move(op)) {}
};

struct IfStatement final : NodeImpl<NodeType::IFSTATEMENT> {
  ExprPtr condition{};
  StmtsPtr then_body{};

  // optional "else"
  std::variant<
    std::monostate,
    std::unique_ptr<IfStatement>, // else if
    StmtsPtr                      // else
  > next;

  IfStatement(ExprPtr condition, StmtsPtr then_body)
    : condition(std::move(condition)),
    then_body(std::move(then_body)) {}
};

struct WhileStatement final : NodeImpl<NodeType::WHILESTATEMENT> {
  ExprPtr condition{};
  StmtsPtr body{};
  WhileStatement(ExprPtr condition, StmtsPtr body)
  : condition(std::move(condition)), body(std::move(body)){}
};

struct ReturnStatement final : NodeImpl<NodeType::RETURNSTATEMENT> {
  ExprPtr expr{};
  ReturnStatement(ExprPtr expr)
  : expr(std::move(expr)){}
};

struct FunctionCall final : NodeImpl<NodeType::FUNCTIONCALL> {
  std::string id{};
  ExprsPtr exprs{};

  FunctionCall(std::string id, ExprsPtr exprs)
  : id(std::move(id)), exprs(std::move(exprs)){ }
};

struct ArrayDecl final : NodeImpl<NodeType::ARRAYDECL> {
  ExprsPtr data{};
  ArrayDecl(ExprsPtr data): data(std::move(data)) { }
};
