#include <gtest/gtest.h>
#include "parser.h"
#include "sema.h"

static auto analyze_ok(std::string_view src) -> void {
  Parser p{src};
  auto ast = p.parse();
  Sema sema;
  ASSERT_NO_THROW(sema.analyze(ast)) << "Expected clean analysis for:\n" << src;
}

// Returns all error messages from a program that is expected to fail.
static auto analyze_errors(std::string_view src) -> std::vector<std::string> {
  Parser p{src};
  auto ast = p.parse();
  Sema sema;
  try {
    sema.analyze(ast);
    return {};
  } catch (const SemanticException& ex) {
    std::vector<std::string> msgs;
    for (auto& e : ex.errors) msgs.push_back(e);
    return msgs;
  }
}

// Asserts that analysis fails and that at least one error contains `substr`.
static auto expect_error(std::string_view src, std::string_view substr) -> void {
  auto errs = analyze_errors(src);
  ASSERT_FALSE(errs.empty()) << "Expected a semantic error for:\n" << src;
  bool found = false;
  for (auto& e : errs)
    if (e.find(substr) != std::string::npos) { found = true; break; }
  EXPECT_TRUE(found)
      << "Expected error containing '" << substr << "' but got:\n"
      << [&]{ std::string s; for (auto& e : errs) s += "  • " + e + "\n"; return s; }();
}

TEST(Sema, VarDeclInteger)        { analyze_ok("var x se 1"); }
TEST(Sema, VarDeclFloat)          { analyze_ok("var x se 3.14"); }
TEST(Sema, VarDeclString)         { analyze_ok(R"(var x se "hola")"); }
TEST(Sema, VarDeclBoolTrue)       { analyze_ok("var x se verdadero"); }
TEST(Sema, VarDeclBoolFalse)      { analyze_ok("var x se falso"); }
TEST(Sema, ConstDecl)             { analyze_ok("const MAX se 100"); }
TEST(Sema, VarDeclRhsUsesKnown) { analyze_ok("var a se 1\nvar b se a"); }
TEST(Sema, VarDeclRhsUnknown) { expect_error("var x se a", "undeclared identifier 'a'"); }
TEST(Sema, RedeclarationSameScope) { expect_error("var x se 1\nvar x se 2", "already declared"); }

