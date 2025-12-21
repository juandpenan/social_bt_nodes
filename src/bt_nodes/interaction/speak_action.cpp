#include "social_bt_nodes/bt_nodes/interaction/speak_action.hpp"

namespace social_bt_nodes
{

SpeakAction::SpeakAction(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf),
  waiting_for_speech_completion_(false)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("SpeakAction: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus SpeakAction::onStart()
{
  // Get input parameters
  if (!getInput("text", text_)) {
    RCLCPP_ERROR(node_->get_logger(), "SpeakAction: missing required input 'text'");
    return BT::NodeStatus::FAILURE;
  }
  
  if (!getInput("service_name", service_name_)) {
    service_name_ = "/tts_service";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 5000;
  }
  
  // Create service client if not already created or if service name changed
  if (!client_ || client_->get_service_name() != service_name_) {
    client_ = node_->create_client<simple_hri_interfaces::srv::Speech>(service_name_);
  }
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(1000))) {
    RCLCPP_WARN(node_->get_logger(), 
      "SpeakAction: Service '%s' not available yet", service_name_.c_str());
    return BT::NodeStatus::FAILURE;
  }
  
  // Prepare and send request
  auto request = std::make_shared<simple_hri_interfaces::srv::Speech::Request>();
  request->text = text_;
  
  RCLCPP_INFO(node_->get_logger(), "SpeakAction: Speaking '%s'", text_.c_str());
  
  future_result_ = std::make_shared<
    rclcpp::Client<simple_hri_interfaces::srv::Speech>::FutureAndRequestId>(
    client_->async_send_request(request));
  
  waiting_for_speech_completion_ = false;
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus SpeakAction::onRunning()
{
  // First, wait for service call to complete
  if (!waiting_for_speech_completion_) {
    if (!future_result_) {
      return BT::NodeStatus::FAILURE;
    }
    
    auto status = future_result_->wait_for(std::chrono::milliseconds(0));
    
    if (status == std::future_status::ready) {
      auto result = future_result_->get();
      
      if (!result->success) {
        RCLCPP_ERROR(node_->get_logger(), 
          "SpeakAction: Speech failed: %s", result->debug.c_str());
        return BT::NodeStatus::FAILURE;
      }
      
      // Service call succeeded, now calculate speech duration
      // Estimate: ~150 words per minute for Spanish speech
      // Average word length: ~5 characters
      // So roughly 750 characters per minute, or 12.5 chars per second
      // Formula: duration (ms) = text_length * 80 (ms per character)
      // This gives approximately 12.5 chars/sec or 150 wpm
      int text_length = text_.length();
      int duration_ms = text_length * 80 + 500;  // +500ms for padding
      
      speech_duration_ = std::chrono::milliseconds(duration_ms);
      speech_start_time_ = std::chrono::steady_clock::now();
      waiting_for_speech_completion_ = true;
      
      RCLCPP_INFO(node_->get_logger(), 
        "SpeakAction: TTS service responded, waiting %d ms for speech completion", 
        duration_ms);
    }
    
    return BT::NodeStatus::RUNNING;
  }
  
  // Now wait for the calculated speech duration to elapse
  auto elapsed = std::chrono::steady_clock::now() - speech_start_time_;
  
  if (elapsed >= speech_duration_) {
    RCLCPP_INFO(node_->get_logger(), "SpeakAction: Speech completed");
    return BT::NodeStatus::SUCCESS;
  }
  
  return BT::NodeStatus::RUNNING;
}

void SpeakAction::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "SpeakAction: Halted");
  future_result_.reset();
}

}  // namespace social_bt_nodes
