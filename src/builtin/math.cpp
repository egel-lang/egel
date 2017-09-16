#include "math.hpp"

#include <stdlib.h>
#include <math.h>

/**
 * Egel's math built-ins.
 *
 * Unstable and untested.
 *
 * Almost all of these combinators work only on floats. 
 **/

// Math.isFinite x
// Test on whether this float is finite.
class IsFinite: public Monadic {
public:
    MONADIC_PREAMBLE(IsFinite, "Math", "isFinite");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isfinite(f));
        } else {
            return nullptr;
        }
    }
};

// Math.isInfinite x
// Test on whether this float is infinite.
class IsInfinite: public Monadic {
public:
    MONADIC_PREAMBLE(IsInfinite, "Math", "isInfinite");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isinf(f));
        } else {
            return nullptr;
        }
    }
};

// Math.isNan x
// Test on whether this float is Not a Number.
class IsNan: public Monadic {
public:
    MONADIC_PREAMBLE(IsNan, "Math", "isNan");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isnan(f));
        } else {
            return nullptr;
        }
    }
};

// Math.isNormal x
// Test on whether this float is normal.
class IsNormal: public Monadic {
public:
    MONADIC_PREAMBLE(IsNormal, "Math", "isNormal");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isnormal(f));
        } else {
            return nullptr;
        }
    }
};

// Math.e
// Euler's constant and the base of natural logarithms, approximately 2.718.
class Euler: public Medadic {
public:
    MEDADIC_PREAMBLE(Euler, "Math", "e");

    VMObjectPtr apply() const override {
        return create_float(M_E);
    }
};

// Math.ln2
// Natural logarithm of 2, approximately 0.693.
class Ln2: public Medadic {
public:
    MEDADIC_PREAMBLE(Ln2, "Math", "ln2");

    VMObjectPtr apply() const override {
        return create_float(M_LN2);
    }
};

// Math.ln10
// Natural logarithm of 10, approximately 2.303.
class Ln10: public Medadic {
public:
    MEDADIC_PREAMBLE(Ln10, "Math", "ln10");

    VMObjectPtr apply() const override {
        return create_float(M_LN10);
    }
};

// Math.log2e
// Base 2 logarithm of E, approximately 1.443.
class Log2e: public Medadic {
public:
    MEDADIC_PREAMBLE(Log2e, "Math", "log2e");

    VMObjectPtr apply() const override {
        return create_float(M_LOG2E);
    }
};

// Math.log10e
// Base 10 logarithm of E, approximately 0.434.
class Log10e: public Medadic {
public:
    MEDADIC_PREAMBLE(Log10e, "Math", "log10e");

    VMObjectPtr apply() const override {
        return create_float(M_LOG10E);
    }
};

// Math.pi
// Ratio of the circumference of a circle to its diameter, approximately 3.14159.
class Pi: public Medadic {
public:
    MEDADIC_PREAMBLE(Pi, "Math", "pi");

    VMObjectPtr apply() const override {
        return create_float(M_PI);
    }
};

// Math.sqrt1_2
// Square root of 1/2; equivalently, 1 over the square root of 2, approximately 0.707.
class Sqrt1_2: public Medadic {
public:
    MEDADIC_PREAMBLE(Sqrt1_2, "Math", "sqrt1_2");

    VMObjectPtr apply() const override {
        return create_float(M_SQRT1_2);
    }
};

// Math.sqrt2
// Square root of 2, approximately 1.414.
class Sqrt2: public Medadic {
public:
    MEDADIC_PREAMBLE(Sqrt2, "Math", "sqrt2");

    VMObjectPtr apply() const override {
        return create_float(M_SQRT2);
    }
};

// Math.abs x
// Returns the absolute value of a number.
class Abs: public Monadic {
public:
    MONADIC_PREAMBLE(Abs, "Math", "abs");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(abs(f));
        } else {
            return nullptr;
        }
    }
};

// Math.acos x
// Returns the arccosine of a number.
class Acos: public Monadic {
public:
    MONADIC_PREAMBLE(Acos, "Math", "acos");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(acos(f));
        } else {
            return nullptr;
        }
    }
};


// Math.acosh x
// Returns the hyperbolic arccosine of a number.
class Acosh: public Monadic {
public:
    MONADIC_PREAMBLE(Acosh, "Math", "acosh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(acosh(f));
        } else {
            return nullptr;
        }
    }
};


// Math.asin x
// Returns the arcsine of a number.
class Asin: public Monadic {
public:
    MONADIC_PREAMBLE(Asin, "Math", "asin");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(asin(f));
        } else {
            return nullptr;
        }
    }
};


