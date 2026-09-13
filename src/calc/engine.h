#pragma once
#include <string>
#include <stdexcept>

namespace calc {

enum class AngleMode { Degree, Radian };

// A small recursive-descent expression evaluator supporting the common
// scientific-calculator vocabulary: + - * / ^, parentheses, unary minus,
// sin/cos/tan (+ inverse), ln/log, sqrt, x^2, 1/x, pi, e and Ans.
// Not a reimplementation of Casio's real math engine (that is proprietary
// and far more elaborate -- CAS, fractions, matrices, graphing, etc.);
// this is an original, from-scratch evaluator good enough for real
// scientific-calculator use.
class Engine {
public:
    void SetAngleMode(AngleMode m) { angle_mode_ = m; }
    AngleMode GetAngleMode() const { return angle_mode_; }

    double Ans() const { return ans_; }

    // Throws std::runtime_error with a short message (e.g. "Ma ERROR",
    // "Syntax ERROR") on failure -- the caller shows it on the display.
    double Evaluate(const std::wstring& expr);

private:
    AngleMode angle_mode_ = AngleMode::Degree;
    double ans_ = 0.0;

    const wchar_t* p_ = nullptr;

    void SkipSpaces();
    double ParseExpression();
    double ParseTerm();
    double ParsePower();
    double ParseUnary();
    double ParseFactor();
    double ParseFunctionArg();
    [[noreturn]] void Fail(const char* msg);
};

} // namespace calc
