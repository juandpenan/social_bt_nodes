#include "behaviortree_cpp/bt_factory.h"
#include "social_bt_nodes/bt_nodes/motion/nao_follow.hpp"
#include "social_bt_nodes/bt_nodes/motion/nao_follow_dynamic.hpp"
#include "social_bt_nodes/bt_nodes/motion/spin_search_action.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_target_detected_condition.hpp"
#include "social_bt_nodes/bt_nodes/interaction/speak_action.hpp"
#include "social_bt_nodes/bt_nodes/interaction/speak_enum_action.hpp"
#include "social_bt_nodes/bt_nodes/interaction/listen_action.hpp"
#include "social_bt_nodes/bt_nodes/interaction/extract_action.hpp"
#include "social_bt_nodes/bt_nodes/interaction/confirmation_action.hpp"

// Plugin registration with extern "C" linkage for dynamic loading
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<social_bt_nodes::NaoFollow>("Follow");
  factory.registerNodeType<social_bt_nodes::NaoFollowDynamic>("FollowDynamic");
  factory.registerNodeType<social_bt_nodes::SpinSearchAction>("SpinSearch");
  factory.registerNodeType<social_bt_nodes::IsTargetDetectedCondition>("IsTargetDetected");
  factory.registerNodeType<social_bt_nodes::SpeakAction>("Speak");
  factory.registerNodeType<social_bt_nodes::SpeakEnumAction>("SpeakEnum");
  factory.registerNodeType<social_bt_nodes::ListenAction>("Listen");
  factory.registerNodeType<social_bt_nodes::ExtractAction>("Extract");
  factory.registerNodeType<social_bt_nodes::ConfirmationAction>("Confirmation");
}
