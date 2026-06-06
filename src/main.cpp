#include <print>
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include "error_manager.h"
#include "nodes.h"
#include "parser.h"
#include "sema.h"
#include "interpreter.h"
#include "utilities.h"
#include "repl.h"

constexpr char const* HELP_PANEL = R"#(
--------------------------
Uso: ./olm
Uso: ./olm -h
Uso: ./olm --ayuda
Uso: ./olm <archivo>
Uso: ./olm --ast <archivo>
--------------------------)#";

auto main(int argc, char** argv) -> int32_t {
  if (argc == 1) {
    Repl repl{};
    repl.run();
    return EXIT_SUCCESS;
  }

  bool show_ast {};
  std::string_view filename{};

  for (auto i{1}; i < argc; i++) {
    std::string_view s = argv[i];
    if (s == "--ayuda" || s == "-h") {
      std::print("{}", HELP_PANEL);
      exit(EXIT_SUCCESS);
    } else if (s == "--ast") {
      show_ast = true;
    } else {
      filename = s;
      break;
    }
  }

  auto opt = read_file(filename);

  if (!opt.has_value()) {
    std::println(stderr, "Error: no se pudo abrir el archivo '{}'", filename);
    return EXIT_FAILURE;
  }

  StmtsPtr ast_buffer{};
  try {
    Parser parser{*opt};
    ast_buffer = parser.parse();
  } catch (const std::runtime_error& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }

  if (show_ast) [[unlikely]] {
    debug_see_nodetype(ast_buffer);
    return EXIT_SUCCESS;
  }

  try {
    Sema sema{};
    sema.analyze(ast_buffer);
  } catch(const SemanticException& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }
  try {
    Interpreter interp;
    interp.run(ast_buffer);
  } catch (const RuntimeError& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

