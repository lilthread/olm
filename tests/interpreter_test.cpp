#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include "nodes.h"
#include "parser.h"
#include "sema.h"
#include "interpreter.h"

struct RunResult {
  std::shared_ptr<StmtsPtr> ast;
  Interpreter interp;
};

static auto run_ok(std::string_view src) -> Interpreter {
  Parser p{src};
  auto ast = p.parse();
  Sema sema;
  sema.analyze(ast);
  Interpreter interp;
  interp.run(ast);
  return interp;
}


static auto run_error(std::string_view src, std::string_view substr) -> void {
  Parser p{src};
  auto ast = p.parse();
  Interpreter interp;
  try {
    interp.run(ast);
    FAIL() << "Expected RuntimeError containing '" << substr << "'";
  } catch (const RuntimeError& e) {
    EXPECT_NE(std::string(e.what()).find(substr), std::string::npos)
        << "Got: " << e.what();
  }
}

// ── Actual clean helper used by all tests ──────────────────────
//
// Each test program must define:   func resultado() ret <value> fin
// This helper runs the program + calls resultado() and returns the Value.
static auto get_result(std::string_view src) -> ValuePtr {
  std::string full = std::string(src) + "\nvar __answer__ se resultado()";
  // We can't read __answer__ from outside.
  // Solution: subclass Interpreter and expose eval — but we can't change prod.
  //
  // REAL solution: just make the test programs assign to a module-level
  // variable and add `func resultado() ret <var> fin` and verify the
  // return value by observing what the function returns when called as
  // a statement that THROWS ReturnSignal.
  //
  // Since ReturnSignal is public, we can catch it from outside:
  std::string trigger = std::string(src) + "\ndevolver resultado()";
  Parser p{trigger};
  auto ast = p.parse();
  Interpreter interp;
  try {
    interp.run(ast);
  } catch (ReturnSignal& rs) {
    return rs.value;
  }
  return nullptr;
}


#define EXPECT_INT(val, n)    EXPECT_TRUE((val)->is_int())    << (val)->to_string(); \
                              EXPECT_EQ((val)->as_int(), (n))
#define EXPECT_FLOAT(val, f)  EXPECT_TRUE((val)->is_float())  << (val)->to_string(); \
                              EXPECT_DOUBLE_EQ((val)->as_float(), (f))
#define EXPECT_BOOL(val, b)   EXPECT_TRUE((val)->is_bool())   << (val)->to_string(); \
                              EXPECT_EQ((val)->as_bool(), (b))
#define EXPECT_STR(val, s)    EXPECT_TRUE((val)->is_string())  << (val)->to_string(); \
                              EXPECT_EQ((val)->as_string(), (s))
#define EXPECT_NULL(val)      EXPECT_TRUE((val)->is_null())    << (val)->to_string()
#define EXPECT_INSTANCE(val)  EXPECT_TRUE((val)->is_instance()) << (val)->to_string()
// CHECK THE STRING REPRESENTATION
#define EXPECT_ARRAY(val, s)  EXPECT_TRUE((val)->is_array()) << (val)->to_string(); \
                              EXPECT_EQ((val)->to_string(), (s))


TEST(Value, TruthinessNull)   { EXPECT_FALSE(Value{}.truthy()); }
TEST(Value, TruthinessTrue)   { EXPECT_TRUE(Value{true}.truthy()); }
TEST(Value, TruthinessFalse)  { EXPECT_FALSE(Value{false}.truthy()); }
TEST(Value, TruthinessZeroInt){ EXPECT_FALSE(Value{int64_t{0}}.truthy()); }
TEST(Value, TruthinessNonZero){ EXPECT_TRUE(Value{int64_t{1}}.truthy()); }
TEST(Value, TruthinessEmptyStr){ EXPECT_FALSE(Value{std::string{}}.truthy()); }
TEST(Value, TruthinessStr)    { EXPECT_TRUE(Value{std::string{"hi"}}.truthy()); }

