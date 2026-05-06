#include "social_bt_nodes/bt_nodes/interaction/nao_set_leds.hpp"
#include "social_bt_nodes/bt_failure.hpp"
#include <sstream>

namespace social_bt_nodes
{

NaoSetLeds::NaoSetLeds(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf),
  goal_accepted_(false),
  goal_completed_(false),
  goal_succeeded_(false)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("NaoSetLeds: 'node' not found in blackboard");
  }
  node_ = node_any;

  // Get action server name from input port
  std::string server_name;
  if (!getInput("action_server", server_name)) {
    server_name = "leds_play";
  }

  // Create action client
  action_client_ = rclcpp_action::create_client<LedsPlay>(
    node_,
    server_name);

  RCLCPP_INFO(node_->get_logger(), 
    "NaoSetLeds: Created action client for '%s'", server_name.c_str());
}

NaoSetLeds::~NaoSetLeds()
{
  if (goal_handle_ && !goal_completed_) {
    RCLCPP_INFO(node_->get_logger(), 
      "NaoSetLeds: Canceling active goal on destruction");
    auto future_cancel = action_client_->async_cancel_goal(goal_handle_);
  }
}

std::vector<uint8_t> NaoSetLeds::parse_led_ids(const std::string & led_ids_str)
{
  std::vector<uint8_t> led_ids;
  std::stringstream ss(led_ids_str);
  std::string item;
  
  while (std::getline(ss, item, ',')) {
    // Trim whitespace
    item.erase(0, item.find_first_not_of(" \t\n\r"));
    item.erase(item.find_last_not_of(" \t\n\r") + 1);
    
    if (!item.empty()) {
      try {
        led_ids.push_back(static_cast<uint8_t>(std::stoi(item)));
      } catch (const std::exception & e) {
        RCLCPP_WARN(node_->get_logger(), 
          "NaoSetLeds: Failed to parse LED ID '%s': %s", item.c_str(), e.what());
      }
    }
  }
  
  return led_ids;
}

