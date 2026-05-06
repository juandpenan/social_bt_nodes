#include "social_bt_nodes/bt_nodes/motion/get_nav_location.hpp"
#include "social_bt_nodes/bt_failure.hpp"
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <vector>

namespace social_bt_nodes
{

GetNavLocation::GetNavLocation(const std::string & name, const BT::NodeConfig & conf)
: BT::SyncActionNode(name, conf)
{
  node_ = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
  initialize_symbol_table();
}

void GetNavLocation::initialize_symbol_table()
{
  // Load YAML from config/locations.yaml in the social_bt_nodes package
  std::string pkg_share;
  try {
    pkg_share = ament_index_cpp::get_package_share_directory("social_bt_nodes");
  } catch (const std::exception & e) {
    if (node_) {
      RCLCPP_ERROR(node_->get_logger(), "Could not find package social_bt_nodes: %s", e.what());
    }
    return;
  }
  std::string yaml_path = pkg_share + "/config/locations.yaml";
  YAML::Node doc;
  try {
    doc = YAML::LoadFile(yaml_path);
  } catch (const YAML::Exception & e) {
    if (node_) {
      RCLCPP_ERROR(node_->get_logger(), "Could not load '%s': %s", yaml_path.c_str(), e.what());
    }
    return;
  }
  symbol_table_.clear();
  for (const auto & entry : doc) {
    if (!entry["location"] || !entry["keywords"]) continue;
    std::string frame = entry["location"].as<std::string>();
    std::vector<std::string> keywords_vec;
    for (const auto & kw : entry["keywords"]) {
      keywords_vec.push_back(kw.as<std::string>());
    }
    symbol_table_[frame] = keywords_vec;
  }
  if (node_) {
    RCLCPP_INFO(node_->get_logger(), "Loaded %zu locations from %s", symbol_table_.size(), yaml_path.c_str());
  }
}

BT::NodeStatus GetNavLocation::tick()
{
  std::string location_description;
  if (!getInput("location_description", location_description) || location_description.empty()) {
    if (node_) {
      RCLCPP_ERROR(
        node_->get_logger(),
        "GetNavLocation: missing required input 'location_description'");
    }
    return bt_failure(
      config(), registrationName(),
      "missing required input 'location_description', received: '" + location_description + "'",
      "bt_config_error");
  }

    // Replace all non-alphanumeric characters EXCEPT hyphen with spaces for robust tokenization
  std::string desc_lower = location_description;
  std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
  for (char &c : desc_lower) {
      if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-')) {
      c = ' ';
    }
  }
  std::vector<std::string> words;
  std::istringstream iss(desc_lower);
  std::string word;
  while (iss >> word) {
    words.push_back(word);
  }

  std::string found_frame;
  for (const auto & kv : symbol_table_) {
    const std::string & frame = kv.first;
    const std::vector<std::string> & keywords = kv.second;
    for (const auto & keyword : keywords) {
      std::string keyword_lower = keyword;
      std::transform(keyword_lower.begin(), keyword_lower.end(), keyword_lower.begin(), ::tolower);
      for (const auto & word : words) {
        if (!keyword_lower.empty() && word == keyword_lower) {
          found_frame = frame;
          break;
        }
      }
      if (!found_frame.empty()) break;
    }
    if (!found_frame.empty()) break;
  }

  if (!found_frame.empty()) {
    setOutput("location_frame", found_frame);
    if (node_) {
      RCLCPP_INFO(
        node_->get_logger(),
        "GetNavLocation: '%s' → frame='%s' (SUCCESS)",
        location_description.c_str(), found_frame.c_str());
    }
    return BT::NodeStatus::SUCCESS;
  } else {
    if (node_) {
      RCLCPP_WARN(
        node_->get_logger(),
        "GetNavLocation: no match for '%s' (FAILURE)",
        location_description.c_str());
    }
    return bt_failure(
      config(), registrationName(),
      "no matching location for input: '" + location_description + "'",
      "bt_location_not_found");
  }
}

}  // namespace social_bt_nodes
