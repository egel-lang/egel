#pragma once

#include <stdlib.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <variant>

#include "runtime.hpp"

/**
 * Lift of C++ time chrono library
 */

namespace egel {

enum class clock_type : int {
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
class Duration : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Duration, STRING_TIME, "duration");
DOCSTRING("Time::duration - a duration");

    Duration(const Duration& d)
        : Opaque(VM_SUB_BUILTIN, d.machine(), d.symbol()) {
        _duration = d.duration();
    }

    static VMObjectPtr create(VM* m,
                              const std::chrono::duration<int, std::milli>& d) {
        auto o = std::make_shared<Duration>(m);
        o->set_duration(d);
        return o;
    }

    static bool is_duration(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(Duration);
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

    void set_duration(const std::chrono::duration<int, std::milli>& d) {
        _duration = d;
    }

    std::chrono::duration<int, std::milli> duration() const {
        return _duration;
    }

    virtual VMObjectPtr op_add(const VMObjectPtr& o) override;

private:
    std::chrono::duration<int, std::milli> _duration;
};

class TimePoint : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, TimePoint, STRING_TIME, "time_point");
DOCSTRING("Time::time_point - a time point");

    TimePoint(const TimePoint& tp)
        : Opaque(VM_SUB_BUILTIN, tp.machine(), tp.symbol()) {
        auto tt = tp.get_clock_type();
        switch (tt) {
            case clock_type::SYSTEM_CLOCK:
                set_time_point_system_clock(tp.time_point_system_clock());
                break;
            case clock_type::STEADY_CLOCK:
                set_time_point_steady_clock(tp.time_point_steady_clock());
                break;
            case clock_type::HIGH_RESOLUTION_CLOCK:
                set_time_point_high_resolution_clock(
                    tp.time_point_high_resolution_clock());
                break;
                /*
                        case UTC_CLOCK:
                            set_time_point_utc_clock(tp.time_point_utc_clock());
                        break;
                        case TAI_CLOCK:
                            set_time_point_tai_clock(tp.time_point_tai_clock());
                        break;
                        case GPS_CLOCK:
                            set_time_point_gps_clock(tp.time_point_gps_clock());
                        break;
                        case FILE_CLOCK:
                            set_time_point_file_clock(tp.time_point_file_clock());
                        break;
                */
            default: {
                PANIC("End of case");
            } break;
        }
    }

    static VMObjectPtr create_system(
        VM* m, const std::chrono::time_point<std::chrono::system_clock>& tp) {
        auto o = std::make_shared<TimePoint>(m);
        o->set_time_point_system_clock(tp);
        return o;
    }

    static VMObjectPtr create_steady(
        VM* m, const std::chrono::time_point<std::chrono::steady_clock>& tp) {
        auto o = std::make_shared<TimePoint>(m);
        o->set_time_point_steady_clock(tp);
        return o;
    }

    static VMObjectPtr create_high_resolution(
        VM* m,
        const std::chrono::time_point<std::chrono::high_resolution_clock>& tp) {
        auto o = std::make_shared<TimePoint>(m);
        o->set_time_point_high_resolution_clock(tp);
        return o;
    }

    static bool is_time_point(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(TimePoint);
    }

    clock_type get_clock_type() const {
        return (clock_type)_time_point.index();
    }

    int less_than(const VMObjectPtr& o) const {
        auto tp = TimePoint::cast(o);
        if (get_clock_type() < tp->get_clock_type()) {
            return true;
        } else if (get_clock_type() > tp->get_clock_type()) {
            return false;
        } else {
            auto tt = get_clock_type();
            switch (tt) {
                case clock_type::SYSTEM_CLOCK:
                    return time_point_system_clock() <
                           tp->time_point_system_clock();
                    break;
                case clock_type::STEADY_CLOCK:
                    return time_point_steady_clock() <
                           tp->time_point_steady_clock();
                    break;
                case clock_type::HIGH_RESOLUTION_CLOCK:
                    return time_point_high_resolution_clock() <
                           tp->time_point_high_resolution_clock();
                    break;
                    /*
                                case UTC_CLOCK:
                                    return time_point_utc_clock() <
                       tp->time_point_utc_clock(); break; case TAI_CLOCK: return
                       time_point_tai_clock() < tp->time_point_tai_clock();
                                break;
                                case GPS_CLOCK:
                                    return time_point_gps_clock() <
                       tp->time_point_gps_clock(); break; case FILE_CLOCK:
                                    return time_point_file_clock() <
                       tp->time_point_file_clock(); break;
                    */
                default: {
                    PANIC("End of case");
                    return 0;
                } break;
            }
        }
    }

    int compare(const VMObjectPtr& o) override {
        auto tp = TimePoint::cast(o);
        if (get_clock_type() < tp->get_clock_type()) {
            return true;
        } else if (get_clock_type() > tp->get_clock_type()) {
            return false;
        } else {
            auto tt = get_clock_type();
            switch (tt) {
                case clock_type::SYSTEM_CLOCK: {
                    auto t0 = time_point_system_clock();
                    auto t1 = tp->time_point_system_clock();
                    if (t0 < t1) {
                        return -1;
                    } else if (t1 < t0) {
                        return 1;
                    } else {
                        return 0;
                    }
                } break;
                case clock_type::STEADY_CLOCK: {
                    auto t0 = time_point_steady_clock();
                    auto t1 = tp->time_point_steady_clock();
                    if (t0 < t1) {
                        return -1;
                    } else if (t1 < t0) {
                        return 1;
                    } else {
                        return 0;
                    }
                } break;
                case clock_type::HIGH_RESOLUTION_CLOCK: {
                    auto t0 = time_point_high_resolution_clock();
                    auto t1 = tp->time_point_high_resolution_clock();
                    if (t0 < t1) {
                        return -1;
                    } else if (t1 < t0) {
                        return 1;
                    } else {
                        return 0;
                    }
                } break;
                    /*
                                case UTC_CLOCK:
                                    return time_point_utc_clock() <
                       tp->time_point_utc_clock(); break; case TAI_CLOCK: return
                       time_point_tai_clock() < tp->time_point_tai_clock();
                                break;
                                case GPS_CLOCK:
                                    return time_point_gps_clock() <
                       tp->time_point_gps_clock(); break; case FILE_CLOCK:
                                    return time_point_file_clock() <
                       tp->time_point_file_clock(); break;
                    */
                default: {
                    PANIC("End of case");
                    return 0;
                } break;
            }
        }
    }

    virtual VMObjectPtr op_add(const VMObjectPtr& o) override {
        if (Duration::is_duration(o)) {
            auto d = Duration::cast(o);
            switch (get_clock_type()) {
                case clock_type::SYSTEM_CLOCK:
                    return TimePoint::create_system(
                        machine(), time_point_system_clock() + d->duration());
                    break;
                case clock_type::STEADY_CLOCK:
                    return TimePoint::create_steady(
                        machine(), time_point_steady_clock() + d->duration());
                    break;
                case clock_type::HIGH_RESOLUTION_CLOCK:
                    return TimePoint::create_high_resolution(
                        machine(),
                        time_point_high_resolution_clock() + d->duration());
                    break;
                    /*
                                case UTC_CLOCK:
                                    return TimePoint::create(machine(),
                       time_point_utc_clock() + d->duration()) break; case
                       TAI_CLOCK: return TimePoint::create(machine(),
                       time_point_tai_clock() + d->duration()) break; case
                       GPS_CLOCK: return TimePoint::create(machine(),
                       time_point_gps_clock() + d->duration()) break; case
                       FILE_CLOCK: return TimePoint::create(machine(),
                       time_point_file_clock() + d->duration()) break;
                    */
                default: {
                    PANIC("End of case");
                    return nullptr;
                } break;
            }
        } else {
            return nullptr;
        }
    }

    virtual VMObjectPtr op_minus(const VMObjectPtr& o) override {
        if (Duration::is_duration(o)) {
            auto d = Duration::cast(o);
            switch (get_clock_type()) {
                case clock_type::SYSTEM_CLOCK:
                    return TimePoint::create_system(
                        machine(), time_point_system_clock() - d->duration());
                    break;
                case clock_type::STEADY_CLOCK:
                    return TimePoint::create_steady(
                        machine(), time_point_steady_clock() - d->duration());
                    break;
                case clock_type::HIGH_RESOLUTION_CLOCK:
                    return TimePoint::create_high_resolution(
                        machine(),
                        time_point_high_resolution_clock() - d->duration());
                    break;
                    /*
                                case UTC_CLOCK:
                                    return TimePoint::create(machine(),
                       time_point_utc_clock() - d->duration()) break; case
                       TAI_CLOCK: return TimePoint::create(machine(),
                       time_point_tai_clock() - d->duration()) break; case
                       GPS_CLOCK: return TimePoint::create(machine(),
                       time_point_gps_clock() - d->duration()) break; case
                       FILE_CLOCK: return TimePoint::create(machine(),
                       time_point_file_clock() - d->duration()) break;
                    */
                default: {
                    PANIC("End of case");
                    return nullptr;
                } break;
            }
        } else if (is_time_point(o)) {
            auto tp = cast(o);
            if (get_clock_type() == tp->get_clock_type()) {
                switch (get_clock_type()) {
                    case clock_type::SYSTEM_CLOCK: {
                        auto d = time_point_system_clock() -
                                 tp->time_point_system_clock();
                        return Duration::create(
                            machine(),
                            std::chrono::duration_cast<
                                std::chrono::duration<int, std::milli>>(d));
                    } break;
                    case clock_type::STEADY_CLOCK: {
                        auto d = time_point_steady_clock() -
                                 tp->time_point_steady_clock();
                        return Duration::create(
                            machine(),
                            std::chrono::duration_cast<
                                std::chrono::duration<int, std::milli>>(d));
                    } break;
                    case clock_type::HIGH_RESOLUTION_CLOCK: {
                        auto d = time_point_high_resolution_clock() -
                                 tp->time_point_high_resolution_clock();
                        return Duration::create(
                            machine(),
                            std::chrono::duration_cast<
                                std::chrono::duration<int, std::milli>>(d));
                    } break;
                        /*
                                        case UTC_CLOCK:
                                            return Duration::create(machine(),
                           time_point_utc_clock() - tp->time_point_utc_clock());
                                        break;
                                        case TAI_CLOCK:
                                            return Duration::create(machine(),
                           time_point_tai_clock() - tp->time_point_tai_clock());
                                        break;
                                        case GPS_CLOCK:
                                            return Duration::create(machine(),
                           time_point_gps_clock() - tp->time_point_gps_clock());
                                        break;
                                        case FILE_CLOCK:
                                            return Duration::create(machine(),
                           time_point_file_clock() -
                           tp->time_point_file_clock()); break;
                        */
                    default: {
                        PANIC("End of case");
                        return nullptr;
                    } break;
                }
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    void set_time_point_system_clock(
        const std::chrono::time_point<std::chrono::system_clock> tp) {
        _time_point.emplace<(int)clock_type::SYSTEM_CLOCK>(tp);
    }

    std::chrono::time_point<std::chrono::system_clock> time_point_system_clock()
        const {
        return std::get<(int)clock_type::SYSTEM_CLOCK>(_time_point);
    }

    void set_time_point_steady_clock(
        const std::chrono::time_point<std::chrono::steady_clock> tp) {
        _time_point.emplace<(int)clock_type::STEADY_CLOCK>(tp);
    }

    std::chrono::time_point<std::chrono::steady_clock> time_point_steady_clock()
        const {
        return std::get<(int)clock_type::STEADY_CLOCK>(_time_point);
    }

    void set_time_point_high_resolution_clock(
        const std::chrono::time_point<std::chrono::high_resolution_clock> tp) {
        _time_point.emplace<(int)clock_type::HIGH_RESOLUTION_CLOCK>(tp);
    }

    std::chrono::time_point<std::chrono::high_resolution_clock>
    time_point_high_resolution_clock() const {
        return std::get<(int)clock_type::HIGH_RESOLUTION_CLOCK>(_time_point);
    }

    /*
        void set_time_point(const
       std::chrono::time_point<std::chrono::utc_clock> tp)
        {
            _time_point = tp;
        }

        std::chrono::time_point<std::chrono::utc_clock>
        time_point_utc_clock() const {
            return std::get<UTC_CLOCK>(_time_point);
        }

        void set_time_point(const
       std::chrono::time_point<std::chrono::tai_clock> tp)
        {
            _time_point = tp;
        }

        std::chrono::time_point<std::chrono::tai_clock>
        time_point_tai_clock() const {
            return std::get<TAI_CLOCK>(_time_point);
        }

        void set_time_point(const
       std::chrono::time_point<std::chrono::gps_clock> tp)
        {
            _time_point = tp;
        }

        std::chrono::time_point<std::chrono::gps_clock>
        time_point_gps_clock() const {
            return std::get<GPS_CLOCK>(_time_point);
        }

        void set_time_point(const
       std::chrono::time_point<std::chrono::file_clock> tp)
        {
            _time_point = tp;
        }

        std::chrono::time_point<std::chrono::file_clock>
        time_point_file_clock() const {
            return std::get<FILE_CLOCK>(_time_point);
        }
    */

private:
    std::variant<std::chrono::time_point<std::chrono::system_clock>,
                 std::chrono::time_point<std::chrono::steady_clock>,
                 std::chrono::time_point<std::chrono::high_resolution_clock>
                 /*
                     std::chrono::time_point<std::chrono::utc_clock>,
                     std::chrono::time_point<std::chrono::tai_clock>,
                     std::chrono::time_point<std::chrono::gps_clock>,
                     std::chrono::time_point<std::chrono::file_clock>
                 */
                 >
        _time_point;
};

VMObjectPtr Duration::op_add(const VMObjectPtr& o) {
    if (Duration::is_duration(o)) {
        auto d = Duration::cast(o);
        return Duration::create(machine(), duration() + d->duration());
    } else if (TimePoint::is_time_point(o)) {
        auto tp = TimePoint::cast(o);
        switch (tp->get_clock_type()) {
            case clock_type::SYSTEM_CLOCK: {
                return TimePoint::create_system(
                    machine(), tp->time_point_system_clock() + duration());
            } break;
            case clock_type::STEADY_CLOCK: {
                return TimePoint::create_steady(
                    machine(), tp->time_point_steady_clock() + duration());
            } break;
            case clock_type::HIGH_RESOLUTION_CLOCK: {
                return TimePoint::create_high_resolution(
                    machine(),
                    tp->time_point_high_resolution_clock() + duration());
            } break;
            default: {
                PANIC("end of case");
                return nullptr;
            } break;
        }
    } else {
        return nullptr;
    }
}

class TimeClock : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, TimeClock, STRING_TIME, "time_clock");
DOCSTRING("Time::time_clock - opaque values that represent clocks");

    TimeClock(const TimeClock& clock)
        : Opaque(VM_SUB_BUILTIN, clock.machine(), clock.symbol()) {
        _clock_type = clock.get_clock_type();
    }

    void set_clock_type(const clock_type& ct) {
        _clock_type = ct;
    }

    clock_type get_clock_type() const {
        return _clock_type;
    }

    static VMObjectPtr create(VM* m, const clock_type& tp) {
        auto tc = std::make_shared<TimeClock>(m);
        tc->set_clock_type(tp);
        return tc;
    }

    static bool is_time_clock(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(TimeClock);
    }

    int compare(const VMObjectPtr& o) override {
        auto tp = cast(o)->get_clock_type();
        if (get_clock_type() < tp)
            return -1;
        else if (tp < get_clock_type())
            return 1;
        else
            return 0;
    }

    bool is_steady() const {
        switch (get_clock_type()) {
            case clock_type::SYSTEM_CLOCK:
                return std::chrono::system_clock::is_steady;
                break;
            case clock_type::STEADY_CLOCK:
                return std::chrono::steady_clock::is_steady;
                break;
            case clock_type::HIGH_RESOLUTION_CLOCK:
                return std::chrono::high_resolution_clock::is_steady;
                break;
                /*
                            case UTC_CLOCK:
                                return utc_clock::is_steady;
                                break;
                            case TAI_CLOCK:
                                return tai_clock::is_steady;
                                break;
                            case GPS_CLOCK:
                                return gps_clock::is_steady;
                                break;
                            default:
                                break;
                */
            default: {
                PANIC("End of case");
                return false;
            } break;
        }
    }

    VMObjectPtr now() {
        switch (get_clock_type()) {
            case clock_type::SYSTEM_CLOCK:
                return TimePoint::create_system(
                    machine(), std::chrono::system_clock::now());
                break;
            case clock_type::STEADY_CLOCK:
                return TimePoint::create_steady(
                    machine(), std::chrono::steady_clock::now());
                break;
            case clock_type::HIGH_RESOLUTION_CLOCK:
                return TimePoint::create_high_resolution(
                    machine(), std::chrono::high_resolution_clock::now());
                break;
                /*
                            case UTC_CLOCK:
                                return TimePoint::create(machine(),
                   utc_clock::now()); break; case TAI_CLOCK: return
                   TimePoint::create(machine(), tai_clock::now()); break; case
                   GPS_CLOCK: return TimePoint::create(machine(),
                   gps_clock::now()); break; case FILE_CLOCK: return
                   TimePoint::create(machine(), file_clock::now()); break;
                */
            default: {
                PANIC("End of case");
                return nullptr;
            } break;
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
    enum clock_type _clock_type;
};

class Date : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_BUILTIN, Date, STRING_TIME, "date");
DOCSTRING("Time::date - a date");

    Date(const Date& d) : Opaque(VM_SUB_BUILTIN, d.machine(), d.symbol()) {
        _date = d.date();
    }

    static VMObjectPtr create(VM* m, const std::tm d) {
        auto o = std::make_shared<Date>(m);
        o->set_date(d);
        return o;
    }

    static bool is_date(const VMObjectPtr& o) {
        auto& r = *o.get();
        return typeid(r) == typeid(Date);
    }

    int compare(const VMObjectPtr& o) override {
        auto tm0 = date();
        auto tm1 = cast(o)->date();
        auto t0 = std::mktime(&tm0);
        auto t1 = std::mktime(&tm1);
        if (t0 < t1) {
            return -1;
        } else if (t1 < t0) {
            return 1;
        } else {
            return 0;
        }
    }

    void set_date(const std::tm& d) {
        _date = d;
    }

    std::tm date() const {
        return _date;
    }

    static VMObjectPtr to_tuple(VM* vm, const VMObjectPtr& o) {
        auto d = cast(o)->date();
        VMObjectPtrs oo;
        oo.push_back(vm->create_integer(d.tm_year + 1900));
        oo.push_back(vm->create_integer(d.tm_mon));
        oo.push_back(vm->create_integer(d.tm_mday));
        oo.push_back(vm->create_integer(d.tm_hour));
        oo.push_back(vm->create_integer(d.tm_min));
        oo.push_back(vm->create_integer(d.tm_sec));
        return vm->create_tuple(oo);
    }

    static bool is_tuple(VM* vm, const VMObjectPtr& o) {
        if (vm->is_array(o) && vm->array_size(o) == 7 &&
            vm->is_tuple(vm->array_get(o, 0))) {
            VMObjectPtrs oo = vm->from_tuple(o);
            for (auto const& o : oo) {
                if (!vm->is_integer(o)) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    static VMObjectPtr from_tuple(VM* vm, const VMObjectPtr& o) {
        VMObjectPtrs oo = vm->from_tuple(o);
        std::tm tm{};
        tm.tm_year = vm->get_integer(oo[0]) - 1900;
        tm.tm_mon = vm->get_integer(oo[1]);
        tm.tm_mday = vm->get_integer(oo[2]);
        tm.tm_hour = vm->get_integer(oo[3]);
        tm.tm_min = vm->get_integer(oo[4]);
        tm.tm_sec = vm->get_integer(oo[5]);
        tm.tm_isdst = -1;  // XXX pray this works
        std::mktime(&tm);  // call to adjust all fields
        return create(vm, tm);
    }

private:
    std::tm _date;
};

class Clock : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Clock, STRING_TIME, "clock");
DOCSTRING("Time::clock s - create a clock object");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_none(arg0)) {
            auto c = TimeClock::create(machine(), clock_type::SYSTEM_CLOCK);
            return c;
        } else if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            if (s == "system") {
                auto c = TimeClock::create(machine(), clock_type::SYSTEM_CLOCK);
                return c;
            } else if (s == "steady") {
                auto c = TimeClock::create(machine(), clock_type::STEADY_CLOCK);
                return c;
            } else if (s == "high_resolution") {
                auto c = TimeClock::create(machine(),
                                           clock_type::HIGH_RESOLUTION_CLOCK);
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
DOCSTRING("Time::now c - current time according to a clock");

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
DOCSTRING("Time::is_steady c - is a steady clock");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimeClock::is_time_clock(arg0)) {
            auto c = TimeClock::cast(arg0);
            return machine()->create_bool(c->is_steady());
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

/*
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
*/

class Milliseconds : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Milliseconds, STRING_TIME, "milliseconds");
DOCSTRING("Time::milliseconds n - a number of milliseconds");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(),
                                    std::chrono::duration<int, std::milli>(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Seconds : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Seconds, STRING_TIME, "seconds");
DOCSTRING("Time::seconds n - a number of seconds");

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
DOCSTRING("Time::minutes n - a number of minutes");

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
DOCSTRING("Time::hours n - a number of hours");

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
DOCSTRING("Time::days n - a number of days");

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
DOCSTRING("Time::weeks n - a number of weeks");

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
DOCSTRING("Time::months n - a number of months");

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
DOCSTRING("Time::years n - a number of years");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_integer(arg0)) {
            auto i = machine()->get_integer(arg0);
            return Duration::create(machine(), std::chrono::years(i));
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Localtime : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, Localtime, STRING_TIME, "local_time");
DOCSTRING("Time::local_time n - time point to local date");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimePoint::is_time_point(arg0)) {
            auto tp = TimePoint::cast(arg0);
            switch (tp->get_clock_type()) {
                case clock_type::SYSTEM_CLOCK: {
                    auto t = tp->time_point_system_clock();
                    std::time_t time_t_val =
                        std::chrono::system_clock::to_time_t(t);
                    std::tm tm_val = *std::localtime(&time_t_val);
                    return Date::create(machine(), tm_val);
                } break;
                default: {
                    throw machine()->bad_args(this, arg0);
                }
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class GMTime : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, GMTime, STRING_TIME, "gm_time");
DOCSTRING("Time::gm_time n - time point to gm date");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (TimePoint::is_time_point(arg0)) {
            auto tp = TimePoint::cast(arg0);
            switch (tp->get_clock_type()) {
                case clock_type::SYSTEM_CLOCK: {
                    auto t = tp->time_point_system_clock();
                    std::time_t time_t_val =
                        std::chrono::system_clock::to_time_t(t);
                    std::tm tm_val = *std::gmtime(&time_t_val);
                    return Date::create(machine(), tm_val);
                } break;
                default: {
                    throw machine()->bad_args(this, arg0);
                }
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

/*
// Time::zonetime n - time point to zone date
class Zonetime : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_BUILTIN, Zonetime, STRING_TIME, "zonetime");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const
override { if (machine()->is_text(arg0) && TimePoint::is_time_point(arg1)) {
            auto s = machine()->get_text(arg0);
            auto time_zone = std::chrono::locate_zone(VM::unicode_to_string(s));
            auto tp = TimePoint::cast(arg1);
            switch (tp->get_clock_type()) {
                case clock_type::SYSTEM_CLOCK: {
                    auto t = tp->time_point_system_clock();
                    //std::time_t time_t_val =
std::chrono::system_clock::to_time_t(t); std::chrono::zoned_time
zoned_time(time_zone, t); auto local_time = zoned_time.get_local_time();
                    std::time_t local_time_t =
std::chrono::local_time<std::chrono::seconds>(local_time).time_since_epoch().count();
                    std::tm tm_val = *std::localtime(&localtime_t);
                    return Date::create(machine(), tm_val);
                }
                break;
                default: {
                    throw machine()->bad_args(this, arg0);
                }
           }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};
*/

class DateToTime : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, DateToTime, STRING_TIME, "date_to_time");
DOCSTRING("Time::date_to_time n - date to time point");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (Date::is_date(arg0)) {
            auto d = Date::cast(arg0);
            std::tm tm = d->date();
            std::time_t time_t_value = std::mktime(&tm);
            auto tp = std::chrono::system_clock::from_time_t(time_t_value);
            return TimePoint::create_system(machine(), tp);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class DateToTuple : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, DateToTuple, STRING_TIME, "date_to_tuple");
DOCSTRING("Time::date_to_tuple n - date to tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (Date::is_date(arg0)) {
            return Date::to_tuple(machine(), arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class DateFromTuple : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_BUILTIN, DateFromTuple, STRING_TIME,
                     "date_from_tuple");
DOCSTRING("Time::date_from_tuple n - date from tuple");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (Date::is_tuple(machine(), arg0)) {
            return Date::from_tuple(machine(), arg0);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class TimeModule: CModule {
    icu::UnicodeString name() const override {
        return "time";
    }

    icu::UnicodeString docstring() const override {
        return "The 'time' module defines time and date combinators.";
    }

    std::vector<VMObjectPtr> exports(VM *vm) override {
        std::vector<VMObjectPtr> oo;

        oo.push_back(Duration::create(vm));
        oo.push_back(TimePoint::create(vm));
        oo.push_back(TimeClock::create(vm));
        oo.push_back(Clock::create(vm));
        oo.push_back(Now::create(vm));
        oo.push_back(IsSteady::create(vm));
        oo.push_back(Milliseconds::create(vm));
        oo.push_back(Seconds::create(vm));
        oo.push_back(Minutes::create(vm));
        oo.push_back(Hours::create(vm));
        oo.push_back(Days::create(vm));
        oo.push_back(Weeks::create(vm));
        oo.push_back(Months::create(vm));
        oo.push_back(Years::create(vm));
        oo.push_back(Localtime::create(vm));
        oo.push_back(GMTime::create(vm));
        oo.push_back(DateToTime::create(vm));
        oo.push_back(DateToTuple::create(vm));
        oo.push_back(DateFromTuple::create(vm));

        return oo;
    }
};

inline std::vector<VMObjectPtr> builtin_time(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Duration::create(vm));
    oo.push_back(TimePoint::create(vm));
    oo.push_back(TimeClock::create(vm));
    oo.push_back(Clock::create(vm));
    oo.push_back(Now::create(vm));
    oo.push_back(IsSteady::create(vm));
    oo.push_back(Milliseconds::create(vm));
    oo.push_back(Seconds::create(vm));
    oo.push_back(Minutes::create(vm));
    oo.push_back(Hours::create(vm));
    oo.push_back(Days::create(vm));
    oo.push_back(Weeks::create(vm));
    oo.push_back(Months::create(vm));
    oo.push_back(Years::create(vm));
    oo.push_back(Localtime::create(vm));
    oo.push_back(GMTime::create(vm));
    oo.push_back(DateToTime::create(vm));
    oo.push_back(DateToTuple::create(vm));
    oo.push_back(DateFromTuple::create(vm));

    return oo;
}

}  // namespace egel
