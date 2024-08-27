#pragma once

#include <cmath>
#include <iostream>

#include "runtime.hpp"

namespace egel {

/**
 * Egel's math built-ins.
 *
 * Unstable and untested.
 *
 * Almost all of these combinators work only on floats.
 **/

//DOCSTRING("namespace Math - builtin mathematical operators");

class IsFinite : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsFinite, "Math", "is_finite");
DOCSTRING("Math::is_finite x - test whether this float is finite");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(std::isfinite(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsInfinite : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsInfinite, "Math", "is_infinite");
DOCSTRING("Math::is_infinite x - test whether this float is infinite");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(std::isinf(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsNan : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsNan, "Math", "is_nan");
DOCSTRING("Math::is_nan x - test whether this float is Not a Number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(std::isnan(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsNormal : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsNormal, "Math", "is_normal");
DOCSTRING("Math::is_normal x - test whether this float is normal");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(std::isnormal(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

//  approximately 2.718
class Euler : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Euler, "Math", "e");
DOCSTRING("Math::e - Euler's constant and the base of natural logarithms,");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_E);
    }
};

class Ln2 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Ln2, "Math", "ln2");
DOCSTRING("Math::ln2 - Natural logarithm of 2, approximately 0.693");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_LN2);
    }
};

class Ln10 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Ln10, "Math", "ln10");

DOCSTRING("Math::ln10 - natural logarithm of 10, approximately 2.303");
    VMObjectPtr apply() const override {
        return machine()->create_float(M_LN10);
    }
};

class Log2e : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Log2e, "Math", "log2e");

DOCSTRING("Math::log2e - base 2 logarithm of E, approximately 1.443");
    VMObjectPtr apply() const override {
        return machine()->create_float(M_LOG2E);
    }
};

class Log10e : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Log10e, "Math", "log10e");
DOCSTRING("Math::log10e - base 10 logarithm of E, approximately 0.434");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_LOG10E);
    }
};

//  approximately 3.14159
class Pi : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Pi, "Math", "pi");
DOCSTRING("Math::pi - ratio of the circumference of a circle to its diameter,");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_PI);
    }
};

//  of 2, approximately 0.707
class Sqrt1_2 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Sqrt1_2, "Math", "sqrt1_2");
DOCSTRING("Math::sqrt1_2 - square root of 1/2; equivalently, 1 over the square root");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_SQRT1_2);
    }
};

class Sqrt2 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Sqrt2, "Math", "sqrt2");
DOCSTRING("Math::sqrt2 - square root of 2, approximately 1.414");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_SQRT2);
    }
};