BT::NodeStatus NaoSetLeds::onStart()
{
  // Reset state
  goal_accepted_ = false;
  goal_completed_ = false;
  goal_succeeded_ = false;
  goal_handle_.reset();

  // Get timeout
  if (!getInput("timeout", timeout_)) {
    timeout_ = 10.0;
  }

  start_time_ = node_->now();

  // Get LED IDs
  std::string led_ids_str;
  if (!getInput("led_ids", led_ids_str)) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoSetLeds: Missing required input 'led_ids'");
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "missing required input 'led_ids'", "bt_config_error");
  }

  auto led_ids = parse_led_ids(led_ids_str);
  if (led_ids.empty()) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoSetLeds: No valid LED IDs parsed from '%s'", led_ids_str.c_str());
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "no valid LED IDs parsed from '" + led_ids_str + "'");
  }

  // Wait for action server
  if (!action_client_->wait_for_action_server(std::chrono::seconds(2))) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoSetLeds: Action server not available");
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "action server not available");
  }

  // Prepare goal
  auto goal_msg = LedsPlay::Goal();
  
  // Set LED IDs (pad with 0 if only one LED specified)
  goal_msg.leds[0] = led_ids[0];
  goal_msg.leds[1] = (led_ids.size() > 1) ? led_ids[1] : 0;

  // Get mode
  int mode;
  if (!getInput("mode", mode)) {
    mode = 0; // Default to STEADY
  }
  goal_msg.mode = static_cast<uint8_t>(mode);

  // Get color
  double color_r, color_g, color_b;
  if (!getInput("color_r", color_r)) color_r = 1.0;
  if (!getInput("color_g", color_g)) color_g = 1.0;
  if (!getInput("color_b", color_b)) color_b = 1.0;

  std_msgs::msg::ColorRGBA color;
  color.r = static_cast<float>(color_r);
  color.g = static_cast<float>(color_g);
  color.b = static_cast<float>(color_b);
  color.a = 1.0;

  // Fill colors array (for eyes - up to 8 colors)
  for (size_t i = 0; i < 8; ++i) {
    goal_msg.colors[i] = color;
  }

  // Get intensity
  double intensity;
  if (!getInput("intensity", intensity)) {
    intensity = 1.0;
  }
  
  // Fill intensities array (for head/ears - up to 12 intensities)
  for (size_t i = 0; i < 12; ++i) {
    goal_msg.intensities[i] = static_cast<float>(intensity);
  }

  // Get frequency
  double frequency;
  if (!getInput("frequency", frequency)) {
    frequency = 2.0;
  }
  goal_msg.frequency = static_cast<float>(frequency);

  // Get duration
  double duration;
  if (!getInput("duration", duration)) {
    duration = -1.0; // Default to infinite
  }
  goal_msg.duration = static_cast<float>(duration);

  RCLCPP_INFO(node_->get_logger(), 
    "NaoSetLeds: Sending goal (LEDs: [%d,%d], mode: %d, color: [%.2f,%.2f,%.2f], freq: %.2f, duration: %.2f)",
    goal_msg.leds[0], goal_msg.leds[1], goal_msg.mode, 
    color_r, color_g, color_b, frequency, duration);

  // Send goal
  auto send_goal_options = rclcpp_action::Client<LedsPlay>::SendGoalOptions();
  send_goal_options.goal_response_callback =
    std::bind(&NaoSetLeds::goal_response_callback, this, std::placeholders::_1);
  send_goal_options.result_callback =
    std::bind(&NaoSetLeds::result_callback, this, std::placeholders::_1);

  action_client_->async_send_goal(goal_msg, send_goal_options);

  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus NaoSetLeds::onRunning()
{
  // Check timeout
  auto elapsed = (node_->now() - start_time_).seconds();
  if (elapsed > timeout_) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoSetLeds: Action timeout exceeded (%.1f seconds)", timeout_);
    setOutput("success", false);
    
    if (goal_handle_) {
      auto future_cancel = action_client_->async_cancel_goal(goal_handle_);
    }
    
    return bt_failure(config(), registrationName(), "action timeout exceeded");
  }

  // Check if goal was rejected
  if (!goal_accepted_ && !goal_handle_) {
    // Still waiting for goal response
    return BT::NodeStatus::RUNNING;
  }

  if (!goal_accepted_) {
    RCLCPP_ERROR(node_->get_logger(), "NaoSetLeds: Goal was rejected");
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "goal was rejected");
  }

  // Check if action completed
  if (goal_completed_) {
    setOutput("success", goal_succeeded_);
    if (!goal_succeeded_) {
      return bt_failure(config(), registrationName(), "action completed but did not succeed");
    }
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::RUNNING;
}

void NaoSetLeds::onHalted()
{
  RCLCPP_INFO(node_->get_logger(), "NaoSetLeds: Action halted");
  
  if (goal_handle_ && !goal_completed_) {
    auto future_cancel = action_client_->async_cancel_goal(goal_handle_);
  }
  
  setOutput("success", false);
}

void NaoSetLeds::goal_response_callback(
  const GoalHandleLedsPlay::SharedPtr & goal_handle)
{
  if (!goal_handle) {
    RCLCPP_ERROR(node_->get_logger(), "NaoSetLeds: Goal was rejected by server");
    goal_accepted_ = false;
  } else {
    RCLCPP_INFO(node_->get_logger(), "NaoSetLeds: Goal accepted by server");
    goal_handle_ = goal_handle;
    goal_accepted_ = true;
  }
}

void NaoSetLeds::result_callback(
  const GoalHandleLedsPlay::WrappedResult & result)
{
  goal_completed_ = true;

  switch (result.code) {
    case rclcpp_action::ResultCode::SUCCEEDED:
      RCLCPP_INFO(node_->get_logger(), 
        "NaoSetLeds: Action succeeded (result: %s)", 
        result.result->success ? "true" : "false");
      goal_succeeded_ = result.result->success;
      break;
    case rclcpp_action::ResultCode::ABORTED:
      RCLCPP_ERROR(node_->get_logger(), "NaoSetLeds: Action was aborted");
      goal_succeeded_ = false;
      break;
    case rclcpp_action::ResultCode::CANCELED:
      RCLCPP_WARN(node_->get_logger(), "NaoSetLeds: Action was canceled");
      goal_succeeded_ = false;
      break;
    default:
      RCLCPP_ERROR(node_->get_logger(), "NaoSetLeds: Unknown result code");
      goal_succeeded_ = false;
      break;
  }
}

}  // namespace social_bt_nodes
