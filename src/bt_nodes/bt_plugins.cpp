#include "behaviortree_cpp/bt_factory.h"
#include "social_bt_nodes/bt_failure.hpp"
#include "social_bt_nodes/bt_nodes/motion/nao_follow.hpp"
#include "social_bt_nodes/bt_nodes/motion/nao_follow_dynamic.hpp"
#include "social_bt_nodes/bt_nodes/motion/spin_search.hpp"
#include "social_bt_nodes/bt_nodes/motion/navigate_to.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_target_detected.hpp"
#include "social_bt_nodes/bt_nodes/perception/set_perception_target.hpp"
#include "social_bt_nodes/bt_nodes/interaction/speak.hpp"
#include "social_bt_nodes/bt_nodes/interaction/speak_enum.hpp"
#include "social_bt_nodes/bt_nodes/interaction/listen.hpp"
#include "social_bt_nodes/bt_nodes/interaction/extract.hpp"
#include "social_bt_nodes/bt_nodes/interaction/confirmation.hpp"
#include "social_bt_nodes/bt_nodes/interaction/nao_position.hpp"
#include "social_bt_nodes/bt_nodes/interaction/nao_set_leds.hpp"
#include "social_bt_nodes/bt_nodes/support/set_ros2_param.hpp"

// Plugin registration with extern "C" linkage for dynamic loading
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<social_bt_nodes::NaoFollow>("Follow");
  social_bt_nodes::bt_register_node_description("Follow", social_bt_nodes::NaoFollow::node_description);
  factory.registerNodeType<social_bt_nodes::NaoFollowDynamic>("FollowDynamic");
  social_bt_nodes::bt_register_node_description("FollowDynamic", social_bt_nodes::NaoFollowDynamic::node_description);
  factory.registerNodeType<social_bt_nodes::SpinSearch>("SpinSearch");
  social_bt_nodes::bt_register_node_description("SpinSearch", social_bt_nodes::SpinSearch::node_description);
  factory.registerNodeType<social_bt_nodes::NavigateTo>("NavigateTo");
  social_bt_nodes::bt_register_node_description("NavigateTo", social_bt_nodes::NavigateTo::node_description);
  factory.registerNodeType<social_bt_nodes::IsTargetDetected>("IsTargetDetected");
  social_bt_nodes::bt_register_node_description("IsTargetDetected", social_bt_nodes::IsTargetDetected::node_description);
  factory.registerNodeType<social_bt_nodes::SetPerceptionTarget>("SetPerceptionTarget");
  social_bt_nodes::bt_register_node_description("SetPerceptionTarget", social_bt_nodes::SetPerceptionTarget::node_description);
  factory.registerNodeType<social_bt_nodes::Speak>("Speak");
  social_bt_nodes::bt_register_node_description("Speak", social_bt_nodes::Speak::node_description);
  factory.registerNodeType<social_bt_nodes::SpeakEnum>("SpeakEnum");
  social_bt_nodes::bt_register_node_description("SpeakEnum", social_bt_nodes::SpeakEnum::node_description);
  factory.registerNodeType<social_bt_nodes::Listen>("Listen");
  social_bt_nodes::bt_register_node_description("Listen", social_bt_nodes::Listen::node_description);
  factory.registerNodeType<social_bt_nodes::Extract>("Extract");
  social_bt_nodes::bt_register_node_description("Extract", social_bt_nodes::Extract::node_description);
  factory.registerNodeType<social_bt_nodes::Confirmation>("Confirmation");
  social_bt_nodes::bt_register_node_description("Confirmation", social_bt_nodes::Confirmation::node_description);
  factory.registerNodeType<social_bt_nodes::NaoPosition>("NaoPosition");
  social_bt_nodes::bt_register_node_description("NaoPosition", social_bt_nodes::NaoPosition::node_description);
  factory.registerNodeType<social_bt_nodes::NaoSetLeds>("NaoSetLeds");
  social_bt_nodes::bt_register_node_description("NaoSetLeds", social_bt_nodes::NaoSetLeds::node_description);
  factory.registerNodeType<social_bt_nodes::SetRos2Param>("SetRos2Param");
  social_bt_nodes::bt_register_node_description("SetRos2Param", social_bt_nodes::SetRos2Param::node_description);
}
