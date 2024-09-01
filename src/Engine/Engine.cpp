#include "Engine.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <chrono>
#include <bitset>

Engine::Engine(int window_width, int window_height, const char* window_title)
    : window_{window_width, window_height, window_title}, camera_{{0.0f, 12.0f, 0.0f}, glm::radians(90.0f)},
    GUI_{}, line_batch_{} {

    camera_.Rotate(0.0f, glm::radians(180.0f), 0.0f);
    camera_.camera_rotation_X = glm::radians(180.0f);

    chunks_ = new Chunks(10, {0, 7, 0});

    if (chunks_ == nullptr) {
        std::cout << "Bad alloc: chunks_ (Engine)" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    //glLineWidth(2.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glfwSwapInterval(0);
}

void Engine::SaveScreenshot(const char* filename, int width, int height) const {
    std::vector<unsigned char> pixels(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    stbi_flip_vertically_on_write(1);
    if (stbi_write_png(filename, width, height, 3, pixels.data(), width * 3)) {
        std::cout << "Screenshot saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save screenshot" << std::endl;
    }
}

void Engine::MainLoop() {
    double last_time = glfwGetTime();
    double delta_time = 0.0f;
    double current_time = 0.0f;
    double total_time = 0.0;
    int frame_count = 0;

    float speed = 15.0f;

    int index = 1;

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

            if (Events::KeyIsClicked(GLFW_KEY_1)) {
                chunks_->storage_.selected_block = 1;
            } else if (Events::KeyIsClicked(GLFW_KEY_2)) {
                chunks_->storage_.selected_block = 2;
            } else if (Events::KeyIsClicked(GLFW_KEY_3)) {
                chunks_->storage_.selected_block = 3;
            } else if (Events::KeyIsClicked(GLFW_KEY_4)) {
                chunks_->storage_.selected_block = 4;
            } else if (Events::KeyIsClicked(GLFW_KEY_5)) {
                chunks_->storage_.selected_block = 5;
            } else if (Events::KeyIsClicked(GLFW_KEY_6)) {
                chunks_->storage_.selected_block = 6;
            }

            if (Events::KeyIsClicked(GLFW_KEY_F3)) {
                SaveScreenshot(("screenshot" + std::to_string(index) + ".png").c_str(), window_.width, window_.height);
                ++index;
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

                chunks_->frustum_TL = glm::vec3(camera_.rotation * glm::vec4(glm::normalize(glm::vec3(-chunks_->w_near, chunks_->h_near, -1)), 1));
                chunks_->frustum_TR = glm::vec3(camera_.rotation * glm::vec4(glm::normalize(glm::vec3(chunks_->w_near, chunks_->h_near, -1)), 1));
                chunks_->frustum_BR = glm::vec3(camera_.rotation * glm::vec4(glm::normalize(glm::vec3(chunks_->w_near, -chunks_->h_near, -1)), 1));
                chunks_->frustum_BL = glm::vec3(camera_.rotation * glm::vec4(glm::normalize(glm::vec3(-chunks_->w_near, -chunks_->h_near, -1)), 1));

                chunks_->rotation = glm::mat4(1.0f);
                chunks_->rotation = glm::rotate(chunks_->rotation, -camera_.camera_rotation_Y, glm::vec3(1, 0, 0));
                chunks_->rotation = glm::rotate(chunks_->rotation, -camera_.camera_rotation_X, glm::vec3(0, 1, 0));
            }

            if (Events::KeyIsClicked(GLFW_KEY_E)) {
                chunks_->debug_mode = !chunks_->debug_mode;
                if (chunks_->debug_mode) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            chunks_->storage_.CaptureTarget(line_batch_, camera_);

            chunks_->storage_.UpdateChanges();
            chunks_->PollUpdates();

            if (line_batch_.draw_box) {
                line_batch_.Draw(camera_);
            }

            chunks_->FrustumCulling(camera_.position);
            chunks_->ACCA(camera_.position);

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