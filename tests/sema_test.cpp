
#include <gtest/gtest.h>
#include "parser.h"
#include "sema.h"

static auto analyze_ok(std::string_view src) -> void {
  Parser p{src};
  auto ast = p.parse();
  Sema sema;
  ASSERT_NO_THROW(sema.analyze(ast)) << "Expected clean analysis for:\n" << src;
}

static auto analyze_errors(std::string_view src) -> std::vector<SemanticError> {
  Parser p{src};
  auto ast = p.parse();
  Sema sema;
  try {
    sema.analyze(ast);
    return {};
  } catch (const SemanticException& ex) {
    return ex.errors;
  }
}
 
static auto expect_error(std::string_view src, SemanticErrorCode expected) -> void {
  auto errs = analyze_errors(src);
  ASSERT_FALSE(errs.empty()) << "Expected a semantic error for:\n" << src;
  bool found = false;
  for (auto& e : errs)
    if (e.code == expected) { found = true; break; }
  EXPECT_TRUE(found)
      << "Expected error code but got:\n"
      << [&]{ std::string s; for (auto& e : errs) s += "  * " + e.to_string() + "\n"; return s; }();
}

TEST(Sema, VarDeclInteger)        { analyze_ok("var x se 1"); }
TEST(Sema, VarDeclFloat)          { analyze_ok("var x se 3.14"); }
TEST(Sema, VarDeclString)         { analyze_ok(R"(var x se "hola")"); }
TEST(Sema, VarDeclBoolTrue)       { analyze_ok("var x se verdadero"); }
TEST(Sema, VarDeclBoolFalse)      { analyze_ok("var x se falso"); }
TEST(Sema, ConstDecl)             { analyze_ok("const MAX se 100"); }
TEST(Sema, VarDeclRhsUsesKnown) { analyze_ok("var a se 1\n var b se a"); }
TEST(Sema, VarDeclRhsUnknown) { expect_error("var x se a", SemanticErrorCode::UNDECLARED_ID); }
TEST(Sema, RedeclarationSameScope) { expect_error("var x se 1\nvar x se 2", SemanticErrorCode::REDEFINITION); }

TEST(Sema, RedeclarationDifferentScopes) {
  // Shadowing in inner scope is allowed.
  analyze_ok(
    "var x se 1\n"
    "si x = verdadero haz\n"
    "  var x se 2\n"
    "fin"
  );
}

TEST(Sema, EqualNotEqual) {
  analyze_ok(
    "var a se 0\n"
    "var b se 1\n"
    "si a != 1 y b = 1 haz\n"
    "  var temp se a\n"
    "  a se b\n"
    "  b se a\n"
    "fin"
  );
}

TEST(Sema, AssignToConst) {
  expect_error("const C se 1\nC se 2", SemanticErrorCode::ASSIGNMENT_TO_CONST);
}

TEST(Sema, AssignToFunction) {
  expect_error("func f() fin\nf se 1", SemanticErrorCode::ASSIGNMENT_TO_FUNC);
}

TEST(Sema, AssignToClass) {
  expect_error("clase A fin\nA se 1", SemanticErrorCode::ASSIGNMENT_TO_CLASS);
}

TEST(Sema, AssignDeclared) {
  analyze_ok("var x se 0\nx se 42");
}

TEST(Sema, AssignUndeclared) {
  expect_error("x se 1", SemanticErrorCode::ASSIGNMENT_TO_UNDECLARED);
}

