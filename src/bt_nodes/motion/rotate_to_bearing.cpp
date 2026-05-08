#include "social_bt_nodes/bt_nodes/motion/rotate_to_bearing.hpp"

#include <algorithm>
#include <cctype>

#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

RotateToBearing::RotateToBearing(const std::string & name, const BT::NodeConfig & conf)
: BT::SyncActionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("RotateToBearing: 'node' not found in blackboard");
  }
  node_ = node_any;

  cmd_vel_pub_ = node_->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
}

BT::NodeStatus RotateToBearing::tick()
{
  std::string bearing;
  if (!getInput("bearing", bearing)) {
    return bt_failure(
      config(), registrationName(),
      "missing required input 'bearing'",
      "bt_config_error");
  }

  double angular_speed = 0.5;
  getInput("angular_speed", angular_speed);
  if (angular_speed <= 0.0) {
    return bt_failure(
      config(), registrationName(),
      "'angular_speed' must be greater than zero",
      "bt_config_error");
  }

  std::string normalized = bearing;
  std::transform(
    normalized.begin(), normalized.end(), normalized.begin(),
    [](unsigned char c) {return static_cast<char>(std::tolower(c));});

  if (normalized != "left" && normalized != "right") {
    return bt_failure(
      config(), registrationName(),
      "invalid input 'bearing' (expected 'left' or 'right')",
      "bt_config_error");
  }

  geometry_msgs::msg::Twist cmd;
  cmd.angular.z = normalized == "left" ? angular_speed : -angular_speed;
  cmd_vel_pub_->publish(cmd);

  RCLCPP_DEBUG(
    node_->get_logger(),
    "RotateToBearing: bearing=\"%s\", published angular z=%.3f",
    normalized.c_str(),
    cmd.angular.z);

  return BT::NodeStatus::SUCCESS;
}

}  // namespace social_bt_nodes