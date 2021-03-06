#include "builtin_math.hpp"

#include <stdlib.h>
#include <math.h>

/**
 * Egel's math built-ins.
 *
 * Unstable and untested.
 *
 * Almost all of these combinators work only on floats. 
 **/

//## namespace Math - builtin mathematical operators

//## Math:is_finite x - test whether this float is finite
class IsFinite: public Monadic {
public:
    MONADIC_PREAMBLE(IsFinite, "Math", "is_finite");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isfinite(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:is_infinite x - test whether this float is infinite
class IsInfinite: public Monadic {
public:
    MONADIC_PREAMBLE(IsInfinite, "Math", "is_infinite");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isinf(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:is_nan x - test whether this float is Not a Number
class IsNan: public Monadic {
public:
    MONADIC_PREAMBLE(IsNan, "Math", "is_nan");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isnan(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:is_normal x - test whether this float is normal
class IsNormal: public Monadic {
public:
    MONADIC_PREAMBLE(IsNormal, "Math", "is_normal");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_bool(isnormal(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:e - Euler's constant and the base of natural logarithms, approximately 2.718
class Euler: public Medadic {
public:
    MEDADIC_PREAMBLE(Euler, "Math", "e");

    VMObjectPtr apply() const override {
        return create_float(M_E);
    }
};

//## Math:ln2 - Natural logarithm of 2, approximately 0.693
class Ln2: public Medadic {
public:
    MEDADIC_PREAMBLE(Ln2, "Math", "ln2");

    VMObjectPtr apply() const override {
        return create_float(M_LN2);
    }
};

//## Math:ln10 - natural logarithm of 10, approximately 2.303
class Ln10: public Medadic {
public:
    MEDADIC_PREAMBLE(Ln10, "Math", "ln10");

    VMObjectPtr apply() const override {
        return create_float(M_LN10);
    }
};

//## Math:log2e - base 2 logarithm of E, approximately 1.443
class Log2e: public Medadic {
public:
    MEDADIC_PREAMBLE(Log2e, "Math", "log2e");

    VMObjectPtr apply() const override {
        return create_float(M_LOG2E);
    }
};

//## Math:log10e - base 10 logarithm of E, approximately 0.434
class Log10e: public Medadic {
public:
    MEDADIC_PREAMBLE(Log10e, "Math", "log10e");

    VMObjectPtr apply() const override {
        return create_float(M_LOG10E);
    }
};

//## Math:pi - ratio of the circumference of a circle to its diameter, approximately 3.14159
class Pi: public Medadic {
public:
    MEDADIC_PREAMBLE(Pi, "Math", "pi");

    VMObjectPtr apply() const override {
        return create_float(M_PI);
    }
};

//## Math:sqrt1_2 - square root of 1/2; equivalently, 1 over the square root of 2, approximately 0.707
class Sqrt1_2: public Medadic {
public:
    MEDADIC_PREAMBLE(Sqrt1_2, "Math", "sqrt1_2");

    VMObjectPtr apply() const override {
        return create_float(M_SQRT1_2);
    }
};

//## Math:sqrt2 - square root of 2, approximately 1.414
class Sqrt2: public Medadic {
public:
    MEDADIC_PREAMBLE(Sqrt2, "Math", "sqrt2");

    VMObjectPtr apply() const override {
        return create_float(M_SQRT2);
    }
};

//## Math:abs x - returns the absolute value of a number
class Abs: public Monadic {
public:
    MONADIC_PREAMBLE(Abs, "Math", "abs");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(abs(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:acos x - returns the arccosine of a number
class Acos: public Monadic {
public:
    MONADIC_PREAMBLE(Acos, "Math", "acos");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(acos(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:acosh x - returns the hyperbolic arccosine of a number
class Acosh: public Monadic {
public:
    MONADIC_PREAMBLE(Acosh, "Math", "acosh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(acosh(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:asin x - returns the arcsine of a number
class Asin: public Monadic {
public:
    MONADIC_PREAMBLE(Asin, "Math", "asin");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(asin(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:asinh x - returns the hyperbolic arcsine of a number
class Asinh: public Monadic {
public:
    MONADIC_PREAMBLE(Asinh, "Math", "asinh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(asinh(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:atan x - returns the arctangent of a number
class Atan: public Monadic {
public:
    MONADIC_PREAMBLE(Atan, "Math", "atan");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(atan(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:atanh x - returns the hyperbolic arctangent of a number
class Atanh: public Monadic {
public:
    MONADIC_PREAMBLE(Atanh, "Math", "atanh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(atanh(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:atan2 y x - returns the arctangent of the quotient of its arguments
class Atan2: public Dyadic {
public:
    DYADIC_PREAMBLE(Atan2, "Math", "atan2");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return create_float(atan2(f0, f1));
        } else {
            BADARGS;
        }
    }
};

//## Math:cbrt x - returns the cube root of a number
class Cbrt: public Monadic {
public:
    MONADIC_PREAMBLE(Cbrt, "Math", "cbrt");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(cbrt(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:ceil x - returns the smallest integer greater than or equal to a number
class Ceil: public Monadic {
public:
    MONADIC_PREAMBLE(Ceil, "Math", "ceil");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(ceil(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:cos x - returns the cosine of a number
class Cos: public Monadic {
public:
    MONADIC_PREAMBLE(Cos, "Math", "cos");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(cos(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:cosh x - returns the hyperbolic cosine of a number
class Cosh: public Monadic {
public:
    MONADIC_PREAMBLE(Cosh, "Math", "cosh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(cosh(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:exp x - Returns Ex, where x is the argument, and E is Euler's constant (2.718…), the base of the natural logarithm
class Exp: public Monadic {
public:
    MONADIC_PREAMBLE(Exp, "Math", "exp");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(exp(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:expm1 x - returns subtracting 1 from exp x
class Expm1: public Monadic {
public:
    MONADIC_PREAMBLE(Expm1, "Math", "expm1");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(expm1(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:floor x - returns the largest integer less than or equal to a number
class Floor: public Monadic {
public:
    MONADIC_PREAMBLE(Floor, "Math", "floor");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(floor(f));
        } else {
            BADARGS;
        }
    }
};


/*
// Math:fround x - returns the nearest single precision float representation of a number
class Fround: public Monadic {
public:
    MONADIC_PREAMBLE(Fround, "Math", "fround");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(fround(f));
        } else {
            BADARGS;
        }
    }
};
*/


//## Math:log x - returns the natural logarithm (loge, also ln) of a number
class Log: public Monadic {
public:
    MONADIC_PREAMBLE(Log, "Math", "log");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:log1p x - returns the natural logarithm (loge, also ln) of 1 + x for a number x
class Log1p: public Monadic {
public:
    MONADIC_PREAMBLE(Log1p, "Math", "log1p");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log1p(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:log10 x - returns the base 10 logarithm of a number
class Log10: public Monadic {
public:
    MONADIC_PREAMBLE(Log10, "Math", "log10");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log10(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:log2 x - returns the base 2 logarithm of a number
class Log2: public Monadic {
public:
    MONADIC_PREAMBLE(Log2, "Math", "log2");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(log2(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:max x y - returns the largest of two numbers
class Max: public Dyadic {
public:
    DYADIC_PREAMBLE(Max, "Math", "max");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return create_float((f0<f1)?f1:f0);
        } else {
            BADARGS;
        }
    }
};


//## Math:min x y - returns the smallest of two numbers
class MMin: public Dyadic {
public:
    DYADIC_PREAMBLE(MMin, "Math", "min");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return create_float((f0<f1)?f0:f1);
        } else {
            BADARGS;
        }
    }
};

//## Math:pow x y - returns base to the exponent power, that is, baseexponent
class Pow: public Dyadic {
public:
    DYADIC_PREAMBLE(Pow, "Math", "pow");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_FLOAT) && (arg1->tag() == VM_OBJECT_FLOAT)) {
            auto f0 = VM_OBJECT_FLOAT_VALUE(arg0);
            auto f1 = VM_OBJECT_FLOAT_VALUE(arg1);
            return create_float(pow(f0, f1));
        } else {
            BADARGS;
        }
    }
};

//## Math:random - returns a pseudo-random number between 0 and 1
class Random: public Medadic {
public:
    MEDADIC_PREAMBLE(Random, "Math", "random");

    VMObjectPtr apply() const override {
        return create_float(random());
    }
};

//## Math:round x - returns the value of a number rounded to the nearest integer
class Round: public Monadic {
public:
    MONADIC_PREAMBLE(Round, "Math", "round");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_integer(lround(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:sign x - returns the sign of the x, indicating whether x is positive, negative or zero
class Sign: public Monadic {
public:
    MONADIC_PREAMBLE(Sign, "Math", "sign");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            auto b = signbit(f);
            return create_integer((b!=0)? (-1) : (1));
        } else {
            BADARGS;
        }
    }
};


//## Math:sin x - returns the sine of a number
class Sin: public Monadic {
public:
    MONADIC_PREAMBLE(Sin, "Math", "sin");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(sin(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:sinh x - returns the hyperbolic sine of a number
class Sinh: public Monadic {
public:
    MONADIC_PREAMBLE(Sinh, "Math", "sinh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(sinh(f));
        } else {
            BADARGS;
        }
    }
};


//## Math:sqrt x - returns the positive square root of a number
class Sqrt: public Monadic {
public:
    MONADIC_PREAMBLE(Sqrt, "Math", "sqrt");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(sqrt(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:tan x - returns the tangent of a number
class Tan: public Monadic {
public:
    MONADIC_PREAMBLE(Tan, "Math", "tan");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(tan(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:tanh x - returns the hyperbolic tangent of a number
class Tanh: public Monadic {
public:
    MONADIC_PREAMBLE(Tanh, "Math", "tanh");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(tanh(f));
        } else {
            BADARGS;
        }
    }
};

//## Math:trunc x - returns the integral part of the number x, removing any fractional digits
class Trunc: public Monadic {
public:
    MONADIC_PREAMBLE(Trunc, "Math", "trunc");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_FLOAT) {
            auto f = VM_OBJECT_FLOAT_VALUE(arg0);
            return create_float(trunc(f));
        } else {
            BADARGS;
        }
    }
};

std::vector<VMObjectPtr> builtin_math(VM* vm) {
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
    oo.push_back(MMin(vm).clone());
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
