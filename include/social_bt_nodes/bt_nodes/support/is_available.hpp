#ifndef SOCIAL_BT_NODES__BT_NODES__SUPPORT__IS_AVAILABLE_HPP_
#define SOCIAL_BT_NODES__BT_NODES__SUPPORT__IS_AVAILABLE_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "rclcpp/rclcpp.hpp"

namespace social_bt_nodes
{

class IsAvailable : public BT::ConditionNode
{
public:
  IsAvailable(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Checks whether all requested items are present in the available item list and outputs unavailable items when any are missing.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("available_items", "Semicolon-separated list of available items"),
      BT::InputPort<std::string>("items", "Semicolon-separated list of requested items"),
      BT::OutputPort<std::string>("unavailable_items", "Semicolon-separated list of unavailable items (optional)")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__SUPPORT__IS_AVAILABLE_HPP_