#ifndef SOCIAL_BT_NODES__BT_NODES__SUPPORT__STOP_CURRENT_TASK_HPP_
#define SOCIAL_BT_NODES__BT_NODES__SUPPORT__STOP_CURRENT_TASK_HPP_

#include <atomic>
#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/empty.hpp"

namespace social_bt_nodes
{

/// Condition that consumes an external stop event and requests task interruption.
/// Returns SUCCESS exactly once per received event message, then resets to FAILURE
/// until another event arrives.
class StopCurrentTask : public BT::ConditionNode
{
public:
  StopCurrentTask(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Checks if a stop-current-task event has been received. Returns SUCCESS exactly once per event.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("event_topic", "/stop_current_task", "Topic that publishes stop events (std_msgs/Empty)"),
      BT::InputPort<int>("qos_depth", 10, "Subscription QoS depth")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr stop_event_sub_;
  std::atomic_bool stop_event_pending_{false};
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__SUPPORT__STOP_CURRENT_TASK_HPP_