class Abs : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Abs, "Math", "abs");
DOCSTRING("Math::abs x - the absolute value of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(abs(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Acos : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Acos, "Math", "acos");
DOCSTRING("Math::acos x - the arccosine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(acos(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Acosh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Acosh, "Math", "acosh");
DOCSTRING("Math::acosh x - the hyperbolic arccosine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(acosh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Asin : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Asin, "Math", "asin");
DOCSTRING("Math::asin x - the arcsine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(asin(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Asinh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Asinh, "Math", "asinh");
DOCSTRING("Math::asinh x - the hyperbolic arcsine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(asinh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Atan : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Atan, "Math", "atan");
DOCSTRING("Math::atan x - the arctangent of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(atan(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Atanh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Atanh, "Math", "atanh");
DOCSTRING("Math::atanh x - the hyperbolic arctangent of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(atanh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Atan2 : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Atan2, "Math", "atan2");
DOCSTRING("Math::atan2 y x - the arctangent of the quotient of its arguments");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float(atan2(f0, f1));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Cbrt : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Cbrt, "Math", "cbrt");
DOCSTRING("Math::cbrt x - the cube root of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(cbrt(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Ceil : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Ceil, "Math", "ceil");
DOCSTRING("Math::ceil x - the ceiling of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(ceil(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Cos : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Cos, "Math", "cos");
DOCSTRING("Math::cos x - the cosine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(cos(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Cosh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Cosh, "Math", "cosh");
DOCSTRING("Math::cosh x - the hyperbolic cosine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(cosh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Exp : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Exp, "Math", "exp");
DOCSTRING("Math::exp x - the exp of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(exp(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Expm1 : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Expm1, "Math", "expm1");
DOCSTRING("Math::expm1 x - subtracting 1 from exp");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(expm1(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Floor : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Floor, "Math", "floor");
DOCSTRING("Math::floor x - the largest integer less than or equal to a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(floor(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

/*
// Math::fround x - the nearest single precision float representation
of a number class Fround: public Monadic { public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Fround, "Math", "fround");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return create_float(fround(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};
*/

class Log : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log, "Math", "log");
DOCSTRING("Math::log x - the natural logarithm (loge, also ln) of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Log1p : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log1p, "Math", "log1p");
DOCSTRING("Math::log1p x - the natural logarithm of the next number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log1p(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Log10 : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log10, "Math", "log10");
DOCSTRING("Math::log10 x - the base 10 logarithm of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log10(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Log2 : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log2, "Math", "log2");
DOCSTRING("Math::log2 x - the base 2 logarithm of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log2(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Max : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Max, "Math", "max");
DOCSTRING("Math::max x y - the largest of two numbers");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float((f0 < f1) ? f1 : f0);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class MMin : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, MMin, "Math", "min");
DOCSTRING("Math::min x y - the smallest of two numbers");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float((f0 < f1) ? f0 : f1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Pow : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Pow, "Math", "pow");
DOCSTRING("Math::pow x y - base to the exponent power, that is, baseexponent");

    VMObjectPtr apply(const VMObjectPtr &arg0,
                      const VMObjectPtr &arg1) const override {
        if ((machine()->is_float(arg0)) && (machine()->is_float(arg1))) {
            auto f0 = machine()->get_float(arg0);
            auto f1 = machine()->get_float(arg1);
            return machine()->create_float(pow(f0, f1));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Random : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Random, "Math", "random");
DOCSTRING("Math::random - a pseudo-random number between 0 and 1");

    VMObjectPtr apply() const override {
        return machine()->create_float(random());
    }
};

//  integer
class Round : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Round, "Math", "round");
DOCSTRING("Math::round x - the value of a number rounded to the nearest");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_integer(lround(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Sign : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sign, "Math", "sign");
DOCSTRING("Math::sign x - the sign of the a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            auto b = std::signbit(f);
            return machine()->create_integer((b != 0) ? (-1) : (1));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Sin : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sin, "Math", "sin");
DOCSTRING("Math::sin x - the sine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(sin(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Sinh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sinh, "Math", "sinh");
DOCSTRING("Math::sinh x - the hyperbolic sine of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(sinh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Sqrt : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sqrt, "Math", "sqrt");
DOCSTRING("Math::sqrt x - the positive square root of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(sqrt(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Tan : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Tan, "Math", "tan");
DOCSTRING("Math::tan x - the tangent of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(tan(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Tanh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Tanh, "Math", "tanh");
DOCSTRING("Math::tanh x - the hyperbolic tangent of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(tanh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Trunc : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Trunc, "Math", "trunc");
DOCSTRING("Math::trunc x - the integral part of a number");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(trunc(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class MathModule: public CModule {
public:
    icu::UnicodeString name() const override {
        return "math";
    }

    icu::UnicodeString docstring() const override {
        return "The 'math' module defines trigonometic and some other often used combinators.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;

    oo.push_back(Euler::create(vm));
    oo.push_back(Ln2::create(vm));
    oo.push_back(Ln10::create(vm));
    oo.push_back(Log2e::create(vm));
    oo.push_back(Log10e::create(vm));
    oo.push_back(Pi::create(vm));
    oo.push_back(Sqrt1_2::create(vm));
    oo.push_back(Sqrt2::create(vm));
    oo.push_back(Abs::create(vm));
    oo.push_back(Acos::create(vm));
    oo.push_back(Acosh::create(vm));
    oo.push_back(Asin::create(vm));
    oo.push_back(Asinh::create(vm));
    oo.push_back(Atan::create(vm));
    oo.push_back(Atanh::create(vm));
    oo.push_back(Atan2::create(vm));
    oo.push_back(Cbrt::create(vm));
    oo.push_back(Ceil::create(vm));
    oo.push_back(Cos::create(vm));
    oo.push_back(Cosh::create(vm));
    oo.push_back(Exp::create(vm));
    oo.push_back(Expm1::create(vm));
    oo.push_back(Floor::create(vm));
    //    oo.push_back(Fround::create(vm));
    oo.push_back(Log::create(vm));
    oo.push_back(Log1p::create(vm));
    oo.push_back(Log10::create(vm));
    oo.push_back(Log2::create(vm));
    oo.push_back(Max::create(vm));
    oo.push_back(MMin::create(vm));
    oo.push_back(Pow::create(vm));
    oo.push_back(Random::create(vm));
    oo.push_back(Round::create(vm));
    oo.push_back(Sign::create(vm));
    oo.push_back(Sin::create(vm));
    oo.push_back(Sinh::create(vm));
    oo.push_back(Sqrt::create(vm));
    oo.push_back(Tan::create(vm));
    oo.push_back(Tanh::create(vm));
    oo.push_back(Trunc::create(vm));


        return oo;
    }
};

std::vector<VMObjectPtr> builtin_math(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Euler::create(vm));
    oo.push_back(Ln2::create(vm));
    oo.push_back(Ln10::create(vm));
    oo.push_back(Log2e::create(vm));
    oo.push_back(Log10e::create(vm));
    oo.push_back(Pi::create(vm));
    oo.push_back(Sqrt1_2::create(vm));
    oo.push_back(Sqrt2::create(vm));
    oo.push_back(Abs::create(vm));
    oo.push_back(Acos::create(vm));
    oo.push_back(Acosh::create(vm));
    oo.push_back(Asin::create(vm));
    oo.push_back(Asinh::create(vm));
    oo.push_back(Atan::create(vm));
    oo.push_back(Atanh::create(vm));
    oo.push_back(Atan2::create(vm));
    oo.push_back(Cbrt::create(vm));
    oo.push_back(Ceil::create(vm));
    oo.push_back(Cos::create(vm));
    oo.push_back(Cosh::create(vm));
    oo.push_back(Exp::create(vm));
    oo.push_back(Expm1::create(vm));
    oo.push_back(Floor::create(vm));
    //    oo.push_back(Fround::create(vm));
    oo.push_back(Log::create(vm));
    oo.push_back(Log1p::create(vm));
    oo.push_back(Log10::create(vm));
    oo.push_back(Log2::create(vm));
    oo.push_back(Max::create(vm));
    oo.push_back(MMin::create(vm));
    oo.push_back(Pow::create(vm));
    oo.push_back(Random::create(vm));
    oo.push_back(Round::create(vm));
    oo.push_back(Sign::create(vm));
    oo.push_back(Sin::create(vm));
    oo.push_back(Sinh::create(vm));
    oo.push_back(Sqrt::create(vm));
    oo.push_back(Tan::create(vm));
    oo.push_back(Tanh::create(vm));
    oo.push_back(Trunc::create(vm));

    return oo;
}

}  // namespace egel
