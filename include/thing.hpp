#pragma once

#include <concepts>
#include <cstdint>
#include <deque>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <type_traits>
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
        }

        return Entity{_nextEntity++};
    }

    void killEntity(Entity entity)
    {
        _freeEntities.push_back(entity);
    }

private:
    std::deque<Entity> _freeEntities;
    Entity::ValueType _nextEntity = 0;
};

class UnknownTypeComponents {
public:
    virtual ~UnknownTypeComponents() = default;
    virtual void killEntity(Entity entity) = 0;
};

template <class Component>
class OneTypeComponents final : public UnknownTypeComponents {
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
        }

        return _components.at(iterator->second);
    }

    Component& add(Entity entity, Component&& component)
    {
        auto [iterator, inserted] =
            _entityIndex.insert({entity, _components.size()});
        if (inserted) {
            _components.push_back(std::forward<Component>(component));
            _entities.push_back(entity);
            return _components.back();
        }

        return _components.at(iterator->second);
    }

    void killEntity(Entity entity) override
    {
        size_t index = _entityIndex.at(entity);
        size_t size = _components.size();

        if (index + 1 < size) {
            std::swap(_components.at(index), _components.back());
            std::swap(_entities.at(index), _entities.back());
            _entityIndex.at(_entities.at(index)) = index;
        }
        _components.resize(size - 1);
        _entities.resize(size - 1);
        _entityIndex.erase(entity);
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
        return static_cast<const OneTypeComponents<Component>&>(
            *_components.at(std::type_index{typeid(Component)}));
    }

    template <class Component>
    OneTypeComponents<Component>& at()
    {
        return static_cast<OneTypeComponents<Component>&>(
            *_components.at(std::type_index{typeid(Component)}));
    }

    const UnknownTypeComponents& at(const std::type_index typeIndex) const
    {
        return *_components.at(typeIndex);
    }

    UnknownTypeComponents& at(const std::type_index typeIndex)
    {
        return *_components.at(typeIndex);
    }

    template <class Component>
    OneTypeComponents<Component>& create()
    {
        auto [iterator, inserted] = _components.insert({
            std::type_index{typeid(Component)},
            std::make_unique<OneTypeComponents<Component>>()});
        return *static_cast<OneTypeComponents<Component>*>(
            iterator->second.get());
    }

private:
    std::map<
        std::type_index,
        std::unique_ptr<UnknownTypeComponents>> _components;
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
        return _components.create<Component>().add(
            entity, std::forward<Component>(component));
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
