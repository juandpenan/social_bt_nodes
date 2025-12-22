#ifndef SOCIAL_BT_NODES__BT_NODES__PERCEPTION__SET_PERCEPTION_TARGET_HPP_
#define SOCIAL_BT_NODES__BT_NODES__PERCEPTION__SET_PERCEPTION_TARGET_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "simple_perception_interfaces/srv/set_target_class.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree action node to set the perception target class
 * 
 * This node calls the set_perception_target service to dynamically change
 * the target class being tracked by the perception system.
 * 
 * XML Usage:
 *   <SetPerceptionTarget 
 *     service_name="/set_perception_target" 
 *     target_class="person"/>
 * 
 * Ports:
 *   Input:
 *     - service_name (string, default: "/set_perception_target"): Service name
 *     - target_class (string): Target class to track (e.g., "person", "tv", "bottle")
 *     - timeout (int, default: 2000): Service call timeout in ms
 *   Output:
 *     - success (bool): Whether the service call succeeded
 *     - message (string): Response message from the service
 */
class SetPerceptionTarget : public BT::StatefulActionNode
{
public:
  SetPerceptionTarget(
    const std::string & name,
    const BT::NodeConfig & conf);

  SetPerceptionTarget() = delete;

  ~SetPerceptionTarget() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("service_name", "/set_perception_target", 
        "Name of the perception target service"),
      BT::InputPort<std::string>("target_class", "Target class to track"),
      BT::InputPort<int>("timeout", 2000, "Service call timeout (ms)"),
      BT::OutputPort<bool>("success", "Whether the service call succeeded"),
      BT::OutputPort<std::string>("message", "Response message from service")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<simple_perception_interfaces::srv::SetTargetClass>::SharedPtr client_;
  std::shared_ptr<
    rclcpp::Client<simple_perception_interfaces::srv::SetTargetClass>::FutureAndRequestId
  > future_result_;
  
  std::string service_name_;
  std::string target_class_;
  int timeout_ms_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__PERCEPTION__SET_PERCEPTION_TARGET_HPP_
