#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__SPEAK_REACTIVE_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__SPEAK_REACTIVE_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "simple_hri_interfaces/srv/speech.hpp"

namespace social_bt_nodes
{

class SpeakReactive : public BT::StatefulActionNode
{
public:
  SpeakReactive(
    const std::string & name,
    const BT::NodeConfig & conf);

  SpeakReactive() = delete;

  ~SpeakReactive() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Synthesizes and speaks the specified text using a TTS service, then waits for the estimated utterance duration.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("text", "Text to speak"),
      BT::InputPort<std::string>("service_name", "/tts_service", "TTS service name"),
      BT::InputPort<int>("timeout", 5000, "Service call timeout (ms)")
    };
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<simple_hri_interfaces::srv::Speech>::SharedPtr client_;
  std::shared_ptr<rclcpp::Client<simple_hri_interfaces::srv::Speech>::FutureAndRequestId> future_result_;

  std::string text_;
  std::string service_name_;
  int timeout_ms_;

  std::chrono::steady_clock::time_point speech_start_time_;
  std::chrono::milliseconds speech_duration_;
  bool waiting_for_speech_completion_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__SPEAK_REACTIVE_HPP_