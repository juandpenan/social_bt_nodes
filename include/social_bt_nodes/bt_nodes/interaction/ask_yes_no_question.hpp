#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__ASK_YES_NO_QUESTION_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__ASK_YES_NO_QUESTION_HPP_

#include <memory>
#include <string>
#include <chrono>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "simple_hri_interfaces/srv/speech.hpp"
#include "simple_hri_interfaces/srv/yes_no.hpp"
#include "std_srvs/srv/set_bool.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree node that asks a yes/no question and calls the YesNo service for confirmation
 * 
 * This node calls the /yesno_service to detect yes/no responses.
 * Returns SUCCESS if the user confirms (YES), FAILURE otherwise.
 * 
 * XML Usage:
 *   <AskYesNoQuestion question="{input_text}"
 *                 service_name="/yesno_service" timeout="10000"/>
 * 
 * Ports:
 *   Input:
 *     - question (string): The prompt to ask and analyze for yes/no
 *     - service_name (string, default: "/yesno_service"): YesNo service name
 *     - timeout (int, default: 10000): Service call timeout in ms
 *   Output:
 *     - confirmed (bool): true if the answer is YES, false if the answer is NO
 * 
 * Returns:
 *   - SUCCESS: User confirmed (result is "YES")
 *   - FAILURE: User did not confirm (result is "NO") or errors occurred
 *   - RUNNING: Waiting for service response
 */
class AskYesNoQuestion : public BT::StatefulActionNode
{
public:
  AskYesNoQuestion(
    const std::string & name,
    const BT::NodeConfig & conf);

  AskYesNoQuestion() = delete;

  ~AskYesNoQuestion() = default;

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Asks a yes/no question via TTS and then checks the user's response with the YesNo service.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("question", "Confirmation prompt to ask the user"),
      BT::OutputPort<bool>("confirmed", "Confirmation result (true or false)"),
      BT::InputPort<std::string>("tts_service_name", "/tts_service", "TTS service name"),
      BT::InputPort<std::string>("stt_service_name", "/stt_service", "STT service name"),
      BT::InputPort<std::string>("service_name", "/yesno_service", "YesNo service name"),
      BT::InputPort<int>("timeout", 10000, "Service call timeout (ms)")
    };
  }

private:
  enum class Phase
  {
    IDLE,
    WAITING_TTS,
    WAITING_STT,
    WAITING_YESNO
  };

  rclcpp::Node::SharedPtr node_;
  rclcpp::Client<simple_hri_interfaces::srv::Speech>::SharedPtr tts_client_;
  rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr stt_client_;
  rclcpp::Client<simple_hri_interfaces::srv::YesNo>::SharedPtr client_;
  std::shared_ptr<rclcpp::Client<simple_hri_interfaces::srv::Speech>::FutureAndRequestId>
  tts_future_result_;
  std::shared_ptr<rclcpp::Client<std_srvs::srv::SetBool>::FutureAndRequestId>
  stt_future_result_;
  std::shared_ptr<rclcpp::Client<simple_hri_interfaces::srv::YesNo>::FutureAndRequestId> future_result_;
  
  std::string question_;
  std::string tts_service_name_;
  std::string stt_service_name_;
  std::string service_name_;
  std::string user_answer_;
  int timeout_ms_;
  Phase phase_;
  bool waiting_for_tts_playback_completion_;
  std::chrono::steady_clock::time_point tts_playback_start_time_;
  std::chrono::milliseconds tts_playback_duration_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__ASK_YES_NO_QUESTION_HPP_
