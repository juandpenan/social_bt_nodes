#ifndef SOCIAL_BT_NODES__BT_NODES__MOTION__NAVIGATE_TO_HPP_
#define SOCIAL_BT_NODES__BT_NODES__MOTION__NAVIGATE_TO_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

namespace social_bt_nodes
{

class NavigateTo : public BT::StatefulActionNode
{
public:
  using NavigateToPose = nav2_msgs::action::NavigateToPose;
  using GoalHandleNavigateToPose = rclcpp_action::ClientGoalHandle<NavigateToPose>;

  NavigateTo(
    const std::string & action_name,
    const BT::NodeConfig & conf);

  NavigateTo() = delete;

  ~NavigateTo();

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Navigates to a target position using a Nav2 action server.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<double>("x", "X coordinate in meters (map frame)"),
      BT::InputPort<double>("y", "Y coordinate in meters (map frame)"),
      BT::InputPort<double>("yaw", 0.0, "Yaw orientation in radians (map frame)"),
      BT::InputPort<std::string>("target_frame", "Target TF frame to navigate to (alternative to x,y,yaw)"),
      BT::InputPort<std::string>("frame_id", "map", "Frame ID for the goal pose"),
      BT::InputPort<std::string>("action_name", "navigate_to_pose", "Nav2 action server name"),
      BT::InputPort<double>("timeout", 300.0, "Timeout for navigation in seconds"),
      BT::OutputPort<std::string>("error_msg", "Error message if navigation fails")
    };
  }

private:
  void goal_response_callback(const GoalHandleNavigateToPose::SharedPtr & goal_handle);
  void feedback_callback(
    GoalHandleNavigateToPose::SharedPtr,
    const std::shared_ptr<const NavigateToPose::Feedback> feedback);
  void result_callback(const GoalHandleNavigateToPose::WrappedResult & result);

  geometry_msgs::msg::PoseStamped create_goal_pose_from_coordinates(
    double x, double y, double yaw, const std::string & frame_id);
  geometry_msgs::msg::PoseStamped create_goal_pose_from_tf(
    const std::string & target_frame, const std::string & frame_id);

  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<NavigateToPose>::SharedPtr action_client_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  GoalHandleNavigateToPose::SharedPtr goal_handle_;
  rclcpp::Time start_time_;
  double timeout_;
  std::string error_msg_;
  bool goal_accepted_;
  bool goal_completed_;
  bool goal_succeeded_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__MOTION__NAVIGATE_TO_HPP_
