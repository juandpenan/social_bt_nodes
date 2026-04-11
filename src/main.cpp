#include <memory>
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/groot2_publisher.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  
  auto node = std::make_shared<rclcpp::Node>("social_bt_nodes");
  
  // Declare parameters
  node->declare_parameter("bt_xml", "");
  node->declare_parameter("bt_loop_duration", 100);  // ms
  node->declare_parameter("plugin_list", std::vector<std::string>());
  
  // Get parameters
  std::string bt_xml = node->get_parameter("bt_xml").as_string();
  int bt_loop_duration = node->get_parameter("bt_loop_duration").as_int();
  std::vector<std::string> plugin_list = 
    node->get_parameter("plugin_list").as_string_array();
  
  if (bt_xml.empty()) {
    RCLCPP_ERROR(node->get_logger(), "bt_xml parameter is required");
    return 1;
  }
  
  RCLCPP_INFO(node->get_logger(), "Loading behavior tree from: %s", bt_xml.c_str());
  
  // Create BehaviorTree factory
  BT::BehaviorTreeFactory factory;
  
  // Create blackboard and put node in it
  auto blackboard = BT::Blackboard::create();
  blackboard->set("node", node);
  
  // Load plugins
  for (const auto& plugin : plugin_list) {
    try {
      RCLCPP_INFO(node->get_logger(), "Loading plugin: %s", plugin.c_str());
      factory.registerFromPlugin(plugin);
    } catch (const std::exception& e) {
      RCLCPP_ERROR(node->get_logger(), 
        "Failed to load plugin %s: %s", plugin.c_str(), e.what());
      return 1;
    }
  }
  
  // Create tree
  BT::Tree tree;
  try {
    tree = factory.createTreeFromFile(bt_xml, blackboard);
    RCLCPP_INFO(node->get_logger(), "Behavior tree created successfully");
  } catch (const std::exception& e) {
    RCLCPP_ERROR(node->get_logger(), 
      "Failed to create tree: %s", e.what());
    return 1;
  }
  
  // StdCout logger: prints node status transitions to stdout
  BT::StdCoutLogger cout_logger(tree);

  // Optional: Enable Groot2 monitoring
  std::unique_ptr<BT::Groot2Publisher> groot_publisher;
  // try {
  //   groot_publisher = std::make_unique<BT::Groot2Publisher>(tree);
  //   RCLCPP_INFO(node->get_logger(), "Groot2 publisher enabled");
  // } catch (const std::exception& e) {
  //   RCLCPP_WARN(node->get_logger(), 
  //     "Groot2 publisher not available: %s", e.what());
  // }
  
  // Tick the tree periodically
  RCLCPP_INFO(node->get_logger(), 
    "Starting behavior tree execution (loop: %d ms)", bt_loop_duration);
  
  rclcpp::WallRate rate{std::chrono::milliseconds(bt_loop_duration)};
  
  while (rclcpp::ok()) {
    // Spin ROS callbacks
    rclcpp::spin_some(node);
    
    // Tick the tree
    BT::NodeStatus status = tree.tickOnce();
    
    // Log status changes
    static BT::NodeStatus last_status = BT::NodeStatus::IDLE;
    if (status != last_status) {
      RCLCPP_INFO(node->get_logger(), 
        "Tree status: %s", BT::toStr(status).c_str());
      last_status = status;
    }
    
    // Handle terminal states
    if (status == BT::NodeStatus::SUCCESS) {
      RCLCPP_INFO(node->get_logger(), "Behavior tree succeeded");
      break;
    } else if (status == BT::NodeStatus::FAILURE) {
      RCLCPP_WARN(node->get_logger(), "Behavior tree failed");
      break;
    }
    
    rate.sleep();
  }
  
  RCLCPP_INFO(node->get_logger(), "Shutting down behavior tree");
  rclcpp::shutdown();
  
  return 0;
}
