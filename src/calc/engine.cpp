#include "engine.h"
#include <cmath>
#include <cwctype>
#include <cstdlib>

namespace calc {

namespace {
constexpr double kPi = 3.14159265358979323846;
double DegToRad(double d) { return d * kPi / 180.0; }
double RadToDeg(double r) { return r * 180.0 / kPi; }
} // namespace

void Engine::Fail(const char* msg) { throw std::runtime_error(msg); }

void Engine::SkipSpaces() {
    while (*p_ == L' ') ++p_;
}

double Engine::ParseFunctionArg() {
    SkipSpaces();
    if (*p_ != L'(') Fail("Syntax ERROR");
    ++p_;
    double v = ParseExpression();
    SkipSpaces();
    if (*p_ != L')') Fail("Syntax ERROR");
    ++p_;
    return v;
}

double Engine::ParseFactor() {
    SkipSpaces();
    double v;
    if (*p_ == L'(') {
        ++p_;
        v = ParseExpression();
        SkipSpaces();
        if (*p_ != L')') Fail("Syntax ERROR");
        ++p_;
    } else if (iswdigit(*p_) || *p_ == L'.') {
        wchar_t* end = nullptr;
        v = wcstod(p_, &end);
        if (end == p_) Fail("Syntax ERROR");
        p_ = end;
    } else if (iswalpha(*p_)) {
        const wchar_t* start = p_;
        while (iswalpha(*p_)) ++p_;
        std::wstring name(start, p_);

        if (name == L"sin") {
            double a = ParseFunctionArg();
            v = std::sin(angle_mode_ == AngleMode::Degree ? DegToRad(a) : a);
        } else if (name == L"cos") {
            double a = ParseFunctionArg();
            v = std::cos(angle_mode_ == AngleMode::Degree ? DegToRad(a) : a);
        } else if (name == L"tan") {
            double a = ParseFunctionArg();
            v = std::tan(angle_mode_ == AngleMode::Degree ? DegToRad(a) : a);
        } else if (name == L"asin") {
            double a = ParseFunctionArg();
            if (a < -1.0 || a > 1.0) Fail("Math ERROR");
            v = std::asin(a);
            if (angle_mode_ == AngleMode::Degree) v = RadToDeg(v);
        } else if (name == L"acos") {
            double a = ParseFunctionArg();
            if (a < -1.0 || a > 1.0) Fail("Math ERROR");
            v = std::acos(a);
            if (angle_mode_ == AngleMode::Degree) v = RadToDeg(v);
        } else if (name == L"atan") {
            double a = ParseFunctionArg();
            v = std::atan(a);
            if (angle_mode_ == AngleMode::Degree) v = RadToDeg(v);
        } else if (name == L"ln") {
            double a = ParseFunctionArg();
            if (a <= 0.0) Fail("Math ERROR");
            v = std::log(a);
        } else if (name == L"log") {
            double a = ParseFunctionArg();
            if (a <= 0.0) Fail("Math ERROR");
            v = std::log10(a);
        } else if (name == L"sqrt") {
            double a = ParseFunctionArg();
            if (a < 0.0) Fail("Math ERROR");
            v = std::sqrt(a);
        } else if (name == L"abs") {
            v = std::fabs(ParseFunctionArg());
        } else if (name == L"pi") {
            v = kPi;
        } else if (name == L"e") {
            v = 2.71828182845904523536;
        } else if (name == L"Ans") {
            v = ans_;
        } else {
            Fail("Syntax ERROR");
        }
    } else {
        Fail("Syntax ERROR");
    }

    SkipSpaces();
    if (*p_ == L'%') { ++p_; v /= 100.0; }
    return v;
}

// Precedence (low to high): Expression(+ -) > Term(* /) > Unary(-) > Power(^) > Factor.
// This makes "-3^2" parse as -(3^2) = -9 (unary minus applies to the whole
// power, matching standard math/calculator convention), while "2^-3" still
// works because the exponent itself is parsed as a Unary.
double Engine::ParseUnary() {
    SkipSpaces();
    if (*p_ == L'-') { ++p_; return -ParseUnary(); }
    if (*p_ == L'+') { ++p_; return ParseUnary(); }
    return ParsePower();
}

double Engine::ParsePower() {
    double base = ParseFactor();
    SkipSpaces();
    if (*p_ == L'^') {
        ++p_;
        double exp = ParseUnary(); // right-associative, exponent may itself be signed
        return std::pow(base, exp);
    }
    return base;
}

double Engine::ParseTerm() {
    double v = ParseUnary();
    for (;;) {
        SkipSpaces();
        if (*p_ == L'*') {
            ++p_;
            v *= ParseUnary();
        } else if (*p_ == L'/') {
            ++p_;
            double d = ParseUnary();
            if (d == 0.0) Fail("Math ERROR");
            v /= d;
        } else if (*p_ == L'(' || iswalpha(*p_)) {
            // implicit multiplication, e.g. "2sin(30)" or "(2+3)(4+5)"
            v *= ParseUnary();
        } else {
            break;
        }
    }
    return v;
}

double Engine::ParseExpression() {
    double v = ParseTerm();
    for (;;) {
        SkipSpaces();
        if (*p_ == L'+') { ++p_; v += ParseTerm(); }
        else if (*p_ == L'-') { ++p_; v -= ParseTerm(); }
        else break;
    }
    return v;
}

double Engine::Evaluate(const std::wstring& expr) {
    p_ = expr.c_str();
    double result = ParseExpression();
    SkipSpaces();
    if (*p_ != 0) Fail("Syntax ERROR");
    if (std::isnan(result) || std::isinf(result)) Fail("Math ERROR");
    ans_ = result;
    return result;
}

} // namespace calc
