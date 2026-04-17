#include "parser.h"
#include "tokens.h"
#include <format>
#include <stdexcept>
#include <string>

auto Parser::at_end() const -> bool {
  return _current.type == TokenType::END_OF_FILE;
}

auto Parser::peek() const -> const Token& {
  return _current;
}

auto Parser::check(TokenType t) const -> bool {
  return _current.type == t;
}

auto Parser::advance() -> Token {
  _previous = _current;
  if (!at_end())
    _current = _lexer.next();
  return _previous;
}

auto Parser::match(TokenType t) -> bool {
  if (!check(t)) return false;
  advance();
  return true;
}

auto Parser::expect(TokenType t, std::string_view msg) -> Token {
  if (!check(t))
    error(msg);
  return advance();
}

auto Parser::error(std::string_view msg) const -> void {
  throw std::runtime_error(
      std::format("ParseError [line {} col {}]: {} (got '{}')",
                  _current.row, _current.col, msg, _current.literal));
}

auto Parser::parse() -> StmtsPtr {
  StmtsPtr stmts;
  while (!at_end())
    stmts.push_back(parse_statement());
  return stmts;
}

auto Parser::parse_statement() -> ExprPtr {
  using enum TokenType;
  switch (_current.type) {
    case VAR:      advance(); return parse_var_decl(false);
    case CONST:    advance(); return parse_var_decl(true);
    case FUNCTION: advance(); return parse_function_decl();
    case CLASS:    advance(); return parse_class_decl();
    case IF:       advance(); return parse_if_statement();
    case WHILE:    advance(); return parse_while_statement();
    case RETURN:   advance(); return parse_return_statement();
    default:                  return parse_assignment_or_call();
  }
}

auto Parser::parse_var_decl(bool is_const) -> ExprPtr {
  auto id_tok = expect(TokenType::IDENTIFIER, "expected variable name");
  expect(TokenType::ASSIGN, "expected 'se' after variable name");
  auto expr = parse_expression();
  return std::make_unique<VariableDecl>(is_const, id_tok.literal, std::move(expr));
}

auto Parser::parse_function_decl() -> ExprPtr {
  auto id_tok = expect(TokenType::IDENTIFIER, "expected function name");
  expect(TokenType::LPAREN, "expected '(' after function name");
  auto params = parse_param_list();
  expect(TokenType::RPAREN, "expected ')' after parameters");
  auto body = parse_block();
  expect(TokenType::END, "expected 'fin' to close function");
  return std::make_unique<FunctionDecl>(id_tok.literal, std::move(params), std::move(body));
}

auto Parser::parse_class_decl() -> ExprPtr {
  auto id_tok = expect(TokenType::IDENTIFIER, "expected class name");
  auto members = parse_block();
  expect(TokenType::END, "expected 'fin' to close class");
  return std::make_unique<ClassDecl>(id_tok.literal, std::move(members));
}

auto Parser::parse_if_statement() -> ExprPtr {
  auto condition = parse_expression();
  expect(TokenType::DO, "expected 'haz' after if-condition");
  auto then_body = parse_block();

  auto node = std::make_unique<IfStatement>(std::move(condition), std::move(then_body));

  if (match(TokenType::ELSE)) {
    if (match(TokenType::IF)) {
      // else-if chain: parse recursively
      node->next = std::unique_ptr<IfStatement>(
          static_cast<IfStatement*>(parse_if_statement().release()));
      // note: recursive call already consumed its own 'fin'
      return node;
    } else {
      // plain else block
      expect(TokenType::DO, "expected 'haz' after 'sino'");
      node->next = parse_block();
    }
  }

  expect(TokenType::END, "expected 'fin' to close if-statement");
  return node;
}

