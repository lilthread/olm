#include "parser.h"
#include "nodes.h"
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
  throw std::runtime_error(std::format("Error de sintaxis [lina {} col {}]: {} (error: '{}')", _current.loc.row, _current.loc.col, msg, _current.literal));
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
    case CONTINUE: advance(); return std::make_unique<ContinueStatement>();
    default:                  return parse_assignment_or_call();
  }
}

auto Parser::parse_var_decl(bool is_const) -> ExprPtr {
  auto id_tok = expect(TokenType::IDENTIFIER, "se esperaba nombre de variable");
  expect(TokenType::ASSIGN, "esperado 'se' después nombre de variable");
  auto expr = parse_expression();
  return std::make_unique<VariableDecl>(is_const, id_tok.literal, std::move(expr));
}

auto Parser::parse_function_decl() -> ExprPtr {
  auto id_tok = expect(TokenType::IDENTIFIER, "se esperaba nombre de función");
  expect(TokenType::LPAREN, "esperado '(' después función name");
  auto params = parse_param_list();
  expect(TokenType::RPAREN, "esperado ')' después parámetros");
  auto body = parse_block();
  expect(TokenType::END, "esperado 'fin' al cerrar función");
  return std::make_unique<FunctionDecl>(id_tok.literal, std::move(params), std::move(body));
}

auto Parser::parse_class_decl() -> ExprPtr {
  auto id_tok = expect(TokenType::IDENTIFIER, "se esperaba un nombre a clase");
  auto members = parse_block();
  expect(TokenType::END, "esperado 'fin' al cerrar clase");
  return std::make_unique<ClassDecl>(id_tok.literal, std::move(members));
}

auto Parser::parse_if_statement() -> ExprPtr {
  auto condition = parse_expression();
  expect(TokenType::DO, "se esperaba 'haz' después condición-si");
  auto then_body = parse_block();

  auto node = std::make_unique<IfStatement>(std::move(condition), std::move(then_body));

  if (match(TokenType::ELSE)) {
    if (match(TokenType::IF)) {
      node->next = std::unique_ptr<IfStatement>(
          static_cast<IfStatement*>(parse_if_statement().release()));
      return node;
    } else
      node->next = parse_block();
  }

  expect(TokenType::END, "esperado 'fin' al cerrar condición-si");
  return node;
}

