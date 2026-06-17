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

bool read_input_or_raw(
  BT::TreeNode & node,
  const BT::NodeConfig & node_config,
  const char * port_name,
  std::string & value)
{
  if (node.getInput(port_name, value)) {
    return true;
  }

  const auto it = node_config.input_ports.find(port_name);
  if (it != node_config.input_ports.end()) {
    const std::string raw = trim(it->second);
    if (!raw.empty()) {
      value = raw;
      return true;
    }
  }
  return false;
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

bool resolve_semicolon_refs(
  const BT::Blackboard::Ptr & blackboard,
  const std::string & raw,
  std::string & resolved,
  std::string & error)
{
  if (raw.find('{') == std::string::npos && raw.find('}') == std::string::npos) {
    resolved = raw;
    return true;
  }

  std::stringstream ss(raw);
  std::string token;
  std::vector<std::string> values;
  while (std::getline(ss, token, ';')) {
    token = trim(token);
    if (token.empty()) {
      continue;
    }
    if (token.size() < 3 || token.front() != '{' || token.back() != '}') {
      resolved = raw;
      return true;
    }

    const std::string key = trim(token.substr(1, token.size() - 2));
    if (key.empty()) {
      error = "empty blackboard key in concatenated reference";
      return false;
    }

    try {
      values.push_back(blackboard->get<std::string>(key));
    } catch (const std::exception & e) {
      error = "blackboard key '" + key + "' unavailable: " + e.what();
      return false;
    }
  }

  std::ostringstream out;
  for (size_t i = 0; i < values.size(); ++i) {
    if (i > 0) {
      out << ";";
    }
    out << values[i];
  }
  resolved = out.str();
  return true;
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

  if (!read_input_or_raw(*this, config(), "available_items", available_items_raw)) {
    RCLCPP_ERROR(node_->get_logger(), "IsAvailable: missing required input 'available_items'");
    return bt_failure(
      config(), registrationName(),
      "missing required input 'available_items'",
      "bt_config_error");
  }

  if (!read_input_or_raw(*this, config(), "items", requested_items_raw)) {
    RCLCPP_ERROR(node_->get_logger(), "IsAvailable: missing required input 'items'");
    return bt_failure(
      config(), registrationName(),
      "missing required input 'items'",
      "bt_config_error");
  }

  std::string resolve_error;
  std::string available_items_resolved = available_items_raw;
  std::string requested_items_resolved = requested_items_raw;
  if (!resolve_semicolon_refs(config().blackboard, available_items_raw, available_items_resolved, resolve_error)) {
    return bt_failure(
      config(), registrationName(),
      "invalid 'available_items' blackboard reference: " + resolve_error,
      "bt_config_error");
  }
  if (!resolve_semicolon_refs(config().blackboard, requested_items_raw, requested_items_resolved, resolve_error)) {
    return bt_failure(
      config(), registrationName(),
      "invalid 'items' blackboard reference: " + resolve_error,
      "bt_config_error");
  }

  const auto available_items = split_semicolon(available_items_resolved);
  std::unordered_set<std::string> available_set;
  for (const auto & item : available_items) {
    available_set.insert(item);
  }

  const auto requested_items = split_semicolon(requested_items_resolved);
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
    return bt_failure(config(), registrationName(), "NO_REAL_FAILURE");
  }

  setOutput("unavailable_items", std::string{});
  return BT::NodeStatus::SUCCESS;
}

}  // namespace social_bt_nodes