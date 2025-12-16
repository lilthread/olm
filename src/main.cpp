#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <print>
#include "lexer.h"

std::string read_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error al abrir el archivo: " << filename << "\n";
    exit(1);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main(int argc, char** argv) {
  if(argc < 2) {
    std::cerr << "Uso: " << argv[0] << " <archivo>\n";
    return EXIT_FAILURE;
  }
  auto filename = argv[1];
  auto source = read_file(filename);
  auto tkns = std::move(Lexer(source).tokenize());
  for (const auto& t : tkns){
    std::println("TOKENLITERAL: {}", t.literal);
  }
  // TODO: ...
  return EXIT_SUCCESS;
}
