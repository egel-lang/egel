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
        auto d = (std::static_pointer_cast<Duration>(o))->duration();
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

// ## Time::clock - opaque values that represent clocks
class Clock: public Opaque {
public:
    OPAQUE_PREAMBLE(Clock, TIME_STRING, "clock");

    ClockValue(const Clock& clock)
        : Opaque(clock.machine(), clock.symbol()) {
        _clock_type = clock.clock_type();
    }

    void set_clock_type(const clock_type& ct) {
        _clock_type = ct;
    }

    clock_type clock_type() {
        return _clock_type;
    }

    static VMObjectPtr create(const clock_type& tp) const {
        return std::make_shared<ClockValue>(*this);
    }

    int compare(const VMObjectPtr& o) override {
        auto v = (std::static_pointer_cast<FlatClock>(o))->value();
        if (_value < v)
            return -1;
        else if (v < _value)
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

    time_point now() {
        switch (_clock_type) {
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
        switch (_clock_type) {
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
        switch (_clock_type) {
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
class NewClock : public Monadic {
public:
    MONADIC_PREAMBLE(NewClock, OS_STRING, "new_clock");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = machine()->get_text(arg0);
            if (s == "utc") {
                Clock c();
            
            } else if (s == "system") {
            } else {
                THROW_BADARGS;
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

    // oo.push_back(Empty::create(vm));

    return oo;
}