// Math.asinh x
// Returns the hyperbolic arcsine of a number.
class Asinh: public Monadic {
public:
    MONADIC_PREAMBLE(Asinh, "Math", "asinh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(asinh(f));
        } else {
            return nullptr;
        }
    }
};


// Math.atan x
// Returns the arctangent of a number.
class Atan: public Monadic {
public:
    MONADIC_PREAMBLE(Atan, "Math", "atan");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(atan(f));
        } else {
            return nullptr;
        }
    }
};

// Math.atanh x
// Returns the hyperbolic arctangent of a number.
class Atanh: public Monadic {
public:
    MONADIC_PREAMBLE(Atanh, "Math", "atanh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(atanh(f));
        } else {
            return nullptr;
        }
    }
};


// Math.atan2 y x
// Returns the arctangent of the quotient of its arguments.
class Atan2: public Dyadic {
public:
    DYADIC_PREAMBLE(Atan2, "Math", "atan2");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(atan2(f0, f1));
        } else {
            return nullptr;
        }
    }
};

// Math.cbrt x
// Returns the cube root of a number.
class Cbrt: public Monadic {
public:
    MONADIC_PREAMBLE(Cbrt, "Math", "cbrt");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(cbrt(f));
        } else {
            return nullptr;
        }
    }
};


// Math.ceil x
// Returns the smallest integer greater than or equal to a number.
class Ceil: public Monadic {
public:
    MONADIC_PREAMBLE(Ceil, "Math", "ceil");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(ceil(f));
        } else {
            return nullptr;
        }
    }
};

// Math.cos x
// Returns the cosine of a number.
class Cos: public Monadic {
public:
    MONADIC_PREAMBLE(Cos, "Math", "cos");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(cos(f));
        } else {
            return nullptr;
        }
    }
};


// Math.cosh x
// Returns the hyperbolic cosine of a number.
class Cosh: public Monadic {
public:
    MONADIC_PREAMBLE(Cosh, "Math", "cosh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(cosh(f));
        } else {
            return nullptr;
        }
    }
};

// Math.exp x
// Returns Ex, where x is the argument, and E is Euler's constant (2.718…), the base of the natural logarithm.
class Exp: public Monadic {
public:
    MONADIC_PREAMBLE(Exp, "Math", "exp");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(exp(f));
        } else {
            return nullptr;
        }
    }
};


// Math.expm1 x
// Returns subtracting 1 from exp x.
class Expm1: public Monadic {
public:
    MONADIC_PREAMBLE(Expm1, "Math", "expm1");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(expm1(f));
        } else {
            return nullptr;
        }
    }
};

// Math.floor x
// Returns the largest integer less than or equal to a number.
class Floor: public Monadic {
public:
    MONADIC_PREAMBLE(Floor, "Math", "floor");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(floor(f));
        } else {
            return nullptr;
        }
    }
};


/*
// Math.fround x
// Returns the nearest single precision float representation of a number.
class Fround: public Monadic {
public:
    MONADIC_PREAMBLE(Fround, "Math", "fround");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(fround(f));
        } else {
            return nullptr;
        }
    }
};
*/


// Math.log x
// Returns the natural logarithm (loge, also ln) of a number.
class Log: public Monadic {
public:
    MONADIC_PREAMBLE(Log, "Math", "log");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log(f));
        } else {
            return nullptr;
        }
    }
};


// Math.log1p x
// Returns the natural logarithm (loge, also ln) of 1 + x for a number x.
class Log1p: public Monadic {
public:
    MONADIC_PREAMBLE(Log1p, "Math", "log1p");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log1p(f));
        } else {
            return nullptr;
        }
    }
};


// Math.log10 x
// Returns the base 10 logarithm of a number.
class Log10: public Monadic {
public:
    MONADIC_PREAMBLE(Log10, "Math", "log10");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log10(f));
        } else {
            return nullptr;
        }
    }
};


// Math.log2 x
// Returns the base 2 logarithm of a number.
class Log2: public Monadic {
public:
    MONADIC_PREAMBLE(Log2, "Math", "log2");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log2(f));
        } else {
            return nullptr;
        }
    }
};

// Math.max x y
// Returns the largest of two numbers.
class Max: public Dyadic {
public:
    DYADIC_PREAMBLE(Max, "Math", "max");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float((f0<f1)?f1:f0);
        } else {
            return nullptr;
        }
    }
};


// Math.min x y
// Returns the smallest of two numbers.
class Min: public Dyadic {
public:
    DYADIC_PREAMBLE(Min, "Math", "min");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float((f0<f1)?f0:f1);
        } else {
            return nullptr;
        }
    }
};

