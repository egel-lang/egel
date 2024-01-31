#pragma once

#include <stdlib.h>
#include <filesystem>
#include <chrono>

#include "runtime.hpp"

/**
 * Lift of C++ time chrono library
 */

#define STRING_TIME "TIME"

namespace egel {

enum class clock_type {
    SYSTEM_CLOCK,
    STEADY_CLOCK,
    HIGH_RESOLUTION_CLOCK,
/* not supported in clang yet
    UTC_CLOCK,
    TAI_CLOCK,
    GPS_CLOCK,
    FILE_CLOCK
*/
};

/*
 Date/time classes revolve about four abstractions: clocks, time points,
 durations, and calendar times.
*/
class Duration: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Duration, STRING_TIME, "duration");

    Duration(const Duration& d)
        : Opaque(VM_SUB_BUILTIN, d.machine(), d.symbol()) {
        _duration = d.duration();
    }

    static VMObjectPtr create(VM* m, const std::chrono::duration<std::milli>& d) {
        auto o = std::make_shared<Duration>(m);
        o->set_duration(d);
        return o;
    }

    static bool is_duration(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(Duration);
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

    void set_duration(const std::chrono::duration<std::milli> &d) {
        _duration = d;
    }

    std::chrono::duration<std::milli> duration() const {
        return _duration;
    }

    virtual VMObjectPtr op_add(const VMObjectPtr& o) override {
        if (Duration::is_duration(o)) {
            auto d = Duration::cast(o);
            return Duration::create(machine(), duration() + d->duration());
        } else {
            return nullptr;
        }
    }

private:
    std::chrono::duration<std::milli> _duration;
};

class TimePoint: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, TimePoint, STRING_TIME, "time_point");

    TimePoint(const TimePoint& tp)
        : Opaque(VM_SUB_BUILTIN, tp.machine(), tp.symbol()) {
        _time_point = tp.time_point();
    }

    static VMObjectPtr create(VM* m, const std::chrono::time_point<std::chrono::system_clock>& tp) {
        auto o = TimePoint(m);
        o.set_time_point(tp);
        return o;
    }

    static bool is_time_point(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(TimePoint);
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
/*
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
*/
        }
    
    }

    int compare(const VMObjectPtr& o) override {
        auto tp = cast(o)->time_point();
        if (less_than(o)) {
            return -1;
        } else if (tp->less_than(this)) {
            return 1;
        } else {
            return 0;
        }
    }

    virtual VMObjectPtr op_add(const VMObjectPtr& o) override {
        if (Duration::is_duration(o)) {
            auto d = Duration::cast(o);
            switch (clock_type()) {
            case SYSTEM_CLOCK:
                return TimePoint::create(machine(), time_point_system_clock() + d->duration())
            break;
            case STEADY_CLOCK:
                return TimePoint::create(machine(), time_point_steady_clock() + d->duration())
            break;
            case HIGH_RESOLUTION_CLOCK:
                return TimePoint::create(machine(), time_point_high_resolution_clock() + d->duration())
            break;
/*
            case UTC_CLOCK:
                return TimePoint::create(machine(), time_point_utc_clock() + d->duration())
            break;
            case TAI_CLOCK:
                return TimePoint::create(machine(), time_point_tai_clock() + d->duration())
            break;
            case GPS_CLOCK:
                return TimePoint::create(machine(), time_point_gps_clock() + d->duration())
            break;
            case FILE_CLOCK:
                return TimePoint::create(machine(), time_point_file_clock() + d->duration())
            break;
*/
            }
        } else {
            return nullptr;
        }
    }

    virtual VMObjectPtr op_minus(const VMObjectPtr& o) override {
        if (Duration::is_duration(o)) {
            auto d = Duration::cast(o);
            switch (clock_type()) {
            case SYSTEM_CLOCK:
                return TimePoint::create(machine(), time_point_system_clock() - d->duration())
            break;
            case STEADY_CLOCK:
                return TimePoint::create(machine(), time_point_steady_clock() - d->duration())
            break;
            case HIGH_RESOLUTION_CLOCK:
                return TimePoint::create(machine(), time_point_high_resolution_clock() - d->duration())
            break;
/*
            case UTC_CLOCK:
                return TimePoint::create(machine(), time_point_utc_clock() - d->duration())
            break;
            case TAI_CLOCK:
                return TimePoint::create(machine(), time_point_tai_clock() - d->duration())
            break;
            case GPS_CLOCK:
                return TimePoint::create(machine(), time_point_gps_clock() - d->duration())
            break;
            case FILE_CLOCK:
                return TimePoint::create(machine(), time_point_file_clock() - d->duration())
            break;
*/
            }
        } else if (is_time_point(o) {
            auto tp = cast(o);
            if (clock_type() == tp->clock_type()) {
                switch (clock_type()) {
                case SYSTEM_CLOCK:
                    return Duration::create(machine(), time_point_system_clock() - tp->time_point_system_clock());
                break;
                case STEADY_CLOCK:
                    return Duration::create(machine(), time_point_steady_clock() - tp->time_point_steady_clock());
                break;
                case HIGH_RESOLUTION_CLOCK:
                    return Duration::create(machine(), time_point_high_resolution_clock() - tp->time_point_high_resolution_clock());
                break;
/*
                case UTC_CLOCK:
                    return Duration::create(machine(), time_point_utc_clock() - tp->time_point_utc_clock());
                break;
                case TAI_CLOCK:
                    return Duration::create(machine(), time_point_tai_clock() - tp->time_point_tai_clock());
                break;
                case GPS_CLOCK:
                    return Duration::create(machine(), time_point_gps_clock() - tp->time_point_gps_clock());
                break;
                case FILE_CLOCK:
                    return Duration::create(machine(), time_point_file_clock() - tp->time_point_file_clock());
                break;
*/
                }
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
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

/*
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
*/

private:
    std::variant <
    std::chrono::time_point<std::chrono::system_clock>, 
    std::chrono::time_point<std::chrono::steady_clock>, 
    std::chrono::time_point<std::chrono::high_resolution_clock>,
/*
    std::chrono::time_point<std::chrono::utc_clock>,
    std::chrono::time_point<std::chrono::tai_clock>,
    std::chrono::time_point<std::chrono::gps_clock>, 
    std::chrono::time_point<std::chrono::file_clock> 
*/
    > _time_point;
};

// ## Time::time_clock - opaque values that represent clocks
class TimeClock: public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, TimeClock, STRING_TIME, "time_clock");

    TimeClock(const TimeClock& clock)
        : Opaque(VM_SUB_BUILTIN, clock.machine(), clock.symbol()) {
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
/*
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
*/
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
/*
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
*/
        }
    }

/*
    std::chrono::sys_time<common_type_t<Duration, seconds>> to_sys(
        const gps_time<Duration>& t) {
    }

    std::chrono::sys_time<common_type_t<Duration, seconds>> to_sys(
        const utc_time<Duration>& t) {
    }

    std::chrono::sys_time<common_type_t<Duration, seconds>> to_sys(
        const tai_time<Duration>& t) {
    }

    std::chrono::gps_time<common_type_t<Duration, seconds>> gps_from_sys(
        const sys_time<Duration>& t) {
    }

    std::chrono::utc_time<common_type_t<Duration, seconds>> utc_from_sys(
        const sys_time<Duration>& t) {
    }

    std::chrono::tai_time<common_type_t<Duration, seconds>> tai_from_sys(
        const sys_time<Duration>& t) {
    }
*/

protected:
    clock_type _clock_type;
};

