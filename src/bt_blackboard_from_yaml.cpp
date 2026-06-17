#include <cstdint>
#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "behaviortree_cpp/blackboard.h"
#include "yaml-cpp/yaml.h"

namespace
{

std::string to_lower_copy(std::string value)
{
  std::transform(
    value.begin(), value.end(), value.begin(),
    [](unsigned char c) {return static_cast<char>(std::tolower(c));});
  return value;
}

bool set_typed_value(
  BT::Blackboard::Ptr blackboard, const std::string & key, const YAML::Node & entry,
  std::string & error)
{
  if (!entry.IsMap()) {
    error = "entry must be a map with fields 'type' and 'value'";
    return false;
  }

  if (!entry["type"] || !entry["type"].IsScalar()) {
    error = "missing scalar field 'type'";
    return false;
  }

  if (!entry["value"]) {
    error = "missing field 'value'";
    return false;
  }

  const std::string type = to_lower_copy(entry["type"].as<std::string>());
  const YAML::Node value = entry["value"];

  try {
    if (type == "bool" || type == "boolean") {
      blackboard->set<bool>(key, value.as<bool>());
      std::cout << "[SET] " << key << " (bool) = " << (value.as<bool>() ? "true" : "false")
                << std::endl;
      return true;
    }

    if (type == "int" || type == "int64") {
      const int64_t parsed = value.as<int64_t>();
      blackboard->set<int64_t>(key, parsed);
      std::cout << "[SET] " << key << " (int64) = " << parsed << std::endl;
      return true;
    }

    if (type == "double" || type == "float") {
      const double parsed = value.as<double>();
      blackboard->set<double>(key, parsed);
      std::cout << "[SET] " << key << " (double) = " << parsed << std::endl;
      return true;
    }

    if (type == "string") {
      const std::string parsed = value.as<std::string>();
      blackboard->set<std::string>(key, parsed);
      std::cout << "[SET] " << key << " (string) = '" << parsed << "'" << std::endl;
      return true;
    }

    if (type == "yaml") {
      std::stringstream serialized;
      serialized << value;
      blackboard->set<std::string>(key, serialized.str());
      std::cout << "[SET] " << key << " (string, serialized YAML) = '" << serialized.str()
                << "'" << std::endl;
      return true;
    }

    error = "unsupported type '" + type + "' (supported: bool, int64, double, string, yaml)";
    return false;
  } catch (const std::exception & e) {
    error = "type conversion failed: " + std::string(e.what());
    return false;
  }
}

}  // namespace

int main(int argc, char ** argv)
{
  if (argc < 2) {
    std::cerr
      << "Usage: bt_blackboard_from_yaml <yaml_file>\n"
      << "Expected YAML format:\n"
      << "  some_key:\n"
      << "    type: string\n"
      << "    value: hello\n"
      << "  counter:\n"
      << "    type: int64\n"
      << "    value: 3\n";
    return 1;
  }

  const std::string yaml_path = argv[1];

  YAML::Node root;
  try {
    root = YAML::LoadFile(yaml_path);
  } catch (const YAML::Exception & e) {
    std::cerr << "Failed to read YAML file '" << yaml_path << "': " << e.what() << std::endl;
    return 1;
  }

  if (!root.IsMap()) {
    std::cerr << "YAML root must be a map of key/value pairs." << std::endl;
    return 1;
  }

  auto blackboard = BT::Blackboard::create();

  for (const auto & entry : root) {
    if (!entry.first.IsScalar()) {
      std::cerr << "Skipping non-scalar key in YAML map." << std::endl;
      continue;
    }

    const std::string key = entry.first.as<std::string>();
    const YAML::Node value = entry.second;

    std::string error;
    if (!set_typed_value(blackboard, key, value, error)) {
      std::cerr << "[SKIP] " << key << ": " << error << std::endl;
    }
  }

  std::cout << "\nBlackboard snapshot:" << std::endl;
  blackboard->debugMessage();

  return 0;
}