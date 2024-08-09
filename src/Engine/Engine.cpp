#include "Engine.h"
#include <chrono>

Engine::Engine(const int window_width, const int window_height, const char* window_title)
    : window_{window_width, window_height, window_title}, camera_{{0.0f, 0.0f, -5.0f}, glm::radians(90.0f)}, GUI_{} {

    camera_.Rotate(0.0f, glm::radians(180.0f), 0.0f);
    camera_.camera_rotation_X = glm::radians(180.0f);

    chunks_ = new Chunks(3, {0, 8, 0});

    if (chunks_ == nullptr) {
        std::cout << "Bad alloc: chunks_ (Engine)" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glfwSwapInterval(1);
}

void Engine::MainLoop() {

    double last_time = glfwGetTime();
    double delta_time = 0.0f;
    double current_time = 0.0f;
    double total_time = 0.0;
    int frame_count = 0;

    float speed = 15.0f;

    bool switcher = false;
    while (!window_.IsShouldClose()) {
        current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        // std::cout << 1 / delta_time << '\n';

        if (!window_.is_iconfied) {
            if (Events::KeyIsClicked(GLFW_KEY_ESCAPE)) {
                window_.SetShouldClose(true);
            }
            if (Events::KeyIsClicked(GLFW_KEY_TAB)) {
                Events::SwitchCursor();
            }

            if (Events::KeyIsPressed(GLFW_KEY_W)) {
                camera_.position += camera_.vector_front * delta_time * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_S)) {
                camera_.position -= camera_.vector_front * delta_time * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_D)) {
                camera_.position += camera_.vector_right * delta_time * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_A)) {
                camera_.position -= camera_.vector_right * delta_time * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_SPACE)) {
                camera_.position.y += delta_time * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_LEFT_SHIFT)) {
                camera_.position.y -= delta_time * speed;
            }

            if (window_.is_resized) {
                GUI_.crosshair.UpdateModel();
                window_.is_resized = false;
            }

            if (Events::cursor_is_locked) {
                camera_.camera_rotation_X += -2 * Events::cursor_delta_x / window_.height;
                camera_.camera_rotation_Y += -2 * Events::cursor_delta_y / window_.height;

                if (camera_.camera_rotation_Y < -glm::radians(90.0f)) {
                    camera_.camera_rotation_Y = -glm::radians(90.0f);
                } else if (camera_.camera_rotation_Y > glm::radians(90.0f)) {
                    camera_.camera_rotation_Y = glm::radians(90.0f);
                }

                camera_.rotation = glm::mat4(1.0f);
                camera_.Rotate(camera_.camera_rotation_Y, camera_.camera_rotation_X, 0.0f);
            }

            if (Events::KeyIsClicked(GLFW_KEY_E)) {
                switcher = !switcher;
                if (switcher) {
                    chunks_->rendering_mode_ = GL_LINES;
                } else {
                    chunks_->rendering_mode_ = GL_TRIANGLES;
                }
            }

            {
                glm::vec3 end;
                glm::vec3 norm;
                glm::vec3 iend;
                Voxel* voxel = chunks_->RayCast(camera_.position, camera_.vector_front, 10.0f, end, norm, iend);
                if (voxel != nullptr) {
                    if (Events::MouseIsClicked(GLFW_MOUSE_BUTTON_1)) {
                        chunks_->SetVoxel(static_cast<int>(iend.x), static_cast<int>(iend.y), static_cast<int>(iend.z), 0);
                    }
                    if (Events::MouseIsClicked(GLFW_MOUSE_BUTTON_2)) {
                        chunks_->SetVoxel(static_cast<int>(iend.x) + static_cast<int>(norm.x),
                                          static_cast<int>(iend.y) + static_cast<int>(norm.y),
                                          static_cast<int>(iend.z) + static_cast<int>(norm.z), 4);
                    }
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            chunks_->PollUpdates();
            chunks_->Draw(camera_);

            GUI_.crosshair.Draw();
        }

        window_.SwapBuffers();
        Events::PollEvents();

        total_time += delta_time;
        ++frame_count;
        if (total_time > 1.0) {
            std::cout << frame_count / total_time << '\n';
            frame_count = 0;
            total_time = 0;
        }
    }
}

Engine::~Engine() {
    delete chunks_;
}