#pragma once
#include <string_view>
#include <optional>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

struct SourceLocation final {
  std::size_t row {};
  std::size_t col {};
  auto operator<=>(const SourceLocation&) const = default;

  auto to_string() const -> std::string { return std::format("{}:{}", row, col); }
};

static inline auto read_file(std::string_view filename) -> std::optional<std::string> {
  std::ifstream file{filename.data()};
  if (!file.is_open())
    return std::nullopt;
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
