#include <stdlib.h>

#include <filesystem>

#include "../../src/runtime.hpp"

namespace fs = std::chrono;
/**
 * Lift of C++ time chrono library
 */

#define TIME_STRING "TIME"

enum class clock_type {
    SYSTEM_CLOCK,
    STEADY_CLOCK,
    HIGH_RESOLUTION_CLOCK,
    UTC_CLOCK,
    TAI_CLOCK,
    GPS_CLOCK,
    FILE_CLOCK
};

/*
 Date/time classes revolve about four abstractions: clocks, time points,
 durations, and calendar times.
*/
class TimePoint: public Opaque {
public:
    OPAQUE_PREAMBLE(TimePoint, TIME_STRING, "time_point");

    TimePoint(const TimePoint& tp)
        : Opaque(tp.machine(), tp.symbol()) {
        _time_point = tp.time_point();
    }

    static VMObjectPtr create(VM* m, const std::chrono::time_point<std::chrono::system_clock>& tp) {
        auto o = TimePoint(m);
        o.set_time_point(tp);
        return o;
    }

    static bool is_time_point(const VMObjectPtr& o) {
        return typeid(*o) == typeid(TimePoint);
    }

    static std::shared_ptr<TimePoint> cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<TimePoint>(o);
    }

    clock_type clock_type() const {
        return tp.index();
    }

    int less_than(const VMObjectPtr& o) const {
        auto tp = (std::static_pointer_cast<TimePoint>(o))->time_point();
        if (clock_type() < tp->clock_type()) {
            return true;
        } else if (clock_type() > tp->clock_type()) {
            return false;
        } else {
            auto tt = clock_type();
            switch (tt) {
            case SYSTEM_CLOCK:
                return time_point_system_clock() < tp->time_point_system_clock();
            break;
            case STEADY_CLOCK:
                return time_point_steady_clock() < tp->time_point_steady_clock();
            break;
            case HIGH_RESOLUTION_CLOCK:
                return time_point_high_resolution_clock() < tp->time_point_high_resolution_clock();
            break;
            case UTC_CLOCK:
                return time_point_utc_clock() < tp->time_point_utc_clock();
            break;
            case TAI_CLOCK:
                return time_point_tai_clock() < tp->time_point_tai_clock();
            break;
            case GPS_CLOCK:
                return time_point_gps_clock() < tp->time_point_gps_clock();
            break;
            case FILE_CLOCK:
                return time_point_file_clock() < tp->time_point_file_clock();
            break;
            }
        }
    
    }

    int compare(const VMObjectPtr& o) override {
        auto tp = (std::static_pointer_cast<TimePoint>(o))->time_point();
        if (less_than(o)) {
            return -1;
        } else if (tp->less_than(this)) {
            return 1;
        } else {
            return 0;
        }
    }

    void set_time_point(const std::chrono::time_point<std::chrono::system_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::system_clock>
    time_point_system_clock() const {
        return std::get<SYSTEM_CLOCK>(_time_point);
    }

    void set_time_point(const std::chrono::time_point<std::chrono::steady_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::steady_clock>
    time_point_steady_clock() const {
        return std::get<STEADY_CLOCK>(_time_point);
    }


    void set_time_point(const std::chrono::time_point<std::chrono::high_resolution_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock>
    time_point_high_resolution_clock() const {
        return std::get<HIGH_RESOLUTION_CLOCK>(_time_point);
    }

    void set_time_point(const std::chrono::time_point<std::chrono::utc_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::utc_clock>
    time_point_utc_clock() const {
        return std::get<UTC_CLOCK>(_time_point);
    }

    void set_time_point(const std::chrono::time_point<std::chrono::tai_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::tai_clock>
    time_point_tai_clock() const {
        return std::get<TAI_CLOCK>(_time_point);
    }

    void set_time_point(const std::chrono::time_point<std::chrono::gps_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::gps_clock>
    time_point_gps_clock() const {
        return std::get<GPS_CLOCK>(_time_point);
    }

    void set_time_point(const std::chrono::time_point<std::chrono::file_clock> tp)
    {
        _time_point = tp;
    }

    std::chrono::time_point<std::chrono::file_clock>
    time_point_file_clock() const {
        return std::get<FILE_CLOCK>(_time_point);
    }

private:
    std::variant <
    std::chrono::time_point<std::chrono::system_clock>, 
    std::chrono::time_point<std::chrono::steady_clock>, 
    std::chrono::time_point<std::chrono::high_resolution_clock>,
    std::chrono::time_point<std::chrono::utc_clock>,
    std::chrono::time_point<std::chrono::tai_clock>,
    std::chrono::time_point<std::chrono::gps_clock>, 
    std::chrono::time_point<std::chrono::file_clock> 
    > _time_point;
};

class Duration: public Opaque {
public:
    OPAQUE_PREAMBLE(Duration, TIME_STRING, "duration");

    Duration(const Duration& d)
        : Opaque(d.machine(), d.symbol()) {
        _duration = d.duration();
    }

    static VMObjectPtr create(VM* m, const std::chrono::duration& d) {
        auto o = Duration(m);
        o.set_duration(d);
        return o;
    }

    static bool is_duration(const VMObjectPtr& o) {
        return typeid(*o) == typeid(Duration);
    }

    static std::shared_ptr<Duration> cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<Duration>(o);
    }

    int compare(const VMObjectPtr& o) override {
        auto d = cast(o)->duration();
        if (duration() < d) {
            return -1;
        } else if (d < duration()) {
            return 1;
        } else {
            return 0;
        }
    }

    void set_duration(const std::chrono::duration d) {
        _duration = d;
    }

    std::chrono::duration duration() const {
        return _duration;
    }
private:
    std::chrono::duration _duration;
};

// ## Time::time_clock - opaque values that represent clocks
class TimeClock: public Opaque {
public:
    OPAQUE_PREAMBLE(TimeClock, TIME_STRING, "time_clock");

    TimeClock(const TimeClock& clock)
        : Opaque(clock.machine(), clock.symbol()) {
        _clock_type = clock.clock_type();
    }

    void set_clock_type(const clock_type& ct) {
        _clock_type = ct;
    }

    clock_type clock_type() const {
        return _clock_type;
    }

    static VMObjectPtr create(const clock_type& tp) const {
        return std::make_shared<ClockValue>(*this);
    }

    static bool is_time_clock(const VMObjectPtr& o) {
        return typeid(*o) == typeid(TimeClock);
    }

    static std::shared_ptr<TimeClock> cast(const VMObjectPtr& o) {
        return std::static_pointer_cast<TimeClock>(o);
    }

    int compare(const VMObjectPtr& o) override {
        auto tp = cast(o)->clock_type();
        if (clock_type() < tp)
            return -1;
        else if (tp < clock_type())
            return 1;
        else
            return 0;
    }

    bool is_steady() {
        switch (_clock_type) {
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

    VMObjectPtr now() {
        switch (_clock_type) {
            case SYSTEM_CLOCK:
                return TimePoint::create(machine(), system_clock::now());
                break;
            case STEADY_CLOCK:
                return TimePoint::create(machine(), steady_clock::now());
                break;
            case HIGH_RESOLUTION_CLOCK:
                return TimePoint::create(machine(), high_resolution_clock::now());
                break;
            case UTC_CLOCK:
                return TimePoint::create(machine(), utc_clock::now());
                break;
            case TAI_CLOCK:
                return TimePoint::create(machine(), tai_clock::now());
                break;
            case GPS_CLOCK:
                return TimePoint::create(machine(), gps_clock::now());
                break;
            case FILE_CLOCK:
                return TimePoint::create(machine(), file_clock::now());
                break;
        }
    }

    sys_time<common_type_t<Duration, seconds>> to_sys(
        const gps_time<Duration>& t) {
    }

    sys_time<common_type_t<Duration, seconds>> to_sys(
        const utc_time<Duration>& t) {
    }

    sys_time<common_type_t<Duration, seconds>> to_sys(
        const tai_time<Duration>& t) {
    }

    gps_time<common_type_t<Duration, seconds>> gps_from_sys(
        const sys_time<Duration>& t) {
    }

    utc_time<common_type_t<Duration, seconds>> utc_from_sys(
        const sys_time<Duration>& t) {
    }

    tai_time<common_type_t<Duration, seconds>> tai_from_sys(
        const sys_time<Duration>& t) {
    }

protected:
    clock_type _clock_type;
};

// ## Time::duration - opaque values which represent time durations
// ## Time::time_points - opaque values which represent time points
// ## Time::tm - opaque values which represent calendar times

// ## OS:empty p - checks whether the path is empty
class Clock : public Monadic {
public:
    MONADIC_PREAMBLE(Clock, OS_STRING, "clock");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            if (s == "system") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(SYSTEM_CLOCK);
                return c;
            } else if (s == "steady") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(STEADY_CLOCK);
                return c;
            } else if (s == "high_resolution") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(HIGH_RESOLUTION_CLOCK);
                return c;
            } else if (s == "utc") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(UTC_CLOCK);
                return c;
            } else if (s == "tai") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(TAI_CLOCK);
                return c;
            } else if (s == "gps") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(GPS_CLOCK);
                return c;
            } else if (s == "file") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(FILE_CLOCK);
                return c;
            } else {
                THROW_BADARGS;
            }
        } else {
            THROW_BADARGS;
        }
    }
};

