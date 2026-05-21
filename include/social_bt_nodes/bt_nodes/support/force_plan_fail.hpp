#ifndef SOCIAL_BT_NODES__BT_NODES__SUPPORT__FORCE_PLAN_FAIL_HPP_
#define SOCIAL_BT_NODES__BT_NODES__SUPPORT__FORCE_PLAN_FAIL_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "rclcpp/rclcpp.hpp"

namespace social_bt_nodes
{

/// Condition that always fails and marks the failure as a forced plan restart.
class ForcePlanFail : public BT::ConditionNode
{
public:
  ForcePlanFail(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Always returns FAILURE and writes FORCED_FAILURE to bt_last_failure_code. "
    "Use this node when the current sub-goal is definitively unachievable and "
    "the mission must restart from step 0.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("fail_message", "Human-readable reason for the forced plan restart")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__SUPPORT__FORCE_PLAN_FAIL_HPP_
