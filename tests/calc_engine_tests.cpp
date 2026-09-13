#include <cstdio>
#include <cmath>
#include "../src/calc/engine.h"

namespace {
int g_failures = 0;

void CheckNear(double got, double want, const char* what) {
    if (std::fabs(got - want) > 1e-6) {
        std::fprintf(stderr, "FALLO: %s (obtuve %.9g, esperaba %.9g)\n", what, got, want);
        g_failures++;
    } else {
        std::printf("OK: %s\n", what);
    }
}
} // namespace

int main() {
    calc::Engine eng; // default: grados

    CheckNear(eng.Evaluate(L"2+3*4"), 14.0, "precedencia * sobre +");
    CheckNear(eng.Evaluate(L"(2+3)*4"), 20.0, "parentesis");
    CheckNear(eng.Evaluate(L"2^3^2"), 512.0, "potencia asociativa a la derecha (2^(3^2))");
    CheckNear(eng.Evaluate(L"-3^2"), -9.0, "menos unario aplica sobre toda la potencia");
    CheckNear(eng.Evaluate(L"2^-3"), 0.125, "exponente con signo");
    CheckNear(eng.Evaluate(L"sqrt(16)"), 4.0, "sqrt");
    CheckNear(eng.Evaluate(L"sin(30)"), 0.5, "sin en modo grados");
    CheckNear(eng.Evaluate(L"2(3+4)"), 14.0, "multiplicacion implicita con parentesis");
    CheckNear(eng.Evaluate(L"2sin(30)"), 1.0, "multiplicacion implicita con funcion");
    CheckNear(eng.Evaluate(L"1/4*100"), 25.0, "division y multiplicacion en orden");
    CheckNear(eng.Evaluate(L"ln(e)"), 1.0, "ln(e)");
    CheckNear(eng.Evaluate(L"log(100)"), 2.0, "log base 10");

    eng.Evaluate(L"21");
    CheckNear(eng.Evaluate(L"Ans*2"), 42.0, "Ans reutiliza el resultado anterior");

    eng.SetAngleMode(calc::AngleMode::Radian);
    CheckNear(eng.Evaluate(L"sin(pi/2)"), 1.0, "sin en modo radianes");

    bool threw = false;
    try { eng.Evaluate(L"1/0"); } catch (const std::exception&) { threw = true; }
    if (!threw) { std::fprintf(stderr, "FALLO: division por cero deberia lanzar Math ERROR\n"); g_failures++; }
    else std::printf("OK: division por cero lanza error\n");

    if (g_failures > 0) {
        std::fprintf(stderr, "\n%d prueba(s) fallaron.\n", g_failures);
        return 1;
    }
    std::printf("\nTodas las pruebas del motor de calculo pasaron.\n");
    return 0;
}
