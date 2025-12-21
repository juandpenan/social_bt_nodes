#include "social_bt_nodes/bt_nodes/interaction/listen_action.hpp"

namespace social_bt_nodes
{

ListenAction::ListenAction(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("ListenAction: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus ListenAction::onStart()
{
  // Get input parameters
  if (!getInput("service_name", service_name_)) {
    service_name_ = "/stt_service";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 10000;
  }
  
  // Create service client if not already created or if service name changed
  if (!client_ || client_->get_service_name() != service_name_) {
    client_ = node_->create_client<std_srvs::srv::SetBool>(service_name_);
  }
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(1000))) {
    RCLCPP_WARN(node_->get_logger(), 
      "ListenAction: Service '%s' not available yet", service_name_.c_str());
    return BT::NodeStatus::FAILURE;
  }
  
  // Prepare and send request
  auto request = std::make_shared<std_srvs::srv::SetBool::Request>();
  request->data = true;  // Start listening
  
  RCLCPP_INFO(node_->get_logger(), "ListenAction: Starting to listen...");
  
  future_result_ = std::make_shared<
    rclcpp::Client<std_srvs::srv::SetBool>::FutureAndRequestId>(
    client_->async_send_request(request));
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus ListenAction::onRunning()
{
  // Check if service call is complete
  if (!future_result_) {
    return BT::NodeStatus::FAILURE;
  }
  
  auto status = future_result_->wait_for(std::chrono::milliseconds(0));
  
  if (status == std::future_status::ready) {
    auto result = future_result_->get();
    
    if (result->success) {
      // The transcribed text is in the message field
      std::string transcribed_text = result->message;
      RCLCPP_INFO(node_->get_logger(), 
        "ListenAction: Transcribed text: '%s'", transcribed_text.c_str());
      
      // Set output port with the transcribed text
      setOutput("transcribed_text", transcribed_text);
      
      return BT::NodeStatus::SUCCESS;
    } else {
      RCLCPP_ERROR(node_->get_logger(), 
        "ListenAction: Failed to transcribe: %s", result->message.c_str());
      return BT::NodeStatus::FAILURE;
    }
  }
  
  return BT::NodeStatus::RUNNING;
}

void ListenAction::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "ListenAction: Halted");
  future_result_.reset();
}

}  // namespace social_bt_nodes
