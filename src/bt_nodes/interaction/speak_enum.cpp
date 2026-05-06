#include "social_bt_nodes/bt_nodes/interaction/speak_enum.hpp"
#include "social_bt_nodes/bt_failure.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace social_bt_nodes
{

namespace
{

std::string trim_copy(const std::string & value)
{
  std::size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }
  std::size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }
  return value.substr(start, end - start);
}

bool resolve_blackboard_template(
  const std::string & raw,
  const BT::Blackboard::Ptr & blackboard,
  std::string & resolved,
  std::string & error)
{
  resolved.clear();
  bool saw_placeholder = false;
  std::size_t pos = 0;

  while (pos < raw.size()) {
    const std::size_t open = raw.find('{', pos);
    if (open == std::string::npos) {
      resolved += raw.substr(pos);
      break;
    }

    resolved += raw.substr(pos, open - pos);
    const std::size_t close = raw.find('}', open + 1);
    if (close == std::string::npos) {
      error = "unmatched '{' in text template: '" + raw + "'";
      return false;
    }

    const std::string key = trim_copy(raw.substr(open + 1, close - open - 1));
    if (key.empty()) {
      error = "empty blackboard key in text template: '" + raw + "'";
      return false;
    }

    try {
      resolved += blackboard->get<std::string>(key);
    } catch (const std::exception & e) {
      error = "blackboard key '" + key + "' unavailable in text template: " + e.what();
      return false;
    }

    saw_placeholder = true;
    pos = close + 1;
  }

  if (!saw_placeholder) {
    error = "template contains no blackboard placeholders";
    return false;
  }
  return true;
}

bool resolve_blackboard_list_expression(
  const std::string & raw,
  const BT::Blackboard::Ptr & blackboard,
  std::string & resolved,
  std::string & error)
{
  const char sep = (raw.find(';') != std::string::npos) ? ';' : ',';

  std::vector<std::string> tokens;
  std::stringstream ss(raw);
  std::string token;
  while (std::getline(ss, token, sep)) {
    std::string key = trim_copy(token);
    if (key.empty()) {
      continue;
    }
    if (!key.empty() && key.front() == '{') {
      key.erase(key.begin());
    }
    if (!key.empty() && key.back() == '}') {
      key.pop_back();
    }
    key = trim_copy(key);
    if (!key.empty()) {
      tokens.push_back(key);
    }
  }

  if (tokens.empty()) {
    error = "no tokens found in list expression: '" + raw + "'";
    return false;
  }

  resolved.clear();
  for (std::size_t i = 0; i < tokens.size(); ++i) {
    const auto & key = tokens[i];
    try {
      if (i > 0) {
        resolved += std::string(1, sep);
      }
      resolved += blackboard->get<std::string>(key);
    } catch (const std::exception & e) {
      error = "blackboard key '" + key + "' unavailable in list expression: " + e.what();
      return false;
    }
  }

  return !resolved.empty();
}

}  // namespace

SpeakEnum::SpeakEnum(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf),
  waiting_for_speech_completion_(false)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("SpeakEnum: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus SpeakEnum::onStart()
{
  // Get input parameters
  std::string list, separator, language;
  
  if (!getInput("list", list)) {
    // BT.CPP doesn't resolve expressions like "{a},{b}" as a single input.
    // Resolve placeholders manually for this common generation pattern.
    std::string raw_list;
    auto input_it = config().input_ports.find("list");
    if (input_it != config().input_ports.end()) {
      raw_list = input_it->second;
    }

    std::string resolved_list;
    std::string resolve_error;
    if (!raw_list.empty() &&
      (resolve_blackboard_template(raw_list, config().blackboard, resolved_list, resolve_error) ||
      resolve_blackboard_list_expression(raw_list, config().blackboard, resolved_list, resolve_error)) &&
      !resolved_list.empty())
    {
      list = resolved_list;
    } else {
      if (!raw_list.empty()) {
        RCLCPP_WARN(
          node_->get_logger(),
          "SpeakEnum: could not resolve list template '%s': %s",
          raw_list.c_str(), resolve_error.c_str());
      }
      RCLCPP_ERROR(node_->get_logger(), "SpeakEnum: missing required input 'list'");
      return bt_failure(config(), registrationName(), "missing required input 'list'", "bt_config_error");
    }
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
  
  // Split the list into items
  auto items = split_string(list, separator);
  
  if (items.empty()) {
    RCLCPP_ERROR(node_->get_logger(), "SpeakEnum: no items found in list");
    return bt_failure(config(), registrationName(), "no items found in list");
  }
  
  // Build the enumerated text
  enumerated_text_ = build_enumerated_text(items, language);
  
  RCLCPP_INFO(node_->get_logger(), 
    "SpeakEnum: Enumerated list: '%s'", enumerated_text_.c_str());
  
  // Create service client if not already created or if service name changed
  if (!client_ || client_->get_service_name() != service_name_) {
    client_ = node_->create_client<simple_hri_interfaces::srv::Speech>(service_name_);
  }
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(timeout_ms_))) {
    RCLCPP_WARN(node_->get_logger(), 
      "SpeakEnum: Service '%s' not available yet", service_name_.c_str());
    return bt_failure(config(), registrationName(), "service '" + service_name_ + "' not available");
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

BT::NodeStatus SpeakEnum::onRunning()
{
  // First, wait for service call to complete
  if (!waiting_for_speech_completion_) {
    if (!future_result_) {
      return bt_failure(config(), registrationName(), "no pending service future");
    }
    
    auto status = future_result_->wait_for(std::chrono::milliseconds(0));
    
    if (status == std::future_status::ready) {
      auto result = future_result_->get();
      
      if (!result->success) {
        RCLCPP_ERROR(node_->get_logger(), 
          "SpeakEnum: Speech failed: %s", result->debug.c_str());
        return bt_failure(config(), registrationName(), "speech service failed: " + result->debug);
      }
      
      // Service call succeeded, now calculate speech duration
      int text_length = enumerated_text_.length();
      int duration_ms = text_length * 80 + 500;  // Same formula as SpeakAction
      
      speech_duration_ = std::chrono::milliseconds(duration_ms);
      speech_start_time_ = std::chrono::steady_clock::now();
      waiting_for_speech_completion_ = true;
      
      RCLCPP_INFO(node_->get_logger(), 
        "SpeakEnum: TTS service responded, waiting %d ms for speech completion", 
        duration_ms);
    }
    
    return BT::NodeStatus::RUNNING;
  }
  
  // Now wait for the calculated speech duration to elapse
  auto elapsed = std::chrono::steady_clock::now() - speech_start_time_;
  
  if (elapsed >= speech_duration_) {
    RCLCPP_INFO(node_->get_logger(), "SpeakEnum: Speech completed");
    return BT::NodeStatus::SUCCESS;
  }
  
  return BT::NodeStatus::RUNNING;
}

void SpeakEnum::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "SpeakEnum halted");
  future_result_.reset();
}

std::string SpeakEnum::build_enumerated_text(
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

std::vector<std::string> SpeakEnum::split_string(
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

std::string SpeakEnum::trim(const std::string & str)
{
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

}  // namespace social_bt_nodes
