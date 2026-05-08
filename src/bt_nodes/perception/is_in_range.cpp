#include "social_bt_nodes/bt_nodes/perception/is_in_range.hpp"

#include <cmath>

#include "social_bt_nodes/bt_failure.hpp"
#include "tf2/exceptions.h"

namespace social_bt_nodes
{

IsInRange::IsInRange(const std::string & name, const BT::NodeConfig & conf)
: BT::ConditionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("IsInRange: 'node' not found in blackboard");
  }
  node_ = node_any;

  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node_->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
}

BT::NodeStatus IsInRange::tick()
{
  std::string target_frame;
  if (!getInput("target_frame", target_frame)) {
    RCLCPP_ERROR(node_->get_logger(), "IsInRange: missing required input 'target_frame'");
    return bt_failure(
      config(), registrationName(),
      "missing required input 'target_frame'",
      "bt_config_error");
  }

  std::string base_frame = "base_link";
  getInput("base_frame", base_frame);

  double distance_threshold = 1.0;
  getInput("distance_threshold", distance_threshold);

  double timeout = 0.5;
  getInput("timeout", timeout);

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

    const double dx = transform.transform.translation.x;
    const double dy = transform.transform.translation.y;
    const double dz = transform.transform.translation.z;
    const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    return distance <= distance_threshold ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  } catch (const tf2::TransformException & ex) {
    return bt_failure(
      config(), registrationName(),
      "failed TF lookup: " + std::string(ex.what()),
      "bt_tf_lookup_failed");
  }
}

}  // namespace social_bt_nodes