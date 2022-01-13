#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <filesystem>

namespace fs = std::chrono;
/**
 * Lift of C++ time chrono library
 */

#define TIME_STRING    "TIME"

typedef std::vector<icu::UnicodeString> UnicodeStrings;

// XXX: move this to the VM once
VMObjectPtr bool_to_object(VM* vm, const bool& b) {
    if (b) {
        return vm->get_data_string("System", "true");
    } else {
        return vm->get_data_string("System", "false");
    }
}

VMObjectPtr int_to_object(const vm_int_t& n) {
    return VMObjectInteger::create(n);
}

VMObjectPtr none(VM* vm) {
    return vm->get_data_string("System", "none");
}

// for some reason C++ doesn't derive clocks from a base class
// because of that, clocks are flattened into one value where
// we (again) follow the pattern of throwing exceptions for
// unsupported functionality

enum class clock_type {
    SYSTEM_CLOCK,
    STEADY_CLOCK,
    HIGH_RESOLUTION_CLOCK,
    UTC_CLOCK,
    TAI_CLOCK,
    GPS_CLOCK
};

/* unnecessary. all methods are static on a clock
typedef std::shared_ptr<system_clock>          system_clock_ptr;
typedef std::shared_ptr<utc_clock>             utc_clock_ptr;
typedef std::shared_ptr<tai_clock>             tai_clock_ptr;
typedef std::shared_ptr<gps_clock>             gps_clock_ptr;
typedef std::shared_ptr<steady_clock>          steady_clock_ptr;
typedef std::shared_ptr<high_resolution_clock> high_resolution_clock_ptr;
*/

/*
 Date/time classes revolve about four abstractions: clocks, time points,
 durations, and calendar times.

 A clock is primarily used to derive a _time point_ 
*/
class FlatClock {
public:

    FlatClock(clock_type tp) {
        _clock_type = tp;
    }

    clock_type clock_type() {
        return _clock_type;
    }

    bool is_steady() {
        switch(_clock_type) {
        case SYSTEM_CLOCK:
            return system_clock::is_steady();
            break;
        case STEADY_CLOCK:
            return steady_clock::is_steady();
            break;
        case HIGH_RESOLUTION_CLOCK:
            return high_resolution_clock::is_steady();
            break;
        case UTC_CLOCK:
            return utc_clock::is_steady();
            break;
        case TAI_CLOCK:
            return tai_clock::is_steady();
            break;
        case GPS_CLOCK:
            return gps_clock::is_steady();
            break;
        default:
            break;
        }
    }

    time_point now() {
        switch(_clock_type) {
        case SYSTEM_CLOCK:
            return system_clock::now();
            break;
        case STEADY_CLOCK:
            return steady_clock::now();
            break;
        case HIGH_RESOLUTION_CLOCK:
            return high_resolution_clock::now();
            break;
        case UTC_CLOCK:
            return utc_clock::now();
            break;
        case TAI_CLOCK:
            return tai_clock::now();
            break;
        case GPS_CLOCK:
            return gps_clock::now();
            break;
        default:
            break;
        }
    }
 
    time_t to_time_t(const time_point& t) {
        switch(_clock_type) {
        case SYSTEM_CLOCK:
            return system_clock::to_time_t(t);
            break;
        case STEADY_CLOCK:
            return steady_clock::to_time_t(t);
            break;
        case HIGH_RESOLUTION_CLOCK:
            return high_resolution_clock::to_time_t(t);
            break;
        case UTC_CLOCK:
            return utc_clock::to_time_t(t);
            break;
        case TAI_CLOCK:
            return tai_clock::to_time_t(t);
            break;
        case GPS_CLOCK:
            return gps_clock::to_time_t(t);
            break;
        default:
            break;
        }
    }

    time_point from_time_t(time_t t) {
        switch(_clock_type) {
        case SYSTEM_CLOCK:
            return system_clock::from_time_t(t);
            break;
        case STEADY_CLOCK:
            return steady_clock::from_time_t(t);
            break;
        case HIGH_RESOLUTION_CLOCK:
            return high_resolution_clock::from_time_t(t);
            break;
        case UTC_CLOCK:
            return utc_clock::from_time_t(t);
            break;
        case TAI_CLOCK:
            return tai_clock::from_time_t(t);
            break;
        case GPS_CLOCK:
            return gps_clock::from_time_t(t);
            break;
        default:
            break;
        }
    }

    sys_time<common_type_t<Duration, seconds>>
    to_sys(const gps_time<Duration>& t) {
        
    }

    sys_time<common_type_t<Duration, seconds>>
    to_sys(const utc_time<Duration>& t) {
    }

    sys_time<common_type_t<Duration, seconds>>
    to_sys(const tai_time<Duration>& t) {
    }

    gps_time<common_type_t<Duration, seconds>>
    gps_from_sys(const sys_time<Duration>& t) {
    }

    utc_time<common_type_t<Duration, seconds>>
    utc_from_sys(const sys_time<Duration>& t) {
    }

    tai_time<common_type_t<Duration, seconds>>
    tai_from_sys(const sys_time<Duration>& t) {
    }

protected:
    clock_type _clock_type;
};

//## Time::clock - opaque values which represent clocks
class ClockValue: public Opaque {
public:
    OPAQUE_PREAMBLE(ClockValue, TIME_STRING, "clock");

    ClockValue(const ChannelValue& chan): Opaque(chan.machine(), chan.symbol()) {
        _value = chan.value();
    }

    VMObjectPtr create() const override {
        return VMObjectPtr(new ClockValue(*this));
    }

    int compare(const VMObjectPtr& o) override {
        auto v = (std::static_pointer_cast<FlatClock>(o))->value();
        if (_value < v) return -1;
        else if (v < _value) return 1;
        else return 0;
    }

    FlatClock value() const {
        return _value;
    }

protected:
    FlatClock _value;
};

//## Time::duration - opaque values which represent time durations
//## Time::time_points - opaque values which represent time points
//## Time::tm - opaque values which represent calendar times


//## OS:empty p - checks whether the path is empty
class Empty: public Monadic {
public:
    MONADIC_PREAMBLE(Empty, OS_STRING, "empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.empty();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS:absolute p - composes an absolute path
class Absolute: public Monadic {
public:
    MONADIC_PREAMBLE(Absolute, OS_STRING, "absolute");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::absolute(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    //oo.push_back(Empty::create(vm));

    return oo;
}
