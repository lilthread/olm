#include <print>
#include <cstdlib>
#include <stdexcept>
#include "error_manager.h"
#include "nodes.h"
#include "parser.h"
#include "sema.h"
#include "interpreter.h"
#include "utilities.h"
#include "repl.h"

static bool show_ast {};

auto main(int argc, char** argv) -> int32_t {
  if (argc == 1){
    Repl repl;
    repl.run();
    return EXIT_SUCCESS;
  }
  if (std::string_view{argv[1]} == "--ayuda" || std::string_view{argv[1]} == "-h")
    help_panel();

  auto opt = read_file(std::move(argv[1]));

  if (!opt.has_value()) {
    std::println(stderr, "Error: no se pudo abrir el archivo '{}'", argv[1]);
    return EXIT_FAILURE;
  }

  StmtsPtr ast{};
  try {
    Parser parser{opt.value()};
    ast = parser.parse();
  } catch (const std::runtime_error& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }

  if (show_ast) [[unlikely]] {
    debug_see_nodetype(ast);
    return EXIT_SUCCESS;
  }

  try {
    Sema sema{};
    sema.analyze(ast);
  } catch(const SemanticException& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }
  try {
    Interpreter interp;
    interp.run(ast);
  } catch (const RuntimeError& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

