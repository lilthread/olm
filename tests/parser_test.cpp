#include <gtest/gtest.h>
#include "nodes.h"
#include "parser.h"
#include "tokens.h"

template<class T>
static auto as(const ExprPtr& ptr) -> T* {
  return static_cast<T*>(ptr.get());
}

static auto parse_ok(std::string_view src) -> StmtsPtr {
  Parser p{src};
  return p.parse();
}

TEST(Parser, VarDeclInteger) {
  auto stmts = parse_ok("var x se 42");
  ASSERT_EQ(stmts.size(), 1u);
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);
  EXPECT_FALSE(decl->is_const);
  EXPECT_EQ(decl->id, "x");
  auto* lit = as<Literal>(decl->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.type, TokenType::INTEGER);
  EXPECT_EQ(lit->token.literal, "42");
}

TEST(Parser, VarDeclFloat) {
  auto stmts = parse_ok("var pi se 3.14");
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);
  auto* lit = as<Literal>(decl->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.type, TokenType::FLOAT);
  EXPECT_EQ(lit->token.literal, "3.14");
}

TEST(Parser, VarDeclString) {
  auto stmts = parse_ok(R"(var s se "hola")");
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);
  auto* lit = as<Literal>(decl->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.type, TokenType::STRING);
}

TEST(Parser, VarDeclBoolTrue) {
  auto stmts = parse_ok("var b se verdadero");
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);
  auto* lit = as<Literal>(decl->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.type, TokenType::BOOL);
  EXPECT_EQ(lit->token.literal, "verdadero");
}

TEST(Parser, VarDeclBoolFalse) {
  auto stmts = parse_ok("var b se falso");
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);
  auto* lit = as<Literal>(decl->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.literal, "falso");
}

TEST(Parser, ConstDecl) {
  auto stmts = parse_ok("const MAX se 100");
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);
  EXPECT_TRUE(decl->is_const);
  EXPECT_EQ(decl->id, "MAX");
}

TEST(Parser, VarDeclMissingEquals) {
  EXPECT_THROW(parse_ok("var x 42"), std::runtime_error);
}

TEST(Parser, VarDeclMissingName) {
  EXPECT_THROW(parse_ok("var se 42"), std::runtime_error);
}

TEST(Parser, AssignmentWithExpression) {
  auto stmts = parse_ok("x se a + b");
  auto* asgn = as<Assignment>(stmts[0]);
  ASSERT_NE(asgn, nullptr);
  auto* bin = as<BinaryOp>(asgn->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::PLUS);
}

TEST(Parser, BinaryAdd) {
  auto stmts = parse_ok("var r se 1 + 2");
  auto* decl = as<VariableDecl>(stmts[0]);
  auto* bin  = as<BinaryOp>(decl->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::PLUS);
  EXPECT_EQ(as<Literal>(bin->left)->token.literal, "1");
  EXPECT_EQ(as<Literal>(bin->right)->token.literal, "2");
}

TEST(Parser, BinarySub) {
  auto stmts = parse_ok("var r se 10 - 3");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::MINUS);
}

TEST(Parser, BinaryMul) {
  auto stmts = parse_ok("var r se 4 * 5");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::STAR);
}

TEST(Parser, BinaryDiv) {
  auto stmts = parse_ok("var r se 8 / 2");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::SLASH);
}

TEST(Parser, PrecedenceMulOverAdd) {
  // 1 + 2 * 3  →  BinaryOp(+, 1, BinaryOp(*, 2, 3))
  auto stmts = parse_ok("var r se 1 + 2 * 3");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::PLUS);
  auto* rhs = as<BinaryOp>(bin->right);
  ASSERT_NE(rhs, nullptr);
  EXPECT_EQ(rhs->op.type, TokenType::STAR);
}

