// Intended function: merge worker-produced gameplay commands by explicit stable keys so thread arrival order never becomes authoritative.
#include "ecs/GameplayCommandBuffer.hpp"
#include <algorithm>
namespace elysium {
void GameplayCommandBuffer::submit(GameplayCommand c){commands_.push_back(std::move(c));}
void GameplayCommandBuffer::append(std::vector<GameplayCommand> c){for(auto&v:c)commands_.push_back(std::move(v));}
std::vector<GameplayCommand> GameplayCommandBuffer::drainDeterministic(){
 std::stable_sort(commands_.begin(),commands_.end(),[](const GameplayCommand&a,const GameplayCommand&b){if(a.phase!=b.phase)return a.phase<b.phase;if(a.kind!=b.kind)return a.kind<b.kind;if(a.stableKey!=b.stableKey)return a.stableKey<b.stableKey;if(a.producerStableId!=b.producerStableId)return a.producerStableId<b.producerStableId;return a.localOrdinal<b.localOrdinal;});
 auto out=std::move(commands_);commands_.clear();return out;
}
} // namespace elysium
