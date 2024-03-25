#pragma once

#include "Object.hpp"
#include "LevelInterface.hpp"
#include "Box.hpp"

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>

enum EntityFlag : uint32_t
{
    EFL_NO_FLAGS = 0,
    EFL_PENDING_KILL = 1 << 0,
    EFL_DONT_DESTROY = 1 << 1
};

struct Basis
{
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
};

class BaseEntity : public Object, public std::enable_shared_from_this<BaseEntity>
{
    using KeyValuePackage = std::map<std::string, Datapack>;

public:
    DEFINE_CLASS_BASE(BaseEntity, Object);
    virtual ~BaseEntity();

public:
    enum
    {
        MAX_ENTITY_NAME_LENGTH = 256
    };

    void Activate();
    void Deactivate();

    virtual bool KeyValue(std::string_view keyName, std::string_view value);
    virtual bool KeyValue(std::string_view keyName, float value);
    virtual bool KeyValue(std::string_view keyName, const glm::vec3& value);

    virtual bool GetKeyValue(std::string_view keyName, Datapack& outKeyValue);

    virtual bool Init(int entityNum, std::weak_ptr<LevelInterface> world);

    /* IRenderable */
public:

    glm::vec3 GetRenderOrigin() const;
    glm::quat GetRenderAngles() const;
    bool ShouldDraw() const;
    const glm::mat4& GetWorldSpaceTransformFromRenderable() const;

    virtual bool GetColorModulation(glm::vec4& color);
    virtual void SetColorModulation(const glm::vec4& color);

    virtual float GetDamageScale() const;

    virtual EntityFlag GetEntityFlags() const;
    virtual void SetEntityFlags(EntityFlag flag);

    void AddEntityFlag(EntityFlag flag);
    void RemoveEntityFlag(EntityFlag flag);
    bool IsFlagSet(EntityFlag flag) const;

    virtual int GetEntityIndex() const;
    bool IsMarkedForDestruction() const;

    virtual void OnSave();
    virtual void OnRestore();

    virtual void AddToVisibleEntityList();

    virtual glm::vec3 GetWorldOrigin() const;
    virtual glm::quat GetWorldAngles() const;

    virtual glm::vec3 GetLocalOrigin() const;
    virtual void SetLocalOrigin(glm::vec3 origin);
    virtual glm::quat GetLocalAngles() const;
    virtual void SetLocalAngles(float pitch, float yaw, float roll);

    virtual void SetLocalTransform(const glm::mat4& matrix);

    virtual glm::vec3 GetWorldAlignMin() const;
    virtual glm::vec3 GetWorldAlignMax() const;

    void SetBaseCollisionBounds(glm::vec3 mins, glm::vec3 maxs);

    float GetBoundingRadius() const;

    const glm::mat4& EntityToWorldTransform() const;

    void EntityToWorldSpace(glm::vec3 in, glm::vec3& out) const;
    void WorldToEntitySpace(glm::vec3 in, glm::vec3& out) const;

    void GetVectors(Basis& outBasis) const;

    bool IsSimulatedEveryTick() const;
    bool IsAnimatedEveryTick() const;
    void SetSimulatedEveryTick(bool sim);
    void SetAnimatedEveryTick(bool anim);

    virtual void SetHealth(int health);
    virtual int GetHealth() const;

    virtual int GetMaxHealth() const;
    float GetHealthFraction() const;

    void UnlinkChild(std::shared_ptr<BaseEntity> child);
    void LinkChild(std::shared_ptr<BaseEntity>  child);
    void UnlinkFromHierarchy();

    std::shared_ptr<BaseEntity> GetOwnerEntity() const;
    std::weak_ptr<LevelInterface> GetOwningWorld() const;

    virtual bool IsAlive();

    static std::shared_ptr<BaseEntity> GetInstance(int entityId);

public:

    // Inherited via ICollideable
    std::weak_ptr<BaseEntity> GetEntityHandle();
    glm::vec3 GetOBBMinsPreScaled() const;
    glm::vec3 GetOBBMaxsPreScaled() const;
    glm::vec3 GetOBBMins() const;
    glm::vec3 GetOBBMaxs() const;

public:
    virtual void Spawned();
    virtual void OnDestruct();
    virtual void Tick(float dt);
    virtual void OnActivate();
    virtual void OnDeactivate();

public:
    glm::vec3 RotationOrigin{0.0f, 0.0f, 0.0f};

protected:
    glm::vec3 m_BoxMin{0, 0, 0};
    glm::vec3 m_BoxMax{0, 0, 0};

    Box m_ScaledBox;
    float m_BoundingRadius{0.0f};
    bool m_bAlreadySpawned : 1{false};
    bool m_bTickable : 1{false};

protected:
    virtual void InitializeSubObject();

private:
    std::vector<std::weak_ptr<BaseEntity>> m_Children;
    std::weak_ptr<BaseEntity> m_Parent;

    uint32_t m_Flags{EFL_NO_FLAGS};
    int m_EntityId{-1};
    int m_Health{100};

    bool m_bAnimationTickable : 1{false};

    glm::vec3 m_LocalOrigin{0.0f, 0.0f, 0.0f};
    glm::quat m_LocalRotation{};
    glm::vec3 m_LocalScale;

    mutable bool m_bNeedUpdateWorldValues{false};
    glm::vec3 m_WorldOrigin{0.0f, 0.0f, 0.0f};
    glm::quat m_WorldRotation{};
    glm::vec3 m_WorldScale{1.0f, 1.0f, 1.0f};
    glm::mat4 m_WorldMatrix{1.0f};
    glm::mat4 m_LocalMatrix{1.0f};

    glm::vec3 m_ForwardVector{0, 0, -1};
    glm::vec3 m_RightVector{1, 0, 0};
    glm::vec3 m_UpVector{0, 1, 0};

    std::weak_ptr<LevelInterface> m_World;

private:
    void UpdateWorldTransform();
    void MarkUpdateWorldTransformNeedUpdate();

    glm::mat4 GetTransformMatrix();
};

constexpr size_t sz = sizeof(BaseEntity);