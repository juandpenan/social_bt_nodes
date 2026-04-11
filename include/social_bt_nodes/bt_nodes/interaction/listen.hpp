#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__LISTEN_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__LISTEN_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/set_bool.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree node that calls the STT service to listen and transcribe speech
 * 
 * This node calls the /stt_service to record audio and transcribe it to text.
 * 
 * XML Usage:
 *   <Listen transcribed_text="{text_output}" service_name="/stt_service" timeout="10000"/>
 * 
 * Ports:
 *   Input:
 *     - service_name (string, default: "/stt_service"): STT service name
 *     - timeout (int, default: 10000): Service call timeout in ms
 *   Output:
 *     - transcribed_text (string): The transcribed text from audio
 */
class Listen : public BT::StatefulActionNode
{
public:
  Listen(
    const std::string & name,
    const BT::NodeConfig & conf);

  Listen() = delete;

  ~Listen() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Listens and transcribes speech to text using an STT service.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("service_name", "/stt_service", "STT service name"),
      BT::InputPort<int>("timeout", 10000, "Service call timeout (ms)"),
      BT::OutputPort<std::string>("transcribed_text", "The transcribed text from audio")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr client_;
  std::shared_ptr<rclcpp::Client<std_srvs::srv::SetBool>::FutureAndRequestId> future_result_;
  
  std::string service_name_;
  int timeout_ms_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__LISTEN_HPP_
