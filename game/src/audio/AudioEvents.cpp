// Intended function: deduplicate and priority-sort transient sound cues while guaranteeing audio never becomes gameplay authority.
#include "audio/AudioEvents.hpp"
#include <algorithm>
#include <unordered_set>
namespace elysium{void AudioEventQueue::emit(AudioEvent e){events_.push_back(e);}std::vector<AudioEvent>AudioEventQueue::drain(std::size_t max){std::sort(events_.begin(),events_.end(),[](auto&a,auto&b){if(a.priority!=b.priority)return a.priority>b.priority;if(a.kind!=b.kind)return a.kind<b.kind;return a.eventId<b.eventId;});std::unordered_set<std::uint64_t>seen;std::vector<AudioEvent>o;for(auto&e:events_)if(seen.insert(e.eventId).second&&o.size()<max)o.push_back(e);events_.clear();return o;}}
