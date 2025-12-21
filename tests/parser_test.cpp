#include <gtest/gtest.h>
#include "ast.h"
#include "lexer.h"
#include "memoryarena.h"
#include "parser.h"

TEST(ParserTest, ParseExpr) {
  std::string str (R"(
    var arithmetic = 10 + 2 * 3
    arithmetic = 0
  )");
  auto tokens = tokenize(str);
  MemoryArena arena{1024 * 4};
  Parser parser(tokens, arena);

  auto program = parser.parseProgram();
  ASSERT_EQ(program.size(), 2);

  auto* var = static_cast<VariableDecl*>(program.at(0));
  EXPECT_EQ(var->id, "arithmetic");
  EXPECT_EQ(var->expr->kind, ASTKind::BinaryOp);
  auto* first_biop = static_cast<BinaryOp*>(var->expr);

  EXPECT_EQ(first_biop->left->kind, ASTKind::Literal);
  auto* left_num = static_cast<Literal*>(first_biop->left);
  EXPECT_EQ(left_num->token.literal, "10");

  EXPECT_EQ(first_biop->op, "+");

  EXPECT_EQ(first_biop->right->kind, ASTKind::BinaryOp);
  auto* biop_right = static_cast<BinaryOp*>(first_biop->right);

  EXPECT_EQ(biop_right->left->kind, ASTKind::Literal);
  auto* left_r = static_cast<Literal*>(biop_right->left);
  EXPECT_EQ(left_r->token.literal, "2");

  EXPECT_EQ(biop_right->op, "*");

  EXPECT_EQ(biop_right->left->kind, ASTKind::Literal);
  auto* right_r = static_cast<Literal*>(biop_right->right);
  EXPECT_EQ(right_r->token.literal, "3");


  auto* assign = static_cast<Assignment*>(program.at(1));
  EXPECT_EQ(assign->id, "arithmetic");
  EXPECT_EQ(assign->expr->kind, ASTKind::Literal);
  auto* new_value = static_cast<Literal*>(assign->expr);
  EXPECT_EQ(new_value->token.literal, "0");
}

TEST(ParserTest, ParseVariableDeclarationLiteral) {
  std::string str (R"(

    var mi_numero = 10
    constante PI = 3.14
    var mi_string = "hello there"

  )");
  auto tokens = tokenize(str);
  MemoryArena arena{1024 * 4};
  Parser parser(tokens, arena);
  auto program = parser.parseProgram();


  EXPECT_EQ(program.at(0)->kind, ASTKind::VariableDecl) << "First element is not a variable";
  auto* noconst = static_cast<VariableDecl*>(program.at(0));
  EXPECT_EQ(noconst->id, "mi_numero");
  EXPECT_EQ(noconst->is_const, false);
  EXPECT_EQ(noconst->expr->kind, ASTKind::Literal);
  auto* first_expr = static_cast<Literal*>(noconst->expr);
  EXPECT_EQ(first_expr->token.literal, "10");
  EXPECT_EQ(first_expr->token.type, TokenType::INTEGER);


  EXPECT_EQ(program.at(1)->kind, ASTKind::VariableDecl) << "Second element is not a variable";
  auto* const_variable = static_cast<VariableDecl*>(program.at(1));
  EXPECT_EQ(const_variable->is_const, true);
  EXPECT_EQ(const_variable->id, "PI");
  EXPECT_EQ(const_variable->expr->kind, ASTKind::Literal);
  auto* secound_expr = static_cast<Literal*>(const_variable->expr);
  EXPECT_EQ(secound_expr->token.literal, "3.14");
  EXPECT_EQ(secound_expr->token.type, TokenType::FLOAT);


  EXPECT_EQ(program.at(2)->kind, ASTKind::VariableDecl) << "Third element is not a variable";
  auto* str_variable = static_cast<VariableDecl*>(program.at(2));
  EXPECT_EQ(str_variable->id, "mi_string");
  EXPECT_EQ(str_variable->expr->kind, ASTKind::Literal);
  auto* thrid_expr = static_cast<Literal*>(str_variable->expr);
  EXPECT_EQ(thrid_expr->token.literal, "hello there");
  EXPECT_EQ(thrid_expr->token.type, TokenType::STRING);
}

TEST(ParserTest, ParseFunction) {
  std::string str (R"(

    fn el_mas_grande(a,b,c): Numero {}

  )");
  auto tokens = tokenize(str);
  MemoryArena arena{1024 * 4};
  Parser parser(tokens, arena);
  auto program = parser.parseProgram();
  ASSERT_EQ(program.at(0)->kind, ASTKind::FunctionDecl) << "First element is not a function";
  auto* function_decl = static_cast<FunctionDecl*>(program.at(0));
  ASSERT_EQ(function_decl->id, "el_mas_grande");
  ASSERT_EQ(function_decl->params.size(), 3);
  ArenaVec function_statements_body = function_decl->body;
}

TEST(ParserTest, ParseClass) {
  std::string str (R"(

    clase Skibidi {
      fn toilet(): Void {}
    }

  )");
  auto tokens = tokenize(str);
  MemoryArena arena{1024 * 4};
  Parser parser(tokens, arena);
  auto program = parser.parseProgram();

  ASSERT_EQ(program.at(0)->kind, ASTKind::ClassDecl) << "First element is not a class";
  auto* class_declaration = static_cast<ClassDecl*>(program.at(0));
  EXPECT_EQ(class_declaration->id, "Skibidi");
  EXPECT_EQ(class_declaration->members.size(), 1);

  EXPECT_EQ(class_declaration->members.at(0)->kind, ASTKind::FunctionDecl) << "first member isn't a method class: " + class_declaration->id;
  auto* method_declaration = static_cast<FunctionDecl*>(class_declaration->members.at(0));
  EXPECT_EQ(method_declaration->id, "toilet");
  EXPECT_EQ(method_declaration->params.size(), 0);

  ArenaVec function_statements_body = method_declaration->body;
  EXPECT_EQ(function_statements_body.size(), 0);
}

