#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__EXTRACT_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__EXTRACT_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "simple_hri_interfaces/srv/extract.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree node that calls the Extract service
 * 
 * This node calls the /extract_service to extract specific information from text.
 * 
 * XML Usage:
 *   <Extract interest="person's name" text="{input_text}" 
 *            extracted_info="{output}" service_name="/extract_service" timeout="10000"/>
 * 
 * Ports:
 *   Input:
 *     - interest (string): What information to extract (e.g., "person's name", "drink preference")
 *     - text (string): The text to analyze
 *     - service_name (string, default: "/extract_service"): Extract service name
 *     - timeout (int, default: 10000): Service call timeout in ms
 *   Output:
 *     - extracted_info (string): The extracted information
 */
class Extract : public BT::StatefulActionNode
{
public:
  Extract(
    const std::string & name,
    const BT::NodeConfig & conf);

  Extract() = delete;

  ~Extract() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("interest", "What information to extract"),
      BT::InputPort<std::string>("text", "The text to analyze"),
      BT::InputPort<std::string>("service_name", "/extract_service", "Extract service name"),
      BT::InputPort<int>("timeout", 10000, "Service call timeout (ms)"),
      BT::OutputPort<std::string>("extracted_info", "The extracted information")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<simple_hri_interfaces::srv::Extract>::SharedPtr client_;
  std::shared_ptr<rclcpp::Client<simple_hri_interfaces::srv::Extract>::FutureAndRequestId> future_result_;
  
  std::string interest_;
  std::string text_;
  std::string service_name_;
  int timeout_ms_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__EXTRACT_HPP_
