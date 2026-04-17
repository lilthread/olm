#include "nodes.h"
#include <print>
#include <variant>

auto debug_see_nodetype(const IAST* node, int indent) noexcept -> void {
  if (!node) {
    std::println("{}NULO", std::string(indent, ' '));
    return;
  }
  auto pad = std::string(indent, ' ');

  switch (node->node_type) {

    case NodeType::IFSTATEMENT: {
      auto* x = static_cast<const IfStatement*>(node);

      std::println("{}SI {{", pad);
      std::println("{}  CONDICIÓN:", pad);
      debug_see_nodetype(x->condition.get(), indent + 4);

      std::println("{}  HAZ {{", pad);
      for (const auto& stmt : x->then_body)
        debug_see_nodetype(stmt.get(), indent + 4);
      std::println("{}  }}", pad);

      std::visit([&](const auto& next) {
        using T = std::decay_t<decltype(next)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
          // nothing
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<IfStatement>>) {
          std::println("{}  SINO SI {{", pad);
          debug_see_nodetype(next.get(), indent + 4);
          std::println("{}  }}", pad);
        }
        else if constexpr (std::is_same_v<T, StmtsPtr>) {
          std::println("{}  SINO {{", pad);
          for (const auto& stmt : next)
            debug_see_nodetype(stmt.get(), indent + 4);
          std::println("{}  }}", pad);
        }
      }, x->next);

      std::println("{}}}", pad);
      break;
    }

    case NodeType::WHILESTATEMENT: {
      auto* x = static_cast<const WhileStatement*>(node);
      std::println("{}MIENTRAS {{", pad);

      std::println("{}  CONDICIÓN:", pad);
      debug_see_nodetype(x->condition.get(), indent + 4);

      std::println("{}  CUERPO {{", pad);
      for (const auto& stmt : x->body)
        debug_see_nodetype(stmt.get(), indent + 4);
      std::println("{}  }}", pad);

      std::println("{}}}", pad);
      break;
    }

    case NodeType::FUNCTIONDECL: {
      auto* x = static_cast<const FunctionDecl*>(node);
      std::println("{}FUNCIÓN/MÉTODO: {} {{", pad, x->id);

      for (const auto& stmt : x->body)
        debug_see_nodetype(stmt.get(), indent + 2);

      std::println("{}}}", pad);
      break;
    }

    case NodeType::VARIABLEDECL: {
      auto* x = static_cast<const VariableDecl*>(node);
      if (!x->is_const)
        std::print("{}VAR: {} = ", pad, x->id);
      else
        std::println("{}const: {} = ", pad, x->id);

      if (x->expr) {
        std::println("{{", pad);
        debug_see_nodetype(x->expr.get(), indent + 2);
        std::println("{}}}", pad);
      }

      break;
    }

    case NodeType::ASSIGNMENT: {
      auto* x = static_cast<const Assignment*>(node);
      std::print("{}ASIGNACIÓN: {} =", pad, x->id);

      if (x->expr) {
        std::println("{{", pad);
        debug_see_nodetype(x->expr.get(), indent + 2);
        std::println("{}}}", pad);
      }

      break;
    }

    case NodeType::LITERAL: {
      auto* x = static_cast<const Literal*>(node);
      std::println("{}LITERAL: {}", pad, x->token.literal);
      break;
    }

    case NodeType::BINARYOP: {
      auto* x = static_cast<const BinaryOp*>(node);
      std::println("{}OPERACIÓN BINARIA {{", pad);

      debug_see_nodetype(x->left.get(), indent + 2);
      debug_see_nodetype(x->right.get(), indent + 2);

      std::println("{}}}", pad);
      break;
    }

    case NodeType::UNARYOP: {
      auto* x = static_cast<const UnaryOp*>(node);
      std::println("{}OPERACIÓN UNARIA {{", pad);

      debug_see_nodetype(x->operand.get(), indent + 2);
      std::println("{}}}", pad);
      break;
    }

    case NodeType::RETURNSTATEMENT: {
      auto* x = static_cast<const ReturnStatement*>(node);
      std::print("{}RET: ", pad);

      if (x->expr) {
        std::println("{{", pad);
        debug_see_nodetype(x->expr.get(), indent + 2);
        std::println("{}}}", pad);
      }

      break;
    }

    case NodeType::FUNCTIONCALL: {
      auto* x = static_cast<const FunctionCall*>(node);
      std::println("{}CALL: {}() {{", pad, x->id);

      for (const auto& arg : x->exprs)
        debug_see_nodetype(arg.get(), indent + 2);

      std::println("{}}}", pad);
      break;
    }

    case NodeType::ARRAYDECL: {
      auto* x = static_cast<const ArrayDecl*>(node);
      std::println("{}ARRAY {{", pad);

      for (const auto& el : x->data)
        debug_see_nodetype(el.get(), indent + 2);

      std::println("{}}}", pad);
      break;
    }

    case NodeType::CLASSDECL: {
      auto* x = static_cast<const ClassDecl*>(node);
      std::println("{}CLASE {} {{", pad, x->id);

      for (const auto& m : x->members)
        debug_see_nodetype(m.get(), indent + 2);

      std::println("{}}}", pad);
      break;
    }

    default:
      std::println("{}ILEGAL", pad);
      break;
  }
}