TEST(Value, ToStringNull)   { EXPECT_EQ(Value{}.to_string(), "nulo"); }
TEST(Value, ToStringTrue)   { EXPECT_EQ(Value{true}.to_string(), "verdadero"); }
TEST(Value, ToStringFalse)  { EXPECT_EQ(Value{false}.to_string(), "falso"); }
TEST(Value, ToStringInt)    { EXPECT_EQ(Value{int64_t{42}}.to_string(), "42"); }
TEST(Value, ToStringString) { EXPECT_EQ(Value{std::string{"hola"}}.to_string(), "hola"); }
TEST(Value, ToStringArray)  {
  std::vector<ValuePtr> v;
  v.push_back(std::make_shared<Value>(int64_t{1}));
  v.push_back(std::make_shared<Value>(int64_t{2}));
  EXPECT_EQ(Value{std::move(v)}.to_string(), "[1, 2]");
}

TEST(Interp, LiteralInt) {
  auto v = get_result("func resultado() devolver 42 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 42);
}

TEST(Interp, LiteralFloat) {
  auto v = get_result("func resultado() devolver 3.14 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_TRUE(v->is_float());
}

TEST(Interp, LiteralBoolTrue) {
  auto v = get_result("func resultado() devolver verdadero fin");
  ASSERT_NE(v, nullptr);
  EXPECT_BOOL(v, true);
}

TEST(Interp, LiteralBoolFalse) {
  auto v = get_result("func resultado() devolver falso fin");
  ASSERT_NE(v, nullptr);
  EXPECT_BOOL(v, false);
}

TEST(Interp, LiteralString) {
  auto v = get_result(R"(func resultado() devolver "hola" fin)");
  ASSERT_NE(v, nullptr);
  EXPECT_STR(v, "hola");
}


TEST(Interp, VarDeclAndRead) {
  auto v = get_result("var x se 7\nfunc resultado() devolver x fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 7);
}

TEST(Interp, ConstDeclAndRead) {
  auto v = get_result("const MAX se 100\nfunc resultado() devolver MAX fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 100);
}

TEST(Interp, VarDeclRhsExpression) {
  auto v = get_result("var x se 3 + 4\nfunc resultado() devolver x fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 7);
}

TEST(Interp, Assignment) {
  auto v = get_result("var x se 1\nx se 99\nfunc resultado() devolver x fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 99);
}

TEST(Interp, AssignmentOverwrite) {
  auto v = get_result("var x se 1\nx se 2\nx se 3\nfunc resultado() devolver x fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 3);
}

TEST(Interp, AssignmentUndeclaredThrows) {
  run_error("x se 1", "no declarada");
}

TEST(Interp, Add)  { auto v = get_result("func resultado() devolver 2 + 3 fin");    EXPECT_INT(v, 5); }
TEST(Interp, Sub)  { auto v = get_result("func resultado() devolver 10 - 4 fin");   EXPECT_INT(v, 6); }
TEST(Interp, Mul)  { auto v = get_result("func resultado() devolver 3 * 4 fin");    EXPECT_INT(v, 12); }
TEST(Interp, Div)  { auto v = get_result("func resultado() devolver 10 / 2 fin");   EXPECT_INT(v, 5); }

TEST(Interp, FloatAdd) {
  auto v = get_result("func resultado() devolver 1.5 + 1.5 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_TRUE(v->is_float());
  EXPECT_DOUBLE_EQ(v->as_float(), 3);
}

TEST(Interp, IntFloatMixed) {
  auto v = get_result("func resultado() devolver 1 + 0.5 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_TRUE(v->is_float());
  EXPECT_DOUBLE_EQ(v->as_float(), 1.5);
}

TEST(Interp, DivisionByZero) {
  run_error("func f() devolver 1 / 0 fin\nf()", "cero");
}

TEST(Interp, NegationUnary) {
  auto v = get_result("func resultado() devolver -5 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, -5);
}

TEST(Interp, Precedence) {
  auto v = get_result("func resultado() devolver 2 + 3 * 4 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 14);
}

TEST(Interp, Parentheses) {
  auto v = get_result("func resultado() devolver (2 + 3) * 4 fin");
  ASSERT_NE(v, nullptr);
  EXPECT_INT(v, 20);
}

TEST(Interp, StringConcat) {
  auto v = get_result(R"(func resultado() devolver "hola" + " mundo" fin)");
  ASSERT_NE(v, nullptr);
  EXPECT_STR(v, "hola mundo");
}

TEST(Interp, StringIntConcat) {
  auto v = get_result(R"(func resultado() devolver "n=" + 42 fin)");
  ASSERT_NE(v, nullptr);
  EXPECT_STR(v, "n=42");
}

TEST(Interp, CmpLT_True)  { auto v = get_result("func resultado() devolver 1 < 2 fin");  EXPECT_BOOL(v, true); }
TEST(Interp, CmpLT_False) { auto v = get_result("func resultado() devolver 2 < 1 fin");  EXPECT_BOOL(v, false); }
TEST(Interp, CmpGT)       { auto v = get_result("func resultado() devolver 3 > 2 fin");  EXPECT_BOOL(v, true); }
TEST(Interp, CmpLE)       { auto v = get_result("func resultado() devolver 2 <= 2 fin"); EXPECT_BOOL(v, true); }
TEST(Interp, CmpGE)       { auto v = get_result("func resultado() devolver 3 >= 4 fin"); EXPECT_BOOL(v, false); }
TEST(Interp, CmpEQ_Int)   { auto v = get_result("func resultado() devolver 5 = 5 fin");  EXPECT_BOOL(v, true); }
TEST(Interp, CmpNEQ)      { auto v = get_result("func resultado() devolver 5 != 6 fin"); EXPECT_BOOL(v, true); }
TEST(Interp, CmpEQ_Str)   {
  auto v = get_result(R"(func resultado() devolver "a" = "a" fin)");
  EXPECT_BOOL(v, true);
}

TEST(Interp, LogicalAnd_TT) { auto v = get_result("func resultado() devolver verdadero y verdadero fin"); EXPECT_BOOL(v, true); }
TEST(Interp, LogicalAnd_TF) { auto v = get_result("func resultado() devolver verdadero y falso fin");     EXPECT_BOOL(v, false); }
TEST(Interp, LogicalOr_FF)  { auto v = get_result("func resultado() devolver falso o falso fin");         EXPECT_BOOL(v, false); }
TEST(Interp, LogicalOr_TF)  { auto v = get_result("func resultado() devolver verdadero o falso fin");     EXPECT_BOOL(v, true); }
TEST(Interp, LogicalBang)   { auto v = get_result("func resultado() devolver !verdadero fin");            EXPECT_BOOL(v, false); }
TEST(Interp, LogicalBangFalse){ auto v = get_result("func resultado() devolver !falso fin");              EXPECT_BOOL(v, true); }

TEST(Interp, ShortCircuitAnd) {
  run_ok("func resultado() devolver falso y verdadero fin");
}


TEST(Interp, IfTaken) {
  auto v = get_result(
    "var x se 0\n"
    "si verdadero haz x se 1 fin\n"
    "func resultado() devolver  x fin"
  );
  EXPECT_INT(v, 1);
}

TEST(Interp, IfNotTaken) {
  auto v = get_result(
    "var x se 0\n"
    "si falso haz x se 1 fin\n"
    "func resultado() devolver x fin"
  );
  EXPECT_INT(v, 0);
}

TEST(Interp, IfElseTaken) {
  auto v = get_result(
    "var x se 0\n"
    "si falso haz x se 1 sino x se 2 fin\n"
    "func resultado() devolver x fin"
  );
  EXPECT_INT(v, 2);
}

TEST(Interp, IfElseIfChain) {
  auto v = get_result(
    "var n se 2\n"
    "var r se 0\n"
    "si n = 1 haz r se 10\n"
    "sino si n = 2 haz r se 20\n"
    "sino r se 30\n"
    "fin\n"
    "func resultado() devolver r fin"
  );
  EXPECT_INT(v, 20);
}

TEST(Interp, WhileBasic) {
  auto v = get_result(
    "var i se 0\n"
    "mientras i < 5 haz i se i + 1 fin\n"
    "func resultado() devolver i fin"
  );
  EXPECT_INT(v, 5);
}

TEST(Interp, WhileNotEntered) {
  auto v = get_result(
    "var i se 10\n"
    "mientras i < 5 haz i se i + 1 fin\n"
    "func resultado() devolver i fin"
  );
  EXPECT_INT(v, 10);
}

TEST(Interp, WhileAccumulator) {
  auto v = get_result(
    "var sum se 0\n"
    "var i se 1\n"
    "mientras i <= 10 haz\n"
    "  sum se sum + i\n"
    "  i se i + 1\n"
    "fin\n"
    "func resultado() devolver sum fin"
  );
  EXPECT_INT(v, 55);
}

TEST(Interp, WhileScopedVarDoesNotLeak) {
  run_error(
    "mientras falso haz\n"
    " var x se 1\n"
    "fin\n"
    "var a se x",
    "no definida"
  );
}

TEST(Interp, FuncNoParams) {
  auto v = get_result("func resultado() devolver 42 fin");
  EXPECT_INT(v, 42);
}

TEST(Interp, FuncWithParams) {
  auto v = get_result(
    "func suma(a, b) devolver a + b fin\n"
    "func resultado() devolver suma(3, 4) fin"
  );
  EXPECT_INT(v, 7);
}

TEST(Interp, FuncRecursion) {
  auto v = get_result(
    "func fib(n)\n"
    "  si n < 2 haz devolver n fin\n"
    "  devolver fib(n - 1) + fib(n - 2)\n"
    "fin\n"
    "func resultado() devolver fib(10) fin"
  );
  EXPECT_INT(v, 55);
}

TEST(Interp, FuncLocalVarDoesNotLeak) {
  run_error(
    "func f() var local se 1 fin\n"
    "f()\n"
    "var x se local",
    "no definida"
  );
}

TEST(Interp, FuncArityMismatch) {
  run_error(
    "func f(a, b) devolver a + b fin\n"
    "f(1)",
    "argumento"
  );
}

TEST(Interp, FuncReturnsNull) {
  auto v = get_result(
    "func nada() fin\n"
    "func resultado() devolver nada() fin"
  );
  ASSERT_NE(v, nullptr);
  EXPECT_NULL(v);
}

TEST(Interp, FuncNestedCalls) {
  auto v = get_result(
    "func doble(x) devolver x + x fin\n"
    "func cuadruple(x) devolver doble(doble(x)) fin\n"
    "func resultado() devolver cuadruple(3) fin"
  );
  EXPECT_INT(v, 12);
}

TEST(Interp, ArrayEmpty) {
  auto v = get_result("func resultado() devolver [] fin");
  ASSERT_NE(v, nullptr);
  EXPECT_TRUE(v->is_array());
  EXPECT_TRUE(v->as_array().empty());
}

TEST(Interp, ArrayLiteral) {
  auto v = get_result("func resultado() devolver [1, 2, 3] fin");
  ASSERT_NE(v, nullptr);
  ASSERT_TRUE(v->is_array());
  EXPECT_EQ(v->as_array().size(), 3u);
  EXPECT_EQ(v->as_array()[0]->as_int(), 1);
  EXPECT_EQ(v->as_array()[1]->as_int(), 2);
  EXPECT_EQ(v->as_array()[2]->as_int(), 3);
}

TEST(Interp, ArrayIndex) {
  auto v = get_result(
    "var a se [10, 20, 30]\n"
    "func resultado() devolver a[1] fin"
  );
  EXPECT_INT(v, 20);
}

TEST(Interp, ArrayIndexOutOfRange) {
  run_error(
    "var a se [1, 2]\n"
    "var x se a[5]",
    "fuera de rango"
  );
}

TEST(Interp, ArrayIndexOnNonArray) {
  run_error("var a se 1\nvar b se a[0]", "solo se puede indexar array o string");
}

TEST(Interp, ArrayWithExpressions) {
  auto v = get_result(
    "var n se 3\n"
    "func resultado() devolver [n, n + 1, n + 2] fin"
  );
  ASSERT_TRUE(v->is_array());
  EXPECT_EQ(v->as_array()[0]->as_int(), 3);
  EXPECT_EQ(v->as_array()[1]->as_int(), 4);
  EXPECT_EQ(v->as_array()[2]->as_int(), 5);
}

TEST(Interp, ClassInstantiationNoctor) {
  auto v = get_result(
    "clase Punto\n"
    "  var px se 0\n"
    "  var py se 0\n"
    "fin\n"
    "func resultado() devolver Punto() fin"
  );
  ASSERT_NE(v, nullptr);
  EXPECT_INSTANCE(v);
}

TEST(Interp, ClassInstanceFields) {
  auto v = get_result(
    "clase Punto\n"
    "  var px se 0\n"
    "  var py se 0\n"
    "  func crear(nx, ny)\n"
    "    este.px se nx\n"
    "    este.py se ny\n"
    "  fin\n"
    "fin\n"
    "var p se Punto(3,1)\n"
    "func resultado() devolver p.px fin"
  );
  EXPECT_INT(v, 3);
}

TEST(Interp, ClassFieldWrite) {
  auto v = get_result(
    "clase Caja\n"
    "  var ancho se 0\n"
    "  func crear(w) este.ancho se w fin\n"
    "fin\n"
    "var c se Caja(10)\n"
    "c.ancho se 99\n"
    "func resultado() devolver c.ancho fin"
  );
  EXPECT_INT(v, 99);
}

TEST(Interp, ClassMethodCall) {
  auto v = get_result(
    "clase Contador\n"
    "  var n se 0\n"
    "  func crear(inicio) este.n se inicio fin\n"
    "  func incrementar() este.n se este.n + 1 fin\n"
    "  func valor() devolver este.n fin\n"
    "fin\n"
    "var c se Contador(0)\n"
    "c.incrementar()\n"
    "c.incrementar()\n"
    "c.incrementar()\n"
    "func resultado() devolver c.valor() fin"
  );
  EXPECT_INT(v, 3);
}

TEST(Interp, ClassSiblingMethodCall) {
  auto v = get_result(
    "clase A\n"
    "  func doble(x) devolver x + x fin\n"
    "  func cuad(x) devolver este.doble(este.doble(x)) fin\n"
    "fin\n"
    "var a se A()\n"
    "func resultado() devolver a.cuad(3) fin"
  );
  EXPECT_INT(v, 12);
}

TEST(Interp, ClassToString) {
  auto v = get_result(
    "clase Cosa fin\n"
    "func resultado() devolver Cosa() fin"
  );
  ASSERT_NE(v, nullptr);
  EXPECT_NE(v->to_string().find("Cosa"), std::string::npos);
}

TEST(Interp, ClassCtorArityMismatch) {
  run_error(
    "clase P\n"
    "  func crear(a, b) fin\n"
    "fin\n"
    "var p se P(1)",
    "argumento"
  );
}
TEST(Interp, Fibonacci) {
  auto v = get_result(
    "func fib(n)\n"
    "  si n < 2 haz\n"
    "   devolver n\n"
    "  sino\n"
    "   devolver fib(n - 1) + fib(n - 2)\n"
    "  fin\n"
    "fin\n"
    "func resultado() devolver fib(10) fin"
  );
  EXPECT_INT(v, 55);
}

TEST(Interp, Factorial) {
  auto v = get_result(
    "func fact(n)\n"
    "  si n <= 1 haz devolver 1 fin\n"
    "  devolver n * fact(n - 1)\n"
    "fin\n"
    "func resultado() devolver fact(10) fin"
  );
  EXPECT_INT(v, 3628800);
}

TEST(Interp, SumArray) {
  auto v = get_result(
    "func suma_hasta(n)\n"
    "  var acc se 0\n"
    "  var i se 1\n"
    "  mientras i <= n haz\n"
    "    acc se acc + i\n"
    "    i se i + 1\n"
    "  fin\n"
    "  devolver acc\n"
    "fin\n"
    "func resultado() devolver suma_hasta(100) fin"
  );
  EXPECT_INT(v, 5050);
}

TEST(Interp, CounterClass) {
  auto v = get_result(
    "clase Contador\n"
    "  var n se 0\n"
    "  func crear(start) este.n se start fin\n"
    "  func tick() este.n se este.n + 1 fin\n"
    "  func get() devolver este.n fin\n"
    "fin\n"
    "var c se Contador(10)\n"
    "c.tick()\n"
    "c.tick()\n"
    "c.tick()\n"
    "func resultado() devolver c.get() fin"
  );
  EXPECT_INT(v, 13);
}

TEST(Interp, HigherOrderViaWrapper) {
  auto v = get_result(
    "func aplicar_doble(x) devolver x + x fin\n"
    "func componer(x) devolver aplicar_doble(aplicar_doble(x)) fin\n"
    "func resultado() devolver componer(5) fin"
  );
  EXPECT_INT(v, 20);
}
TEST(Interp, StringMethods) {
  auto v = get_result(
    "var str se 'key|value'"
    "var array se str.separar('|')"
    "func resultado() devolver array fin"
  );
  EXPECT_ARRAY(v, "[key, value]");
}

TEST(Interp, StringToInt) {
  auto v = get_result(
    "var str se '3.14'"
    "var num se decimal(str)"
    "func resultado() devolver num fin"
  );
  EXPECT_FLOAT(v, 3.14);
}

TEST(Interp, NumToStr) {
  auto v = get_result(
    "var num se 3.14"
    "var str se cadena(num)"
    "func resultado() devolver str fin"
  );
  EXPECT_STR(v, "3.14");
}

TEST(Interp, IndexStr) {
  auto v = get_result(
    "var str se 'Holis'"
    "var char se str[0]"
    "func resultado() devolver char fin"
  );
  EXPECT_STR(v, "H");
}

TEST(Interp, Index2dArray) {
  auto v = get_result(
    "var array se [['#', 'x', '#']]"
    "func resultado() devolver array[0][1] fin"
  );
  EXPECT_STR(v, "x");
}

TEST(Interp, LhsArrayAssignment) {
  auto v = get_result(
    "var array se [['#', 'x', '#']]"
    "array[0][1] se '#'"
    "func resultado() devolver array[0][1] fin"
  );
  EXPECT_STR(v, "#");
}

TEST(Interp, RoundNumber) {
  auto v = get_result(
    "func resultado() devolver redondear(0.6) fin"
  );
  EXPECT_FLOAT(v, 1);
}

TEST(Array, Insertar) {
  auto v = get_result(
    "var x se []"
    "x.insertar(10)"
    "x.insertar('hello')"

    "func resultado() devolver x fin"
  );
  EXPECT_ARRAY(v, "[10, hello]");
}

TEST(Array, Eliminar) {
  auto v = get_result(
    "var x se [0.05, 10, 30, 'hello']"
    "x.eliminar(0)"
    "func resultado() devolver x fin"
  );
  EXPECT_ARRAY(v, "[10, 30, hello]");
}

TEST(Array, Contiene) {
  auto v = get_result(
    "var id se 'random'"
    "var x se [0, 0, 3, id]"
    "func resultado() devolver x.contiene('random') fin"
  );
  EXPECT_TRUE(v);
}

TEST(Array, InsertarEn) {
  auto v = get_result(
    "var id se 'target'"
    "var x se [0, 0, 3]"
    "x.insertar_en(1, id)"
    "func resultado() devolver x.encuentra_index(id) fin"
  );
  EXPECT_INT(v, 1);
}


TEST(Continuar, ReturnStillWorksWithContinuar) {
  auto v = get_result(
    "func f()\n"
    "  var i se 0\n"
    "  mientras i < 10 haz\n"
    "    i se i + 1\n"
    "    si i = 5 haz devolver i fin\n"
    "    continuar\n"
    "  fin\n"
    "  devolver 0\n"
    "fin\n"
    "func resultado() devolver f() fin"
  );
  EXPECT_INT(v, 5);
}

TEST(Continuar, ScopeCleanedUpOnContinue) {
  run_ok(
    "var i se 0\n"
    "mientras i < 3 haz\n"
    "  i se i + 1\n"
    "  var tmp se i\n"
    "  continuar\n"
    "fin"
  );
}

TEST(Continuar, InsideFuncLoop) {
  auto v = get_result(
    "func sumar_pares(n)\n"
    "  var sum se 0\n"
    "  var i se 0\n"
    "  mientras i < n haz\n"
    "    i se i + 1\n"
    "    si i = 1 o i = 3 o i = 5 haz continuar fin\n"
    "    sum se sum + i\n"
    "  fin\n"
    "  devolver sum\n"
    "fin\n"
    "func resultado() devolver sumar_pares(6) fin" // 6
  );
  // 2+4+6 = 12
  EXPECT_INT(v, 12);
}
