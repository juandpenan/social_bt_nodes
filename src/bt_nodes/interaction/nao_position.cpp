#include "social_bt_nodes/bt_nodes/interaction/nao_position.hpp"

namespace social_bt_nodes
{

NaoPosition::NaoPosition(
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
    throw BT::RuntimeError("NaoPosition: 'node' not found in blackboard");
  }
  node_ = node_any;

  // Get action server name from input port
  std::string server_name;
  if (!getInput("action_server", server_name)) {
    server_name = "/nao_pos_action";
  }

  // Create action client
  action_client_ = rclcpp_action::create_client<PosPlay>(
    node_,
    server_name);

  RCLCPP_INFO(node_->get_logger(), 
    "NaoPosition: Created action client for '%s'", server_name.c_str());
}

NaoPosition::~NaoPosition()
{
  if (goal_handle_ && !goal_completed_) {
    RCLCPP_INFO(node_->get_logger(), 
      "NaoPosition: Canceling active goal on destruction");
    auto future_cancel = action_client_->async_cancel_goal(goal_handle_);
  }
}

BT::NodeStatus NaoPosition::onStart()
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

  // Get action name
  std::string action_name;
  if (!getInput("action_name", action_name)) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoPosition: Missing required input 'action_name'");
    setOutput("success", false);
    return BT::NodeStatus::FAILURE;
  }

  // Wait for action server
  if (!action_client_->wait_for_action_server(std::chrono::seconds(2))) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoPosition: Action server not available");
    setOutput("success", false);
    return BT::NodeStatus::FAILURE;
  }

  // Prepare and send goal
  auto goal_msg = PosPlay::Goal();
  goal_msg.action_name = action_name;

  RCLCPP_INFO(node_->get_logger(), 
    "NaoPosition: Sending goal to execute action '%s'", action_name.c_str());

  auto send_goal_options = rclcpp_action::Client<PosPlay>::SendGoalOptions();
  send_goal_options.goal_response_callback =
    std::bind(&NaoPosition::goal_response_callback, this, std::placeholders::_1);
  send_goal_options.result_callback =
    std::bind(&NaoPosition::result_callback, this, std::placeholders::_1);

  action_client_->async_send_goal(goal_msg, send_goal_options);

  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus NaoPosition::onRunning()
{
  // Check timeout
  auto elapsed = (node_->now() - start_time_).seconds();
  if (elapsed > timeout_) {
    RCLCPP_ERROR(node_->get_logger(), 
      "NaoPosition: Action timeout exceeded (%.1f seconds)", timeout_);
    setOutput("success", false);
    
    if (goal_handle_) {
      auto future_cancel = action_client_->async_cancel_goal(goal_handle_);
    }
    
    return BT::NodeStatus::FAILURE;
  }

  // Check if goal was rejected
  if (!goal_accepted_ && !goal_handle_) {
    // Still waiting for goal response
    return BT::NodeStatus::RUNNING;
  }

  if (!goal_accepted_) {
    RCLCPP_ERROR(node_->get_logger(), "NaoPosition: Goal was rejected");
    setOutput("success", false);
    return BT::NodeStatus::FAILURE;
  }

  // Check if action completed
  if (goal_completed_) {
    setOutput("success", goal_succeeded_);
    return goal_succeeded_ ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }

  return BT::NodeStatus::RUNNING;
}

void NaoPosition::onHalted()
{
  RCLCPP_INFO(node_->get_logger(), "NaoPosition: Action halted");
  
  if (goal_handle_ && !goal_completed_) {
    auto future_cancel = action_client_->async_cancel_goal(goal_handle_);
  }
  
  setOutput("success", false);
}

void NaoPosition::goal_response_callback(
  const GoalHandlePosPlay::SharedPtr & goal_handle)
{
  if (!goal_handle) {
    RCLCPP_ERROR(node_->get_logger(), "NaoPosition: Goal was rejected by server");
    goal_accepted_ = false;
  } else {
    RCLCPP_INFO(node_->get_logger(), "NaoPosition: Goal accepted by server");
    goal_handle_ = goal_handle;
    goal_accepted_ = true;
  }
}

void NaoPosition::result_callback(
  const GoalHandlePosPlay::WrappedResult & result)
{
  goal_completed_ = true;

  switch (result.code) {
    case rclcpp_action::ResultCode::SUCCEEDED:
      RCLCPP_INFO(node_->get_logger(), 
        "NaoPosition: Action succeeded (result: %s)", 
        result.result->success ? "true" : "false");
      goal_succeeded_ = result.result->success;
      break;
    case rclcpp_action::ResultCode::ABORTED:
      RCLCPP_ERROR(node_->get_logger(), "NaoPosition: Action was aborted");
      goal_succeeded_ = false;
      break;
    case rclcpp_action::ResultCode::CANCELED:
      RCLCPP_WARN(node_->get_logger(), "NaoPosition: Action was canceled");
      goal_succeeded_ = false;
      break;
    default:
      RCLCPP_ERROR(node_->get_logger(), "NaoPosition: Unknown result code");
      goal_succeeded_ = false;
      break;
  }
}

}  // namespace social_bt_nodes
