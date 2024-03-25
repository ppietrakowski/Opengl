#include "Entity.hpp"
#include <glm/gtx/matrix_decompose.hpp>

IMPLEMENT_CLASS_BASE(BaseEntity, Object);

BaseEntity::~BaseEntity()
{
}

void BaseEntity::Activate()
{
}

void BaseEntity::Deactivate()
{
}

bool BaseEntity::KeyValue(std::string_view keyName, std::string_view value)
{
    if (keyName == "name" || keyName == "Name")
    {
        SetObjectName(value);
        return true;
    }

    return false;
}

bool BaseEntity::KeyValue(std::string_view keyName, float value)
{
    return true;
}

bool BaseEntity::KeyValue(std::string_view keyName, const glm::vec3& value)
{
    if (keyName == "WorldPosition")
    {
    }

    return true;
}

bool BaseEntity::GetKeyValue(std::string_view keyName, Datapack& outKeyValue)
{
    if (keyName == "name" || keyName == "Name")
    {
        outKeyValue = GetObjectNameStr();

        return true;
    }

    return false;
}

bool BaseEntity::Init(int entityNum, std::weak_ptr<LevelInterface> world)
{
    assert(!world.expired());

    m_Children.reserve(120);
    m_EntityId = entityNum;
    m_World = world;

    auto w = world.lock();

    InitializeSubObject();

    if (m_bTickable)
    {
        //m_World->AddEntityToTickList(this);
        //m_World->AddEntityToRenderList(this);
    }

    return true;
}

#define FORCE_UPDATE_WORLD_TRANSFORM() const_cast<BaseEntity*>(this)->UpdateWorldTransform()

glm::vec3 BaseEntity::GetWorldOrigin() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_WorldOrigin;
}

glm::quat BaseEntity::GetWorldAngles() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_WorldRotation;
}

glm::vec3 BaseEntity::GetLocalOrigin() const
{
    return m_LocalOrigin;
}

void BaseEntity::SetLocalOrigin(glm::vec3 origin)
{
    m_LocalOrigin = origin;
    MarkUpdateWorldTransformNeedUpdate();
}

glm::quat BaseEntity::GetLocalAngles() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_LocalRotation;
}

void BaseEntity::SetLocalAngles(float pitch, float yaw, float roll)
{
    m_LocalRotation = glm::radians(glm::vec3(pitch, yaw, roll));
    MarkUpdateWorldTransformNeedUpdate();
}

void BaseEntity::SetLocalTransform(const glm::mat4& matrix)
{
    glm::vec3 dummy;
    glm::vec4 dummy2;
    glm::decompose(matrix, m_LocalScale, m_LocalRotation, m_WorldOrigin, dummy, dummy2);
    MarkUpdateWorldTransformNeedUpdate();
}

glm::vec3 BaseEntity::GetWorldAlignMin() const
{
    return m_ScaledBox.MinBounds;
}

glm::vec3 BaseEntity::GetWorldAlignMax() const
{
    return m_ScaledBox.MaxBounds;
}

void BaseEntity::SetBaseCollisionBounds(glm::vec3 mins, glm::vec3 maxs)
{
    m_BoxMin = mins;
    m_BoxMax = maxs;

    m_ScaledBox = {m_BoxMin, m_BoxMax};
    m_ScaledBox = m_ScaledBox.TransformedBy(m_WorldMatrix);
}

float BaseEntity::GetBoundingRadius() const
{
    return m_BoundingRadius;
}

const glm::mat4& BaseEntity::EntityToWorldTransform() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_LocalMatrix;
}

void BaseEntity::EntityToWorldSpace(glm::vec3 in, glm::vec3& out) const
{
    out = EntityToWorldTransform() * glm::vec4(in, 1);
}

void BaseEntity::WorldToEntitySpace(glm::vec3 in, glm::vec3& out) const
{
    out = glm::inverse(EntityToWorldTransform()) * glm::vec4(in, 1);
}

void BaseEntity::GetVectors(Basis& outBasis) const
{
    outBasis.forward = m_ForwardVector;
    outBasis.right = m_RightVector;
    outBasis.up = m_UpVector;
}

bool BaseEntity::IsSimulatedEveryTick() const
{
    return m_bTickable;
}

bool BaseEntity::IsAnimatedEveryTick() const
{
    return m_bAnimationTickable;
}

void BaseEntity::SetSimulatedEveryTick(bool sim)
{
#if 0
    if (m_bTickable)
    {
        if (m_bTickable != sim)
        {
            world->RemoveEntityFromTickList(this);
            world->RemoveEntityFromRenderList(this);
        }
    }
    else
    {
        if (m_bTickable != sim)
        {
            world->AddEntityToTickList(this);
            world->AddEntityToRenderList(this);
        }
    }

#endif
    m_bTickable = sim;
}

void BaseEntity::SetAnimatedEveryTick(bool anim)
{
    m_bAnimationTickable = anim;
}

void BaseEntity::SetHealth(int health)
{
    m_Health = health;
}

int BaseEntity::GetHealth() const
{
    return m_Health;
}

int BaseEntity::GetMaxHealth() const
{
    return 100;
}

float BaseEntity::GetHealthFraction() const
{
    int maxHealth = GetMaxHealth();

    if (maxHealth != 0)
    {
        return static_cast<float>(GetHealth()) / static_cast<float>(maxHealth);
    }

    return 0.0f;
}

void BaseEntity::UnlinkChild(std::shared_ptr<BaseEntity> child)
{
    assert(child);
    assert(this != child.get());
    assert(this == child->m_Parent.lock().get());

    auto it = std::find_if(m_Children.begin(), m_Children.end(), [&child](std::weak_ptr<BaseEntity>& entity)
    {
        return entity.lock() == child;
    });

    if (it != m_Children.end())
    {
        m_Children.erase(it);
        child->m_Parent = {};
    }
}

