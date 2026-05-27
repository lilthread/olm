#include <gtest/gtest.h>
#include <array>
#include <stdexcept>
#include "lexer.h"
#include "tokens.h"
#include "utilities.h"

TEST(LexerTest, TokenizeBools) {
  Lexer lx("verdadero falso continuar");
  auto t1 = lx.next();
  EXPECT_EQ(t1.type, TokenType::BOOL);
  EXPECT_EQ(t1.literal, "verdadero");
  auto t2 = lx.next();
  EXPECT_EQ(t2.type, TokenType::BOOL);
  EXPECT_EQ(t2.literal, "falso");
  auto t3 = lx.next();
  EXPECT_EQ(t3.type, TokenType::CONTINUE);
  EXPECT_EQ(t3.literal, "continuar");

  EXPECT_EQ(lx.next().type, TokenType::END_OF_FILE);
}
TEST(LexerTest, TokenizeIdentifiers) {
  Lexer lx("hello world");
  auto t1 = lx.next();
  EXPECT_EQ(t1.type, TokenType::IDENTIFIER);
  EXPECT_EQ(t1.literal, "hello");
  auto t2 = lx.next();
  EXPECT_EQ(t2.type, TokenType::IDENTIFIER);
  EXPECT_EQ(t2.literal, "world");
  EXPECT_EQ(lx.next().type, TokenType::END_OF_FILE);
}
TEST(LexerTest, TokenizeOperators) {
  Lexer lx{"+ - * / < > se <= >= != ! = []"};
  std::array<Token, 15> expected_tkns {
    Token(TokenType::PLUS, "+"),
    Token(TokenType::MINUS, "-"),
    Token(TokenType::STAR, "*"),
    Token(TokenType::SLASH, "/"),
    Token(TokenType::LESSER_THAN, "<"),
    Token(TokenType::GREATER_THAN, ">"),
    Token(TokenType::ASSIGN, "se"),
    Token(TokenType::LESSER_OR_EQUAL, "<="),
    Token(TokenType::GREATER_OR_EQUAL, ">="),
    Token(TokenType::NOT_EQUAL, "!="),
    Token(TokenType::BANG, "!"),
    Token(TokenType::EQUAL, "="),
    Token(TokenType::LBRACKET, "["),
    Token(TokenType::RBRACKET, "]"),
    Token(TokenType::END_OF_FILE, ""),
  };

  EXPECT_EQ(expected_tkns.at(0), lx.next());
  EXPECT_EQ(expected_tkns.at(1), lx.next());
  EXPECT_EQ(expected_tkns.at(2), lx.next());
  EXPECT_EQ(expected_tkns.at(3), lx.next());
  EXPECT_EQ(expected_tkns.at(4), lx.next());
  EXPECT_EQ(expected_tkns.at(5), lx.next());
  EXPECT_EQ(expected_tkns.at(6), lx.next());
  EXPECT_EQ(expected_tkns.at(7), lx.next());
  EXPECT_EQ(expected_tkns.at(8), lx.next());
  EXPECT_EQ(expected_tkns.at(9), lx.next());
  EXPECT_EQ(expected_tkns.at(10), lx.next());
  EXPECT_EQ(expected_tkns.at(11), lx.next());
  EXPECT_EQ(expected_tkns.at(12), lx.next());
  EXPECT_EQ(expected_tkns.at(13), lx.next());
  EXPECT_EQ(expected_tkns.at(14), lx.next());
}

TEST(LexerTest, TokenizeIgnore) {
  Lexer lx("   \n\r\t\n   \r\t\n\n   ");
  auto tkn = lx.next();
  EXPECT_EQ(tkn.type, TokenType::END_OF_FILE);
  EXPECT_EQ(tkn.literal, "");
}

TEST(LexerTest, TokenizeLiteralStr) {
  std::string lil_str (R"(
    $
      THIS IS 
              A 
                LONG COMMENT
    $
    "this is a string literal "
    "lil bro, be bro."
  )");
  Lexer lx(lil_str);

  auto tkn1 = lx.next();
  EXPECT_EQ(tkn1.type, TokenType::STRING);
  EXPECT_EQ(tkn1.literal, "this is a string literal ");

  auto tkn2 = lx.next();
  EXPECT_EQ(tkn2.type, TokenType::STRING);
  EXPECT_EQ(tkn2.literal, "lil bro, be bro.");

  EXPECT_EQ(lx.next().type, TokenType::END_OF_FILE);
}

TEST(LexerTest, EmptyStr) {
  std::string str (R"(
    ""
  )");
  Lexer lx(str);
  auto tkn = lx.next();
  EXPECT_EQ(tkn.type, TokenType::STRING);
  EXPECT_EQ(tkn.literal, "");
}
TEST(LexerTest, UnterminatedString) {
  std::string str (R"(
    " I forgor 💀
  )");
  Lexer lx(str);
  // "(void)" remove [[nodiscard]] warning
  EXPECT_THROW((void)lx.next(), std::runtime_error);
}

TEST(LexerTest, TokenizeNumbers) {
  Lexer lx("100   84091.140");
  auto tkn1 = lx.next();
  EXPECT_EQ(tkn1.type, TokenType::INTEGER);
  EXPECT_EQ(tkn1.literal, "100");

  auto tkn2 = lx.next();
  EXPECT_EQ(tkn2.type, TokenType::FLOAT);
  EXPECT_EQ(tkn2.literal, "84091.140");

  EXPECT_EQ(lx.next(), TokenType::END_OF_FILE);
}

TEST(LexerTest, TokenizeProgram) {
  std::string s = R"(
    clase Persona
      func new(name)
        _name se name
        devolver este
      fin
      _name
    fin
  )";
  Lexer lx (s);
  std::array<Token, 15> expected_tkns {
    Token{TokenType::CLASS, "clase"},
    Token{TokenType::IDENTIFIER, "Persona"},
    Token{TokenType::FUNCTION, "func"},
    Token{TokenType::IDENTIFIER, "new"},
    Token{TokenType::LPAREN, "("},
    Token{TokenType::IDENTIFIER, "name"},
    Token{TokenType::RPAREN, ")"},
    Token{TokenType::IDENTIFIER, "_name"},
    Token{TokenType::ASSIGN, "se"},
    Token{TokenType::IDENTIFIER, "name"},
    Token{TokenType::RETURN, "devolver"},
    Token{TokenType::SELF, "este"},
    Token{TokenType::END, "fin"},
    Token{TokenType::IDENTIFIER, "_name"},
    Token{TokenType::END, "fin"},
  };
  ASSERT_EQ(expected_tkns.at(0),  lx.next());
  ASSERT_EQ(expected_tkns.at(1),  lx.next());
  ASSERT_EQ(expected_tkns.at(2),  lx.next());
  ASSERT_EQ(expected_tkns.at(3),  lx.next());
  ASSERT_EQ(expected_tkns.at(4),  lx.next());
  ASSERT_EQ(expected_tkns.at(5),  lx.next());
  ASSERT_EQ(expected_tkns.at(6),  lx.next());
  ASSERT_EQ(expected_tkns.at(7),  lx.next());
  ASSERT_EQ(expected_tkns.at(8),  lx.next());
  ASSERT_EQ(expected_tkns.at(9),  lx.next());
  ASSERT_EQ(expected_tkns.at(10), lx.next());
  ASSERT_EQ(expected_tkns.at(11), lx.next());
  ASSERT_EQ(expected_tkns.at(12), lx.next());
  ASSERT_EQ(expected_tkns.at(13), lx.next());
  ASSERT_EQ(expected_tkns.at(14), lx.next());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
