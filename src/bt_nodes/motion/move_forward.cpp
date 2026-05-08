#include "social_bt_nodes/bt_nodes/motion/move_forward.hpp"

#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

MoveForward::MoveForward(const std::string & name, const BT::NodeConfig & conf)
: BT::SyncActionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("MoveForward: 'node' not found in blackboard");
  }
  node_ = node_any;

  cmd_vel_pub_ = node_->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
}

BT::NodeStatus MoveForward::tick()
{
  double speed = 0.5;
  getInput("speed", speed);

  if (speed < 0.0) {
    return bt_failure(
      config(), registrationName(),
      "'speed' must be non-negative",
      "bt_config_error");
  }

  geometry_msgs::msg::Twist cmd;
  cmd.linear.x = speed;
  cmd.angular.z = 0.0;
  cmd_vel_pub_->publish(cmd);

  RCLCPP_DEBUG(node_->get_logger(), "MoveForward: published linear x = %.3f", speed);
  return BT::NodeStatus::SUCCESS;
}

}  // namespace social_bt_nodes