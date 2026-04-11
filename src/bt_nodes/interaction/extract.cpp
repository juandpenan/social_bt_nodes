#include "social_bt_nodes/bt_nodes/interaction/extract.hpp"
#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

Extract::Extract(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("Extract: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus Extract::onStart()
{
  // Get input parameters
  if (!getInput("interest", interest_)) {
    RCLCPP_ERROR(node_->get_logger(), "Extract: missing required input 'interest'");
    return bt_failure(config(), registrationName(), "missing required input 'interest'");
  }
  
  if (!getInput("text", text_)) {
    RCLCPP_ERROR(node_->get_logger(), "Extract: missing required input 'text'");
    return bt_failure(config(), registrationName(), "missing required input 'text'");
  }
  
  if (!getInput("service_name", service_name_)) {
    service_name_ = "/extract_service";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 10000;
  }
  
  // Create service client if not already created or if service name changed
  if (!client_ || client_->get_service_name() != service_name_) {
    client_ = node_->create_client<simple_hri_interfaces::srv::Extract>(service_name_);
  }
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(1000))) {
    RCLCPP_WARN(node_->get_logger(), 
      "Extract: Service '%s' not available yet", service_name_.c_str());
    return bt_failure(config(), registrationName(), "service '" + service_name_ + "' not available");
  }
  
  // Prepare and send request
  auto request = std::make_shared<simple_hri_interfaces::srv::Extract::Request>();
  request->interest = interest_;
  request->text = text_;
  
  RCLCPP_INFO(node_->get_logger(), 
    "Extract: Extracting '%s' from text", interest_.c_str());
  
  future_result_ = std::make_shared<
    rclcpp::Client<simple_hri_interfaces::srv::Extract>::FutureAndRequestId>(
    client_->async_send_request(request));
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus Extract::onRunning()
{
  // Check if service call is complete
  if (!future_result_) {
    return bt_failure(config(), registrationName(), "no pending service future");
  }
  
  auto status = future_result_->wait_for(std::chrono::milliseconds(0));
  
  if (status == std::future_status::ready) {
    auto result = future_result_->get();
    
    // The extracted information is in the result field
    std::string extracted_info = result->result;
    RCLCPP_INFO(node_->get_logger(), 
      "Extract: Extracted info: '%s'", extracted_info.c_str());
    
    // Set output port with the extracted information
    setOutput("extracted_info", extracted_info);
    
    return BT::NodeStatus::SUCCESS;
  }
  
  return BT::NodeStatus::RUNNING;
}

void Extract::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "Extract: Halted");
  future_result_.reset();
}

}  // namespace social_bt_nodes
