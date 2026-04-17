#pragma once
#include "nodes.h"
#include "lexer.h"

class Parser final {
public:
  [[nodiscard]] explicit Parser(std::string_view source)
  : _lexer(source), _current(_lexer.next()) {}

  [[nodiscard]] auto parse() -> StmtsPtr;

private:
  Lexer  _lexer;
  Token  _current;
  Token  _previous;

  auto advance()                                 -> Token;
  auto peek() const                              -> const Token&;
  auto check(TokenType t) const                  -> bool;
  auto match(TokenType t)                        -> bool;
  auto expect(TokenType t, std::string_view msg) -> Token;
  auto at_end() const                            -> bool;


  auto parse_statement()             -> ExprPtr;
  auto parse_var_decl(bool is_const) -> ExprPtr;
  auto parse_function_decl()         -> ExprPtr;
  auto parse_class_decl()            -> ExprPtr;
  auto parse_if_statement()          -> ExprPtr;
  auto parse_while_statement()       -> ExprPtr;
  auto parse_return_statement()      -> ExprPtr;
  auto parse_assignment_or_call()    -> ExprPtr;


  auto parse_expression()     -> ExprPtr;
  auto parse_or()             -> ExprPtr;
  auto parse_and()            -> ExprPtr;
  auto parse_equality()       -> ExprPtr;
  auto parse_comparison()     -> ExprPtr;
  auto parse_additive()       -> ExprPtr;
  auto parse_multiplicative() -> ExprPtr;
  auto parse_unary()          -> ExprPtr;
  auto parse_postfix()        -> ExprPtr;
  auto parse_primary()        -> ExprPtr;

  auto parse_block()         -> StmtsPtr;
  auto parse_param_list()    -> ParamSlice;
  auto parse_arg_list()      -> ExprsPtr;
  auto parse_array_literal() -> ExprPtr;

  [[noreturn]] auto error(std::string_view msg) const -> void;
};
