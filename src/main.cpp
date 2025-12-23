#include <iostream>
#include <cstdlib>
#include <fstream>
#include <print>
#include <sstream>
#include <stdexcept>
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

  std::vector<Token> tkns;
  try{
    tkns = tokenize(read_file(filename));
  }catch(std::runtime_error& e){
    std::cerr << e.what() << std::endl;
  }

  for(auto t : tkns) {
    std::println("token: {}, row: {}, col {}", t.literal, t.row, t.col);
  }
  MemoryArena arena{tkns.size() * 128};
  //Parser parser(tkns, arena);
  //ArenaVec program = parser.parseProgram();
  // TODO:

  return EXIT_SUCCESS;
}
