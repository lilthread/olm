#include "repl.h"
#include <print>
#include "parser.h"

#ifdef _WIN32
#include <cstdio>
#include <cstdlib>
#include <cstring>

char* readline(const char* prompt) {
  static char buffer[2048];

  std::fputs(prompt, stdout);

  if (!std::fgets(buffer, sizeof(buffer), stdin))
    return nullptr;

  buffer[std::strcspn(buffer, "\n")] = '\0';

  char* result = static_cast<char*>(std::malloc(std::strlen(buffer) + 1));

  if (!result)
    return nullptr;

  std::strcpy(result, buffer);

  return result;
}

void add_history(const char*) { }

#else
#include <editline/readline.h>
#endif


static constexpr const char* BLOCK_OPENERS[] = {
  "si", "mientras", "func", "clase"
};

auto Repl::run() -> void {
  std::println("olm v0.1  —  escribe 'salir' para terminar");
  std::println("────────────────────────────────────────────");

  _sema.init_repl();

  while (true) {
    const char* p = _pending.empty() ? ">>> " : "... ";

    char* input = readline(p);

    if (!input) {
      std::println("");
      break;
    }

    std::string line = input;
    free(input);

    auto trimmed = line;
    while (!trimmed.empty() &&
           std::isspace((unsigned char)trimmed.back()))
      trimmed.pop_back();

    if (trimmed == "salir" || trimmed == "exit")
      break;

    // save history
    if (!trimmed.empty())
      add_history(line.c_str());

    if (!_pending.empty())
      _pending += '\n';

    _pending += line;

    if (is_incomplete(_pending))
      continue;

    execute(_pending);
    _pending.clear();
  }
}



auto Repl::is_incomplete(std::string_view src) const -> bool {
  auto depth{0uz};
  std::string tok;
  auto flush = [&]{
    if (tok.empty()) return;
    for (auto* kw : BLOCK_OPENERS)
      if (tok == kw) { ++depth; return; }
    if (tok == "fin") --depth;
    tok.clear();
  };
  for (char c : src) {
    if (std::isspace(static_cast<unsigned char>(c))) { flush(); tok.clear(); }
    else tok += c;
  }
  flush();
  return depth > 0;
}

auto Repl::execute(const std::string& src) -> void {
  StmtsPtr ast;
  try {
    Parser p{src};
    ast = p.parse();
  } catch (const std::runtime_error& e) {
    std::println(stderr, "ParseError: {}", e.what());
    return;
  }

  if (ast.empty()) return;

  try {
    _sema.analyze_incremental(ast);
  } catch (const SemanticException& e) {
    for (auto& err : e.errors)
      std::println(stderr, "SemanticError: {}", err.to_string());
    return;
  }

  _ast_store.push_back(std::move(ast));
  const StmtsPtr& live_ast = _ast_store.back();

  try {
    _interp.run(live_ast);
  } catch (ReturnSignal&) {
  } catch (const RuntimeError& e) {
    std::println(stderr, "{}", e.what());
  }
}

