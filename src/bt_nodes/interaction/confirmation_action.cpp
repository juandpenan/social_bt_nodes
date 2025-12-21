#include "social_bt_nodes/bt_nodes/interaction/confirmation_action.hpp"

namespace social_bt_nodes
{

ConfirmationAction::ConfirmationAction(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("ConfirmationAction: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus ConfirmationAction::onStart()
{
  // Get input parameters
  if (!getInput("text", text_)) {
    RCLCPP_ERROR(node_->get_logger(), "ConfirmationAction: missing required input 'text'");
    return BT::NodeStatus::FAILURE;
  }
  
  if (!getInput("service_name", service_name_)) {
    service_name_ = "/yesno_service";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 10000;
  }
  
  // Create service client if not already created or if service name changed
  if (!client_ || client_->get_service_name() != service_name_) {
    client_ = node_->create_client<simple_hri_interfaces::srv::YesNo>(service_name_);
  }
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(1000))) {
    RCLCPP_WARN(node_->get_logger(), 
      "ConfirmationAction: Service '%s' not available yet", service_name_.c_str());
    return BT::NodeStatus::FAILURE;
  }
  
  // Prepare and send request
  auto request = std::make_shared<simple_hri_interfaces::srv::YesNo::Request>();
  request->text = text_;
  
  RCLCPP_INFO(node_->get_logger(), 
    "ConfirmationAction: Checking for yes/no in text");
  
  future_result_ = std::make_shared<
    rclcpp::Client<simple_hri_interfaces::srv::YesNo>::FutureAndRequestId>(
    client_->async_send_request(request));
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus ConfirmationAction::onRunning()
{
  // Check if service call is complete
  if (!future_result_) {
    return BT::NodeStatus::FAILURE;
  }
  
  auto status = future_result_->wait_for(std::chrono::milliseconds(0));
  
  if (status == std::future_status::ready) {
    auto result = future_result_->get();
    
    // The yes/no result is in the result field
    std::string confirmation_result = result->result;
    RCLCPP_INFO(node_->get_logger(), 
      "ConfirmationAction: Result: '%s'", confirmation_result.c_str());
    
    // Set output port with the result
    setOutput("result", confirmation_result);
    
    return BT::NodeStatus::SUCCESS;
  }
  
  return BT::NodeStatus::RUNNING;
}

void ConfirmationAction::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "ConfirmationAction: Halted");
  future_result_.reset();
}

}  // namespace social_bt_nodes