TEST(Parser, PrecedenceParenOverMul) {
  // (1 + 2) * 3  →  BinaryOp(*, BinaryOp(+,1,2), 3)
  auto stmts = parse_ok("var r se (1 + 2) * 3");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::STAR);
  auto* lhs = as<BinaryOp>(bin->left);
  ASSERT_NE(lhs, nullptr);
  EXPECT_EQ(lhs->op.type, TokenType::PLUS);
}

TEST(Parser, ComparisonLessThan) {
  auto stmts = parse_ok("var r se a < b");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::LESSER_THAN);
}

TEST(Parser, ComparisonGreaterOrEqual) {
  auto stmts = parse_ok("var r se a >= b");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::GREATER_OR_EQUAL);
}

TEST(Parser, EqualityNotEqual) {
  auto stmts = parse_ok("var r se a != b");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::NOT_EQUAL);
}

TEST(Parser, LogicalAnd) {
  auto stmts = parse_ok("var r se a y b");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::AND);
}

TEST(Parser, LogicalOr) {
  auto stmts = parse_ok("var r se a o b");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::OR);
}

TEST(Parser, LogicalPrecedenceAndOverOr) {
  // a o b y c  →  Or(a, And(b, c))
  auto stmts = parse_ok("var r se a o b y c");
  auto* bin  = as<BinaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(bin, nullptr);
  EXPECT_EQ(bin->op.type, TokenType::OR);
  auto* rhs = as<BinaryOp>(bin->right);
  ASSERT_NE(rhs, nullptr);
  EXPECT_EQ(rhs->op.type, TokenType::AND);
}

TEST(Parser, UnaryNegation) {
  auto stmts = parse_ok("var r se -5");
  auto* decl = as<VariableDecl>(stmts[0]);
  auto* unary = as<UnaryOp>(decl->expr);
  ASSERT_NE(unary, nullptr);
  EXPECT_EQ(unary->op, TokenType::MINUS);
  auto* operand = as<Literal>(unary->operand);
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->token.literal, "5");
}

TEST(Parser, UnaryBang) {
  auto stmts = parse_ok("var r se !verdadero");
  auto* unary = as<UnaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(unary, nullptr);
  EXPECT_EQ(unary->op, TokenType::BANG);
}

TEST(Parser, DoubleUnaryNegation) {
  // --5  →  UnaryOp(-, UnaryOp(-, 5))
  auto stmts = parse_ok("var r se -(-5)");
  auto* outer = as<UnaryOp>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(outer, nullptr);
  auto* inner = as<UnaryOp>(outer->operand);
  ASSERT_NE(inner, nullptr);
  EXPECT_EQ(inner->op, TokenType::MINUS);
}

//  Function declaration
TEST(Parser, FuncDeclNoParams) {
  auto stmts = parse_ok("func saludar() fin");
  ASSERT_EQ(stmts.size(), 1u);
  auto* fn = as<FunctionDecl>(stmts[0]);
  ASSERT_NE(fn, nullptr);
  EXPECT_EQ(fn->id, "saludar");
  EXPECT_TRUE(fn->params.empty());
  EXPECT_TRUE(fn->body.empty());
}

TEST(Parser, FuncDeclWithParams) {
  auto stmts = parse_ok("func suma(a, b) fin");
  auto* fn = as<FunctionDecl>(stmts[0]);
  ASSERT_NE(fn, nullptr);
  ASSERT_EQ(fn->params.size(), 2u);
  EXPECT_EQ(fn->params[0], "a");
  EXPECT_EQ(fn->params[1], "b");
}

TEST(Parser, FuncDeclWithBody) {
  auto stmts = parse_ok("func doble(x) devolver x + x fin");
  auto* fn = as<FunctionDecl>(stmts[0]);
  ASSERT_NE(fn, nullptr);
  ASSERT_EQ(fn->body.size(), 1u);
  auto* ret = as<ReturnStatement>(fn->body[0]);
  ASSERT_NE(ret, nullptr);
}

TEST(Parser, FuncDeclMissingFin) {
  EXPECT_THROW(parse_ok("func f() devolver 1"), std::runtime_error);
}

