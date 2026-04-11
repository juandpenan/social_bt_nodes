#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__NAO_POSITION_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__NAO_POSITION_HPP_

#include <memory>
#include <string>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nao_pos_interfaces/action/pos_play.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree action node to execute NAO robot positions
 * 
 * This node uses the nao_pos action server to make the NAO robot
 * execute predefined positions/poses.
 * 
 * XML Usage:
 *   <NaoPosition action_name="wave" action_server="/nao_pos_server" timeout="10.0"/>
 * 
 * Ports:
 *   Input:
 *     - action_name (string): Name of the NAO position/pose to execute
 *     - action_server (string, default: "/nao_pos_server"): Action server name
 *     - timeout (double, default: 10.0): Timeout in seconds
 *   Output:
 *     - success (bool): Whether the action succeeded
 */
class NaoPosition : public BT::StatefulActionNode
{
public:
  using PosPlay = nao_pos_interfaces::action::PosPlay;
  using GoalHandlePosPlay = rclcpp_action::ClientGoalHandle<PosPlay>;

  NaoPosition(
    const std::string & name,
    const BT::NodeConfig & conf);

  NaoPosition() = delete;

  ~NaoPosition();

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static constexpr const char * node_description =
    "Executes a predefined movement action on the NAO robot.";

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("action_name", "Name of the NAO position to execute"),
      BT::InputPort<std::string>("action_server", "/nao_pos_server", "Action server name"),
      BT::InputPort<double>("timeout", 10.0, "Timeout in seconds"),
      BT::OutputPort<bool>("success", "Whether the action succeeded")
    };
  }

private:
  void goal_response_callback(const GoalHandlePosPlay::SharedPtr & goal_handle);
  void result_callback(const GoalHandlePosPlay::WrappedResult & result);

  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<PosPlay>::SharedPtr action_client_;
  
  GoalHandlePosPlay::SharedPtr goal_handle_;
  rclcpp::Time start_time_;
  double timeout_;
  std::string action_name_;
  bool goal_accepted_;
  bool goal_completed_;
  bool goal_succeeded_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__NAO_POSITION_HPP_
