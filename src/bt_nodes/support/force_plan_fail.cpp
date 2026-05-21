#include "social_bt_nodes/bt_nodes/support/force_plan_fail.hpp"

#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

ForcePlanFail::ForcePlanFail(const std::string & name, const BT::NodeConfig & conf)
: BT::ConditionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("ForcePlanFail: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus ForcePlanFail::tick()
{
  std::string reason;
  if (!getInput("fail_message", reason) || reason.empty()) {
    reason = "forced plan restart requested";
  }

  RCLCPP_WARN(node_->get_logger(), "ForcePlanFail: %s", reason.c_str());

  return bt_failure(config(), registrationName(), reason, "FORCED_FAILURE");
}

}  // namespace social_bt_nodes
