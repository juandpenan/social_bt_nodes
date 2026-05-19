#include "social_bt_nodes/bt_nodes/support/is_true.hpp"

#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

IsTrue::IsTrue(const std::string & name, const BT::NodeConfig & conf)
: BT::ConditionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("IsTrue: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus IsTrue::tick()
{
  bool value = false;
  if (!getInput("value", value)) {
    RCLCPP_ERROR(node_->get_logger(), "IsTrue: missing required input 'value'");
    return bt_failure(
      config(), registrationName(),
      "missing required input 'value'",
      "bt_config_error");
  }

  if (value) {
    return BT::NodeStatus::SUCCESS;
  }
  return bt_failure(config(), registrationName(), "NO_REAL_FAILURE");
}

}  // namespace social_bt_nodes