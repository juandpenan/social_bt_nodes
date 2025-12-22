#ifndef SOCIAL_BT_NODES__BT_NODES__INTERACTION__NAO_SET_LEDS_HPP_
#define SOCIAL_BT_NODES__BT_NODES__INTERACTION__NAO_SET_LEDS_HPP_

#include <memory>
#include <string>
#include <vector>

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nao_led_interfaces/action/leds_play.hpp"
#include "std_msgs/msg/color_rgba.hpp"

namespace social_bt_nodes
{

/**
 * @brief BehaviorTree action node to control NAO robot LEDs
 * 
 * This node uses the nao_led action server to control the LEDs of the NAO robot.
 * It supports different LED groups (head, eyes, ears, chest, feet) and modes
 * (steady, blinking, loop).
 * 
 * XML Usage:
 *   <NaoSetLeds led_ids="[4,5]" mode="1" color_r="1.0" color_g="0.0" color_b="0.0" 
 *               frequency="2.0" duration="5.0" action_server="leds_play" timeout="10.0"/>
 * 
 * LED IDs (from nao_led_interfaces/msg/LedIndexes):
 *   - 0: HEAD
 *   - 1: LEAR (Left Ear)
 *   - 2: REAR (Right Ear)
 *   - 3: CHEST
 *   - 4: LFOOT (Left Foot)
 *   - 5: RFOOT (Right Foot)
 *   - 6: LEYE (Left Eye)
 *   - 7: REYE (Right Eye)
 * 
 * Modes (from nao_led_interfaces/msg/LedModes):
 *   - 0: STEADY (constant on)
 *   - 1: BLINKING (blink on/off)
 *   - 2: LOOP (rotating animation)
 * 
 * Ports:
 *   Input:
 *     - led_ids (string): Comma-separated LED IDs (e.g., "4,5" for both feet)
 *     - mode (int): LED mode (0=STEADY, 1=BLINKING, 2=LOOP)
 *     - color_r (double, default: 1.0): Red component (0.0-1.0)
 *     - color_g (double, default: 1.0): Green component (0.0-1.0)
 *     - color_b (double, default: 1.0): Blue component (0.0-1.0)
 *     - intensity (double, default: 1.0): LED intensity (0.0-1.0)
 *     - frequency (double, default: 2.0): Frequency for blinking/loop mode (Hz)
 *     - duration (double, default: -1.0): Duration in seconds (-1 = infinite)
 *     - action_server (string, default: "leds_play"): Action server name
 *     - timeout (double, default: 10.0): Timeout in seconds
 *   Output:
 *     - success (bool): Whether the action succeeded
 */
class NaoSetLeds : public BT::StatefulActionNode
{
public:
  using LedsPlay = nao_led_interfaces::action::LedsPlay;
  using GoalHandleLedsPlay = rclcpp_action::ClientGoalHandle<LedsPlay>;

  NaoSetLeds(
    const std::string & name,
    const BT::NodeConfig & conf);

  NaoSetLeds() = delete;

  ~NaoSetLeds();

  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("led_ids", "Comma-separated LED IDs (e.g., '4,5')"),
      BT::InputPort<int>("mode", 0, "LED mode (0=STEADY, 1=BLINKING, 2=LOOP)"),
      BT::InputPort<double>("color_r", 1.0, "Red component (0.0-1.0)"),
      BT::InputPort<double>("color_g", 1.0, "Green component (0.0-1.0)"),
      BT::InputPort<double>("color_b", 1.0, "Blue component (0.0-1.0)"),
      BT::InputPort<double>("intensity", 1.0, "LED intensity (0.0-1.0)"),
      BT::InputPort<double>("frequency", 2.0, "Frequency for blinking/loop (Hz)"),
      BT::InputPort<double>("duration", -1.0, "Duration in seconds (-1 = infinite)"),
      BT::InputPort<std::string>("action_server", "leds_play", "Action server name"),
      BT::InputPort<double>("timeout", 10.0, "Timeout in seconds"),
      BT::OutputPort<bool>("success", "Whether the action succeeded")
    };
  }

private:
  void goal_response_callback(const GoalHandleLedsPlay::SharedPtr & goal_handle);
  void result_callback(const GoalHandleLedsPlay::WrappedResult & result);
  std::vector<uint8_t> parse_led_ids(const std::string & led_ids_str);

  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<LedsPlay>::SharedPtr action_client_;
  
  GoalHandleLedsPlay::SharedPtr goal_handle_;
  rclcpp::Time start_time_;
  double timeout_;
  bool goal_accepted_;
  bool goal_completed_;
  bool goal_succeeded_;
};

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_NODES__INTERACTION__NAO_SET_LEDS_HPP_
