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

#ifndef SOCIAL_BT_NODES__BT_FAILURE_HPP_
#define SOCIAL_BT_NODES__BT_FAILURE_HPP_

#include <string>
#include <unordered_map>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/tree_node.h"

namespace social_bt_nodes
{

// ---------------------------------------------------------------------------
// Description registry
//
// A process-wide map from BT registration name (e.g. "Speak") to the
// human-readable description of that node type (e.g. "Synthesizes and
// speaks the specified text using a TTS service.").
//
// Implementation note: the map is a function-local static, which in C++
// guarantees a single instance per process (Meyers singleton). Every
// translation unit that includes this header shares the same map at
// runtime because inline functions with a static local have a unique
// definition across the whole shared library.
// ---------------------------------------------------------------------------
inline std::unordered_map<std::string, std::string> & bt_node_description_registry()
{
  static std::unordered_map<std::string, std::string> reg;
  return reg;
}

// ---------------------------------------------------------------------------
// bt_register_node_description
//
// Inserts one entry into the registry.
// Must be called from BT_REGISTER_NODES (bt_plugins.cpp) right after each
// factory.registerNodeType<T>("Name") call:
//
//   factory.registerNodeType<social_bt_nodes::Speak>("Speak");
//   bt_register_node_description("Speak", social_bt_nodes::Speak::node_description);
//
// "Name" must match:
//   - The registration name passed to registerNodeType<T>()
//   - The node name in the YAML description file used by llm_bt_builder
//   - The value returned by registrationName() inside the node at runtime
// ---------------------------------------------------------------------------
inline void bt_register_node_description(
  const std::string & registration_name, const std::string & description)
{
  bt_node_description_registry()[registration_name] = description;
}

// ---------------------------------------------------------------------------
// bt_failure
//
// Helper to be called at every FAILURE return point inside a BT node.
// It writes a structured string to the shared blackboard key
// "bt_last_failure" so that the LLM plan orchestrator can read it and
// decide how to replan.
//
// Arguments:
//   cfg       - result of config() inside the node; gives access to the
//               blackboard shared by the whole BT.
//   node_name - use registrationName() (not name()) so the key always
//               matches the registry regardless of custom XML name= attrs.
//   reason    - human-readable explanation of why this execution failed.
//
// The message written to the blackboard has the form:
//   "node: Speak; description: Synthesizes...; failure: service not available"
// or, if the node was not registered (should not happen in normal use):
//   "node: Speak; failure: service not available"
//
// Returns BT::NodeStatus::FAILURE so callers can write:
//   return bt_failure(config(), registrationName(), "reason");
// ---------------------------------------------------------------------------
inline BT::NodeStatus bt_failure(
  const BT::NodeConfig & cfg,
  const std::string & node_name,
  const std::string & reason)
{
  if (cfg.blackboard) {
    // Look up the description registered at plugin load time.
    const auto & reg = bt_node_description_registry();
    auto it = reg.find(node_name);
    std::string msg;
    if (it != reg.end() && !it->second.empty()) {
      msg = "node: " + node_name + "; description: " + it->second + "; failure: " + reason;
    } else {
      msg = "node: " + node_name + "; failure: " + reason;
    }
    // Overwrite the shared key. The LLM orchestrator polls this key
    // after the tree returns FAILURE to build the replanning prompt.
    cfg.blackboard->set<std::string>("bt_last_failure", msg);
  }
  return BT::NodeStatus::FAILURE;
}

}  // namespace social_bt_nodes

#endif  // SOCIAL_BT_NODES__BT_FAILURE_HPP_
