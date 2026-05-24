#include <print>
#include <cstdlib>
#include <stdexcept>
#include "error_manager.h"
#include "nodes.h"
#include "parser.h"
#include "sema.h"
#include "interpreter.h"
#include "utilities.h"

static bool show_ast {};

[[noreturn]] auto help_panel() -> void {
  std::println("--------------------------");
  std::println("Uso: ./olm -h");
  std::println("Uso: ./olm <archivo>");
  std::println("Uso: ./olm <archivo> --ast");
  std::println("--------------------------");
  exit(EXIT_SUCCESS);
}

constexpr static auto parse_args(int argc, char** argv) -> void {
  for (int i {1uz}; i < argc; i++) {
    std::string_view str = argv[i];

    if ("-h" == str)
      help_panel();
    if ("--ast" == str)
      show_ast = true;
  }
}

auto main(int argc, char** argv) -> int32_t {
  if (argc < 2)
    help_panel();
  parse_args(argc, argv);

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
  } try {
    Interpreter interp;
    interp.run(ast);
  } catch (const RuntimeError& e) {
    std::println(stderr, "{}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

