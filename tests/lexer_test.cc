#include <gtest/gtest.h>
#include <vector>
#include "lexer.h"

TEST(LexerTest, TokenizeOperators) {
  Lexer lexer("+ - * / < > = <= >= != !");
  auto list_of_tokens = lexer.Tokenize();

  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::PLUS);
  EXPECT_EQ(list_of_tokens.at(0).literal, "+");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::MINUS);
  EXPECT_EQ(list_of_tokens.at(1).literal, "-");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::STAR);
  EXPECT_EQ(list_of_tokens.at(2).literal, "*");

  EXPECT_EQ(list_of_tokens.at(3).type, TokenType::SLASH);
  EXPECT_EQ(list_of_tokens.at(3).literal, "/");

  EXPECT_EQ(list_of_tokens.at(4).type, TokenType::LESSER_THAN);
  EXPECT_EQ(list_of_tokens.at(4).literal, "<");

  EXPECT_EQ(list_of_tokens.at(5).type, TokenType::GREATER_THAN);
  EXPECT_EQ(list_of_tokens.at(5).literal, ">");

  EXPECT_EQ(list_of_tokens.at(6).type, TokenType::ASSIGN);
  EXPECT_EQ(list_of_tokens.at(6).literal, "=");

  EXPECT_EQ(list_of_tokens.at(7).type, TokenType::LESSER_OR_EQUAL);
  EXPECT_EQ(list_of_tokens.at(7).literal, "<=");

  EXPECT_EQ(list_of_tokens.at(8).type, TokenType::GREATER_OR_EQUAL);
  EXPECT_EQ(list_of_tokens.at(8).literal, ">=");

  EXPECT_EQ(list_of_tokens.at(9).type, TokenType::NOT_EQUAL);
  EXPECT_EQ(list_of_tokens.at(9).literal, "!=");
 
  EXPECT_EQ(list_of_tokens.at(10).type, TokenType::BANG);
  EXPECT_EQ(list_of_tokens.at(10).literal, "!");

  EXPECT_EQ(list_of_tokens.at(11).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(11).literal, "");

  ASSERT_EQ(list_of_tokens.size(), 12);
}

TEST(LexerTest, TokenizeKeywords) {
  Lexer lexer("while if else let const true false or and return function");
  auto list_of_tokens = lexer.Tokenize();
  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::WHILE);
  EXPECT_EQ(list_of_tokens.at(0).literal, "while");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::IF);
  EXPECT_EQ(list_of_tokens.at(1).literal, "if");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::ELSE);
  EXPECT_EQ(list_of_tokens.at(2).literal, "else");

  EXPECT_EQ(list_of_tokens.at(3).type, TokenType::LET);
  EXPECT_EQ(list_of_tokens.at(3).literal, "let");

  EXPECT_EQ(list_of_tokens.at(4).type, TokenType::CONST);
  EXPECT_EQ(list_of_tokens.at(4).literal, "const");

  EXPECT_EQ(list_of_tokens.at(5).type, TokenType::TRUE);
  EXPECT_EQ(list_of_tokens.at(5).literal, "true");

  EXPECT_EQ(list_of_tokens.at(6).type, TokenType::FALSE);
  EXPECT_EQ(list_of_tokens.at(6).literal, "false");

  EXPECT_EQ(list_of_tokens.at(7).type, TokenType::OR);
  EXPECT_EQ(list_of_tokens.at(7).literal, "or");

  EXPECT_EQ(list_of_tokens.at(8).type, TokenType::AND);
  EXPECT_EQ(list_of_tokens.at(8).literal, "and");

  EXPECT_EQ(list_of_tokens.at(9).type, TokenType::RETURN);
  EXPECT_EQ(list_of_tokens.at(9).literal, "return");

  EXPECT_EQ(list_of_tokens.at(10).type, TokenType::FUNCTION);
  EXPECT_EQ(list_of_tokens.at(10).literal, "function");

  EXPECT_EQ(list_of_tokens.at(11).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(11).literal, "");

  ASSERT_EQ(list_of_tokens.size(), 12);
}

TEST(LexerTest, TokenizeWhitespaceOnly) {
  Lexer lexer("   \n\r\t   ");
  auto list_of_tokens = lexer.Tokenize();

  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(0).literal, "");

  ASSERT_EQ(list_of_tokens.size(), 1);
}

TEST(LexerTest, TokenizeIdentifiers) {
  Lexer lexer("hello world");
  auto list_of_tokens = lexer.Tokenize();

  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::IDENTIFIER);
  EXPECT_EQ(list_of_tokens.at(0).literal, "hello");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::IDENTIFIER);
  EXPECT_EQ(list_of_tokens.at(1).literal, "world");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(2).literal, "");
  ASSERT_EQ(list_of_tokens.size(), 3);
}

TEST(LexerTest, TokenizeNumbers) {
  Lexer lexer(" 100   84091.140  ");
  auto list_of_tokens = lexer.Tokenize();

  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::NUMBER);
  EXPECT_EQ(list_of_tokens.at(0).literal, "100");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::NUMBER);
  EXPECT_EQ(list_of_tokens.at(1).literal, "84091.140");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(2).literal, "");

  ASSERT_EQ(list_of_tokens.size(), 3);
}

TEST(LexerTest, TokenizeProgram) {
  Lexer lexer("let bro = function(){}");
  auto list_of_tokens = lexer.Tokenize();
  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::LET);
  EXPECT_EQ(list_of_tokens.at(0).literal, "let");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::IDENTIFIER);
  EXPECT_EQ(list_of_tokens.at(1).literal, "bro");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::ASSIGN);
  EXPECT_EQ(list_of_tokens.at(2).literal, "=");

  EXPECT_EQ(list_of_tokens.at(3).type, TokenType::FUNCTION);
  EXPECT_EQ(list_of_tokens.at(3).literal, "function");

  EXPECT_EQ(list_of_tokens.at(4).type, TokenType::LPAREN);
  EXPECT_EQ(list_of_tokens.at(4).literal, "(");

  EXPECT_EQ(list_of_tokens.at(5).type, TokenType::RPAREN);
  EXPECT_EQ(list_of_tokens.at(5).literal, ")");

  EXPECT_EQ(list_of_tokens.at(6).type, TokenType::LBRACE);
  EXPECT_EQ(list_of_tokens.at(6).literal, "{");

  EXPECT_EQ(list_of_tokens.at(7).type, TokenType::RBRACE);
  EXPECT_EQ(list_of_tokens.at(7).literal, "}");

  EXPECT_EQ(list_of_tokens.at(8).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(8).literal, "");

  ASSERT_EQ(list_of_tokens.size(), 9);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