void BaseEntity::LinkChild(std::shared_ptr<BaseEntity> child)
{
    assert(child);
    assert(this != child.get());

    m_Children.push_back(child);
    child->m_Parent = GetInstance(m_EntityId);
}

void BaseEntity::UnlinkFromHierarchy()
{
    if (!m_Parent.expired())
    {
        m_Parent.lock()->UnlinkChild(GetInstance(m_EntityId));
    }
}

std::shared_ptr<BaseEntity> BaseEntity::GetOwnerEntity() const
{
    if (!m_Parent.expired())
    {
        return m_Parent.lock();
    }

    return {};
}

std::weak_ptr<LevelInterface> BaseEntity::GetOwningWorld() const
{
    return m_World;
}

bool BaseEntity::IsAlive()
{
    return !(m_Flags & EFL_PENDING_KILL);
}

std::shared_ptr<BaseEntity> BaseEntity::GetInstance(int entityId)
{
    //IEntityContainer* world = IEntityContainer::GetInstance();
    //return world->GetEntityById(entityId).lock();
    return {};
}

void BaseEntity::Spawned()
{
}

void BaseEntity::OnDestruct()
{
}

void BaseEntity::Tick(float dt)
{
}

void BaseEntity::OnActivate()
{
}

void BaseEntity::OnDeactivate()
{
}

void BaseEntity::InitializeSubObject()
{
}

void BaseEntity::UpdateWorldTransform()
{
    if (!m_bNeedUpdateWorldValues)
    {
        return;
    }

    glm::mat4 rotation = glm::translate(glm::mat4{1.0f}, RotationOrigin);

    rotation = glm::mat4_cast(m_LocalRotation) * rotation;

    rotation = glm::translate(rotation, -RotationOrigin);

    m_LocalMatrix = glm::translate(glm::mat4{1.0f}, m_LocalOrigin) * rotation * glm::scale(m_LocalScale);

    m_WorldMatrix = GetTransformMatrix();

    glm::vec3 dummy;
    glm::vec4 dummy2;
    glm::decompose(m_WorldMatrix, m_WorldScale, m_WorldRotation, m_WorldOrigin, dummy, dummy2);

    m_ForwardVector = glm::normalize(glm::vec3(-m_WorldMatrix[2]));
    m_RightVector = glm::normalize(glm::vec3(m_WorldMatrix[0]));
    m_UpVector = glm::normalize(glm::vec3(m_WorldMatrix[1]));

    m_bNeedUpdateWorldValues = false;
}

glm::mat4 BaseEntity::GetTransformMatrix()
{
    FORCE_UPDATE_WORLD_TRANSFORM();

    if (auto parent = m_Parent.lock(); parent)
    {
        return parent->GetTransformMatrix() * m_LocalMatrix;
    }

    return m_LocalMatrix;
}

void BaseEntity::MarkUpdateWorldTransformNeedUpdate()
{
    for (auto child : m_Children)
    {
        if (auto ch = child.lock(); ch)
        {
            ch->m_bNeedUpdateWorldValues = true;
        }
    }

    m_bNeedUpdateWorldValues = true;
}

std::weak_ptr<BaseEntity> BaseEntity::GetEntityHandle()
{
    return GetInstance(m_EntityId);
}

glm::vec3 BaseEntity::GetOBBMinsPreScaled() const
{
    return m_BoxMin;
}

glm::vec3 BaseEntity::GetOBBMaxsPreScaled() const
{
    return m_BoxMax;
}

glm::vec3 BaseEntity::GetOBBMins() const
{
    return m_ScaledBox.MinBounds;
}

glm::vec3 BaseEntity::GetOBBMaxs() const
{
    return m_ScaledBox.MaxBounds;
}

glm::vec3 BaseEntity::GetRenderOrigin() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_WorldOrigin;
}

glm::quat BaseEntity::GetRenderAngles() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_WorldRotation;
}

bool BaseEntity::ShouldDraw() const
{
    return true;
}

const glm::mat4& BaseEntity::GetWorldSpaceTransformFromRenderable() const
{
    FORCE_UPDATE_WORLD_TRANSFORM();
    return m_LocalMatrix;
}

bool BaseEntity::GetColorModulation(glm::vec4& color)
{
    return false;
}

void BaseEntity::SetColorModulation(const glm::vec4& color)
{
}

float BaseEntity::GetDamageScale() const
{
    return 1.0f;
}

EntityFlag BaseEntity::GetEntityFlags() const
{
    return static_cast<EntityFlag>(m_Flags);
}

void BaseEntity::SetEntityFlags(EntityFlag flag)
{
    m_Flags = static_cast<uint32_t>(flag);
}

void BaseEntity::AddEntityFlag(EntityFlag flag)
{
    m_Flags = static_cast<uint32_t>(m_Flags | flag);
}

void BaseEntity::RemoveEntityFlag(EntityFlag flag)
{
    m_Flags = static_cast<uint32_t>(m_Flags & (~flag));
}

bool BaseEntity::IsFlagSet(EntityFlag flag) const
{
    return static_cast<bool>(m_Flags & flag);
}

int BaseEntity::GetEntityIndex() const
{
    return m_EntityId;
}

bool BaseEntity::IsMarkedForDestruction() const
{
    return m_Flags & EFL_PENDING_KILL;
}

void BaseEntity::OnSave()
{
}

void BaseEntity::OnRestore()
{
}

void BaseEntity::AddToVisibleEntityList()
{
}
