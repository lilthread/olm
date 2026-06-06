#include "utilities.h"
#include <print>
#include <fstream>
#include <sstream>

auto SourceLocation::to_string() const -> std::string { return std::format("{}:{}", row, col); }

auto read_file(std::string_view filename) -> std::optional<std::string> {
  std::ifstream file{filename.data()};
  if (!file.is_open())
    return std::nullopt;
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}





