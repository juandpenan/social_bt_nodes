#include "social_bt_nodes/bt_nodes/interaction/yesno.hpp"
#include <chrono>
#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

AskYesNoQuestion::AskYesNoQuestion(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf),
  timeout_ms_(10000),
  phase_(Phase::IDLE),
  waiting_for_tts_playback_completion_(false),
  tts_playback_duration_(std::chrono::milliseconds(0))
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("AskYesNoQuestion: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus AskYesNoQuestion::onStart()
{
  // Preferred input is 'question'. Keep 'text' for backward compatibility.
  if (!getInput("question", question_)) {
    if (!getInput("text", question_)) {
      RCLCPP_ERROR(node_->get_logger(), "AskYesNoQuestion: missing required input 'question'");
      return bt_failure(config(), registrationName(), "missing required input 'question'", "bt_config_error");
    }
    RCLCPP_WARN(node_->get_logger(), "AskYesNoQuestion: input port 'text' is deprecated, use 'question'");
  }

  if (!getInput("tts_service_name", tts_service_name_)) {
    tts_service_name_ = "/tts_service";
  }

  if (!getInput("stt_service_name", stt_service_name_)) {
    stt_service_name_ = "/stt_service";
  }
  
  if (!getInput("service_name", service_name_)) {
    service_name_ = "/yesno_service";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 10000;
  }
  
  // Speak the question first, Ask-style.
  if (!tts_client_ || tts_client_->get_service_name() != tts_service_name_) {
    tts_client_ = node_->create_client<simple_hri_interfaces::srv::Speech>(tts_service_name_);
  }

  if (!tts_client_->wait_for_service(std::chrono::milliseconds(timeout_ms_))) {
    RCLCPP_WARN(node_->get_logger(),
      "AskYesNoQuestion: TTS service '%s' not available yet", tts_service_name_.c_str());
    return bt_failure(config(), registrationName(), "service '" + tts_service_name_ + "' not available");
  }

  auto tts_request = std::make_shared<simple_hri_interfaces::srv::Speech::Request>();
  tts_request->text = question_;

  RCLCPP_INFO(node_->get_logger(), "AskYesNoQuestion: Asking question '%s'", question_.c_str());
  tts_future_result_ = std::make_shared<
    rclcpp::Client<simple_hri_interfaces::srv::Speech>::FutureAndRequestId>(
    tts_client_->async_send_request(tts_request));
  waiting_for_tts_playback_completion_ = false;
  phase_ = Phase::WAITING_TTS;
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus AskYesNoQuestion::onRunning()
{
  if (phase_ == Phase::WAITING_TTS) {
    if (waiting_for_tts_playback_completion_) {
      auto elapsed = std::chrono::steady_clock::now() - tts_playback_start_time_;
      if (elapsed < tts_playback_duration_) {
        return BT::NodeStatus::RUNNING;
      }

      if (!stt_client_ || stt_client_->get_service_name() != stt_service_name_) {
        stt_client_ = node_->create_client<std_srvs::srv::SetBool>(stt_service_name_);
      }

      if (!stt_client_->wait_for_service(std::chrono::milliseconds(timeout_ms_))) {
        RCLCPP_WARN(node_->get_logger(),
          "AskYesNoQuestion: STT service '%s' not available yet", stt_service_name_.c_str());
        return bt_failure(config(), registrationName(), "service '" + stt_service_name_ + "' not available");
      }

      auto stt_request = std::make_shared<std_srvs::srv::SetBool::Request>();
      stt_request->data = true;
      stt_future_result_ = std::make_shared<
        rclcpp::Client<std_srvs::srv::SetBool>::FutureAndRequestId>(
        stt_client_->async_send_request(stt_request));
      waiting_for_tts_playback_completion_ = false;
      tts_future_result_.reset();
      phase_ = Phase::WAITING_STT;
      return BT::NodeStatus::RUNNING;
    }

    if (!tts_future_result_) {
      return bt_failure(config(), registrationName(), "no pending TTS service future");
    }

    auto tts_status = tts_future_result_->wait_for(std::chrono::milliseconds(0));
    if (tts_status != std::future_status::ready) {
      return BT::NodeStatus::RUNNING;
    }

    auto tts_result = tts_future_result_->get();
    if (!tts_result->success) {
      RCLCPP_ERROR(node_->get_logger(), "AskYesNoQuestion: TTS failed: %s", tts_result->debug.c_str());
      return bt_failure(config(), registrationName(), "TTS service failed: " + tts_result->debug);
    }

    int duration_ms = static_cast<int>(question_.length()) * 80 + 500;
    tts_playback_duration_ = std::chrono::milliseconds(duration_ms);
    tts_playback_start_time_ = std::chrono::steady_clock::now();
    waiting_for_tts_playback_completion_ = true;
    return BT::NodeStatus::RUNNING;
  }

  if (phase_ == Phase::WAITING_STT) {
    if (!stt_future_result_) {
      return bt_failure(config(), registrationName(), "no pending STT service future");
    }

    auto stt_status = stt_future_result_->wait_for(std::chrono::milliseconds(0));
    if (stt_status != std::future_status::ready) {
      return BT::NodeStatus::RUNNING;
    }

    auto stt_result = stt_future_result_->get();
    if (!stt_result->success) {
      RCLCPP_ERROR(node_->get_logger(), "AskYesNoQuestion: STT failed: %s", stt_result->message.c_str());
      return bt_failure(config(), registrationName(), "failed to transcribe: " + stt_result->message);
    }

    user_answer_ = stt_result->message;
    RCLCPP_INFO(node_->get_logger(), "AskYesNoQuestion: Captured answer '%s'", user_answer_.c_str());

    if (!client_ || client_->get_service_name() != service_name_) {
      client_ = node_->create_client<simple_hri_interfaces::srv::YesNo>(service_name_);
    }

    if (!client_->wait_for_service(std::chrono::milliseconds(timeout_ms_))) {
      RCLCPP_WARN(node_->get_logger(),
        "AskYesNoQuestion: Service '%s' not available yet", service_name_.c_str());
      return bt_failure(config(), registrationName(), "service '" + service_name_ + "' not available");
    }

    auto request = std::make_shared<simple_hri_interfaces::srv::YesNo::Request>();
    request->text = user_answer_;
    future_result_ = std::make_shared<
      rclcpp::Client<simple_hri_interfaces::srv::YesNo>::FutureAndRequestId>(
      client_->async_send_request(request));
    phase_ = Phase::WAITING_YESNO;
    return BT::NodeStatus::RUNNING;
  }

  if (phase_ == Phase::WAITING_YESNO) {
    if (!future_result_) {
      return bt_failure(config(), registrationName(), "no pending yes/no service future");
    }

    auto status = future_result_->wait_for(std::chrono::milliseconds(0));
    if (status != std::future_status::ready) {
      return BT::NodeStatus::RUNNING;
    }

    auto result = future_result_->get();
    std::string confirmation_result = result->result;
    RCLCPP_INFO(node_->get_logger(),
      "AskYesNoQuestion: Result: '%s'", confirmation_result.c_str());

    setOutput("confirmed", confirmation_result == "YES");
    phase_ = Phase::IDLE;
    
    if (confirmation_result == "YES") {
      return BT::NodeStatus::SUCCESS;
    }

    RCLCPP_INFO(node_->get_logger(),
      "AskYesNoQuestion: User did not confirm (result: '%s')", confirmation_result.c_str());
    return bt_failure(config(), registrationName(), "NO_REAL_FAILURE");
  }

  return bt_failure(config(), registrationName(), "invalid internal state in AskYesNoQuestion");
}

void AskYesNoQuestion::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "AskYesNoQuestion: Halted");
  tts_future_result_.reset();
  stt_future_result_.reset();
  future_result_.reset();
  user_answer_.clear();
  waiting_for_tts_playback_completion_ = false;
  phase_ = Phase::IDLE;
}

}  // namespace social_bt_nodes