TEST(Sema, AssignRhsUnknown) {
  expect_error("var x se 0\nx se a", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, FuncDeclNoParams) {
  analyze_ok("func f() fin");
}

TEST(Sema, FuncDeclWithParams) {
  analyze_ok("func suma(a, b) devolver a + b fin");
}

TEST(Sema, FuncParamsVisibleInBody) {
  analyze_ok("func f(x) var z se x fin");
}

TEST(Sema, FuncParamsNotVisibleOutside) {
  expect_error("func f(x) fin\nvar z se x", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, FuncRedeclaration) {
  expect_error("func f() fin\nfunc f() fin", SemanticErrorCode::REDEFINITION);
}

TEST(Sema, FuncLocalVarNotVisible) {
  expect_error("func f() var local se 1 fin\nvar z se local", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, FuncRecursion) {
  analyze_ok("func fib(n) devolver fib(n) fin");
}

TEST(Sema, MutualRecursionForwardNotSupported) {
  expect_error(
    "func f() g() fin\n"
    "func g() f() fin",
    SemanticErrorCode::UNDECLARED_FUNC
  );
}

TEST(Sema, ReturnInsideFunc) {
  analyze_ok("func f() devolver 1 fin");
}

TEST(Sema, ReturnAtTopLevel) {
  expect_error("devolver 1", SemanticErrorCode::RET_OUTSIDE_FUNC);
}

TEST(Sema, ReturnNestedInIf) {
  analyze_ok("func f(x) si x haz devolver 1 fin fin");
}

TEST(Sema, ReturnNestedInWhile) {
  analyze_ok("func f() mientras verdadero haz devolver 1 fin fin");
}

TEST(Sema, CallDeclaredFunc) {
  analyze_ok("func f() fin\nf()");
}

TEST(Sema, CallUndeclaredFunc) {
  expect_error("foo()", SemanticErrorCode::UNDECLARED_FUNC);
}

TEST(Sema, CallNonFunction) {
  expect_error("var x se 1\nx()", SemanticErrorCode::NOT_A_FUNC);
}

TEST(Sema, CallArityMatch) {
  analyze_ok("func add(a, b) devolver a + b fin\nadd(1, 2)");
}

TEST(Sema, CallArityTooFew) {
  expect_error("func add(a, b) devolver a + b fin\nadd(1)", SemanticErrorCode::FUNC_ARITY_MISMATCH);
}

TEST(Sema, CallArityTooMany) {
  expect_error("func add(a, b) devolver a + b fin\nadd(1, 2, 3)", SemanticErrorCode::FUNC_ARITY_MISMATCH);
}

TEST(Sema, CallArgIsExpression) {
  analyze_ok("func f(x) fin\nvar a se 1\nf(a + 1)");
}

TEST(Sema, CallArgUndeclared) {
  expect_error("func f(x) fin\nf(z)", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, IfSimple) {
  analyze_ok("si verdadero haz fin");
}

TEST(Sema, IfConditionUndeclared) {
  expect_error("si x haz fin", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, IfWithElse) {
  analyze_ok("var x se 1\nsi x haz var a se 1 sino var b se 2 fin");
}

TEST(Sema, IfBranchScopesIsolated) {
  // 'a' declared in then-branch must not be visible in else-branch.
  expect_error(
    "si verdadero haz\n"
    "  var a se 1\n"
    "sino\n"
    "  var b se a\n"
    "fin",
    SemanticErrorCode::UNDECLARED_ID
  );
}

TEST(Sema, IfBranchVarNotVisibleAfter) {
  expect_error(
    "si verdadero haz var inner se 1 fin\n"
    "var z se inner",
    SemanticErrorCode::UNDECLARED_ID
  );
}

TEST(Sema, IfElseIf) {
  analyze_ok(
    "var x se 1\n"
    "si x haz\n"
    "  var a se 1\n"
    "sino si verdadero haz\n"
    "  var b se 2\n"
    "fin"
  );
}

TEST(Sema, WhileSimple) {
  analyze_ok("mientras verdadero haz fin");
}

TEST(Sema, WhileConditionUndeclared) {
  expect_error("mientras x haz fin", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, WhileBodyVarNotVisibleAfter) {
  expect_error(
    "mientras verdadero haz var i se 0 fin\n"
    "var z se i",
    SemanticErrorCode::UNDECLARED_ID
  );
}

TEST(Sema, WhileBodyUndeclared) {
  expect_error("mientras verdadero haz var x se a fin", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, ClassEmpty) {
  analyze_ok("clase Punto fin");
}

TEST(Sema, ClassWithFields) {
  analyze_ok("clase Punto var x se 0 var z se 0 fin");
}

TEST(Sema, ClassRedeclaration) {
  expect_error("clase A fin\nclase A fin", SemanticErrorCode::REDEFINITION);
}

TEST(Sema, ClassMethodCanCallSibling) {
  analyze_ok(
    "clase A\n"
    "  func foo() bar() fin\n"
    "  func bar() fin\n"
    "fin"
  );
}

TEST(Sema, ClassMethodArityEnforced) {
  expect_error(
    "clase A\n"
    "  func foo() bar(1) fin\n"
    "  func bar() fin\n"
    "fin",
    SemanticErrorCode::FUNC_ARITY_MISMATCH
  );
}

TEST(Sema, ClassNotVisibleInsideOtherClass) {
  // Classes are separate — inner scope doesn't see outer class members.
  expect_error(
    "clase A\n"
    "  var x se 0\n"
    "fin\n"
    "clase B\n"
    "  var z se x\n"
    "fin",
    SemanticErrorCode::UNDECLARED_ID
  );
}

TEST(Sema, EsteInsideMethod) {
  analyze_ok(
    "clase A\n"
    "  func f() var r se este fin\n"
    "fin"
  );
}

TEST(Sema, EsteAtTopLevel) {
  expect_error("var x se este", SemanticErrorCode::THIS_USED_OUTSIDE_CLASS);
}

TEST(Sema, EsteInsideFreeFunction) {
  expect_error("func f() var x se este fin", SemanticErrorCode::THIS_USED_OUTSIDE_CLASS);
}

TEST(Sema, ArrayEmpty) {
  analyze_ok("var a se []");
}

TEST(Sema, ArrayLiterals) {
  analyze_ok("var a se [1, 2, 3]");
}

TEST(Sema, Array2D) {
  analyze_ok("var a se [['#', '#', '#'], ['#', '#', '#'], ['#', '#', '#']]");
}

TEST(Sema, ArrayWithIdentifiers) {
  analyze_ok("var a se 1\nvar b se 2\nvar arr se [a, b]");
}

TEST(Sema, ArrayUndeclaredElement) {
  expect_error("var a se [x]", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, SubscriptDeclared) {
  analyze_ok("var a se [1, 2]\nvar r se a[0]");
}

TEST(Sema, SubscriptUndeclared) {
  expect_error("var r se a[0]", SemanticErrorCode::UNDECLARED_ID);
}

TEST(Sema, MultipleErrorsReported) {
  auto errs = analyze_errors("var a se x\nvar b se z");
  EXPECT_GE(errs.size(), 2u) << "Expected at least 2 errors";
}

TEST(Sema, FibonacciOk) {
  analyze_ok(
    "func fib(n)\n"
    "  si n < 2 haz\n"
    "    devolver n\n"
    "  sino\n"
    "    devolver fib(n - 1) + fib(n - 2)\n"
    "  fin\n"
    "fin"
  );
}

TEST(Sema, CounterLoopOk) {
  analyze_ok(
    "var i se 0\n"
    "mientras i < 10 haz\n"
    "  i se i + 1\n"
    "fin"
  );
}

TEST(Sema, ClassWithSelfOk) {
  analyze_ok(
    "clase Contador\n"
    "  var n se 0\n"
    "  func incrementar()\n"
    "    este.n se este.n + 1\n"
    "  fin\n"
    "fin"
  );
}

TEST(Sema, NestedFunctionsOk) {
  analyze_ok(
    "func outer(a)\n"
    "  func inner(b)\n"
    "    devolver a + b\n"
    "  fin\n"
    "  devolver inner(a)\n"
    "fin"
  );
}

TEST(Sema, ChainedCallsOk) {
  analyze_ok(
    "func double(x) devolver x + x fin\n"
    "func quad(x)   devolver double(double(x)) fin"
  );
}


TEST(Sema, NoCtorNoArgs) {
  analyze_ok(
    "clase Vacio fin\n"
    "var v se Vacio()"
  );
}
 
TEST(Sema, NoCtorWithArgs) {
  expect_error(
    "clase Vacio fin\n"
    "var v se Vacio(1, 2)",
    SemanticErrorCode::CLASS_NO_CTOR
  );
}

TEST(Sema, CtorNoParams) {
  analyze_ok(
    "clase Reloj\n"
    "  var ticks se 0\n"
    "  func crear()\n"
    "    este.ticks se 0\n"
    "  fin\n"
    "fin\n"
    "var r se Reloj()"
  );
}

TEST(Sema, CtorNoParamsCalledWithArgs) {
  expect_error(
    "clase Reloj\n"
    "  func crear() fin\n"
    "fin\n"
    "var r se Reloj(1)",
    SemanticErrorCode::CTOR_ARITY_MISMATCH
  );
}

TEST(Sema, CtorArityTooFew) {
  expect_error(
    "clase Punto\n"
    "  func crear(nx, ny) fin\n"
    "fin\n"
    "var p se Punto(1)",
    SemanticErrorCode::CTOR_ARITY_MISMATCH
  );
}

TEST(Sema, CtorArityZeroCalledEmpty) {
  analyze_ok(
    "clase Nodo\n"
    "  func crear() fin\n"
    "fin\n"
    "var n se Nodo.crear()"
  );
}

TEST(Sema, CtorWithParams) {
  analyze_ok(
    "clase Punto\n"
    "  var px se 0\n"
    "  var py se 0\n"
    "  func crear(nx, ny)\n"
    "    este.px se nx\n"
    "    este.py se ny\n"
    "  fin\n"
    "fin\n"
    "var p se Punto.crear(0, 4)"
  );
}

TEST(Sema, CtorArityTooMany) {
  expect_error(
    "clase Punto\n"
    "  func crear(nx, ny) fin\n"
    "fin\n"
    "var p se Punto(1, 2, 3)",
    SemanticErrorCode::CTOR_ARITY_MISMATCH
  );
}

TEST(Sema, CtorArgIsExpression) {
  analyze_ok(
    "clase Caja\n"
    "  func crear(w, h) fin\n"
    "fin\n"
    "var a se 10\n"
    "var b se 20\n"
    "var c se Caja.crear(a + 1, b * 2)"
  );
}


TEST(Sema, CtorArgIsCallResult) {
  analyze_ok(
    "func dims() devolver 5 fin\n"
    "clase Caja\n"
    "  func crear(w, h) fin\n"
    "fin\n"
    "var c se Caja.crear(dims(), dims())"
  );
}
  
TEST(Sema, CtorBodyCanUseEste) {
  analyze_ok(
    "clase Circulo\n"
    "  var radio se 0\n"
    "  func crear(r)\n"
    "    este.radio se r\n"
    "  fin\n"
    "fin"
  );
}

TEST(Sema, CtorBodyCanUseParams) {
  analyze_ok(
    "clase Segmento\n"
    "  func crear(a, b)\n"
    "    var largo se a + b\n"
    "  fin\n"
    "fin"
  );
}

TEST(Sema, CtorBodyParamUndeclared) {
  expect_error(
    "clase Mal\n"
    "  func crear(a)\n"
    "    var x se z\n"
    "  fin\n"
    "fin",
    SemanticErrorCode::UNDECLARED_ID
  );
}

TEST(Sema, CtorCanCallSiblingMethod) {
  analyze_ok(
    "clase Motor\n"
    "  func crear()\n"
    "    iniciar()\n"
    "  fin\n"
    "  func iniciar() fin\n"
    "fin"
  );
}

TEST(Sema, CtorCanReturnValue) {
  analyze_ok(
    "clase Cosa\n"
    "  func crear()\n"
    "    devolver verdadero\n"
    "  fin\n"
    "fin"
  );
}

TEST(Sema, MultipleClassInstances) {
  analyze_ok(
    "clase A\n"
    "  func crear(a) fin\n"
    "fin\n"
    "clase B\n"
    "  func crear(a, b) fin\n"
    "fin\n"
    "var a se A.crear(1)\n"
    "var b se B.crear(1, 2)"
  );
}

TEST(Sema, MultipleClassesArityIndependent) {
  expect_error(
    "clase A\n"
    "  func crear(x) fin\n"
    "fin\n"
    "clase B\n"
    "  func crear(a, b) fin\n"
    "fin\n"
    "var a se A(1)\n"
    "var b se B(1)",
    SemanticErrorCode::CTOR_ARITY_MISMATCH
  );
}

TEST(Sema, InstantiationResultAssigned) {
  analyze_ok(
    "clase Vec\n"
    "  func crear(a, b) fin\n"
    "fin\n"
    "var v se Vec.crear(0, 0)\n"
    "v se Vec(1, 1)"
  );
}

 TEST(Sema, FullClassLifecycle) {
  analyze_ok(
    "clase Contador\n"
    "  var n se 0\n"
    "  func crear(inicio)\n"
    "    este.n se inicio\n"
    "  fin\n"
    "  func incrementar()\n"
    "    este.n se este.n + 1\n"
    "  fin\n"
    "  func valor()\n"
    "    devolver este.n\n"
    "  fin\n"
    "fin\n"
    "var c se Contador(0)\n"
    "c.incrementar()\n"
    "var r se c.valor()"
  );
}

TEST(Sema, CtorInsideFunction) {
  analyze_ok(
    "clase Punto\n"
    "  func crear(a, b) fin\n"
    "fin\n"
    "func hacer_punto(a, b)\n"
    "  devolver Punto(a, b)\n"
    "fin"
  );
}
 
TEST(Sema, CtorInWhileLoop) {
  analyze_ok(
    "clase Nodo\n"
    "  func crear(v) fin\n"
    "fin\n"
    "var i se 0\n"
    "mientras i < 3 haz\n"
    "  var n se Nodo(i)\n"
    "  i se i + 1\n"
    "fin"
  );
}

/*TEST(Sema, Overloaded) {
  analyze_ok(
    "clase C\n"
    "  func crear(a) fin\n"
    "  func crear(a, b) fin\n"
    "fin\n"
    "var a se C(0)\n"
    "var b se C(0,0)\n"
  );
}
*/
TEST(Sema, InnerCtor) {
  analyze_ok(
    "clase C\n"
    "  func crear(a) fin\n"
    "  func wrapper_crear(a, b) devolver C(a - b) fin\n"
    "fin\n"
    "var factory se C(1)\n"
    "var new se factory.wrapper_crear(10, 5)\n"
    );
}

