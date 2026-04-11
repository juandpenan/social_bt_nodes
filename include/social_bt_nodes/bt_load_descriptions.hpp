// Copyright 2025 Rodrigo Pérez-Rodríguez
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOCIAL_BT_NODES__BT_LOAD_DESCRIPTIONS_HPP_
#define SOCIAL_BT_NODES__BT_LOAD_DESCRIPTIONS_HPP_

// ---------------------------------------------------------------------------
// bt_load_descriptions_from_yaml
//
// Alternative to the hardcoded bt_register_node_description() calls in
// bt_plugins.cpp.  Reads all node descriptions from the YAML file that
// llm_bt_builder also uses, so both components always see the same text.
//
// YAML format expected (same as node_descriptions/social_bt_nodes.yaml):
//
//   bt_nodes:
//     - name: "Speak"
//       description: "Synthesizes and speaks the specified text..."
//     - name: "Listen"
//       description: "Listens and transcribes speech..."
//
// Only the "name" and "description" fields are read; all other fields
// (ports, return values, type...) are ignored.
//
// Returns the number of descriptions successfully loaded, or -1 if the
// file could not be opened or parsed.
//
// ---------------------------------------------------------------------------
// HOW TO ACTIVATE (replacing the static bt_register_node_description calls)
// ---------------------------------------------------------------------------
//
// STEP 1 — CMakeLists.txt (social_bt_nodes):
//   Add the following find_package calls (if not already present):
//
//     find_package(yaml_cpp_vendor REQUIRED)
//     find_package(ament_index_cpp REQUIRED)
//
//   Add both to the dependencies set:
//
//     set(dependencies
//       ...
//       yaml_cpp_vendor
//       ament_index_cpp
//     )
//
// STEP 2 — bt_plugins.cpp:
//   a) Add includes at the top:
//
//     #include "ament_index_cpp/get_package_share_directory.hpp"
//     #include "social_bt_nodes/bt_load_descriptions.hpp"
//
//   b) Comment out ALL bt_register_node_description() lines inside
//      BT_REGISTER_NODES(factory), for example:
//
//     // social_bt_nodes::bt_register_node_description("Follow", ...);
//     // social_bt_nodes::bt_register_node_description("Speak", ...);
//     // ... (all 14 lines)
//
//      Keep all factory.registerNodeType<>() lines untouched.
//
//   c) At the END of BT_REGISTER_NODES(factory), add:
//
//     const std::string yaml_path =
//       ament_index_cpp::get_package_share_directory("social_bt_nodes") +
//       "/node_descriptions/social_bt_nodes.yaml";
//     social_bt_nodes::bt_load_descriptions_from_yaml(yaml_path);
//
// ---------------------------------------------------------------------------

#include <string>

#include "yaml-cpp/yaml.h"
#include "social_bt_nodes/bt_failure.hpp"

namespace social_bt_nodes
{

inline int bt_load_descriptions_from_yaml(const std::string & yaml_path)
{
  YAML::Node root;
  try {
    root = YAML::LoadFile(yaml_path);
  } catch (const YAML::Exception & e) {
    // File not found or malformed YAML — caller should log this.
    return -1;
  }

  // The top-level key is "bt_nodes", which is a sequence of node entries.
  const YAML::Node & nodes = root["bt_nodes"];
  if (!nodes || !nodes.IsSequence()) {
    return -1;
  }

  int count = 0;
  for (const auto & entry : nodes) {
    // Both "name" and "description" are required for a useful entry.
    if (!entry["name"] || !entry["description"]) {
      continue;
    }
    const std::string name = entry["name"].as<std::string>();
    const std::string description = entry["description"].as<std::string>();

    // Populate the same registry that bt_failure() queries at runtime.
    bt_register_node_description(name, description);
    ++count;
  }
  return count;
}

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_LOAD_DESCRIPTIONS_HPP_
