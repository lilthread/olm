#include <cstdint>
#include <cstdlib>
#include <optional>
#include <print>
#include <string_view>
#include "nodes.h"
#include "utils.h"
#include "parser.h"
#include "sema.h"

[[noreturn]] auto help_panel() noexcept -> void {
  std::println("--------------------------");
  std::println("Uso: ./olm <archivo> -o <archivo de salida>");
  std::println("Uso: ./olm <archivo> --ast");
  std::println("--------------------------");
  exit(EXIT_SUCCESS);
}

[[noreturn]] auto print_ast(std::string_view file) noexcept -> void {
  auto opt = read_file(file);
  if (!opt.has_value()) {
    std::println(stderr, "Error: no se pudo abrir el archivo '{}'", file);
    exit(EXIT_FAILURE);
  }
  StmtsPtr ast;
  try {
    Parser parser{opt.value()};
    ast = parser.parse();
  } catch (const std::runtime_error& e) {
    std::println(stderr, "{}", e.what());
    exit(EXIT_FAILURE);
  }
  debug_see_nodetype(ast);
  exit(EXIT_SUCCESS);
}

auto main(int argc,  char** argv) noexcept -> int32_t {
  if (argc < 2) {
    help_panel();
    return EXIT_SUCCESS;
  }
  std::string_view input = argv[1];
  const char* out_path = "a.out";
  bool show_ast{};

  for (auto i{2uz}; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg == "-o" && i + 1 < argc) {
      out_path = argv[++i];
    } else if (arg == "--ast") {
      show_ast = true;
      break;
    }
  }

  if (show_ast) {
    print_ast(input);
    return EXIT_SUCCESS;
  }

  auto opt = read_file(input);
  if (!opt.has_value()) {
    std::println(stderr, "Error: no se pudo abrir el archivo '{}'", input);
    exit(EXIT_FAILURE);
  }
  StmtsPtr ast;
  try {
    Parser parser{opt.value()};
    ast = parser.parse();
  } catch (const std::runtime_error& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }
  try {
    Sema sema;
    sema.analyze(ast);
  } catch (const SemanticException& e) {
    for (const auto& err : e.errors)
      std::println(stderr, "SemanticError: {}", err);
    return EXIT_FAILURE;
  }
  std::println("Salida: {}", out_path);
  return EXIT_SUCCESS;
}
