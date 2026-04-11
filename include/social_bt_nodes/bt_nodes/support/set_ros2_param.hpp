#ifndef SOCIAL_BT_NODES__BT_NODES__SUPPORT__SET_ROS2_PARAM_HPP_
#define SOCIAL_BT_NODES__BT_NODES__SUPPORT__SET_ROS2_PARAM_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/srv/set_parameters.hpp"
#include "rcl_interfaces/msg/parameter.hpp"
#include "rcl_interfaces/msg/parameter_value.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree action node to set a ROS2 parameter on any node
 * 
 * This node uses ROS2's parameter service to dynamically change
 * parameters on any running node at runtime.
 * 
 * XML Usage:
 *   <SetRos2Param node_name="/entity_tracker_node" 
 *                 param_name="target_class" 
 *                 param_value="person"
 *                 param_type="string"/>
 * 
 * Ports:
 *   Input:
 *     - node_name (string): Full name of the target node (e.g., "/entity_tracker_node")
 *     - param_name (string): Name of the parameter to set
 *     - param_value (string): Value to set (will be converted based on param_type)
 *     - param_type (string, default: "string"): Type of parameter (string, int, double, bool)
 *     - timeout (int, default: 2000): Service call timeout in ms
 *   Output:
 *     - success (bool): Whether the parameter was set successfully
 */
class SetRos2Param : public BT::StatefulActionNode
{
public:
  SetRos2Param(
    const std::string & name,
    const BT::NodeConfig & conf);

  SetRos2Param() = delete;

  ~SetRos2Param() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Sets a parameter on a ROS 2 node.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("node_name", "Name of the target ROS2 node"),
      BT::InputPort<std::string>("param_name", "Name of the parameter to set"),
      BT::InputPort<std::string>("param_value", "Value to set (as string)"),
      BT::InputPort<std::string>("param_type", "string", "Type: string, int, double, bool"),
      BT::InputPort<int>("timeout", 2000, "Service call timeout (ms)"),
      BT::OutputPort<bool>("success", "Whether the parameter was set successfully")
    };
  }

private:
  rcl_interfaces::msg::ParameterValue createParameterValue(
    const std::string & value_str, const std::string & type);

  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<rcl_interfaces::srv::SetParameters>::SharedPtr client_;
  std::shared_ptr<rclcpp::Client<rcl_interfaces::srv::SetParameters>::FutureAndRequestId> future_result_;
  
  std::string node_name_;
  std::string param_name_;
  std::string param_value_;
  std::string param_type_;
  int timeout_ms_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__SUPPORT__SET_ROS2_PARAM_HPP_
