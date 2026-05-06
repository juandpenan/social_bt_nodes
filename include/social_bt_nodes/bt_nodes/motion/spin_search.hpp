#ifndef SOCIAL_BT_NODES__BT_NODES__MOTION__SPIN_SEARCH_HPP_
#define SOCIAL_BT_NODES__BT_NODES__MOTION__SPIN_SEARCH_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

namespace social_bt_nodes
{

class SpinSearch : public BT::StatefulActionNode
{
public:
  SpinSearch(
    const std::string & action_name,
    const BT::NodeConfig & conf);

  SpinSearch() = delete;

  ~SpinSearch();

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Makes the robot spin in place to search for a target until the behavior tree halts it.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<double>("angular_speed", 0.5, "Angular speed for spinning (rad/s)"),
      BT::InputPort<std::string>("cmd_vel_topic", "/cmd_vel_muxed", "Command velocity topic")
    };
  }

private:
  void stop_robot();

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  
  double angular_speed_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__MOTION__SPIN_SEARCH_HPP_
