// Intended function: imported tests implementation for entt; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace entt {

using entity=std::uint32_t;
inline constexpr entity null=0;

class registry {
    struct IStorage {
        virtual ~IStorage()=default;
        virtual void erase(entity)=0;
        virtual bool contains(entity) const=0;
    };
    template<class T>
    struct Storage final : IStorage {
        std::map<entity,T> values;
        void erase(entity e) override { values.erase(e); }
        bool contains(entity e) const override { return values.contains(e); }
    };

    template<class T> Storage<T>& assure() {
        const std::type_index key(typeid(T));
        auto it=storages_.find(key);
        if(it==storages_.end()) {
            auto storage=std::make_unique<Storage<T>>();
            auto* raw=storage.get();
            storages_.emplace(key,std::move(storage));
            return *raw;
        }
        return *static_cast<Storage<T>*>(it->second.get());
    }
    template<class T> const Storage<T>* findStorage() const {
        const auto it=storages_.find(std::type_index(typeid(T)));
        if(it==storages_.end()) return nullptr;
        return static_cast<const Storage<T>*>(it->second.get());
    }
    template<class T> bool has(entity e) const {
        const auto* s=findStorage<T>();
        return s && s->contains(e);
    }

public:
    entity create() {
        const entity e=++next_;
        alive_.insert(e);
        return e;
    }
    bool valid(entity e) const { return e!=null && alive_.contains(e); }
    void destroy(entity e) {
        alive_.erase(e);
        for(auto& [_,storage]:storages_) storage->erase(e);
    }

    template<class T,class... Args>
    T& emplace(entity e,Args&&... args) {
        if(!valid(e)) throw std::runtime_error("entt stub emplace on invalid entity");
        auto& storage=assure<T>().values;
        auto [it,inserted]=storage.emplace(e,T{std::forward<Args>(args)...});
        if(!inserted) it->second=T{std::forward<Args>(args)...};
        return it->second;
    }
    template<class T> T& get(entity e) { return assure<T>().values.at(e); }
    template<class T> const T& get(entity e) const {
        const auto* storage=findStorage<T>();
        if(!storage) throw std::out_of_range("missing component storage");
        return storage->values.at(e);
    }
    template<class... Ts> bool all_of(entity e) const { return (has<Ts>(e) && ...); }
    template<class T,class... Args>
    T& emplace_or_replace(entity e,Args&&... args) {
        auto& storage=assure<T>().values;
        auto it=storage.find(e);
        if(it==storage.end()) return emplace<T>(e,std::forward<Args>(args)...);
        it->second=T{std::forward<Args>(args)...};
        return it->second;
    }
    template<class T,class... Args>
    T& get_or_emplace(entity e,Args&&... args) {
        auto& storage=assure<T>().values;
        auto it=storage.find(e);
        if(it!=storage.end()) return it->second;
        return emplace<T>(e,std::forward<Args>(args)...);
    }
    template<class T> std::size_t remove(entity e) { return assure<T>().values.erase(e); }
    template<class T> T* try_get(entity e) {
        auto& storage=assure<T>().values;
        auto it=storage.find(e);
        return it==storage.end()?nullptr:&it->second;
    }
    template<class T> const T* try_get(entity e) const {
        const auto* storage=findStorage<T>();
        if(!storage) return nullptr;
        auto it=storage->values.find(e);
        return it==storage->values.end()?nullptr:&it->second;
    }

    template<class... Ts>
    class basic_view {
    public:
        basic_view(registry* registry,std::vector<entity> entities)
            : registry_(registry),entities_(std::move(entities)) {}
        auto begin(){return entities_.begin();}
        auto end(){return entities_.end();}
        auto begin() const{return entities_.begin();}
        auto end() const{return entities_.end();}
        template<class T> T& get(entity e){return registry_->template get<T>(e);}
        template<class T> const T& get(entity e) const{return registry_->template get<T>(e);}
    private:
        registry* registry_{};
        std::vector<entity> entities_;
    };

    template<class... Ts>
    basic_view<Ts...> view() {
        std::vector<entity> entities;
        for(const auto e:alive_) if((has<Ts>(e)&&...)) entities.push_back(e);
        return basic_view<Ts...>(this,std::move(entities));
    }
    template<class... Ts>
    basic_view<Ts...> view() const {
        return const_cast<registry*>(this)->template view<Ts...>();
    }

private:
    entity next_{};
    std::set<entity> alive_;
    std::unordered_map<std::type_index,std::unique_ptr<IStorage>> storages_;
};

} // namespace entt