TEST(Parser, FuncDeclMissingParen) {
  EXPECT_THROW(parse_ok("func f devolver 1 fin"), std::runtime_error);
}

TEST(Parser, ReturnLiteral) {
  auto stmts = parse_ok("func f() devolver 42 fin");
  auto* fn  = as<FunctionDecl>(stmts[0]);
  auto* ret = as<ReturnStatement>(fn->body[0]);
  ASSERT_NE(ret, nullptr);
  auto* lit = as<Literal>(ret->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.literal, "42");
}

TEST(Parser, ReturnExpression) {
  auto stmts = parse_ok("func f(a, b) devolver a + b fin");
  auto* fn  = as<FunctionDecl>(stmts[0]);
  auto* ret = as<ReturnStatement>(fn->body[0]);
  ASSERT_NE(ret, nullptr);
  EXPECT_NE(as<BinaryOp>(ret->expr), nullptr);
}

TEST(Parser, FuncCallNoArgs) {
  auto stmts = parse_ok("foo()");
  ASSERT_EQ(stmts.size(), 1u);
  auto* call = as<FunctionCall>(stmts[0]);
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->id, "foo");
  EXPECT_TRUE(call->exprs.empty());
}

TEST(Parser, FuncCallWithArgs) {
  auto stmts = parse_ok("suma(1, 2, 3)");
  auto* call = as<FunctionCall>(stmts[0]);
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->id, "suma");
  ASSERT_EQ(call->exprs.size(), 3u);
}

TEST(Parser, FuncCallArgIsExpression) {
  auto stmts = parse_ok("foo(a + b)");
  auto* call = as<FunctionCall>(stmts[0]);
  ASSERT_NE(call, nullptr);
  EXPECT_NE(as<BinaryOp>(call->exprs[0]), nullptr);
}

TEST(Parser, FuncCallNested) {
  auto stmts = parse_ok("foo(bar(1))");
  auto* outer = as<FunctionCall>(stmts[0]);
  ASSERT_NE(outer, nullptr);
  auto* inner = as<FunctionCall>(outer->exprs[0]);
  ASSERT_NE(inner, nullptr);
  EXPECT_EQ(inner->id, "bar");
}

TEST(Parser, MethodCall) {
  auto stmts = parse_ok("obj.metodo(1, 2)");
  auto* call = as<MethodCall>(stmts[0]);
  ASSERT_NE(call, nullptr); 
  EXPECT_EQ(call->name, "metodo");
  ASSERT_EQ(call->args.size(), 2u);
}

TEST(Parser, IfSimple) {
  auto stmts = parse_ok("si verdadero haz fin");
  ASSERT_EQ(stmts.size(), 1u);
  auto* ifst = as<IfStatement>(stmts[0]);
  ASSERT_NE(ifst, nullptr);
  EXPECT_TRUE(ifst->then_body.empty());
  EXPECT_TRUE(std::holds_alternative<std::monostate>(ifst->next));
}

TEST(Parser, IfWithBody) {
  auto stmts = parse_ok("si x haz var a se 1 fin");
  auto* ifst = as<IfStatement>(stmts[0]);
  ASSERT_NE(ifst, nullptr);
  ASSERT_EQ(ifst->then_body.size(), 1u);
  EXPECT_NE(as<VariableDecl>(ifst->then_body[0]), nullptr);
}

TEST(Parser, IfElse) {
  auto stmts = parse_ok("si x haz var a se 1 sino var b se 2 fin");
  auto* ifst = as<IfStatement>(stmts[0]);
  ASSERT_NE(ifst, nullptr);
  ASSERT_TRUE(std::holds_alternative<StmtsPtr>(ifst->next));
  auto& else_body = std::get<StmtsPtr>(ifst->next);
  ASSERT_EQ(else_body.size(), 1u);
}

