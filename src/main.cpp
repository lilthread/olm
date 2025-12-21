#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "ast.h"
#include "lexer.h"
#include "memoryarena.h"
#include "parser.h"

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
  std::string filename = argv[1];
  std::vector<Token> tkns = tokenize(read_file(filename));

  MemoryArena arena{1024 * 4};
  Parser parser(tkns, arena);
  ArenaVec program = parser.parseProgram();
  // TODO:

  return EXIT_SUCCESS;
}
