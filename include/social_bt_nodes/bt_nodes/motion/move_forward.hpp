#ifndef SOCIAL_BT_NODES__BT_NODES__MOTION__MOVE_FORWARD_HPP_
#define SOCIAL_BT_NODES__BT_NODES__MOTION__MOVE_FORWARD_HPP_

#include <string>

#include "behaviortree_cpp/action_node.h"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

namespace social_bt_nodes
{

class MoveForward : public BT::SyncActionNode
{
public:
  MoveForward(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Moves the robot forward by publishing a forward velocity command on cmd_vel.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<double>("speed", 0.5, "Forward speed in m/s")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__MOTION__MOVE_FORWARD_HPP_