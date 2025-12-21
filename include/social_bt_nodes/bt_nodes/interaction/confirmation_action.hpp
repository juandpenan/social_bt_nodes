#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__CONFIRMATION_ACTION_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__CONFIRMATION_ACTION_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "simple_hri_interfaces/srv/yes_no.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree node that calls the YesNo service for confirmation
 * 
 * This node calls the /yesno_service to detect yes/no responses.
 * 
 * XML Usage:
 *   <Confirmation text="{input_text}" result="{yes_or_no}" 
 *                 service_name="/yesno_service" timeout="10000"/>
 * 
 * Ports:
 *   Input:
 *     - text (string): The text to analyze for yes/no
 *     - service_name (string, default: "/yesno_service"): YesNo service name
 *     - timeout (int, default: 10000): Service call timeout in ms
 *   Output:
 *     - result (string): "YES" or "NO" response
 */
class ConfirmationAction : public BT::StatefulActionNode
{
public:
  ConfirmationAction(
    const std::string & name,
    const BT::NodeConfig & conf);

  ConfirmationAction() = delete;

  ~ConfirmationAction() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("text", "The text to analyze for yes/no"),
      BT::InputPort<std::string>("service_name", "/yesno_service", "YesNo service name"),
      BT::InputPort<int>("timeout", 10000, "Service call timeout (ms)"),
      BT::OutputPort<std::string>("result", "YES or NO response")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<simple_hri_interfaces::srv::YesNo>::SharedPtr client_;
  std::shared_ptr<rclcpp::Client<simple_hri_interfaces::srv::YesNo>::FutureAndRequestId> future_result_;
  
  std::string text_;
  std::string service_name_;
  int timeout_ms_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__CONFIRMATION_ACTION_HPP_