TEST(Parser, IfElseIf) {
  auto stmts = parse_ok("si a haz var x se 1 sino si b haz var x se 2 fin");
  auto* ifst = as<IfStatement>(stmts[0]);
  ASSERT_NE(ifst, nullptr);
  ASSERT_TRUE(std::holds_alternative<std::unique_ptr<IfStatement>>(ifst->next));
  auto& elif = std::get<std::unique_ptr<IfStatement>>(ifst->next);
  EXPECT_NE(elif, nullptr);
}

TEST(Parser, IfMissingHaz) {
  EXPECT_THROW(parse_ok("si verdadero fin"), std::runtime_error);
}

TEST(Parser, IfMissingFin) {
  EXPECT_THROW(parse_ok("si verdadero haz var x = 1"), std::runtime_error);
}

TEST(Parser, WhileSimple) {
  auto stmts = parse_ok("mientras verdadero haz fin");
  ASSERT_EQ(stmts.size(), 1u);
  auto* wh = as<WhileStatement>(stmts[0]);
  ASSERT_NE(wh, nullptr);
  EXPECT_TRUE(wh->body.empty());
}

TEST(Parser, WhileWithBody) {
  auto stmts = parse_ok("mientras x < 10 haz x se x + 1 fin");
  auto* wh = as<WhileStatement>(stmts[0]);
  ASSERT_NE(wh, nullptr);
  EXPECT_NE(as<BinaryOp>(wh->condition), nullptr);
  ASSERT_EQ(wh->body.size(), 1u);
  EXPECT_NE(as<Assignment>(wh->body[0]), nullptr);
}
TEST(Parser, WhileMissingHaz) {
  EXPECT_THROW(parse_ok("mientras verdadero fin"), std::runtime_error);
}

TEST(Parser, WhileMissingFin) {
  EXPECT_THROW(parse_ok("mientras verdadero haz var x = 1"), std::runtime_error);
}

TEST(Parser, ClassEmpty) {
  auto stmts = parse_ok("clase Punto fin");
  ASSERT_EQ(stmts.size(), 1u);
  auto* cls = as<ClassDecl>(stmts[0]);
  ASSERT_NE(cls, nullptr);
  EXPECT_EQ(cls->id, "Punto");
  EXPECT_TRUE(cls->members.empty());
}

TEST(Parser, ClassWithMembers) {
  auto stmts = parse_ok("clase Punto var coord_x se 0 var coord_y se 0 fin");
  auto* cls = as<ClassDecl>(stmts[0]);
  ASSERT_NE(cls, nullptr);
  ASSERT_EQ(cls->members.size(), 2u);
  EXPECT_NE(as<VariableDecl>(cls->members[0]), nullptr);
  EXPECT_NE(as<VariableDecl>(cls->members[1]), nullptr);
}

TEST(Parser, ClassWithMethod) {
  auto stmts = parse_ok("clase Circulo func area() devolver 0 fin fin");
  auto* cls = as<ClassDecl>(stmts[0]);
  ASSERT_NE(cls, nullptr);
  ASSERT_EQ(cls->members.size(), 1u);
  EXPECT_NE(as<FunctionDecl>(cls->members[0]), nullptr);
}

TEST(Parser, ClassMissingFin) {
  EXPECT_THROW(parse_ok("clase Punto var x se 0"), std::runtime_error);
}

TEST(Parser, ArrayEmpty) {
  auto stmts = parse_ok("var a se []");
  auto* decl = as<VariableDecl>(stmts[0]);
  auto* arr  = as<ArrayDecl>(decl->expr);
  ASSERT_NE(arr, nullptr);
  EXPECT_TRUE(arr->data.empty());
}

TEST(Parser, ArrayLiteral) {
  auto stmts = parse_ok("var a se [1, 2, 3]");
  auto* arr  = as<ArrayDecl>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(arr, nullptr);
  ASSERT_EQ(arr->data.size(), 3u);
  EXPECT_EQ(as<Literal>(arr->data[0])->token.literal, "1");
  EXPECT_EQ(as<Literal>(arr->data[1])->token.literal, "2");
  EXPECT_EQ(as<Literal>(arr->data[2])->token.literal, "3");
}

