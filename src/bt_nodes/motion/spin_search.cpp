#include "social_bt_nodes/bt_nodes/motion/spin_search.hpp"
#include <cmath>

namespace social_bt_nodes
{

SpinSearch::SpinSearch(
  const std::string & action_name,
  const BT::NodeConfig & conf)
: BT::StatefulActionNode(action_name, conf),
  stop_requested_(false)
{
  // Get ROS node from blackboard
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("SpinSearch: 'node' not found in blackboard");
  }
  node_ = node_any;
  
  std::string cmd_vel_topic, touch_topic;
  if (!getInput("cmd_vel_topic", cmd_vel_topic)) {
    cmd_vel_topic = "/cmd_vel";
  }
  if (!getInput("touch_topic", touch_topic)) {
    touch_topic = "/sensors/touch";
  }
  
  cmd_vel_pub_ = node_->create_publisher<geometry_msgs::msg::Twist>(
    cmd_vel_topic, 10);
  
  touch_sub_ = node_->create_subscription<nao_lola_sensor_msgs::msg::Touch>(
    touch_topic, 10,
    std::bind(&SpinSearch::touch_callback, this, std::placeholders::_1));
}

SpinSearch::~SpinSearch()
{
  stop_robot();
}

BT::NodeStatus SpinSearch::onStart()
{
  if (!getInput("angular_speed", angular_speed_)) {
    angular_speed_ = 0.5;
  }
  
  RCLCPP_INFO(node_->get_logger(), 
    "Starting search: Spinning at %.2f rad/s",
    angular_speed_);
  
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus SpinSearch::onRunning()
{
  // Check if stopped by touch sensor
  if (stop_requested_) {
    RCLCPP_INFO(node_->get_logger(), "Robot stopped by touch sensor");
    stop_robot();
    return BT::NodeStatus::RUNNING;
  }
  
  // Just keep spinning - the BT will halt this when target is detected
  RCLCPP_DEBUG(node_->get_logger(), "Searching for target...");
  
  auto twist_msg = geometry_msgs::msg::Twist();
  twist_msg.linear.x = 0.0;
  twist_msg.linear.y = 0.0;
  twist_msg.linear.z = 0.0;
  twist_msg.angular.x = 0.0;
  twist_msg.angular.y = 0.0;
  twist_msg.angular.z = angular_speed_;
  
  cmd_vel_pub_->publish(twist_msg);
  
  return BT::NodeStatus::RUNNING;
}

void SpinSearch::onHalted()
{
  RCLCPP_INFO(node_->get_logger(), "Search halted");
  stop_robot();
}

void SpinSearch::stop_robot()
{
  auto twist_msg = geometry_msgs::msg::Twist();
  twist_msg.linear.x = 0.0;
  twist_msg.linear.y = 0.0;
  twist_msg.linear.z = 0.0;
  twist_msg.angular.x = 0.0;
  twist_msg.angular.y = 0.0;
  twist_msg.angular.z = 0.0;
  
  cmd_vel_pub_->publish(twist_msg);
}

void SpinSearch::touch_callback(
  const nao_lola_sensor_msgs::msg::Touch::SharedPtr msg)
{
  if (msg->head_front || msg->head_middle || msg->head_rear) {
    stop_requested_ = !stop_requested_;
    if (stop_requested_) {
      RCLCPP_INFO(node_->get_logger(), "Touch detected: Stopping robot");
    } else {
      RCLCPP_INFO(node_->get_logger(), "Touch detected: Resuming robot");
    }
  }
}

}  // namespace social_bt_nodes