TEST(Sema, RedeclarationDifferentScopes) {
  // Shadowing in inner scope is allowed.
  analyze_ok(
    "var x se 1\n"
    "si verdadero haz\n"
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
  expect_error("const C se 1\nC se 2", "assignment to constant 'C'");
}

TEST(Sema, AssignToFunction) {
  expect_error("func f() fin\nf se 1", "assignment to function 'f'");
}

TEST(Sema, AssignToClass) {
  expect_error("clase A fin\nA se 1", "assignment to class 'A'");
}

TEST(Sema, AssignDeclared) {
  analyze_ok("var x se 0\nx se 42");
}

TEST(Sema, AssignUndeclared) {
  expect_error("x se 1", "undeclared variable 'x'");
}

TEST(Sema, AssignRhsUnknown) {
  expect_error("var x se 0\nx se a", "undeclared identifier 'a'");
}

TEST(Sema, FuncDeclNoParams) {
  analyze_ok("func f() fin");
}

TEST(Sema, FuncDeclWithParams) {
  analyze_ok("func suma(a, b) ret a + b fin");
}

TEST(Sema, FuncParamsVisibleInBody) {
  analyze_ok("func f(x) var z se x fin");
}

TEST(Sema, FuncParamsNotVisibleOutside) {
  expect_error("func f(x) fin\nvar z se x", "undeclared identifier 'x'");
}

TEST(Sema, FuncRedeclaration) {
  expect_error("func f() fin\nfunc f() fin", "already declared");
}

TEST(Sema, FuncLocalVarNotVisible) {
  expect_error("func f() var local se 1 fin\nvar z se local", "undeclared identifier 'local'");
}

TEST(Sema, FuncRecursion) {
  analyze_ok("func fib(n) ret fib(n) fin");
}

TEST(Sema, MutualRecursionForwardNotSupported) {
  // 'g' is not declared when 'f' is being analyzed, so this must error.
  expect_error(
    "func f() g() fin\n"
    "func g() f() fin",
    "undeclared function 'g'"
  );
}

TEST(Sema, ReturnInsideFunc) {
  analyze_ok("func f() ret 1 fin");
}

TEST(Sema, ReturnAtTopLevel) {
  expect_error("ret 1", "'ret' used outside of a function");
}

TEST(Sema, ReturnNestedInIf) {
  analyze_ok("func f(x) si x haz ret 1 fin fin");
}

TEST(Sema, ReturnNestedInWhile) {
  analyze_ok("func f() mientras verdadero haz ret 1 fin fin");
}

TEST(Sema, CallDeclaredFunc) {
  analyze_ok("func f() fin\nf()");
}

TEST(Sema, CallUndeclaredFunc) {
  expect_error("foo()", "undeclared function 'foo'");
}

TEST(Sema, CallNonFunction) {
  expect_error("var x se 1\nx()", "'x' is not a function");
}

TEST(Sema, CallArityMatch) {
  analyze_ok("func add(a, b) ret a + b fin\nadd(1, 2)");
}

TEST(Sema, CallArityTooFew) {
  expect_error("func add(a, b) ret a + b fin\nadd(1)", "expects 2 argument(s) but got 1");
}

TEST(Sema, CallArityTooMany) {
  expect_error("func add(a, b) ret a + b fin\nadd(1, 2, 3)", "expects 2 argument(s) but got 3");
}

TEST(Sema, CallArgIsExpression) {
  analyze_ok("func f(x) fin\nvar a se 1\nf(a + 1)");
}

TEST(Sema, CallArgUndeclared) {
  expect_error("func f(x) fin\nf(z)", "undeclared identifier 'z'");
}

TEST(Sema, IfSimple) {
  analyze_ok("si verdadero haz fin");
}

TEST(Sema, IfConditionUndeclared) {
  expect_error("si x haz fin", "undeclared identifier 'x'");
}

TEST(Sema, IfWithElse) {
  analyze_ok("var x se 1\nsi x haz var a se 1 sino haz var b se 2 fin");
}

TEST(Sema, IfBranchScopesIsolated) {
  // 'a' declared in then-branch must not be visible in else-branch.
  expect_error(
    "si verdadero haz\n"
    "  var a se 1\n"
    "sino haz\n"
    "  var b se a\n"
    "fin",
    "undeclared identifier 'a'"
  );
}

TEST(Sema, IfBranchVarNotVisibleAfter) {
  expect_error(
    "si verdadero haz var inner se 1 fin\n"
    "var z se inner",
    "undeclared identifier 'inner'"
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
  expect_error("mientras x haz fin", "undeclared identifier 'x'");
}

TEST(Sema, WhileBodyVarNotVisibleAfter) {
  expect_error(
    "mientras verdadero haz var i se 0 fin\n"
    "var z se i",
    "undeclared identifier 'i'"
  );
}

TEST(Sema, WhileBodyUndeclared) {
  expect_error("mientras verdadero haz var x se a fin", "undeclared identifier 'a'");
}

TEST(Sema, ClassEmpty) {
  analyze_ok("clase Punto fin");
}

TEST(Sema, ClassWithFields) {
  analyze_ok("clase Punto var x se 0 var z se 0 fin");
}

TEST(Sema, ClassRedeclaration) {
  expect_error("clase A fin\nclase A fin", "already declared");
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
    "expects 0 argument(s) but got 1"
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
    "undeclared identifier 'x'"
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
  expect_error("var x se este", "'este' used outside");
}

TEST(Sema, EsteInsideFreeFunction) {
  // A free (non-method) function should also reject 'este'.
  expect_error("func f() var x se este fin", "'este' used outside");
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
  expect_error("var a se [x]", "undeclared identifier 'x'");
}

TEST(Sema, SubscriptDeclared) {
  analyze_ok("var a se [1, 2]\nvar r se a[0]");
}

TEST(Sema, SubscriptUndeclared) {
  expect_error("var r se a[0]", "undeclared identifier 'a'");
}

TEST(Sema, MultipleErrorsReported) {
  auto errs = analyze_errors("var a se x\nvar b se z");
  EXPECT_GE(errs.size(), 2u) << "Expected at least 2 errors";
}

TEST(Sema, FibonacciOk) {
  analyze_ok(
    "func fib(n)\n"
    "  si n < 2 haz\n"
    "    ret n\n"
    "  sino haz\n"
    "    ret fib(n - 1) + fib(n - 2)\n"
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
    "    ret a + b\n"
    "  fin\n"
    "  ret inner(a)\n"
    "fin"
  );
}

TEST(Sema, ChainedCallsOk) {
  analyze_ok(
    "func double(x) ret x + x fin\n"
    "func quad(x)   ret double(double(x)) fin"
  );
}


