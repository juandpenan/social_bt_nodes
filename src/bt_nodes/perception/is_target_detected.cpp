#include "social_bt_nodes/bt_nodes/perception/is_target_detected.hpp"
#include "social_bt_nodes/bt_failure.hpp"
#include "tf2/exceptions.h"

namespace social_bt_nodes
{

IsTargetDetected::IsTargetDetected(
  const std::string & condition_name,
  const BT::NodeConfig & conf)
: BT::ConditionNode(condition_name, conf)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("IsTargetDetected: 'node' not found in blackboard");
  }
  node_ = node_any;
  
  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node_->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
}

IsTargetDetected::~IsTargetDetected()
{
}

BT::NodeStatus IsTargetDetected::tick()
{
  std::string target_frame;
  std::string base_frame;
  double timeout;

  RCLCPP_DEBUG(node_->get_logger(), "IsTargetDetected ticked");

  if (!getInput("target_frame", target_frame)) {
    RCLCPP_ERROR(node_->get_logger(), "Missing required input [target_frame]");
    return bt_failure(config(), registrationName(), "missing required input 'target_frame'");
  }

  if (!getInput("base_frame", base_frame)) {
    RCLCPP_ERROR(node_->get_logger(), "Missing required input [base_frame]");
    return bt_failure(config(), registrationName(), "missing required input 'base_frame'");
  }

  if (!getInput("timeout", timeout)) {
    timeout = 3.0;
  }

  try {
    // Try to lookup the transform from base to target
    auto transform = tf_buffer_->lookupTransform(
      base_frame,
      target_frame,
      tf2::TimePointZero,
      tf2::durationFromSec(timeout));

    // Check if transform is recent enough (not stale)
    auto now = node_->get_clock()->now();
    auto transform_time = rclcpp::Time(transform.header.stamp);
    auto age = (now - transform_time).seconds();
    
    if (age > timeout) {
      RCLCPP_INFO(node_->get_logger(), 
        "Target transform is stale (age: %.2f s > timeout: %.2f s)", age, timeout);
      return bt_failure(config(), registrationName(), "target transform is stale");
    }

    RCLCPP_DEBUG(node_->get_logger(), "Target detected at (%f, %f, %f)",
      transform.transform.translation.x,
      transform.transform.translation.y,
      transform.transform.translation.z);

      double angle_to_target = atan2(
      transform.transform.translation.y,
      transform.transform.translation.x) * 180.0 / M_PI;
    RCLCPP_DEBUG(node_->get_logger(), "Angle to target: %.2fº degrees", angle_to_target);

    return BT::NodeStatus::SUCCESS;
  } catch (const tf2::TransformException & ex) {
    RCLCPP_INFO(node_->get_logger(), "Target not detected: %s", ex.what());
    return bt_failure(config(), registrationName(), "target not detected: " + std::string(ex.what()));
  }
}

}  // namespace social_bt_nodes