auto Parser::parse_while_statement() -> ExprPtr {
  auto condition = parse_expression();
  expect(TokenType::DO, "expected 'haz' after while-condition");
  auto body = parse_block();
  expect(TokenType::END, "expected 'fin' to close while-loop");
  return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

auto Parser::parse_return_statement() -> ExprPtr {
  auto expr = parse_expression();
  return std::make_unique<ReturnStatement>(std::move(expr));
}

auto Parser::parse_assignment_or_call() -> ExprPtr {
  // Parse expression normally; assignment is detected by checking what we got.
  // We first parse an expression, then check for a trailing '='.
  auto expr = parse_expression();

  if (match(TokenType::ASSIGN)) { // "se" keyword used as assignment
    // LHS must be an identifier literal
    if (expr->node_type != NodeType::LITERAL)
      error("left-hand side of assignment must be an identifier");
    auto* lit = static_cast<Literal*>(expr.get());
    if (lit->token.type != TokenType::IDENTIFIER)
      error("left-hand side of assignment must be an identifier");
    auto rhs = parse_expression();
    return std::make_unique<Assignment>(lit->token.literal, std::move(rhs));
  }

  return expr;
}

auto Parser::parse_block() -> StmtsPtr {
  StmtsPtr stmts;
  while (!at_end() &&
      !check(TokenType::END) &&
      !check(TokenType::ELSE)) {
    stmts.push_back(parse_statement());
  }
  return stmts;
}

auto Parser::parse_expression() -> ExprPtr { return parse_or(); }

auto Parser::parse_or() -> ExprPtr {
  auto left = parse_and();
  while (check(TokenType::OR)) {
    auto op = advance();
    auto right = parse_and();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_and() -> ExprPtr {
  auto left = parse_equality();
  while (check(TokenType::AND)) {
    auto op = advance();
    auto right = parse_equality();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_equality() -> ExprPtr {
  auto left = parse_comparison();
  while (check(TokenType::EQUAL) || check(TokenType::NOT_EQUAL)) {
    auto op = advance();
    auto right = parse_comparison();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_comparison() -> ExprPtr {
  auto left = parse_additive();
  while (check(TokenType::LESSER_THAN)     || check(TokenType::GREATER_THAN) ||
         check(TokenType::LESSER_OR_EQUAL) || check(TokenType::GREATER_OR_EQUAL)) {
    auto op = advance();
    auto right = parse_additive();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_additive() -> ExprPtr {
  auto left = parse_multiplicative();
  while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
    auto op = advance();
    auto right = parse_multiplicative();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_multiplicative() -> ExprPtr {
  auto left = parse_unary();
  while (check(TokenType::STAR) || check(TokenType::SLASH)) {
    auto op = advance();
    auto right = parse_unary();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_unary() -> ExprPtr {
  if (check(TokenType::BANG) || check(TokenType::MINUS)) {
    auto op = advance();
    auto operand = parse_unary();
    return std::make_unique<UnaryOp>(op.type, std::move(operand));
  }
  return parse_postfix();
}

// id(args) | id.method(args) | expr[idx]
auto Parser::parse_postfix() -> ExprPtr {
  auto expr = parse_primary();

  while (true) {
    if (check(TokenType::LPAREN)) {
      // Direct call: the primary must have been an identifier
      if (expr->node_type != NodeType::LITERAL)
        error("only identifiers are callable");
      auto* lit = static_cast<Literal*>(expr.get());
      if (lit->token.type != TokenType::IDENTIFIER)
        error("only identifiers are callable");
      std::string id = lit->token.literal;
      advance(); // consume '('
      auto args = parse_arg_list();
      expect(TokenType::RPAREN, "expected ')' after argument list");
      expr = std::make_unique<FunctionCall>(std::move(id), std::move(args));

    } else if (match(TokenType::DOT)) {
      // Member access / method call: build "obj.member" as a literal or call
      auto member = expect(TokenType::IDENTIFIER, "expected member name after '.'");
      /*if (check(TokenType::LPAREN)) {
        advance(); // consume '('
        auto args = parse_arg_list();
        expect(TokenType::RPAREN, "expected ')' after argument list");
        // Represent as FunctionCall with id = "lhs.member"
        // We encode the callee as a dotted string; the evaluator can split on '.'.
        std::string callee;
        if (expr->node_type == NodeType::LITERAL)
          callee = static_cast<Literal*>(expr.get())->token.literal + "." + member.literal;
        else if (expr->node_type == NodeType::FUNCTIONCALL)
          callee = static_cast<FunctionCall*>(expr.get())->id + "." + member.literal;
        else
          callee = "<expr>." + member.literal;
        expr = std::make_unique<FunctionCall>(std::move(callee), std::move(args));
      } else {*/
        // Property access: encode as a dotted identifier literal
        Token dot_token{TokenType::IDENTIFIER,
                        static_cast<Literal*>(expr.get())->token.literal + "." + member.literal,
                        member.row, member.col};
        expr = std::make_unique<Literal>(dot_token);
      //}

    }else if (match(TokenType::COLON)){
      auto member = expect(TokenType::IDENTIFIER, "expected member name after ':'");
      expect(TokenType::LPAREN, "expected '(' after identifier of method");
      auto args = parse_arg_list();
      expect(TokenType::RPAREN, "expected ')' after argument list");

      std::string callee;
      if (expr->node_type == NodeType::LITERAL)
        callee = static_cast<Literal*>(expr.get())->token.literal + ":" + member.literal;
      else if (expr->node_type == NodeType::FUNCTIONCALL)
        callee = static_cast<FunctionCall*>(expr.get())->id + ":" + member.literal;
      else
        callee = "<expr>:" + member.literal;

      expr = std::make_unique<FunctionCall>(std::move(callee), std::move(args));

    } else if (match(TokenType::LBRACKET)) {
      // Subscript: encode as binary op with '[' … but TokenType has no LBRACKET op.
      // We represent it as a FunctionCall to the built-in "__index__".
      auto index = parse_expression();
      expect(TokenType::RBRACKET, "expected ']' after index expression");
      ExprsPtr args;
      args.push_back(std::move(expr));
      args.push_back(std::move(index));
      expr = std::make_unique<FunctionCall>("__index__", std::move(args));

    } else {
      break;
    }
  }
  return expr;
}

auto Parser::parse_primary() -> ExprPtr {
  using enum TokenType;

  if (check(INTEGER) || check(FLOAT) || check(STRING) ||
      check(BOOL)    || check(SELF)) {
    return std::make_unique<Literal>(advance());
  }

  if (check(IDENTIFIER)) {
    return std::make_unique<Literal>(advance());
  }

  if (match(LPAREN)) {
    auto expr = parse_expression();
    expect(RPAREN, "expected ')' after expression");
    return expr;
  }

  if (match(LBRACKET)) {
    return parse_array_literal();
  }

  error("unexpected token in expression");
}

auto Parser::parse_param_list() -> ParamSlice {
  ParamSlice params;
  if (check(TokenType::RPAREN)) return params; // empty
  do {
    auto tok = expect(TokenType::IDENTIFIER, "expected parameter name");
    params.push_back(tok.literal);
  } while (match(TokenType::COMMA));
  return params;
}

auto Parser::parse_arg_list() -> ExprsPtr {
  ExprsPtr args;
  if (check(TokenType::RPAREN)) return args; // empty
  do {
    args.push_back(parse_expression());
  } while (match(TokenType::COMMA));
  return args;
}

auto Parser::parse_array_literal() -> ExprPtr {
  ExprsPtr items;
  if (!check(TokenType::RBRACKET)) {
    do {
      items.push_back(parse_expression());
    } while (match(TokenType::COMMA));
  }
  expect(TokenType::RBRACKET, "expected ']' to close array literal");
  return std::make_unique<ArrayDecl>(std::move(items));
}
