#pragma once

#include "runtime.hpp"

#include <cmath>
#include <iostream>

namespace egel {

/**
 * Egel's math built-ins.
 *
 * Unstable and untested.
 *
 * Almost all of these combinators work only on floats.
 **/

// ## namespace Math - builtin mathematical operators

// ## Math::is_finite x - test whether this float is finite
class IsFinite : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsFinite, "Math", "is_finite");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(isfinite(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::is_infinite x - test whether this float is infinite
class IsInfinite : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsInfinite, "Math", "is_infinite");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(isinf(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::is_nan x - test whether this float is Not a Number
class IsNan : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsNan, "Math", "is_nan");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(isnan(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::is_normal x - test whether this float is normal
class IsNormal : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsNormal, "Math", "is_normal");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_bool(isnormal(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::e - Euler's constant and the base of natural logarithms,
//  approximately 2.718
class Euler : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Euler, "Math", "e");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_E);
    }
};

// ## Math::ln2 - Natural logarithm of 2, approximately 0.693
class Ln2 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Ln2, "Math", "ln2");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_LN2);
    }
};

// ## Math::ln10 - natural logarithm of 10, approximately 2.303
class Ln10 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Ln10, "Math", "ln10");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_LN10);
    }
};

// ## Math::log2e - base 2 logarithm of E, approximately 1.443
class Log2e : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Log2e, "Math", "log2e");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_LOG2E);
    }
};

// ## Math::log10e - base 10 logarithm of E, approximately 0.434
class Log10e : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Log10e, "Math", "log10e");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_LOG10E);
    }
};

// ## Math::pi - ratio of the circumference of a circle to its diameter,
//  approximately 3.14159
class Pi : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Pi, "Math", "pi");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_PI);
    }
};

// ## Math::sqrt1_2 - square root of 1/2; equivalently, 1 over the square root
//  of 2, approximately 0.707
class Sqrt1_2 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Sqrt1_2, "Math", "sqrt1_2");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_SQRT1_2);
    }
};

// ## Math::sqrt2 - square root of 2, approximately 1.414
class Sqrt2 : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Sqrt2, "Math", "sqrt2");

    VMObjectPtr apply() const override {
        return machine()->create_float(M_SQRT2);
    }
};

// ## Math::abs x - returns the absolute value of a number
class Abs : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Abs, "Math", "abs");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(abs(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::acos x - returns the arccosine of a number
class Acos : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Acos, "Math", "acos");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(acos(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::acosh x - returns the hyperbolic arccosine of a number
class Acosh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Acosh, "Math", "acosh");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(acosh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::asin x - returns the arcsine of a number
class Asin : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Asin, "Math", "asin");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(asin(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::asinh x - returns the hyperbolic arcsine of a number
class Asinh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Asinh, "Math", "asinh");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(asinh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::atan x - returns the arctangent of a number
class Atan : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Atan, "Math", "atan");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(atan(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::atanh x - returns the hyperbolic arctangent of a number
class Atanh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Atanh, "Math", "atanh");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(atanh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::atan2 y x - returns the arctangent of the quotient of its arguments
class Atan2 : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Atan2, "Math", "atan2");

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

// ## Math::cbrt x - returns the cube root of a number
class Cbrt : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Cbrt, "Math", "cbrt");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(cbrt(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::ceil x - returns the smallest integer greater than or equal to a
//  number
class Ceil : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Ceil, "Math", "ceil");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(ceil(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::cos x - returns the cosine of a number
class Cos : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Cos, "Math", "cos");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(cos(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::cosh x - returns the hyperbolic cosine of a number
class Cosh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Cosh, "Math", "cosh");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(cosh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::exp x - Returns Ex, where x is the argument, and E is Euler's
//  constant (2.718…), the base of the natural logarithm
class Exp : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Exp, "Math", "exp");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(exp(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::expm1 x - returns subtracting 1 from exp x
class Expm1 : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Expm1, "Math", "expm1");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(expm1(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::floor x - returns the largest integer less than or equal to a number
class Floor : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Floor, "Math", "floor");

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
// Math::fround x - returns the nearest single precision float representation
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

// ## Math::log x - returns the natural logarithm (loge, also ln) of a number
class Log : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log, "Math", "log");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::log1p x - returns the natural logarithm (loge, also ln) of 1 + x for
//  a number x
class Log1p : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log1p, "Math", "log1p");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log1p(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::log10 x - returns the base 10 logarithm of a number
class Log10 : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log10, "Math", "log10");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log10(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::log2 x - returns the base 2 logarithm of a number
class Log2 : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Log2, "Math", "log2");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(log2(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::max x y - returns the largest of two numbers
class Max : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Max, "Math", "max");

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

// ## Math::min x y - returns the smallest of two numbers
class MMin : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, MMin, "Math", "min");

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

// ## Math::pow x y - returns base to the exponent power, that is, baseexponent
class Pow : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Pow, "Math", "pow");

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

// ## Math::random - returns a pseudo-random number between 0 and 1
class Random : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_BUILTIN, Random, "Math", "random");

    VMObjectPtr apply() const override {
        return machine()->create_float(random());
    }
};

// ## Math::round x - returns the value of a number rounded to the nearest
//  integer
class Round : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Round, "Math", "round");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_integer(lround(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::sign x - returns the sign of the x, indicating whether x is
//  positive, negative or zero
class Sign : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sign, "Math", "sign");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            auto b = signbit(f);
            return machine()->create_integer((b != 0) ? (-1) : (1));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::sin x - returns the sine of a number
class Sin : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sin, "Math", "sin");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(sin(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::sinh x - returns the hyperbolic sine of a number
class Sinh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sinh, "Math", "sinh");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(sinh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::sqrt x - returns the positive square root of a number
class Sqrt : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Sqrt, "Math", "sqrt");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(sqrt(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::tan x - returns the tangent of a number
class Tan : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Tan, "Math", "tan");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(tan(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::tanh x - returns the hyperbolic tangent of a number
class Tanh : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Tanh, "Math", "tanh");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(tanh(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// ## Math::trunc x - returns the integral part of the number x, removing any
//  fractional digits
class Trunc : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Trunc, "Math", "trunc");

    VMObjectPtr apply(const VMObjectPtr &arg0) const override {
        if (machine()->is_float(arg0)) {
            auto f = machine()->get_float(arg0);
            return machine()->create_float(trunc(f));
        } else {
            throw machine()->bad_args(this, arg0);
        }
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
