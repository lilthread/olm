#include <gtest/gtest.h>
#include <vector>
#include "lexer.h"

TEST(LexerTest, TokenizeIdentifiers) {
  auto list_of_tokens = tokenize("hello world");

  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::IDENTIFIER);
  EXPECT_EQ(list_of_tokens.at(0).literal, "hello");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::IDENTIFIER);
  EXPECT_EQ(list_of_tokens.at(1).literal, "world");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(2).literal, "");
  ASSERT_EQ(list_of_tokens.size(), 3);
}

TEST(LexerTest, TokenizeOperators) {
  auto list_of_tokens = tokenize("+ - * / < > = <= >= != ! == ");
  ASSERT_EQ(list_of_tokens.size(), 13);

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

  EXPECT_EQ(list_of_tokens.at(11).type, TokenType::EQUAL);
  EXPECT_EQ(list_of_tokens.at(11).literal, "==");

  EXPECT_EQ(list_of_tokens.at(12).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(12).literal, "");
}


TEST(LexerTest, TokenizeIgnore) {
  auto list_of_tokens = tokenize("   \n\r\t\n   \r\t\n\n   ");
  ASSERT_EQ(list_of_tokens.size(), 1);
  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(0).literal, "");
}

TEST(LexerTest, TokenizeLiteralStr) {
  std::string lil_str (R"(
    "this is a string literal "
    "lil bro, be bro."
  )");
  auto list_of_tokens = tokenize(lil_str);

  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::STRING);
  EXPECT_EQ(list_of_tokens.at(0).literal, "this is a string literal ");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::STRING);
  EXPECT_EQ(list_of_tokens.at(1).literal, "lil bro, be bro.");


  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(2).literal, "");
  ASSERT_EQ(list_of_tokens.size(), 3);
}


TEST(LexerTest, TokenizeNumbers) {
  auto list_of_tokens = tokenize("100   84091.140");
  EXPECT_EQ(list_of_tokens.at(0).type, TokenType::INTEGER);
  EXPECT_EQ(list_of_tokens.at(0).literal, "100");

  EXPECT_EQ(list_of_tokens.at(1).type, TokenType::FLOAT);
  EXPECT_EQ(list_of_tokens.at(1).literal, "84091.140");

  EXPECT_EQ(list_of_tokens.at(2).type, TokenType::END_OF_FILE);
  EXPECT_EQ(list_of_tokens.at(2).literal, "");

  ASSERT_EQ(list_of_tokens.size(), 3);
}


TEST(LexerTest, TokenizeProgram) {
  std::string program_str(R"(
  clase Persona {
    # This is a comment.
    # colon + identifier( ':' IDENTIFIER ) are ignored just like python.
    fn new(name) {
      _name = name
      ret este 
    }
    _name
  }
  )");
  auto list_of_tokens = tokenize(program_str);
 
  ASSERT_EQ(list_of_tokens.at(0).type, TokenType::CLASS) << "Expect keyword: " + list_of_tokens.at(0).literal;
  ASSERT_EQ(list_of_tokens.at(1).type, TokenType::IDENTIFIER) << "Expect identifier Persona: " + list_of_tokens.at(0).literal;

  ASSERT_EQ(list_of_tokens.at(2).type, TokenType::LBRACE) << "Expect '{' : " + list_of_tokens.at(2).literal;

  ASSERT_EQ(list_of_tokens.at(3).type, TokenType::FUNCTION) << "Expect keyword 'fn': " + list_of_tokens.at(3).literal;

  ASSERT_EQ(list_of_tokens.at(4).type, TokenType::IDENTIFIER) << "Expect identifier 'kew': " + list_of_tokens.at(4).literal;

  ASSERT_EQ(list_of_tokens.at(5).type, TokenType::LPAREN) << "Expect '(': " + list_of_tokens.at(4).literal;


  ASSERT_EQ(list_of_tokens.at(6).type, TokenType::IDENTIFIER) << "Expect identifier 'name': " + list_of_tokens.at(6).literal;

  ASSERT_EQ(list_of_tokens.at(7).type, TokenType::RPAREN) << "Expect ')': " + list_of_tokens.at(7).literal;

  ASSERT_EQ(list_of_tokens.at(8).type, TokenType::LBRACE) << "Expect '{' : " + list_of_tokens.at(8).literal;

  ASSERT_EQ(list_of_tokens.at(9).type, TokenType::IDENTIFIER) << "Expect identifier '_name': " + list_of_tokens.at(9).literal;

  ASSERT_EQ(list_of_tokens.at(10).type, TokenType::ASSIGN) << "Expect op '=': " + list_of_tokens.at(10).literal;

  ASSERT_EQ(list_of_tokens.at(11).type, TokenType::IDENTIFIER) << "Expect identifier 'name': " + list_of_tokens.at(11).literal;

  ASSERT_EQ(list_of_tokens.at(12).type, TokenType::RETURN) << "Expect keyword 'ret': " + list_of_tokens.at(12).literal;

  ASSERT_EQ(list_of_tokens.at(13).type, TokenType::SELF) << "Expect keyword 'este':" + list_of_tokens.at(13).literal;


  ASSERT_EQ(list_of_tokens.at(14).type, TokenType::RBRACE) << "Expect '}': " + list_of_tokens.at(14).literal;

  ASSERT_EQ(list_of_tokens.at(15).type, TokenType::IDENTIFIER);
  ASSERT_EQ(list_of_tokens.at(15).literal, "_name");

  ASSERT_EQ(list_of_tokens.at(16).type, TokenType::RBRACE);
  ASSERT_EQ(list_of_tokens.at(16).literal, "}");


  ASSERT_EQ(list_of_tokens.at(17).type, TokenType::END_OF_FILE);
  ASSERT_EQ(list_of_tokens.at(17).literal, "");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
