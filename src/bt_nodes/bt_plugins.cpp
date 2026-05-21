#include "social_bt_nodes/bt_nodes/motion/get_nav_location.hpp"
#include "behaviortree_cpp/bt_factory.h"
#include "social_bt_nodes/bt_failure.hpp"
#include "social_bt_nodes/bt_nodes/motion/follow.hpp"
#include "social_bt_nodes/bt_nodes/motion/nao/nao_follow.hpp"
#include "social_bt_nodes/bt_nodes/motion/nao/nao_follow_dynamic.hpp"
#include "social_bt_nodes/bt_nodes/motion/spin_search.hpp"
#include "social_bt_nodes/bt_nodes/motion/navigate_to.hpp"
#include "social_bt_nodes/bt_nodes/motion/move_forward.hpp"
#include "social_bt_nodes/bt_nodes/motion/move_towards.hpp"
#include "social_bt_nodes/bt_nodes/motion/rotate_to_bearing.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_target_detected.hpp"
#include "social_bt_nodes/bt_nodes/perception/set_perception_target.hpp"
#include "social_bt_nodes/bt_nodes/perception/get_bearing.hpp"
#include "social_bt_nodes/bt_nodes/perception/get_distance.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_aligned.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_farther_than.hpp"
#include "social_bt_nodes/bt_nodes/perception/is_target_static.hpp"
#include "social_bt_nodes/bt_nodes/interaction/speak.hpp"
#include "social_bt_nodes/bt_nodes/interaction/speak_enum.hpp"
#include "social_bt_nodes/bt_nodes/interaction/ask_open_question.hpp"
#include "social_bt_nodes/bt_nodes/interaction/listen.hpp"
#include "social_bt_nodes/bt_nodes/interaction/extract.hpp"
#include "social_bt_nodes/bt_nodes/interaction/ask_yes_no_question.hpp"
#include "social_bt_nodes/bt_nodes/interaction/nao/nao_position.hpp"
#include "social_bt_nodes/bt_nodes/interaction/nao/nao_set_leds.hpp"
#include "social_bt_nodes/bt_nodes/support/is_available.hpp"
#include "social_bt_nodes/bt_nodes/support/set_ros2_param.hpp"
#include "social_bt_nodes/bt_nodes/support/is_true.hpp"
#include "social_bt_nodes/bt_nodes/support/stop_current_task.hpp"
#include "social_bt_nodes/bt_nodes/support/force_plan_fail.hpp"

