#ifndef SOCIAL_BT_NODES__BT_NODES__SUPPORT__IS_TRUE_HPP_
#define SOCIAL_BT_NODES__BT_NODES__SUPPORT__IS_TRUE_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "rclcpp/rclcpp.hpp"

namespace social_bt_nodes
{

/// Condition that checks whether the input boolean is true.
/// Returns SUCCESS when 'value' is true and FAILURE when it is false.
class IsTrue : public BT::ConditionNode
{
public:
  IsTrue(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Condition that returns SUCCESS when the input boolean value is true and FAILURE when it is false.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<bool>("value", "Boolean value to evaluate")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__SUPPORT__IS_TRUE_HPP_