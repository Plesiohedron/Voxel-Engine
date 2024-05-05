#include "Engine.h"

Engine::Engine(const int window_width, const int window_height, const char* window_title)
    : window_{window_width, window_height, window_title}, camera_{{0.0f, 0.0f, -5.0f}, glm::radians(90.0f)}, GUI_{} {

    camera_.Rotate(0.0f, glm::radians(180.0f), 0.0f);
    camera_.camera_rotation_X = glm::radians(180.0f);

    chunks_ = new Chunks(5, {0, 0, 0});

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glfwSwapInterval(1);
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

    for (size_t vertex_offset = 0, index_offset = 0, i = 0; i < chunks_->chunk_count; ++i) {
        chunks_->unified_voxel_indirect_command_data_[i] = {0, 1, static_cast<GLuint>(index_offset),
                                                            static_cast<GLuint>(vertex_offset) / Chunk::VERTEX_ATTRIBUTES_COUNT, 0};

        chunks_->chunks_[i]->Render(chunks_->unified_voxel_vertex_data_ + vertex_offset, chunks_->unified_voxel_index_data_ + index_offset,
                                    chunks_->unified_voxel_indirect_command_data_ + i);

        chunks_->X_planes_.clear();
        chunks_->Y_planes_.clear();
        chunks_->Z_planes_.clear();

        vertex_offset += Chunk::MAXIMUM_VOXEL_FACES_COUNT * Chunk::VERTEX_ATTRIBUTES_COUNT * Chunk::VERTICES_COUNT_PER_SQUARE;
        index_offset += Chunk::MAXIMUM_VOXEL_FACES_COUNT * Chunk::INDEXES_COUNT_PER_SQUARE;
    }

    chunks_->unified_VAO_->Bind();
    chunks_->unified_VAO_->InitializeVBO(chunks_->unified_voxel_vertex_data_, chunks_->vertex_data_capacity_);
    chunks_->unified_VAO_->InitializeEBO(chunks_->unified_voxel_index_data_, chunks_->index_data_capacity_);
    chunks_->unified_VAO_->InitializeIB(chunks_->unified_voxel_indirect_command_data_, chunks_->indirect_command_data_capacity_);
    chunks_->unified_VAO_->PostInitialization();


    float last_time = static_cast<float>(glfwGetTime());
    float delta_time = 0.0f;
    float current_time = 0.0f;
    float total_time = 0.0;
    int frame_count = 0;

    float speed = 5.0f;

    while (!window_.IsShouldClose()) {
        current_time = static_cast<float>(glfwGetTime());
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

            if (Events::KeyIsPressed(GLFW_KEY_F)) {
                chunks_->rendering_mode_ = GL_LINES;
            } else {
                chunks_->rendering_mode_ = GL_TRIANGLES;
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            chunks_->Draw(camera_);

            // test.Draw(GL_TRIANGLES);
            GUI_.crosshair.Draw();
        }

        window_.SwapBuffers();
        Events::PollEvents();

        total_time += delta_time;
        ++frame_count;
        if (total_time > 1.0f) {
            std::cout << frame_count / total_time << '\n';
            frame_count = 0;
            total_time = 0;
        }
    }
}

Engine::~Engine() {
    delete chunks_;
}