// Plugin registration with extern "C" linkage for dynamic loading
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<social_bt_nodes::GetNavLocation>("GetNavLocation");
  social_bt_nodes::bt_register_node_description("GetNavLocation", social_bt_nodes::GetNavLocation::node_description);
  factory.registerNodeType<social_bt_nodes::Follow>("Follow");
  social_bt_nodes::bt_register_node_description("Follow", social_bt_nodes::Follow::node_description);
  factory.registerNodeType<social_bt_nodes::NaoFollow>("NaoFollow");
  social_bt_nodes::bt_register_node_description("NaoFollow", social_bt_nodes::NaoFollow::node_description);
  factory.registerNodeType<social_bt_nodes::NaoFollowDynamic>("FollowDynamic");
  social_bt_nodes::bt_register_node_description("FollowDynamic", social_bt_nodes::NaoFollowDynamic::node_description);
  factory.registerNodeType<social_bt_nodes::SpinSearch>("Spin");
  social_bt_nodes::bt_register_node_description("Spin", social_bt_nodes::SpinSearch::node_description);
  factory.registerNodeType<social_bt_nodes::NavigateTo>("NavigateTo");
  social_bt_nodes::bt_register_node_description("NavigateTo", social_bt_nodes::NavigateTo::node_description);
  factory.registerNodeType<social_bt_nodes::MoveForward>("MoveForward");
  social_bt_nodes::bt_register_node_description("MoveForward", social_bt_nodes::MoveForward::node_description);
  factory.registerNodeType<social_bt_nodes::MoveTowards>("MoveTowards");
  social_bt_nodes::bt_register_node_description("MoveTowards", social_bt_nodes::MoveTowards::node_description);
  factory.registerNodeType<social_bt_nodes::RotateToBearing>("RotateToBearing");
  social_bt_nodes::bt_register_node_description("RotateToBearing", social_bt_nodes::RotateToBearing::node_description);
  factory.registerNodeType<social_bt_nodes::IsTargetDetected>("IsDetected");
  social_bt_nodes::bt_register_node_description("IsTargetDetected", social_bt_nodes::IsTargetDetected::node_description);
  factory.registerNodeType<social_bt_nodes::SetPerceptionTarget>("SetPerceptionTarget");
  social_bt_nodes::bt_register_node_description("SetPerceptionTarget", social_bt_nodes::SetPerceptionTarget::node_description);
  factory.registerNodeType<social_bt_nodes::GetBearing>("GetBearing");
  social_bt_nodes::bt_register_node_description("GetBearing", social_bt_nodes::GetBearing::node_description);
  factory.registerNodeType<social_bt_nodes::GetDistance>("GetDistance");
  social_bt_nodes::bt_register_node_description("GetDistance", social_bt_nodes::GetDistance::node_description);
  factory.registerNodeType<social_bt_nodes::IsAligned>("IsAligned");
  social_bt_nodes::bt_register_node_description("IsAligned", social_bt_nodes::IsAligned::node_description);
  factory.registerNodeType<social_bt_nodes::IsFartherThan>("IsFartherThan");
  social_bt_nodes::bt_register_node_description("IsFartherThan", social_bt_nodes::IsFartherThan::node_description);
  factory.registerNodeType<social_bt_nodes::IsTargetStatic>("IsTargetStatic");
  social_bt_nodes::bt_register_node_description("IsTargetStatic", social_bt_nodes::IsTargetStatic::node_description);
  factory.registerNodeType<social_bt_nodes::Speak>("Speak");
  social_bt_nodes::bt_register_node_description("Speak", social_bt_nodes::Speak::node_description);
  factory.registerNodeType<social_bt_nodes::SpeakEnum>("SpeakEnum");
  social_bt_nodes::bt_register_node_description("SpeakEnum", social_bt_nodes::SpeakEnum::node_description);
  factory.registerNodeType<social_bt_nodes::AskOpenQuestion>("AskOpenQuestion");
  social_bt_nodes::bt_register_node_description("AskOpenQuestion", social_bt_nodes::AskOpenQuestion::node_description);
  factory.registerNodeType<social_bt_nodes::Listen>("Listen");
  social_bt_nodes::bt_register_node_description("Listen", social_bt_nodes::Listen::node_description);
  factory.registerNodeType<social_bt_nodes::Extract>("Extract");
  social_bt_nodes::bt_register_node_description("Extract", social_bt_nodes::Extract::node_description);
  factory.registerNodeType<social_bt_nodes::AskYesNoQuestion>("AskYesNoQuestion");
  social_bt_nodes::bt_register_node_description("AskYesNoQuestion", social_bt_nodes::AskYesNoQuestion::node_description);
  factory.registerNodeType<social_bt_nodes::IsAvailable>("IsAvailable");
  social_bt_nodes::bt_register_node_description("IsAvailable", social_bt_nodes::IsAvailable::node_description);
  factory.registerNodeType<social_bt_nodes::IsTrue>("IsTrue");
  social_bt_nodes::bt_register_node_description("IsTrue", social_bt_nodes::IsTrue::node_description);
  factory.registerNodeType<social_bt_nodes::NaoPosition>("NaoPosition");
  social_bt_nodes::bt_register_node_description("NaoPosition", social_bt_nodes::NaoPosition::node_description);
  factory.registerNodeType<social_bt_nodes::NaoSetLeds>("NaoSetLeds");
  social_bt_nodes::bt_register_node_description("NaoSetLeds", social_bt_nodes::NaoSetLeds::node_description);
  factory.registerNodeType<social_bt_nodes::SetRos2Param>("SetRos2Param");
  social_bt_nodes::bt_register_node_description("SetRos2Param", social_bt_nodes::SetRos2Param::node_description);
  factory.registerNodeType<social_bt_nodes::StopCurrentTask>("StopCurrentTask");
  social_bt_nodes::bt_register_node_description("StopCurrentTask", social_bt_nodes::StopCurrentTask::node_description);
  factory.registerNodeType<social_bt_nodes::ForcePlanFail>("ForcePlanFail");
  social_bt_nodes::bt_register_node_description("ForcePlanFail", social_bt_nodes::ForcePlanFail::node_description);
}