// ## Time::duration - opaque values which represent time durations
// ## Time::time_points - opaque values which represent time points
// ## Time::tm - opaque values which represent calendar times

// ## OS:empty p - checks whether the path is empty
class Clock : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Clock, OS_STRING, "clock");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            if (s == "system") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::SYSTEM_CLOCK);
                return c;
            } else if (s == "steady") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::STEADY_CLOCK);
                return c;
            } else if (s == "high_resolution") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::HIGH_RESOLUTION_CLOCK);
                return c;
/*
            } else if (s == "utc") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::UTC_CLOCK);
                return c;
            } else if (s == "tai") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::TAI_CLOCK);
                return c;
            } else if (s == "gps") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::GPS_CLOCK);
                return c;
            } else if (s == "file") {
                auto c = TimeClock::create(machine());
                c->set_clock_type(clock_type::FILE_CLOCK);
                return c;
*/
            } else {
                throw machine()->bad_args(this, arg0);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Now : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Now, STRING_TIME, "now");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimeClock::is_time_clock(arg0)) {
            auto c = TimeClock::cast(arg0);
            return c->now();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsSteady : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, IsSteady, STRING_TIME, "is_steady");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimeClock::is_time_clock(arg0)) {
            auto c = TimeClock::cast(arg0);
            return machine()->create_bool(c-is_steady());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Nanoseconds : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Nanoseconds, STRING_TIME, "nanoseconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::nanoseconds(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Milliseconds : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Milliseconds, STRING_TIME, "milliseconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::milliseconds(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Seconds : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Nanoseconds, STRING_TIME, "seconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::seconds(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Minutes : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Minutes, STRING_TIME, "minutes");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::minutes(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Hours : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Hours, STRING_TIME, "hours");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::hours(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Days : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Days, STRING_TIME, "days");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::days(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Weeks : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Weeks, STRING_TIME, "weeks");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::weeks(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Months : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Months, STRING_TIME, "months");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::months(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Years : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Years, STRING_TIME, "years");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::years(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

inline std::vector<VMObjectPtr> builtin_time(VM *vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Duration::create(vm));
    oo.push_back(TimePoint::create(vm));
    oo.push_back(TimeClock::create(vm));
    oo.push_back(Clock::create(vm));
    oo.push_back(Now::create(vm));
    oo.push_back(IsSteady::create(vm));
    oo.push_back(Nanoseconds::create(vm));
    oo.push_back(Milliseconds::create(vm));
    oo.push_back(Seconds::create(vm));
    oo.push_back(Minutes::create(vm));
    oo.push_back(Hours::create(vm));
    oo.push_back(Days::create(vm));
    oo.push_back(Weeks::create(vm));
    oo.push_back(Months::create(vm));
    oo.push_back(Years::create(vm));

    return oo;
}

}
