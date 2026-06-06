#pragma once
#include <string_view>
#include <optional>

struct SourceLocation final {
  std::size_t row {};
  std::size_t col {};

  auto to_string() const -> std::string;
  auto operator<=>(const SourceLocation&) const = default;
  auto is_invalid_position() const -> bool { return col == 0; }
};


auto read_file(std::string_view filename) -> std::optional<std::string>;

template<std::size_t N, class T>
constexpr std::size_t countof(T(&)[N]) { return N; }