auto Parser::parse_while_statement() -> ExprPtr {
  auto condition = parse_expression();
  expect(TokenType::DO, "esperado 'haz' después condición-mientras");
  auto body = parse_block();
  expect(TokenType::END, "esperado 'fin' al cerrar condición-mientras");
  return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

auto Parser::parse_return_statement() -> ExprPtr {
  if (check(TokenType::END)) {
    return std::unique_ptr<ReturnStatement>(nullptr);
  }
  auto expr = parse_expression();
  return std::make_unique<ReturnStatement>(std::move(expr));
}

auto Parser::parse_assignment_or_call() -> ExprPtr {
  auto expr = parse_expression();

  if (match(TokenType::ASSIGN)) {
    auto value = parse_assignment_or_call();

    if (expr->node_type == NodeType::LITERAL) {
      auto* lit = static_cast<Literal*>(expr.get());
      if (lit->token.type != TokenType::IDENTIFIER)
        error("El lado izquierdo debe ser identificador o acceso");
    } else if (expr->node_type != NodeType::INDEXEXPR) {
      error("Lado izquierdo inválido en asignación");
    }
    return std::make_unique<Assignment>(std::move(expr), std::move(value));
  }

  return expr;
}

auto Parser::parse_block() -> StmtsPtr {
  StmtsPtr stmts;
  while (!at_end() and
      !check(TokenType::END) and
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
  while (check(TokenType::EQUAL) or check(TokenType::NOT_EQUAL)) {
    auto op = advance();
    auto right = parse_comparison();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_comparison() -> ExprPtr {
  auto left = parse_additive();
  while (check(TokenType::LESSER_THAN)     or check(TokenType::GREATER_THAN) or 
         check(TokenType::LESSER_OR_EQUAL) or check(TokenType::GREATER_OR_EQUAL)) {
    auto op = advance();
    auto right = parse_additive();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_additive() -> ExprPtr {
  auto left = parse_multiplicative();
  while (check(TokenType::PLUS) or check(TokenType::MINUS)) {
    auto op = advance();
    auto right = parse_multiplicative();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_multiplicative() -> ExprPtr {
  auto left = parse_unary();
  while (check(TokenType::STAR) or check(TokenType::SLASH)) {
    auto op = advance();
    auto right = parse_unary();
    left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op);
  }
  return left;
}

auto Parser::parse_unary() -> ExprPtr {
  if (check(TokenType::BANG) or check(TokenType::MINUS)) {
    auto op = advance();
    auto operand = parse_unary();
    return std::make_unique<UnaryOp>(op.type, std::move(operand));
  }
  return parse_postfix();
}

auto Parser::parse_postfix() -> ExprPtr {
  auto expr = parse_primary();

  while (true) {
    if (check(TokenType::LPAREN)) {
      // Direct call: the primary must have been an identifier
      if (expr->node_type != NodeType::LITERAL)
        error("solo los identificadores son llamables");
      auto* lit = static_cast<Literal*>(expr.get());
      if (lit->token.type != TokenType::IDENTIFIER)
        error("solo los identificadores son llamables");
      std::string id = lit->token.literal;
      advance(); // consume '('
      auto args = parse_arg_list();
      expect(TokenType::RPAREN, "esperado ')' después lista de argumentos");
      expr = std::make_unique<FunctionCall>(std::move(id), std::move(args));

    } else if (match(TokenType::DOT)) {
      auto member = expect(TokenType::IDENTIFIER, "se esperaba id del miembro después '.'");

      if (check(TokenType::LPAREN)) {
        advance(); // consume '('
        auto args = parse_arg_list();
        expect(TokenType::RPAREN, "esperado ')' después de llamada a metodo");
        expr = std::make_unique<MethodCall>(std::move(expr), member.literal, std::move(args), member.loc);
      } else {
        auto dot_token = Token{TokenType::IDENTIFIER, static_cast<Literal*>(expr.get())->token.literal + "." + member.literal, member.loc};
        expr = std::make_unique<Literal>(dot_token);
      }
    } else if (match(TokenType::LBRACKET)) {
      auto index = parse_expression();
      expect(TokenType::RBRACKET, "esperado ']' en index");
      expr = std::make_unique<IndexExpr>( std::move(expr), std::move(index));
    } else {
      break;
    }
  }
  return expr;
}

auto Parser::parse_primary() -> ExprPtr {
  using enum TokenType;

  if (check(INTEGER) or check(FLOAT) or check(STRING) or
      check(BOOL)    or check(SELF)  or check(NIL)) {
    return std::make_unique<Literal>(advance());
  }

  if (check(IDENTIFIER)) {
    return std::make_unique<Literal>(advance());
  }

  if (match(LPAREN)) {
    auto expr = parse_expression();
    expect(RPAREN, "esperado ')' después expresión");
    return expr;
  }

  if (match(LBRACKET)) {
    return parse_array_literal();
  }

  error("token invalido en expresión");
}

auto Parser::parse_param_list() -> ParamSlice {
  ParamSlice params;
  if (check(TokenType::RPAREN)) return params; // empty
  do {
    auto tok = expect(TokenType::IDENTIFIER, "se esperaba nombre del parámetro");
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
    do
      items.push_back(parse_expression());
    while (match(TokenType::COMMA));
  }
  expect(TokenType::RBRACKET, "esperaba que ']' cerrara el array litera");
  return std::make_unique<ArrayDecl>(std::move(items));
}
