#include <gtest/gtest.h>
#include <vector>
#include "lexer.h"

TEST(LexerTest, TokenizeOperators) {
  Lexer lexer("+ - * / < > =");
  auto list_of_tokens = lexer.Tokenize();

  ASSERT_EQ(list_of_tokens.size(), 7);
  EXPECT_EQ(list_of_tokens[0].type, TokenType::PLUS);
  EXPECT_EQ(list_of_tokens[0].literal, "+");

  EXPECT_EQ(list_of_tokens[1].type, TokenType::MINUS);
  EXPECT_EQ(list_of_tokens[1].literal, "-");

  EXPECT_EQ(list_of_tokens[2].type, TokenType::STAR);
  EXPECT_EQ(list_of_tokens[2].literal, "*");

  EXPECT_EQ(list_of_tokens[3].type, TokenType::SLASH);
  EXPECT_EQ(list_of_tokens[3].literal, "/");

  EXPECT_EQ(list_of_tokens[4].type, TokenType::LESSER_THAN);
  EXPECT_EQ(list_of_tokens[4].literal, "<");

  EXPECT_EQ(list_of_tokens[5].type, TokenType::GREATER_THAN);
  EXPECT_EQ(list_of_tokens[5].literal, ">");

  EXPECT_EQ(list_of_tokens[6].type, TokenType::ASSIGN);
  EXPECT_EQ(list_of_tokens[6].literal, "=");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
