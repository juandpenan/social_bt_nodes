#include "social_bt_nodes/bt_nodes/perception/is_target_static.hpp"

#include <cmath>

#include "social_bt_nodes/bt_failure.hpp"
#include "tf2/exceptions.h"

namespace social_bt_nodes
{

IsTargetStatic::IsTargetStatic(const std::string & name, const BT::NodeConfig & conf)
: BT::ConditionNode(name, conf),
  has_last_sample_(false),
  last_target_frame_(""),
  last_x_(0.0),
  last_y_(0.0),
  last_z_(0.0),
  last_motion_time_(0, 0, RCL_ROS_TIME)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("IsTargetStatic: 'node' not found in blackboard");
  }
  node_ = node_any;

  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node_->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
}

BT::NodeStatus IsTargetStatic::tick()
{
  std::string target_frame = "target";
  getInput("target_frame", target_frame);

  std::string base_frame = "base_link";
  getInput("base_frame", base_frame);

  double min_static_time_sec = 5.0;
  getInput("min_static_time_sec", min_static_time_sec);

  double position_epsilon = 0.05;
  getInput("position_epsilon", position_epsilon);

  double timeout = 0.5;
  getInput("timeout", timeout);

  if (min_static_time_sec < 0.0) {
    return bt_failure(
      config(), registrationName(),
      "min_static_time_sec must be non-negative",
      "bt_config_error");
  }

  if (position_epsilon < 0.0) {
    return bt_failure(
      config(), registrationName(),
      "position_epsilon must be non-negative",
      "bt_config_error");
  }

  if (timeout <= 0.0) {
    return bt_failure(
      config(), registrationName(),
      "timeout must be greater than zero",
      "bt_config_error");
  }

  try {
    const auto transform = tf_buffer_->lookupTransform(
      base_frame,
      target_frame,
      tf2::TimePointZero,
      tf2::durationFromSec(timeout));

    const auto now = node_->get_clock()->now();
    const auto tf_time = rclcpp::Time(transform.header.stamp);
    const auto age = (now - tf_time).seconds();
    if (age > timeout) {
      return bt_failure(
        config(), registrationName(),
        "target transform is stale",
        "bt_tf_stale");
    }

    const double x = transform.transform.translation.x;
    const double y = transform.transform.translation.y;
    const double z = transform.transform.translation.z;

    if (!has_last_sample_ || target_frame != last_target_frame_) {
      has_last_sample_ = true;
      last_target_frame_ = target_frame;
      last_x_ = x;
      last_y_ = y;
      last_z_ = z;
      last_motion_time_ = now;
      return bt_failure(config(), registrationName(), "NO_REAL_FAILURE");
    }

    const double dx = x - last_x_;
    const double dy = y - last_y_;
    const double dz = z - last_z_;
    const double displacement = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (displacement > position_epsilon) {
      last_motion_time_ = now;
    }

    last_x_ = x;
    last_y_ = y;
    last_z_ = z;

    const double static_time = (now - last_motion_time_).seconds();
    if (static_time >= min_static_time_sec) {
      return BT::NodeStatus::SUCCESS;
    }
    return bt_failure(config(), registrationName(), "NO_REAL_FAILURE");
  } catch (const tf2::TransformException & ex) {
    return bt_failure(
      config(), registrationName(),
      "failed TF lookup: " + std::string(ex.what()),
      "bt_tf_lookup_failed");
  }
}

}  // namespace social_bt_nodes
