#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__ASK_OPEN_QUESTION_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__ASK_OPEN_QUESTION_HPP_

#include <memory>
#include <string>
#include <chrono>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "simple_hri_interfaces/srv/speech.hpp"
#include "std_srvs/srv/set_bool.hpp"

namespace social_bt_nodes
{

/**
 * @brief Asks an open-ended question and captures the answer in one node.
 *
 * This node performs two chained service calls:
 *  1) TTS service to ask the question.
 *  2) STT service to capture the user answer.
 */
class AskOpenQuestion : public BT::StatefulActionNode
{
public:
  AskOpenQuestion(
    const std::string & name,
    const BT::NodeConfig & conf);

  AskOpenQuestion() = delete;

  ~AskOpenQuestion() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Asks the user an open-ended question and captures the answer in one node.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("question", "Question to ask the user"),
      BT::OutputPort<std::string>("answer", "User answer captured by the node"),
      BT::InputPort<std::string>("tts_service_name", "/tts_service", "TTS service name"),
      BT::InputPort<std::string>("stt_service_name", "/stt_service", "STT service name"),
      BT::InputPort<int>("timeout", 10000, "Service call timeout (ms)")
    };
  }

private:
  enum class Phase
  {
    IDLE,
    WAITING_TTS,
    WAITING_STT
  };

  rclcpp::Node::SharedPtr node_;

  rclcpp::Client<simple_hri_interfaces::srv::Speech>::SharedPtr tts_client_;
  rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr stt_client_;

  std::shared_ptr<rclcpp::Client<simple_hri_interfaces::srv::Speech>::FutureAndRequestId>
  tts_future_result_;
  std::shared_ptr<rclcpp::Client<std_srvs::srv::SetBool>::FutureAndRequestId>
  stt_future_result_;

  std::string question_;
  std::string tts_service_name_;
  std::string stt_service_name_;
  int timeout_ms_;
  Phase phase_;
  bool waiting_for_tts_playback_completion_;
  std::chrono::steady_clock::time_point tts_playback_start_time_;
  std::chrono::milliseconds tts_playback_duration_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__ASK_OPEN_QUESTION_HPP_