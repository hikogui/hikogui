


#pragma once

#include "assert.hpp"
#include <chrono>

namespace tt {

/** A PID controller.
 * A proportional-integral-derivative controller (PID).
 * A PID controller continuously calculates an error value e(t)
 * as the difference between a desired setpoint (SP) and a measured process variable (PV)
 * and applies a correction based on proportional, integral, and derivative terms.
 */
class pid_controller {
public:
    /** Construct PID controller.
     * @param Kp The proportinal gain.
     * @param Ki The integral gain.
     * @param Kd The derivative gain.
     */
    constexpr pid_controller(double Kp, double Ki = 0.0, double Kd = 0.0) noexcept :
        _Kp(Kp), _Ki(Ki), _Kd(Kd), _integral(0.0), _prev_error(0.0) {}

    /** Execute iteration of the PID controller.
     *
     * @param error The error value, PV - SP
     * @param dt The duration since the last iteration.
     * @return The control variable
     */
    [[nodiscard]] constexpr double operator()(double error, std::chrono::nanoseconds dt) noexcept
    {
        ttlet dt_ = narrow_cast<double>(dt.count()) * 1'000'000'000.0;
        tt_axiom(dt_ > 0.0);

        _integral = _integral + error * dt_;
        ttlet derivative = (error - _prev_error) * dt_;

        ttlet output = Kp * error + Ki * _integral + Kd * derivative;
        _prev_error = error;
        return output;
    }

    /** Execute iteration of the PID controller.
     *
     * @param process_variable The value received from a sensor.
     * @param set_point The value to match.
     * @param dt The duration since the last iteration.
     * @return The control variable
     */
    [[nodiscard]] constexpr double operator()(double process_variable, double set_point, std::chrono::nanoseconds dt) noexcept
    {
        return (*this)(process_variable - set_point, dt);
    }

private:
    double _Kp;
    double _Ki;
    double _Kd;
    double _integral;
    double _prev_error;
};


}