// Math.pow x y
// Returns base to the exponent power, that is, baseexponent.
class Pow: public Dyadic {
public:
    DYADIC_PREAMBLE(Pow, "Math", "pow");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(pow(f0, f1));
        } else {
            return nullptr;
        }
    }
};

// Math.random
// Returns a pseudo-random number between 0 and 1.
class Random: public Medadic {
public:
    MEDADIC_PREAMBLE(Random, "Math", "random");

    VMObjectPtr apply() const override {
        return create_float(random());
    }
};

// Math.round x
// Returns the value of a number rounded to the nearest integer.
class Round: public Monadic {
public:
    MONADIC_PREAMBLE(Round, "Math", "round");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_integer(lround(f));
        } else {
            return nullptr;
        }
    }
};

// Math.sign x 
// Returns the sign of the x, indicating whether x is positive, negative or zero.
class Sign: public Monadic {
public:
    MONADIC_PREAMBLE(Sign, "Math", "sign");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            auto b = signbit(f);
            return create_integer((b!=0)? (-1) : (1));
        } else {
            return nullptr;
        }
    }
};


// Math.sin x
// Returns the sine of a number.
class Sin: public Monadic {
public:
    MONADIC_PREAMBLE(Sin, "Math", "sin");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(sin(f));
        } else {
            return nullptr;
        }
    }
};


// Math.sinh x
// Returns the hyperbolic sine of a number.
class Sinh: public Monadic {
public:
    MONADIC_PREAMBLE(Sinh, "Math", "sinh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(sinh(f));
        } else {
            return nullptr;
        }
    }
};


// Math.sqrt x
// Returns the positive square root of a number.
class Sqrt: public Monadic {
public:
    MONADIC_PREAMBLE(Sqrt, "Math", "sqrt");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(sqrt(f));
        } else {
            return nullptr;
        }
    }
};

// Math.tan x
// Returns the tangent of a number.
class Tan: public Monadic {
public:
    MONADIC_PREAMBLE(Tan, "Math", "tan");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(tan(f));
        } else {
            return nullptr;
        }
    }
};

// Math.tanh x
// Returns the hyperbolic tangent of a number.
class Tanh: public Monadic {
public:
    MONADIC_PREAMBLE(Tanh, "Math", "tanh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(tanh(f));
        } else {
            return nullptr;
        }
    }
};

// Math.trunc x
// Returns the integral part of the number x, removing any fractional digits.
class Trunc: public Monadic {
public:
    MONADIC_PREAMBLE(Trunc, "Math", "trunc");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(trunc(f));
        } else {
            return nullptr;
        }
    }
};

extern "C" std::vector<VMObjectPtr> builtin_math(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Euler(vm).clone());
    oo.push_back(Ln2(vm).clone());
    oo.push_back(Ln10(vm).clone());
    oo.push_back(Log2e(vm).clone());
    oo.push_back(Log10e(vm).clone());
    oo.push_back(Pi(vm).clone());
    oo.push_back(Sqrt1_2(vm).clone());
    oo.push_back(Sqrt2(vm).clone());
    oo.push_back(Abs(vm).clone());
    oo.push_back(Acos(vm).clone());
    oo.push_back(Acosh(vm).clone());
    oo.push_back(Asin(vm).clone());
    oo.push_back(Asinh(vm).clone());
    oo.push_back(Atan(vm).clone());
    oo.push_back(Atanh(vm).clone());
    oo.push_back(Atan2(vm).clone());
    oo.push_back(Cbrt(vm).clone());
    oo.push_back(Ceil(vm).clone());
    oo.push_back(Cos(vm).clone());
    oo.push_back(Cosh(vm).clone());
    oo.push_back(Exp(vm).clone());
    oo.push_back(Expm1(vm).clone());
    oo.push_back(Floor(vm).clone());
//    oo.push_back(Fround(vm).clone());
    oo.push_back(Log(vm).clone());
    oo.push_back(Log1p(vm).clone());
    oo.push_back(Log10(vm).clone());
    oo.push_back(Log2(vm).clone());
    oo.push_back(Max(vm).clone());
    oo.push_back(Min(vm).clone());
    oo.push_back(Pow(vm).clone());
    oo.push_back(Random(vm).clone());
    oo.push_back(Round(vm).clone());
    oo.push_back(Sign(vm).clone());
    oo.push_back(Sin(vm).clone());
    oo.push_back(Sinh(vm).clone());
    oo.push_back(Sqrt(vm).clone());
    oo.push_back(Tan(vm).clone());
    oo.push_back(Tanh(vm).clone());
    oo.push_back(Trunc(vm).clone());

    return oo;
}
