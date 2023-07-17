#pragma once

#include <any>
#include <compare>
#include <concepts>
#include <cstdint>
#include <deque>
#include <iterator>
#include <map>
#include <set>
#include <span>
#include <typeindex>
#include <utility>
#include <vector>

namespace thing {

class Entity {
public:
    using ValueType = uint64_t;

    explicit Entity(ValueType id = 0) : _id(id) {}
    operator ValueType() const { return _id; }

    friend auto operator<=>(Entity lhs, Entity rhs)
    {
        return lhs._id <=> rhs._id;
    }

private:
    ValueType _id;
};

namespace internals {

class EntityPool {
public:
    Entity createEntity()
    {
        if (!_freeEntities.empty()) {
            auto result = _freeEntities.front();
            _freeEntities.pop_front();
            return result;
        } else {
            return Entity{_nextEntity++};
        }
    }

    void killEntity(Entity entity)
    {
        _freeEntities.push_back(entity);
    }

private:
    std::deque<Entity> _freeEntities;
    Entity::ValueType _nextEntity = 0;
};

class AbstractComponents {
public:
    virtual ~AbstractComponents() {}
    virtual void killEntity(Entity entity) = 0;
};

template <class Component>
class OneTypeComponents final : public AbstractComponents {
public:
    const Component& component(Entity entity) const
    {
        return _components.at(_entityIndex.at(entity));
    }

    Component& component(Entity entity)
    {
        return _components.at(_entityIndex.at(entity));
    }

    std::span<const Component> components() const
    {
        return _components;
    }

    std::span<Component> components()
    {
        return _components;
    }

    std::span<const Entity> entities() const
    {
        return _entities;
    }

    Component& add(Entity entity) requires std::default_initializable<Component>
    {
        auto [iterator, inserted] =
            _entityIndex.insert({entity, _components.size()});
        if (inserted) {
            _components.emplace_back();
            _entities.push_back(entity);
            return _components.back();
        } else {
            return _components.at(iterator->second);
        }
    }

    Component& add(Entity entity, Component&& component)
    {
        auto [iterator, inserted] =
            _entityIndex.insert({entity, _components.size()});
        if (inserted) {
            _components.push_back(std::move(component));
            _entities.push_back(entity);
            return _components.back();
        } else {
            Component& ref = _components.at(iterator->second);
            ref = std::move(component);
            return ref;
        }
    }

    void killEntity(Entity entity) override
    {
        if (auto it = _entityIndex.find(entity); it != _entityIndex.end()) {
            size_t index = it->second;
            Entity lastEntity = _entities.back();
            _entities.at(index) = lastEntity;
            _components.at(index) = std::move(_components.back());
            _entityIndex.at(lastEntity) = index;
            _entities.erase(std::prev(_entities.end()));
            _components.erase(std::prev(_components.end()));
        }
    }

private:
    std::vector<Component> _components;
    std::vector<Entity> _entities;
    std::map<Entity, size_t> _entityIndex;
};

class AnyTypeComponents {
public:
    template <class Component>
    bool has() const
    {
        return _components.contains(std::type_index{typeid(Component)});
    }

    template <class Component>
    const OneTypeComponents<Component>& at() const
    {
        return std::any_cast<const OneTypeComponents<Component>&>(
            _components.at(std::type_index{typeid(Component)}));
    }

    template <class Component>
    OneTypeComponents<Component>& at()
    {
        return std::any_cast<OneTypeComponents<Component>&>(
            _components.at(std::type_index{typeid(Component)}));
    }

    const AbstractComponents& at(const std::type_index typeIndex) const
    {
        return std::any_cast<const AbstractComponents&>(
            _components.at(typeIndex));
    }

    AbstractComponents& at(const std::type_index typeIndex)
    {
        return std::any_cast<AbstractComponents&>(
            _components.at(typeIndex));
    }

    template <class Component>
    OneTypeComponents<Component>& create()
    {
        auto [iterator, inserted] = _components.insert({
            std::type_index{typeid(Component)},
            std::make_any<OneTypeComponents<Component>>()});
        return std::any_cast<OneTypeComponents<Component>&>(iterator->second);
    }

private:
    std::map<std::type_index, std::any> _components;
};

} // namespace internals

class EntityManager {
public:
    template <class Component>
    const Component& component(Entity entity) const
    {
        return _components.at<Component>().component(entity);
    }

    template <class Component>
    Component& component(Entity entity)
    {
        return _components.at<Component>().component(entity);
    }

    template <class Component>
    std::span<const Component> components() const
    {
        if (!_components.has<Component>()) {
            return {};
        }
        return _components.at<Component>().components();
    }

    template <class Component>
    std::span<Component> components()
    {
        if (!_components.has<Component>()) {
            return {};
        }
        return _components.at<Component>().components();
    }

    template <class Component>
    std::span<const Entity> entities() const
    {
        if (!_components.has<Component>()) {
            return {};
        }
        return _components.at<Component>().entities();
    }

    template <class Component>
    Component& add(Entity entity)
    {
        _entityComponentTypeIndex[entity].insert(
            std::type_index{typeid(Component)});
        return _components.create<Component>().add(entity);
    }

    template <class Component>
    Component& add(Entity entity, Component&& component)
    {
        _entityComponentTypeIndex[entity].insert(
            std::type_index{typeid(Component)});
        return _components.create<Component>().add(entity, std::move(component));
    }

    Entity createEntity()
    {
        return _entityPool.createEntity();
    }

    void killEntity(Entity entity)
    {
        _entityPool.killEntity(entity);
        for (const auto& typeIndex : _entityComponentTypeIndex.at(entity)) {
            _components.at(typeIndex).killEntity(entity);
        }
    }

private:
    internals::EntityPool _entityPool;
    internals::AnyTypeComponents _components;
    std::map<Entity, std::set<std::type_index>> _entityComponentTypeIndex;
};

} // namespace thing