TEST(Parser, ArrayWithExpressions) {
  auto stmts = parse_ok("var a se [1 + 2, foo()]");
  auto* arr  = as<ArrayDecl>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(arr, nullptr);
  ASSERT_EQ(arr->data.size(), 2u);
  EXPECT_NE(as<BinaryOp>(arr->data[0]), nullptr);
  EXPECT_NE(as<FunctionCall>(arr->data[1]), nullptr);
}

TEST(Parser, ArrayMissingBracket) {
  EXPECT_THROW(parse_ok("var a se [1, 2"), std::runtime_error);
}

TEST(Parser, Subscript) {
  auto stmts = parse_ok("var r se arr[0]");
  auto* decl = as<VariableDecl>(stmts[0]);
  ASSERT_NE(decl, nullptr);

  auto* idx = as<IndexExpr>(decl->expr);
  ASSERT_NE(idx, nullptr);

  auto* obj = as<Literal>(idx->object);
  ASSERT_NE(obj, nullptr);
  ASSERT_EQ(obj->token.literal, "arr");

  auto* index = as<Literal>(idx->index);
  ASSERT_NE(index, nullptr);
  ASSERT_EQ(index->token.literal, "0");
}

TEST(Parser, SubscriptAssignment) {
  auto stmts = parse_ok("arr[0] se 5");

  auto* assign = as<Assignment>(stmts[0]);
  ASSERT_NE(assign, nullptr);

  ASSERT_EQ(assign->target->node_type, NodeType::INDEXEXPR);
}

TEST(Parser, SelfKeyword) {
  auto stmts = parse_ok("var r se este");
  auto* lit  = as<Literal>(as<VariableDecl>(stmts[0])->expr);
  ASSERT_NE(lit, nullptr);
  EXPECT_EQ(lit->token.type, TokenType::SELF);
}

TEST(Parser, MultipleStatements) {
  auto stmts = parse_ok("var a se 1\nvar b se 2\nvar c se 3");
  EXPECT_EQ(stmts.size(), 3u);
}

TEST(Parser, EmptySource) {
  auto stmts = parse_ok("");
  EXPECT_TRUE(stmts.empty());
}

TEST(Parser, FibonacciFunction) {
  const char* src = R"(
    func fib(n)
      si n < 2 haz
        devolver n
      sino
        devolver fib(n - 1) + fib(n - 2)
      fin
    fin
  )";
  auto stmts = parse_ok(src);
  ASSERT_EQ(stmts.size(), 1u);
  auto* fn = as<FunctionDecl>(stmts[0]);
  ASSERT_NE(fn, nullptr);
  EXPECT_EQ(fn->id, "fib");
  ASSERT_EQ(fn->params.size(), 1u);
  ASSERT_EQ(fn->body.size(), 1u);
  EXPECT_NE(as<IfStatement>(fn->body[0]), nullptr);
}

TEST(Parser, CounterWhileLoop) {
  const char* src = R"(
    var i se 0
    mientras i < 10 haz
      i se i + 1
    fin
  )";
  auto stmts = parse_ok(src);
  ASSERT_EQ(stmts.size(), 2u);
  EXPECT_NE(as<VariableDecl>(stmts[0]), nullptr);
  auto* wh = as<WhileStatement>(stmts[1]);
  ASSERT_NE(wh, nullptr);
  ASSERT_EQ(wh->body.size(), 1u);
  EXPECT_NE(as<Assignment>(wh->body[0]), nullptr);
}

TEST(Parser, ClassWithMethodAndSelf) {
  const char* src = R"(
    clase Contador
      var n se 0
      func incrementar()
        este.n se este.n + 1
      fin
    fin
  )";
  auto stmts = parse_ok(src);
  auto* cls = as<ClassDecl>(stmts[0]);
  ASSERT_NE(cls, nullptr);
  ASSERT_EQ(cls->members.size(), 2u);
  EXPECT_NE(as<FunctionDecl>(cls->members[1]), nullptr);
}
