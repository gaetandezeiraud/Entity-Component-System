// Implementation of Entity template methods
// Included from Registry.h after Registry is defined

namespace ECS
{

    template<typename T, typename ...TArgs>
    void Entity::AddComponent(TArgs&& ...args)
    {
        registry->template AddComponent<T>(*this, std::forward<TArgs>(args)...);
    }

    template<typename T>
    void Entity::RemoveComponent()
    {
        registry->template RemoveComponent<T>(*this);
    }

    template<typename T>
    bool Entity::HasComponent() const
    {
        return registry->template HasComponent<T>(*this);
    }

    template<typename T>
    T& Entity::GetComponent() const
    {
        return registry->template GetComponent<T>(*this);
    }

}
