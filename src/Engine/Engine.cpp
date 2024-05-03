#include "Engine.h"

Engine::Engine(const int window_width, const int window_height, const char* window_title)
    : window_{window_width, window_height, window_title}, camera_{{0.0f, 0.0f, -5.0f}, glm::radians(90.0f)}, GUI_{} {

    camera_.Rotate(0.0f, glm::radians(180.0f), 0.0f);
    camera_.camera_rotation_X = glm::radians(180.0f);

    chunk_storage_ = new ChunkStorage(3, {0, 0, 0});

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
}

void Engine::MainLoop() {
    /*GL::VAO test;

    std::vector<GLushort> vertices = {
        ((0 << 10) | (1 << 5) | 0), ((1 << 10) | (0 << 5) | 0), ((15 << 12) | (15 << 8) | (15 << 4) | 15),
        ((0 << 10) | (0 << 5) | 0), ((1 << 10) | (0 << 5) | 1), ((15 << 12) | (15 << 8) | (15 << 4) | 15),

        ((2 << 10) | (1 << 5) | 0), ((1 << 10) | (2 << 5) | 0), ((15 << 12) | (15 << 8) | (15 << 4) | 15),
        ((2 << 10) | (0 << 5) | 0), ((1 << 10) | (2 << 5) | 1), ((15 << 12) | (15 << 8) | (15 << 4) | 15),
    };

    std::vector<GLushort> indexes = {
        0, 2, 1, 1, 2, 3
    };

    test.Bind();
    test.InitializeChunkVBO(vertices);
    test.InitializeEBO(indexes);
    test.PostInitialization();*/

    for (int y = 0; y < ChunkStorage::storage_sizes.y; ++y) {
        for (int z = 0; z < ChunkStorage::storage_sizes.z; ++z) {
            for (int x = 0; x < ChunkStorage::storage_sizes.z; ++x) {
                Chunk* chunk = chunk_storage_->chunks_[(y * ChunkStorage::storage_sizes.z + z) * ChunkStorage::storage_sizes.x + x];

                chunk->Render();

                chunk_storage_->X_planes_.clear();
                chunk_storage_->Y_planes_.clear();
                chunk_storage_->Z_planes_.clear();

                GL::VAO* chunk_VAO = chunk_storage_->chunks_VAOs_[(y * ChunkStorage::storage_sizes.z + z) * ChunkStorage::storage_sizes.x + x];

                chunk_VAO->Bind();
                chunk_VAO->InitializeChunkVBO(chunk->vertices, chunk->vertices_array_size);
                chunk_VAO->InitializeEBO(chunk->indexes, chunk->indexes_array_size);
                chunk_VAO->PostInitialization();
            }
        }
    }


    float last_time = static_cast<float>(glfwGetTime());
    float delta_time = 0.0f;
    float current_time = 0.0f;

    float speed = 5.0f;

    while (!window_.IsShouldClose()) {
        current_time = static_cast<float>(glfwGetTime());
        delta_time = current_time - last_time;
        last_time = current_time;

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

            if (Events::KeyIsPressed(GLFW_KEY_F)) {
                chunk_storage_->rendering_mode_ = GL_LINES;
            } else {
                chunk_storage_->rendering_mode_ = GL_TRIANGLES;
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            chunk_storage_->Draw(camera_);
            GUI_.crosshair.Draw();
        }

        window_.SwapBuffers();
        Events::PollEvents();
    }
}

Engine::~Engine() {
    delete chunk_storage_;
}