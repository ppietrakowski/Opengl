#include "sandbox_game_layer.h"

#include <functional>
#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

SandboxGameLayer::SandboxGameLayer() :
    camera_rotation_{glm::vec3{0, 0, 0}},
    camera_position_{0.0f, 0.0f, 0.0f}
{
    default_shader_ = ResourceManager::GetShader("shaders/default.shd");
    debug_shader_ = ResourceManager::GetShader("shaders/unshaded.shd");
    auto skeletal_shader = ResourceManager::GetShader("shaders/skeletal_default.shd");

    ResourceManager::CreateMaterial("shaders/default.shd", "default");

    test_skeletal_mesh_ = ResourceManager::GetSkeletalMesh("untitled.fbx");
    test_skeletal_mesh_->MainMaterial = ResourceManager::CreateMaterial("shaders/skeletal_default.shd", "skeletal1");
    current_used_shader_ = default_shader_;
    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_light_color", glm::vec3{1, 1, 1});
    current_used_shader_->SetUniform("u_light_pos", glm::vec3{-1, 0, -5});
    default_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    current_used_shader_->SetUniform("u_material.ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    current_used_shader_->SetUniform("u_material.specular", glm::vec3{0.797357, 0.723991, 0.208006});
    current_used_shader_->SetUniform("u_material.shininess", 87.2f);

    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    current_used_shader_->Use();

    default_shader_->SetUniform("u_projection_view", glm::identity<glm::mat4>());
    default_shader_->SetUniform("u_transform", glm::identity<glm::mat4>());
    glm::vec3 white{1.0f, 1.0f, 1.0f};

    default_shader_->SetUniform("u_material.Tint", white);

    debug_material_ = ResourceManager::CreateMaterial("shaders/unshaded.shd", "wireframe");
    default_material_ = ResourceManager::CreateMaterial("shaders/default.shd", "postac_material");

    default_material_->SetVector3Property("diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    default_material_->SetVector3Property("ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    default_material_->SetVector3Property("specular", glm::vec3{0.797357, 0.723991, 0.208006});
    default_material_->SetFloatProperty("shininess", 87.2f);
    debug_material_->bUseWireframe = true;
    current_material_ = default_material_;
    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    static_mesh_ = ResourceManager::GetStaticMesh("cube.obj");
    static_mesh_->MainMaterial = default_material_;


    std::shared_ptr<Material> material = test_skeletal_mesh_->MainMaterial;
    std::uint32_t textureIndex = 0;

    for (const auto& path : test_skeletal_mesh_->Textures)
    {
        std::string name = "diffuse" + std::to_string(textureIndex + 1);
        material->SetTextureProperty(name.c_str(), ResourceManager::GetTexture2D(path));
        ++textureIndex;
    }

    camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    static_mesh_position_ = {2, 0, -10};
    std::vector<std::string> animations = std::move(test_skeletal_mesh_->GetAnimationNames());
    RenderCommand::SetClearColor(RgbaColor{50, 30, 170});

    for (std::int32_t i = 0; i < 2; ++i)
    {
        skeletal_mesh_actor_ = level_.CreateActor("SkeletalMesh" + std::to_string(i));
        skeletal_mesh_actor_.AddComponent<SkeletalMeshComponent>(test_skeletal_mesh_);
        skeletal_mesh_actor_.GetComponent<TransformComponent>().Scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletal_mesh_actor_.GetComponent<TransformComponent>().Position = glm::vec3{0, -2 * i - 2, -i - 1};
    }

    Actor static_mesh_actor = level_.CreateActor("StaticMeshActor");
    static_mesh_actor.AddComponent<StaticMeshComponent>(static_mesh_);
    static_mesh_actor.GetComponent<TransformComponent>().SetEulerAngles(glm::vec3{0, 90, 0});

    bbox_min_ = test_skeletal_mesh_->GetBboxMin();
    bbox_max_ = test_skeletal_mesh_->GetBboxMax();
    instanced_mesh_ = std::make_shared<InstancedMesh>(static_mesh_);

    for (std::int32_t i = 0; i < 10; ++i)
    {
        for (std::int32_t j = 0; j < 20; ++j)
        {
            Transform transform{glm::vec3{5.0f * i, 2.0f, 3.0f * j}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
            instanced_mesh_->AddInstance(transform, 0);
            Renderer::DrawDebugBox(instanced_mesh_->GetMesh().GetBBoxMin(), instanced_mesh_->GetMesh().GetBBoxMax(), transform);
        }
    }

    m_Player = level_.CreateActor("Player");
    m_Player.AddComponent<PlayerController>(m_Player);

    PlayerController& controller = m_Player.GetComponent<PlayerController>();
    controller.BindForwardCallback(std::bind(&SandboxGameLayer::MoveForward, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindRightCallback(std::bind(&SandboxGameLayer::MoveRight, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindMouseMoveCallback(std::bind(&SandboxGameLayer::RotateCamera, this, std::placeholders::_1, std::placeholders::_2));
}

void SandboxGameLayer::Update(Duration delta_time)
{
    float dt = delta_time.GetSeconds();

    if (Input::IsKeyPressed(Keys::E))
    {
        camera_yaw_ -= yaw_rotation_rate_ * dt;
        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    } else if (Input::IsKeyPressed(Keys::Q))
    {
        camera_yaw_ += yaw_rotation_rate_ * dt;
        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    }

    if (Input::IsKeyPressed(Keys::Y))
    {
        glm::vec3 world_up = glm::vec3{0, 1, 0};
        camera_position_ += ascend_speed_ * world_up * dt;
    } else if (Input::IsKeyPressed(Keys::H))
    {
        glm::vec3 world_down = glm::vec3{0, -1, 0};
        camera_position_ += ascend_speed_ * world_down * dt;
    }
    last_delta_seconds_ = delta_time;

    level_.BroadcastUpdate(delta_time);
}

void SandboxGameLayer::Render(Duration delta_time)
{
    Transform cameraTransform = m_Player.GetComponent<TransformComponent>().GetAsTransform();

    Renderer::BeginScene(glm::inverse(glm::translate(cameraTransform.Position) * glm::mat4_cast(cameraTransform.Rotation)), cameraTransform.Position, cameraTransform.Rotation);
    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    instanced_mesh_->Draw(glm::identity<glm::mat4>(), *static_mesh_->MainMaterial);
    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Renderer::DrawDebugBox(static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax(), Transform{static_mesh_position_, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 0, 1});
    Renderer::DrawDebugBox(static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax(), Transform{static_mesh_position_ + glm::vec3{10, 0, 0}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 0, 1});
    Renderer::DrawDebugBox(test_skeletal_mesh_->GetBboxMin(), test_skeletal_mesh_->GetBboxMax(), Transform{glm::vec3{0, -2, -1}});

    Renderer::DrawDebugBox(bbox_min_, bbox_max_, Transform{glm::vec3{10, 2, 10}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 1, 1});
    level_.BroadcastRender(delta_time);
    Renderer::FlushDrawDebug(*debug_material_);
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (event.Type == EventType::MouseButtonPressed)
    {
        DO_ONCE([this]() {
            Actor actor = level_.FindActor("StaticMeshActor");
            actor.DestroyActor();

            instanced_mesh_->RemoveInstance(5);
        }
        );
    }

    if (event.Type == EventType::KeyPressed && event.Key.Key == Keys::P)
    {
        if (current_material_.get() == debug_material_.get())
        {
            current_material_ = default_material_;
        } else
        {
            current_material_ = debug_material_;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame()
{
    static std::int32_t last_framerate = 0;
    static std::int32_t num_frame{0};

    if (num_frame == 2)
    {
        last_framerate = (last_framerate + static_cast<std::int32_t>(1000 / last_delta_seconds_.GetMilliseconds())) / 2;
    } else
    {
        num_frame++;
    }

    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &static_mesh_position_[0], -10, 10);

    RenderStats stats = RenderCommand::GetRenderStats();

    ImGui::Text("Fps: %i", last_framerate);
    ImGui::Text("Frame time: %.2f ms", last_delta_seconds_.GetMilliseconds());
    ImGui::Text("Drawcalls: %u", stats.NumDrawcalls);
    ImGui::End();
}

void SandboxGameLayer::MoveForward(Actor& player, float axisValue)
{
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 forward = axisValue * transform.GetForwardVector() * last_delta_seconds_.GetSeconds() * move_speed_;
    transform.Translate(forward);
}

void SandboxGameLayer::MoveRight(Actor& player, float axisValue)
{
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 right = axisValue * transform.GetRightVector() * last_delta_seconds_.GetSeconds() * move_speed_;
    transform.Translate(right);
}

void SandboxGameLayer::RotateCamera(Actor& player, glm::vec2 mouseMoveDelta)
{
    float dt = last_delta_seconds_.GetSeconds();

    TransformComponent& transform = player.GetComponent<TransformComponent>();

    camera_yaw_ -= yaw_rotation_rate_ * mouseMoveDelta.x * dt;
    camera_pitch_ -= pitch_rotation_rate_ * mouseMoveDelta.y * dt;

    if (camera_pitch_ < -89)
    {
        camera_pitch_ = -89;
    }
    if (camera_pitch_ > 89)
    {
        camera_pitch_ = 89;
    }

    transform.SetEulerAngles(camera_pitch_, camera_yaw_, 0.0f);
}