#ifndef SOCIAL_BT_NODES__BT_NODES__MOTION__ROTATE_TO_BEARING_HPP_
#define SOCIAL_BT_NODES__BT_NODES__MOTION__ROTATE_TO_BEARING_HPP_

#include <string>

#include "behaviortree_cpp/action_node.h"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

namespace social_bt_nodes
{

class RotateToBearing : public BT::SyncActionNode
{
public:
  RotateToBearing(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Rotates the robot in place toward the requested bearing direction ('left' or 'right') by publishing an angular velocity command.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("bearing", "Target bearing direction ('left' or 'right')"),
      BT::InputPort<double>("angular_speed", 0.5, "Angular speed in rad/s")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__MOTION__ROTATE_TO_BEARING_HPP_