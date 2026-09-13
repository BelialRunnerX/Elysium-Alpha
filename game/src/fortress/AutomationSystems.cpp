#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

PowerAllocationResult allocatePower(std::span<PowerNode> nodes, float dt) {
    PowerAllocationResult result{};
    for (const auto& node : nodes) {
        if (node.isolated) continue;
        result.generated += std::max(0.0f, node.generation);
        result.demand += std::max(0.0f, node.demand);
    }

    float available = result.generated;
    for (auto& node : nodes) {
        if (node.isolated) continue;
        available += std::min(node.storage, std::max(0.0f, node.demand) * std::max(0.0f, dt));
    }

    std::vector<std::size_t> order(nodes.size());
    for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        if (nodes[a].priority != nodes[b].priority) return nodes[a].priority < nodes[b].priority;
        return nodes[a].id.value < nodes[b].id.value;
    });

    for (const auto index : order) {
        auto& node = nodes[index];
        if (node.isolated || node.demand <= 0.0f) {
            node.powered = false;
            continue;
        }
        if (available >= node.demand) {
            available -= node.demand;
            result.served += node.demand;
            node.powered = true;
        } else {
            node.powered = false;
            result.shedNodes.push_back(node.id);
        }
    }

    const float surplus = std::max(0.0f, available);
    if (surplus > 0.0f) {
        float remaining = surplus * std::max(0.0f, dt);
        for (auto& node : nodes) {
            if (node.isolated || node.storageCapacity <= node.storage || remaining <= 0.0f) continue;
            const float accepted = std::min(remaining, node.storageCapacity - node.storage);
            node.storage += accepted;
            remaining -= accepted;
            result.storedDelta += accepted;
        }
    }
    return result;
}

bool evaluateAutomationRule(AutomationRule& rule, std::span<const AutomationSignal> signals) {
    if (!rule.enabled) return false;
    const auto it = std::find_if(signals.begin(), signals.end(), [&](const AutomationSignal& signal) {
        return signal.name == rule.trigger;
    });
    if (it == signals.end()) return false;

    bool fired = it->boolean || it->numeric >= rule.threshold;
    if (rule.condition.value.find("below") != std::string::npos) fired = it->numeric <= rule.threshold;
    if (rule.condition.value.find("transition") != std::string::npos) {
        const bool edge = fired && !rule.latched;
        rule.latched = fired;
        return edge;
    }
    rule.latched = fired;
    return fired;
}

void advanceRailCart(RailCartState& cart, float routeLength, float traction, float dt) {
    if (cart.derailed || routeLength <= 0.0f) return;
    const float massPenalty = 1.0f / (1.0f + std::max(0.0f, cart.mass) * 0.002f);
    const float targetSpeed = 12.0f * saturate(traction) * massPenalty;
    cart.speed += (targetSpeed - cart.speed) * saturate(std::max(0.0f, dt) * 0.8f);
    cart.brakingDistance = cart.speed * cart.speed / std::max(0.5f, 2.0f * 4.0f * saturate(traction));
    cart.progress = saturate(cart.progress + cart.speed * std::max(0.0f, dt) / routeLength);
}

} // namespace elysium::fortress
