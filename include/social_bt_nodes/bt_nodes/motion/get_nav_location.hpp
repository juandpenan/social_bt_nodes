#ifndef SOCIAL_BT_NODES__BT_NODES__MOTION__GET_NAV_LOCATION_HPP_
#define SOCIAL_BT_NODES__BT_NODES__MOTION__GET_NAV_LOCATION_HPP_

#include <string>
#include <unordered_map>
#include <vector>
#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"

namespace social_bt_nodes
{

/// Action node: resolves a location description to a navigation frame using a symbol table.
class GetNavLocation : public BT::SyncActionNode
{
public:
  GetNavLocation(const std::string & name, const BT::NodeConfig & conf);

  BT::NodeStatus tick() override;

  static constexpr const char * node_description =
    "Action that resolves a human-provided location description to a known navigation frame "
    "in the map and writes it to 'location_frame'.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>(
        "location_description",
        "Description of the location to navigate to (required)"),
      BT::OutputPort<std::string>(
        "location_frame",
        "Navigation frame corresponding to the location description"),
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  std::unordered_map<std::string, std::vector<std::string>> symbol_table_;
  void initialize_symbol_table();
};

}  // namespace social_bt_nodes
#endif  // SOCIAL_BT_NODES__BT_NODES__MOTION__GET_NAV_LOCATION_HPP_
