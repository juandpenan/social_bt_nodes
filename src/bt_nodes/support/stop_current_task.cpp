#include "social_bt_nodes/bt_nodes/support/stop_current_task.hpp"

#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

StopCurrentTask::StopCurrentTask(const std::string & name, const BT::NodeConfig & conf)
: BT::ConditionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("StopCurrentTask: 'node' not found in blackboard");
  }
  node_ = node_any;

  std::string event_topic = "/stop_current_task";
  int qos_depth = 10;
  (void)getInput("event_topic", event_topic);
  (void)getInput("qos_depth", qos_depth);

  stop_event_sub_ = node_->create_subscription<std_msgs::msg::Empty>(
    event_topic,
    rclcpp::QoS(static_cast<size_t>(qos_depth > 0 ? qos_depth : 1)),
    [this](const std_msgs::msg::Empty::SharedPtr)
    {
      stop_event_pending_.store(true, std::memory_order_release);
      RCLCPP_WARN(node_->get_logger(), "StopCurrentTask: stop event received");
    });

  RCLCPP_INFO(
    node_->get_logger(),
    "StopCurrentTask listening for events on '%s'",
    event_topic.c_str());
}

BT::NodeStatus StopCurrentTask::tick()
{
  const bool stop_requested = stop_event_pending_.exchange(false, std::memory_order_acq_rel);

  if (stop_requested) {
    RCLCPP_WARN(node_->get_logger(), "StopCurrentTask: consuming stop event -> SUCCESS");
    return BT::NodeStatus::SUCCESS;
  }

  return bt_failure(config(), registrationName(), "NO_REAL_FAILURE");
}

}  // namespace social_bt_nodes
