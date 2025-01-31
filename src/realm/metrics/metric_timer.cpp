/*************************************************************************
 *
 * Copyright 2016 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#include <realm/metrics/metric_timer.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace realm;
using namespace realm::metrics;


MetricTimerResult::MetricTimerResult()
    : m_elapsed_seconds(0)
{
}

MetricTimerResult::~MetricTimerResult()
{
}

double MetricTimerResult::get_elapsed_seconds() const
{
    return m_elapsed_seconds;
}

void MetricTimerResult::report_seconds(double time)
{
    m_elapsed_seconds = time;
}


MetricTimer::MetricTimer(std::shared_ptr<MetricTimerResult> destination)
    : m_dest(destination)
{
    reset();
}

MetricTimer::~MetricTimer()
{
    if (m_dest) {
        m_dest->report_seconds(get_elapsed_time());
    }
}

MetricTimer::time_point MetricTimer::get_timer_ticks() const
{
    return clock_type::now();
}

double MetricTimer::calc_elapsed_seconds(MetricTimer::time_point begin, MetricTimer::time_point end) const
{
    std::chrono::duration<double> elapsed = end - begin;
    return elapsed.count();
}

std::string MetricTimer::format(double seconds)
{
    std::ostringstream out;
    format(seconds, out);
    return out.str();
}

namespace {

int64_t round_to_int64(double x)
{
    // Note: this is std::llround() from <cmath> as of c++11,
    // but this will actually use a native implementation on android.
    return llround(x);
}

} // end anonymous namespace

// see also test/util/Timer.cpp
void MetricTimer::format(double seconds_float, std::ostream& out)
{
    int64_t rounded_minutes = round_to_int64(seconds_float / 60);
    if (rounded_minutes > 60) {
        // 1h0m -> inf
        int64_t hours = rounded_minutes / 60;
        int64_t minutes = rounded_minutes % 60;
        out << hours << "h" << minutes << "m";
    }
    else {
        int64_t rounded_seconds = round_to_int64(seconds_float);
        if (rounded_seconds > 60) {
            // 1m0s -> 59m59s
            int64_t minutes = rounded_seconds / 60;
            int64_t seconds = rounded_seconds % 60;
            out << minutes << "m" << seconds << "s";
        }
        else {
            int64_t rounded_centies = round_to_int64(seconds_float * 100);
            if (rounded_centies > 100) {
                // 1s -> 59.99s
                int64_t seconds = rounded_centies / 100;
                int64_t centies = rounded_centies % 100;
                out << seconds;
                if (centies > 0) {
                    out << '.' << std::setw(2) << std::setfill('0') << centies;
                }
                out << 's';
            }
            else {
                int64_t rounded_centi_ms = round_to_int64(seconds_float * 100000);
                if (rounded_centi_ms > 100) {
                    // 0.1ms -> 999.99ms
                    int64_t ms = rounded_centi_ms / 100;
                    int64_t centi_ms = rounded_centi_ms % 100;
                    out << ms;
                    if (centi_ms > 0) {
                        out << '.' << std::setw(2) << std::setfill('0') << centi_ms;
                    }
                    out << "ms";
                }
                else {
                    // 0 -> 999µs
                    int64_t us = round_to_int64(seconds_float * 1000000);
                    out << us << "us";
                }
            }
        }
    }
}