class Now : public Monadic {
public:
    MONADIC_PREAMBLE(Now, TIME_STRING, "now");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimeClock::is_time_clock(arg0)) {
            auto c = TimeClock::cast(arg0);
            return c->now();
        } else {
            THROW_BADARGS;
        }
    }
};

class IsSteady : public Monadic {
public:
    MONADIC_PREAMBLE(IsSteady, TIME_STRING, "is_steady");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimeClock::is_time_clock(arg0)) {
            auto c = TimeClock::cast(arg0);
            return machine()->create_bool(c-is_steady());
        } else {
            THROW_BADARGS;
        }
    }
};

class Nanoseconds : public Monadic {
public:
    MONADIC_PREAMBLE(Nanoseconds, TIME_STRING, "nanoseconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::nanoseconds(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Milliseconds : public Monadic {
public:
    MONADIC_PREAMBLE(Milliseconds, TIME_STRING, "milliseconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::milliseconds(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Seconds : public Monadic {
public:
    MONADIC_PREAMBLE(Nanoseconds, TIME_STRING, "seconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::seconds(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Minutes : public Monadic {
public:
    MONADIC_PREAMBLE(Minutes, TIME_STRING, "minutes");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::minutes(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Hours : public Monadic {
public:
    MONADIC_PREAMBLE(Hours, TIME_STRING, "hours");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::hours(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Days : public Monadic {
public:
    MONADIC_PREAMBLE(Days, TIME_STRING, "days");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::days(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Weeks : public Monadic {
public:
    MONADIC_PREAMBLE(Weeks, TIME_STRING, "weeks");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::weeks(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Months : public Monadic {
public:
    MONADIC_PREAMBLE(Months, TIME_STRING, "months");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::months(i));
        } else {
            THROW_BADARGS;
        }
    }
};

class Years : public Monadic {
public:
    MONADIC_PREAMBLE(Years, TIME_STRING, "years");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::years(i));
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

    // oo.push_back(Empty::create(vm));

    return oo;
}
