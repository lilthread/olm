#pragma once
#include <string_view>
#include <optional>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

static auto read_file(std::string_view filename) -> std::optional<std::string> {
  std::ifstream file{filename.data()};
  if (!file.is_open())
    return std::nullopt;
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

