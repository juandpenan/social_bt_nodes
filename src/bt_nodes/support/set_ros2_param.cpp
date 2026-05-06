#include "social_bt_nodes/bt_nodes/support/set_ros2_param.hpp"
#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

SetRos2Param::SetRos2Param(
  const std::string & name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(name, conf)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("SetRos2Param: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus SetRos2Param::onStart()
{
  // Get input parameters
  if (!getInput("node_name", node_name_)) {
    RCLCPP_ERROR(node_->get_logger(), 
      "SetRos2Param: missing required input 'node_name'");
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "missing required input 'node_name'", "bt_config_error");
  }
  
  if (!getInput("param_name", param_name_)) {
    RCLCPP_ERROR(node_->get_logger(), 
      "SetRos2Param: missing required input 'param_name'");
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "missing required input 'param_name'", "bt_config_error");
  }
  
  if (!getInput("param_value", param_value_)) {
    RCLCPP_ERROR(node_->get_logger(), 
      "SetRos2Param: missing required input 'param_value'");
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "missing required input 'param_value'", "bt_config_error");
  }
  
  if (!getInput("param_type", param_type_)) {
    param_type_ = "string";
  }
  
  if (!getInput("timeout", timeout_ms_)) {
    timeout_ms_ = 2000;
  }
  
  // Create service client for the target node's parameter service
  // Remove leading slash if present, as ROS2 will add it
  std::string clean_node_name = node_name_;
  if (clean_node_name[0] == '/') {
    clean_node_name = clean_node_name.substr(1);
  }
  std::string service_name = clean_node_name + "/set_parameters";
  
  RCLCPP_INFO(node_->get_logger(), 
    "SetRos2Param: Attempting to connect to service '%s'", service_name.c_str());
  
  client_ = node_->create_client<rcl_interfaces::srv::SetParameters>(service_name);
  
  // Wait for service to be available
  if (!client_->wait_for_service(std::chrono::milliseconds(timeout_ms_))) {
    RCLCPP_ERROR(node_->get_logger(), 
      "SetRos2Param: Service '%s' not available after %d ms. Is node '%s' running?", 
      service_name.c_str(), timeout_ms_, node_name_.c_str());
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "set_parameters service for node '" + node_name_ + "' not available");
  }
  
  RCLCPP_INFO(node_->get_logger(), 
    "SetRos2Param: Service '%s' found", service_name.c_str());
  
  // Prepare request
  auto request = std::make_shared<rcl_interfaces::srv::SetParameters::Request>();
  rcl_interfaces::msg::Parameter param;
  param.name = param_name_;
  param.value = createParameterValue(param_value_, param_type_);
  request->parameters.push_back(param);
  
  RCLCPP_INFO(node_->get_logger(), 
    "SetRos2Param: Setting %s.%s = %s (type: %s)", 
    node_name_.c_str(), param_name_.c_str(), param_value_.c_str(), param_type_.c_str());
  
  // Send request
  future_result_ = std::make_shared<
    rclcpp::Client<rcl_interfaces::srv::SetParameters>::FutureAndRequestId>(
    client_->async_send_request(request));
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus SetRos2Param::onRunning()
{
  if (!future_result_) {
    setOutput("success", false);
    return bt_failure(config(), registrationName(), "no pending service future");
  }
  
  auto status = future_result_->wait_for(std::chrono::milliseconds(0));
  
  if (status == std::future_status::ready) {
    auto result = future_result_->get();
    
    // Check if parameter was set successfully
    if (!result->results.empty() && result->results[0].successful) {
      RCLCPP_INFO(node_->get_logger(), 
        "SetRos2Param: Successfully set %s.%s = %s", 
        node_name_.c_str(), param_name_.c_str(), param_value_.c_str());
      setOutput("success", true);
      return BT::NodeStatus::SUCCESS;
    } else {
      std::string reason = result->results.empty() ? 
        "no result" : result->results[0].reason;
      RCLCPP_ERROR(node_->get_logger(), 
        "SetRos2Param: Failed to set parameter: %s", reason.c_str());
      setOutput("success", false);
      return bt_failure(config(), registrationName(), "failed to set " + node_name_ + "." + param_name_ + ": " + reason);
    }
  }
  
  return BT::NodeStatus::RUNNING;
}

void SetRos2Param::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "SetRos2Param: Halted");
  future_result_.reset();
  setOutput("success", false);
}

rcl_interfaces::msg::ParameterValue SetRos2Param::createParameterValue(
  const std::string & value_str, const std::string & type)
{
  rcl_interfaces::msg::ParameterValue param_value;
  
  if (type == "string") {
    param_value.type = rcl_interfaces::msg::ParameterType::PARAMETER_STRING;
    param_value.string_value = value_str;
  } else if (type == "int") {
    param_value.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
    param_value.integer_value = std::stoll(value_str);
  } else if (type == "double") {
    param_value.type = rcl_interfaces::msg::ParameterType::PARAMETER_DOUBLE;
    param_value.double_value = std::stod(value_str);
  } else if (type == "bool") {
    param_value.type = rcl_interfaces::msg::ParameterType::PARAMETER_BOOL;
    param_value.bool_value = (value_str == "true" || value_str == "True" || 
                               value_str == "1" || value_str == "yes");
  } else {
    RCLCPP_WARN(node_->get_logger(), 
      "SetRos2Param: Unknown parameter type '%s', defaulting to string", type.c_str());
    param_value.type = rcl_interfaces::msg::ParameterType::PARAMETER_STRING;
    param_value.string_value = value_str;
  }
  
  return param_value;
}

}  // namespace social_bt_nodes
