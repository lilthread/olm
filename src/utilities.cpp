#include "utilities.h"
#include <print>
#include <fstream>
#include <sstream>

auto SourceLocation::to_string() const -> std::string { return std::format("{}:{}", row, col); }

auto help_panel() -> void {
  std::println("--------------------------");
  std::println("Uso: ./olm -h");
  std::println("Uso: ./olm --ayuda");
  std::println("Uso: ./olm <archivo>");
  std::println("Uso: ./olm <archivo> --ast");
  std::println("--------------------------");
  exit(EXIT_SUCCESS);
}
auto read_file(std::string_view filename) -> std::optional<std::string> {
  std::ifstream file{filename.data()};
  if (!file.is_open())
    return std::nullopt;
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}





