#include "behaviortree_cpp/bt_factory.h"
#include "social_bt_nodes/bt_nodes/motion/nao_follow.hpp"
#include "social_bt_nodes/bt_nodes/motion/nao_follow_dynamic.hpp"
#include "social_bt_nodes/bt_nodes/motion/spin_search_action.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_target_detected_condition.hpp"

// Plugin registration with extern "C" linkage for dynamic loading
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<social_bt_nodes::NaoFollow>("Follow");
  factory.registerNodeType<social_bt_nodes::NaoFollowDynamic>("FollowDynamic");
  factory.registerNodeType<social_bt_nodes::SpinSearchAction>("SpinSearch");
  factory.registerNodeType<social_bt_nodes::IsTargetDetectedCondition>("IsTargetDetected");
}
