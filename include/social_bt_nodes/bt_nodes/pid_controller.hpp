#ifndef SOCIAL_BT_NODES__BT_NODES__PID_CONTROLLER_HPP_
#define SOCIAL_BT_NODES__BT_NODES__PID_CONTROLLER_HPP_

#include <algorithm>
#include <cmath>

namespace social_bt_nodes
{

/**
 * @brief A generic PID controller for continuous control applications
 * 
 * This class implements a discrete PID controller with anti-windup protection.
 * It can be used for various control applications such as angular velocity control,
 * linear velocity control, or any other continuous control task.
 */
class PIDController
{
public:
  /**
   * @brief Construct a new PID Controller
   * 
   * @param kp Proportional gain
   * @param ki Integral gain
   * @param kd Derivative gain
   * @param output_min Minimum output value (default: -infinity)
   * @param output_max Maximum output value (default: +infinity)
   */
  PIDController(
    double kp,
    double ki,
    double kd,
    double output_min = -std::numeric_limits<double>::infinity(),
    double output_max = std::numeric_limits<double>::infinity())
  : kp_(kp),
    ki_(ki),
    kd_(kd),
    output_min_(output_min),
    output_max_(output_max),
    prev_error_(0.0),
    integral_(0.0)
  {
  }

  /**
   * @brief Reset the controller state (integral and previous error)
   */
  void reset()
  {
    prev_error_ = 0.0;
    integral_ = 0.0;
  }

  /**
   * @brief Compute the control output based on the current error
   * 
   * @param error The current error (setpoint - measurement)
   * @param dt Time step since last update (seconds)
   * @return double The control output
   */
  double compute(double error, double dt)
  {
    if (dt <= 0.0) {
      return 0.0;
    }

    // Integral term with anti-windup
    integral_ += error * dt;
    
    // Anti-windup: clamp integral term to prevent excessive buildup
    double max_integral = 0.0;
    if (std::abs(ki_) > 1e-6) {
      max_integral = std::max(output_max_, -output_min_) / ki_;
      integral_ = std::clamp(integral_, -max_integral, max_integral);
    }

    // Derivative term
    double derivative = (error - prev_error_) / dt;

    // PID output
    double output = kp_ * error + ki_ * integral_ + kd_ * derivative;

    // Clamp output to limits
    output = std::clamp(output, output_min_, output_max_);

    // Update state
    prev_error_ = error;

    return output;
  }

  /**
   * @brief Set the PID gains
   * 
   * @param kp Proportional gain
   * @param ki Integral gain
   * @param kd Derivative gain
   */
  void setGains(double kp, double ki, double kd)
  {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
  }

  /**
   * @brief Set the output limits
   * 
   * @param output_min Minimum output value
   * @param output_max Maximum output value
   */
  void setOutputLimits(double output_min, double output_max)
  {
    output_min_ = output_min;
    output_max_ = output_max;
  }

  /**
   * @brief Get the current gains
   */
  void getGains(double & kp, double & ki, double & kd) const
  {
    kp = kp_;
    ki = ki_;
    kd = kd_;
  }

  /**
   * @brief Get the current integral term value
   */
  double getIntegral() const
  {
    return integral_;
  }

  /**
   * @brief Get the previous error value
   */
  double getPreviousError() const
  {
    return prev_error_;
  }

private:
  double kp_;           // Proportional gain
  double ki_;           // Integral gain
  double kd_;           // Derivative gain
  double output_min_;   // Minimum output limit
  double output_max_;   // Maximum output limit
  double prev_error_;   // Previous error for derivative calculation
  double integral_;     // Accumulated integral term
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__PID_CONTROLLER_HPP_
