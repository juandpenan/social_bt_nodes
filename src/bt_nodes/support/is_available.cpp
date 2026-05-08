#include "social_bt_nodes/bt_nodes/support/is_available.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

namespace
{

std::string trim(const std::string & value)
{
  const auto first = value.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) {
    return "";
  }
  const auto last = value.find_last_not_of(" \t\n\r");
  return value.substr(first, last - first + 1);
}

std::string to_lower(std::string value)
{
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

std::vector<std::string> split_semicolon(const std::string & raw)
{
  std::vector<std::string> out;
  std::stringstream ss(raw);
  std::string token;
  while (std::getline(ss, token, ';')) {
    token = to_lower(trim(token));
    if (!token.empty()) {
      out.push_back(token);
    }
  }
  return out;
}

std::string join_semicolon(const std::vector<std::string> & items)
{
  std::ostringstream oss;
  for (size_t i = 0; i < items.size(); ++i) {
    if (i > 0) {
      oss << ";";
    }
    oss << items[i];
  }
  return oss.str();
}

}  // namespace

IsAvailable::IsAvailable(const std::string & name, const BT::NodeConfig & conf)
: BT::ConditionNode(name, conf)
{
  auto node_any = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  if (!node_any) {
    throw BT::RuntimeError("IsAvailable: 'node' not found in blackboard");
  }
  node_ = node_any;
}

BT::NodeStatus IsAvailable::tick()
{
  std::string available_items_raw;
  std::string requested_items_raw;

  if (!getInput("available_items", available_items_raw)) {
    RCLCPP_ERROR(node_->get_logger(), "IsAvailable: missing required input 'available_items'");
    return bt_failure(
      config(), registrationName(),
      "missing required input 'available_items'",
      "bt_config_error");
  }

  if (!getInput("items", requested_items_raw)) {
    RCLCPP_ERROR(node_->get_logger(), "IsAvailable: missing required input 'items'");
    return bt_failure(
      config(), registrationName(),
      "missing required input 'items'",
      "bt_config_error");
  }

  const auto available_items = split_semicolon(available_items_raw);
  std::unordered_set<std::string> available_set;
  for (const auto & item : available_items) {
    available_set.insert(item);
  }

  const auto requested_items = split_semicolon(requested_items_raw);
  std::vector<std::string> unavailable;
  unavailable.reserve(requested_items.size());

  for (const auto & item : requested_items) {
    if (available_set.find(item) == available_set.end()) {
      unavailable.push_back(item);
    }
  }

  if (!unavailable.empty()) {
    const auto unavailable_joined = join_semicolon(unavailable);
    setOutput("unavailable_items", unavailable_joined);
    return bt_failure(
      config(), registrationName(),
      "unavailable items: '" + unavailable_joined + "'",
      "bt_item_unavailable");
  }

  setOutput("unavailable_items", std::string{});
  return BT::NodeStatus::SUCCESS;
}

}  // namespace social_bt_nodes