#include "social_bt_nodes/bt_nodes/interaction/speak_enum_action.hpp"
#include <sstream>
#include <algorithm>

namespace social_bt_nodes
{

SpeakEnumAction::SpeakEnumAction(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf),
  waiting_for_speech_completion_(false)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("SpeakEnumAction: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus SpeakEnumAction::onStart()
{
  // Get input parameters
  std::string text, separator, language;
  
  if (!getInput("text", text)) {
    RCLCPP_ERROR(node_->get_logger(), "SpeakEnumAction: missing required input 'text'");
    return BT::NodeStatus::FAILURE;
  }
  
  if (!getInput("separator", separator)) {
    separator = ",";
  }
  
  if (!getInput("language", language)) {
    language = "es";
  }
  
  if (!getInput("service_name", service_name_)) {
    service_name_ = "/tts_service";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 5000;
  }
  
  // Split the text into items
  auto items = split_string(text, separator);
  
  if (items.empty()) {
    RCLCPP_ERROR(node_->get_logger(), "SpeakEnumAction: no items found in text");
    return BT::NodeStatus::FAILURE;
  }
  
  // Build the enumerated text
  enumerated_text_ = build_enumerated_text(items, language);
  
  RCLCPP_INFO(node_->get_logger(), 
    "SpeakEnumAction: Enumerated text: '%s'", enumerated_text_.c_str());
  
  // Create service client if not already created or if service name changed
  if (!client_ || client_->get_service_name() != service_name_) {
    client_ = node_->create_client<simple_hri_interfaces::srv::Speech>(service_name_);
  }
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(1000))) {
    RCLCPP_WARN(node_->get_logger(), 
      "SpeakEnumAction: Service '%s' not available yet", service_name_.c_str());
    return BT::NodeStatus::FAILURE;
  }
  
  // Prepare and send request
  auto request = std::make_shared<simple_hri_interfaces::srv::Speech::Request>();
  request->text = enumerated_text_;
  
  future_result_ = std::make_shared<
    rclcpp::Client<simple_hri_interfaces::srv::Speech>::FutureAndRequestId>(
    client_->async_send_request(request));
  
  waiting_for_speech_completion_ = false;
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus SpeakEnumAction::onRunning()
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
          "SpeakEnumAction: Speech failed: %s", result->debug.c_str());
        return BT::NodeStatus::FAILURE;
      }
      
      // Service call succeeded, now calculate speech duration
      int text_length = enumerated_text_.length();
      int duration_ms = text_length * 80 + 500;  // Same formula as SpeakAction
      
      speech_duration_ = std::chrono::milliseconds(duration_ms);
      speech_start_time_ = std::chrono::steady_clock::now();
      waiting_for_speech_completion_ = true;
      
      RCLCPP_INFO(node_->get_logger(), 
        "SpeakEnumAction: TTS service responded, waiting %d ms for speech completion", 
        duration_ms);
    }
    
    return BT::NodeStatus::RUNNING;
  }
  
  // Now wait for the calculated speech duration to elapse
  auto elapsed = std::chrono::steady_clock::now() - speech_start_time_;
  
  if (elapsed >= speech_duration_) {
    RCLCPP_INFO(node_->get_logger(), "SpeakEnumAction: Speech completed");
    return BT::NodeStatus::SUCCESS;
  }
  
  return BT::NodeStatus::RUNNING;
}

void SpeakEnumAction::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "SpeakEnumAction: Halted");
  future_result_.reset();
}

std::string SpeakEnumAction::build_enumerated_text(
  const std::vector<std::string> & items,
  const std::string & language)
{
  if (items.empty()) {
    return "";
  }
  
  if (items.size() == 1) {
    return items[0];
  }
  
  // Determine the conjunction word based on language
  std::string conjunction = (language == "en") ? "and" : "y";
  
  std::ostringstream oss;
  
  // Add all items except the last one, separated by commas
  for (size_t i = 0; i < items.size() - 1; ++i) {
    oss << items[i];
    if (i < items.size() - 2) {
      oss << ", ";
    }
  }
  
  // Add conjunction and last item
  oss << " " << conjunction << " " << items.back();
  
  return oss.str();
}

std::vector<std::string> SpeakEnumAction::split_string(
  const std::string & text,
  const std::string & separator)
{
  std::vector<std::string> items;
  size_t start = 0;
  size_t end = text.find(separator);
  
  while (end != std::string::npos) {
    std::string item = trim(text.substr(start, end - start));
    if (!item.empty()) {
      items.push_back(item);
    }
    start = end + separator.length();
    end = text.find(separator, start);
  }
  
  // Add the last item
  std::string last_item = trim(text.substr(start));
  if (!last_item.empty()) {
    items.push_back(last_item);
  }
  
  return items;
}

std::string SpeakEnumAction::trim(const std::string & str)
{
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

}  // namespace social_bt_nodes
