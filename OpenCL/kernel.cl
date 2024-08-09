#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16
#define CHUNK_DEPTH 16

__kernel void PreCulling(__global const uchar* voxels, 
                             __global uint* X_rows, 
                             __global uint* Y_rows, 
                             __global uint* Z_rows) {
    ushort x = get_global_id(0);
    ushort y = get_global_id(1);
    ushort z = get_global_id(2);

    if (voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]) {
        if (z % 2) {
            atomic_or(&X_rows[y * (CHUNK_DEPTH / 2) + z / 2], (1 << (x + 16)));
        } else {
            atomic_or(&X_rows[y * (CHUNK_DEPTH / 2) + z / 2], (1 << x));
        }

        if (x % 2) {
            atomic_or(&Y_rows[z * (CHUNK_WIDTH / 2) + x / 2], (1 << (y + 16)));
        } else {
            atomic_or(&Y_rows[z * (CHUNK_WIDTH / 2) + x / 2], (1 << y));
        }

        if (y % 2) {
            atomic_or(&Z_rows[x * (CHUNK_HEIGHT / 2) + y / 2], (1 << (z + 16)));
        } else {
            atomic_or(&Z_rows[x * (CHUNK_HEIGHT / 2) + y / 2], (1 << z));
        }
    }
}

#define CHUNK_DIRECTION_SIZE 16
#define CHUNK_DIRECTION_SIZE_P2 256
#define FACES_COUNT_PER_CUBE 6

/*__kernel void Culling(__global const ushort* X_rows,
                      __global const ushort* Y_rows,
                      __global const ushort* Z_rows, 
                      __global uint* face_planes) {
    
    __local uint face_planes_local[FACES_COUNT_PER_CUBE * CHUNK_DIRECTION_SIZE_P2 / 2];

    ushort direction = get_global_id(0);
    ushort direction1 = get_global_id(1);
    ushort direction2 = get_global_id(2);
    ushort direction3;

    ushort faces_row;
    uint bit;
    
    if (direction2 % 2) {
        face_planes_local[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction1 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2] = 0;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    

    if (direction == 0) {
        faces_row = (X_rows[direction1 * CHUNK_DEPTH + direction2] & ~(X_rows[direction1 * CHUNK_DEPTH + direction2] >> 1));
    } else if (direction == 1) {
        faces_row = (X_rows[direction1 * CHUNK_DEPTH + direction2] & ~(X_rows[direction1 * CHUNK_DEPTH + direction2] << 1));
    } else if (direction == 2) {
        faces_row = (Y_rows[direction1 * CHUNK_WIDTH + direction2] & ~(Y_rows[direction1 * CHUNK_WIDTH + direction2] >> 1));
    } else if (direction == 3) {
        faces_row = (Y_rows[direction1 * CHUNK_WIDTH + direction2] & ~(Y_rows[direction1 * CHUNK_WIDTH + direction2] << 1));
    } else if (direction == 4) {
        faces_row = (Z_rows[direction1 * CHUNK_HEIGHT + direction2] & ~(Z_rows[direction1 * CHUNK_HEIGHT + direction2] >> 1));
    } else { // if (direction == 5)
        faces_row = (Z_rows[direction1 * CHUNK_HEIGHT + direction2] & ~(Z_rows[direction1 * CHUNK_HEIGHT + direction2] << 1));
    }

    for (direction3 = 0; direction3 < CHUNK_DIRECTION_SIZE; ++direction3) {
        bit = (((faces_row >> direction3) & 1) << direction1);

        if (bit) {
            if (direction2 % 2) {
                atomic_or(&face_planes_local[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction3 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2], (bit << 16));
            } else {
                atomic_or(&face_planes_local[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction3 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2], bit);
            }
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    
    if (direction2 % 2) {
        face_planes[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction1 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2] = face_planes_local[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction1 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2];
    }
        
}*/

__kernel void Culling(__global const ushort* X_rows,
                      __global const ushort* Y_rows,
                      __global const ushort* Z_rows,
                      __global uint* face_planes) {

    ushort direction = get_global_id(0);
    ushort direction1 = get_global_id(1);
    ushort direction2 = get_global_id(2);
    ushort direction3;

    ushort faces_row;
    uint bit;

    if (direction == 0) {
        faces_row = (X_rows[direction1 * CHUNK_DEPTH + direction2] & ~(X_rows[direction1 * CHUNK_DEPTH + direction2] >> 1));
    } else if (direction == 1) {
        faces_row = (X_rows[direction1 * CHUNK_DEPTH + direction2] & ~(X_rows[direction1 * CHUNK_DEPTH + direction2] << 1));
    } else if (direction == 2) {
        faces_row = (Y_rows[direction1 * CHUNK_WIDTH + direction2] & ~(Y_rows[direction1 * CHUNK_WIDTH + direction2] >> 1));
    } else if (direction == 3) {
        faces_row = (Y_rows[direction1 * CHUNK_WIDTH + direction2] & ~(Y_rows[direction1 * CHUNK_WIDTH + direction2] << 1));
    } else if (direction == 4) {
        faces_row = (Z_rows[direction1 * CHUNK_HEIGHT + direction2] & ~(Z_rows[direction1 * CHUNK_HEIGHT + direction2] >> 1));
    } else { // if (direction == 5)
        faces_row = (Z_rows[direction1 * CHUNK_HEIGHT + direction2] & ~(Z_rows[direction1 * CHUNK_HEIGHT + direction2] << 1));
    }

    for (direction3 = 0; direction3 < CHUNK_DIRECTION_SIZE; ++direction3) {
        bit = (((faces_row >> direction3) & 1) << direction1);

        if (bit) {
            if (direction2 % 2) {
                atomic_or(&face_planes[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction3 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2], (bit << 16));
            } else {
                atomic_or(&face_planes[direction * (CHUNK_DIRECTION_SIZE_P2 / 2) + direction3 * (CHUNK_DIRECTION_SIZE / 2) + direction2 / 2], bit);
            }
        }
    }
}


inline bool IsBlocked(__global const uchar* voxels, short x, short y, short z) {
    if (0 <= x && x < CHUNK_WIDTH && 0 <= y && y < CHUNK_HEIGHT && 0 <= z && z < CHUNK_DEPTH) {
        return voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
    }

    return false;
}

#define VERTICES_COUNT_PER_SQUARE 4

inline ulong AmbientOcclusion(__global const uchar* voxels, short x, short y, short z, short direction) {
    uint side1[VERTICES_COUNT_PER_SQUARE];
    uint side2[VERTICES_COUNT_PER_SQUARE];
    uint corner[VERTICES_COUNT_PER_SQUARE];

    if (direction == 0) {
        corner[0] = IsBlocked(voxels, x + 1, y + 1, z + 1);
        side2[0] = side1[1] = IsBlocked(voxels, x + 1, y + 1, z);
        corner[1] = IsBlocked(voxels, x + 1, y + 1, z - 1);
        side2[1] = side1[2] = IsBlocked(voxels, x + 1, y, z - 1);
        corner[2] = IsBlocked(voxels, x + 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(voxels, x + 1, y - 1, z);
        corner[3] = IsBlocked(voxels, x + 1, y - 1, z + 1);
        side2[3] = side1[0] = IsBlocked(voxels, x + 1, y, z + 1);
    } else if (direction == 1) {
        corner[0] = IsBlocked(voxels, x - 1, y + 1, z - 1);
        side2[0] = side1[1] = IsBlocked(voxels, x - 1, y + 1, z);
        corner[1] = IsBlocked(voxels, x - 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(voxels, x - 1, y, z + 1);
        corner[2] = IsBlocked(voxels, x - 1, y - 1, z + 1);
        side2[2] = side1[3] = IsBlocked(voxels, x - 1, y - 1, z);
        corner[3] = IsBlocked(voxels, x - 1, y - 1, z - 1);
        side2[3] = side1[0] = IsBlocked(voxels, x - 1, y, z - 1);
    } else if (direction == 2) {
        corner[0] = IsBlocked(voxels, x + 1, y + 1, z + 1);
        side2[0] = side1[1] = IsBlocked(voxels, x, y + 1, z + 1);
        corner[1] = IsBlocked(voxels, x - 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(voxels, x - 1, y + 1, z);
        corner[2] = IsBlocked(voxels, x - 1, y + 1, z - 1);
        side2[2] = side1[3] = IsBlocked(voxels, x, y + 1, z - 1);
        corner[3] = IsBlocked(voxels, x + 1, y + 1, z - 1);
        side2[3] = side1[0] = IsBlocked(voxels, x + 1, y + 1, z);
    } else if (direction == 3) {
        corner[0] = IsBlocked(voxels, x - 1, y - 1, z + 1);
        side2[0] = side1[1] = IsBlocked(voxels, x, y - 1, z + 1);
        corner[1] = IsBlocked(voxels, x + 1, y - 1, z + 1);
        side2[1] = side1[2] = IsBlocked(voxels, x + 1, y - 1, z);
        corner[2] = IsBlocked(voxels, x + 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(voxels, x, y - 1, z - 1);
        corner[3] = IsBlocked(voxels, x - 1, y - 1, z - 1);
        side2[3] = side1[0] = IsBlocked(voxels, x - 1, y - 1, z);
    } else if (direction == 4) {
        corner[0] = IsBlocked(voxels, x + 1, y - 1, z + 1);
        side2[0] = side1[1] = IsBlocked(voxels, x + 1, y, z + 1);
        corner[1] = IsBlocked(voxels, x + 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(voxels, x, y + 1, z + 1);
        corner[2] = IsBlocked(voxels, x - 1, y + 1, z + 1);
        side2[2] = side1[3] = IsBlocked(voxels, x - 1, y, z + 1);
        corner[3] = IsBlocked(voxels, x - 1, y - 1, z + 1);
        side2[3] = side1[0] = IsBlocked(voxels, x, y - 1, z + 1);
    } else if (direction == 5) {
        corner[0] = IsBlocked(voxels, x + 1, y + 1, z - 1);
        side2[0] = side1[1] = IsBlocked(voxels, x + 1, y, z - 1);
        corner[1] = IsBlocked(voxels, x + 1, y - 1, z - 1);
        side2[1] = side1[2] = IsBlocked(voxels, x, y - 1, z - 1);
        corner[2] = IsBlocked(voxels, x - 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(voxels, x - 1, y, z - 1);
        corner[3] = IsBlocked(voxels, x - 1, y + 1, z - 1);
        side2[3] = side1[0] = IsBlocked(voxels, x, y + 1, z - 1);
    }

    ulong result = 0;
    uchar i;
    for (i = 0; i < VERTICES_COUNT_PER_SQUARE; ++i) {
        result |= ((side2[i] + corner[i] + side1[i]) << (i * 2));
        result |= ((side2[i] << (10 + i * 3)) | (corner[i] << (9 + i * 3)) | (side1[i] << (8 + i * 3)));
    }

    return result;
}


__kernel void GreedyMesh16bit(__global const uchar* voxels,
                              __global const ushort* face_planes,
                              __global ulong* vertex_data,
                              __global uint* vertex_data_size) {

    __local uint sector_vertices_size[FACES_COUNT_PER_CUBE * CHUNK_DIRECTION_SIZE + 1];

    uint direction = get_global_id(0);
    uint plane = get_global_id(1);


    uint vertex_data_index = (direction * CHUNK_DIRECTION_SIZE + plane) * (VERTICES_COUNT_PER_SQUARE * CHUNK_DIRECTION_SIZE_P2);

    ushort is_processed[CHUNK_DIRECTION_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                 0, 0, 0, 0, 0, 0, 0, 0};


    ushort bit_mask;
    ushort next_bit_mask;

    ushort row_mask;
    ushort current_row;

    ulong current_type;
    ulong next_type;

    uint brithness;
    uint w, h;
    uint vertexAO[VERTICES_COUNT_PER_SQUARE];

    uint row;
    uint bit;
    uint next_bit;
    uint next_row;


    if (direction < 2) {
        brithness = 13;
    } else if (direction == 2) {
        brithness = 15;
    } else if (direction == 3) {
        brithness = 9;
    } else {
        brithness = 11;
    }


    bool flag;
    for (row = 0; row < CHUNK_DIRECTION_SIZE; ++row) {
        current_row = face_planes[direction * CHUNK_DIRECTION_SIZE_P2 + plane * CHUNK_DIRECTION_SIZE + row];

        if (is_processed[row] == 0xFFFF || !current_row) {
            continue;
        }

        for (bit = 0; bit < CHUNK_DIRECTION_SIZE; ) {
            bit_mask = (1 << bit);

            if ((is_processed[row] & bit_mask) || !(current_row & bit_mask)) {
                ++bit;
                continue;
            }

            if (direction < 2) {
                current_type = ((AmbientOcclusion(voxels, plane, bit, row, direction) << 24) |
                                (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                voxels[(bit * CHUNK_DEPTH + row) * CHUNK_WIDTH + plane]);
            } else if (direction < 4) {
                current_type = ((AmbientOcclusion(voxels, row, plane, bit, direction) << 24) |
                                (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                voxels[(plane * CHUNK_DEPTH + bit) * CHUNK_WIDTH + row]);
            } else {
                current_type = ((AmbientOcclusion(voxels, bit, row, plane, direction) << 24) |
                                (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                voxels[(row * CHUNK_DEPTH + plane) * CHUNK_WIDTH + bit]);
            }


            w = h = 1;
            row_mask = bit_mask;
            is_processed[row] |= bit_mask;


            for (next_bit = bit + 1; next_bit < CHUNK_DIRECTION_SIZE; ++next_bit) {
                next_bit_mask = (1 << next_bit);

                if ((is_processed[row] & next_bit_mask) || !(current_row & next_bit_mask)) {
                    break;
                }

                if (direction < 2) {
                    next_type = ((AmbientOcclusion(voxels, plane, next_bit, row, direction) << 24) |
                                    (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                    voxels[(next_bit * CHUNK_DEPTH + row) * CHUNK_WIDTH + plane]);
                } else if (direction < 4) {
                    next_type = ((AmbientOcclusion(voxels, row, plane, next_bit, direction) << 24) |
                                    (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                    voxels[(plane * CHUNK_DEPTH + next_bit) * CHUNK_WIDTH + row]);
                } else {
                    next_type = ((AmbientOcclusion(voxels, next_bit, row, plane, direction) << 24) |
                                    (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                    voxels[(row * CHUNK_DEPTH + plane) * CHUNK_WIDTH + next_bit]);
                }

                if (current_type != next_type) {
                    break;
                }

                ++w;
                row_mask |= next_bit_mask;
                is_processed[row] |= next_bit_mask;
            }


            flag = false;
            for (next_row = row + 1; next_row < CHUNK_DIRECTION_SIZE; ++next_row) {

                if ((face_planes[direction * CHUNK_DIRECTION_SIZE_P2 + plane * CHUNK_DIRECTION_SIZE + next_row] & row_mask) != row_mask) {
                    break;
                }

                for (next_bit = bit; next_bit < bit + w; ++next_bit) {

                    if (direction < 2) {
                        next_type = ((AmbientOcclusion(voxels, plane, next_bit, next_row, direction) << 24) |
                                        (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                        voxels[(next_bit * CHUNK_DEPTH + next_row) * CHUNK_WIDTH + plane]);
                    } else if (direction < 4) {
                        next_type = ((AmbientOcclusion(voxels, next_row, plane, next_bit, direction) << 24) |
                                        (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                        voxels[(plane * CHUNK_DEPTH + next_bit) * CHUNK_WIDTH + next_row]);
                    } else {
                        next_type = ((AmbientOcclusion(voxels, next_bit, next_row, plane, direction) << 24) |
                                        (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                        voxels[(next_row * CHUNK_DEPTH + plane) * CHUNK_WIDTH + next_bit]);
                    }

                    if (current_type != next_type) {
                        flag = true;
                        break;
                    }
                }
                if (flag) {
                    break;
                }

                ++h;
                is_processed[next_row] |= row_mask;
            }

            vertexAO[0] = (current_type >> 24) & 3;
            vertexAO[1] = (current_type >> 26) & 3;
            vertexAO[2] = (current_type >> 28) & 3;
            vertexAO[3] = (current_type >> 30) & 3;

            if (vertexAO[0] + vertexAO[2] < vertexAO[1] + vertexAO[3]) {
                
                if (direction == 0) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | (row << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | (row << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | ((row + h) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | ((row + h) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 1) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (plane << 24) | (bit << 18)       | ((row + h) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (plane << 24) | ((bit + w) << 18) | ((row + h) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (plane << 24) | ((bit + w) << 18) | (row << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (plane << 24) | (bit << 18)       | (row << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 2) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | (bit << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | ((bit + w) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | ((bit + w) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | (bit << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 3) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((row + h) << 24) | (plane << 18) | (bit << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((row + h) << 24) | (plane << 18) | ((bit + w) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (row << 24)       | (plane << 18) | ((bit + w) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (row << 24)       | (plane << 18) | (bit << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 4) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | ((row + h) << 18) | ((plane + 1) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | (row << 18)       | ((plane + 1) << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | (row << 18)       | ((plane + 1) << 12) | (w << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | ((plane + 1) << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                } else { // if (direction == 5)
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | (row << 18)       | (plane << 12) | (w << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | ((row + h) << 18) | (plane << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | (plane << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | (row << 18)       | (plane << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                }
                
            } else {
                
                if (direction == 0) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | ((row + h) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | (row << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | (row << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | ((row + h) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 1) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (plane << 24) | (bit << 18)       | (row << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (plane << 24) | (bit << 18)       | ((row + h) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (plane << 24) | ((bit + w) << 18) | ((row + h) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (plane << 24) | ((bit + w) << 18) | (row << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 2) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | (bit << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | (bit << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | ((bit + w) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | ((bit + w) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 3) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (row << 24)       | (plane << 18) | (bit << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((row + h) << 24) | (plane << 18) | (bit << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((row + h) << 24) | (plane << 18) | ((bit + w) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (row << 24)       | (plane << 18) | ((bit + w) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 4) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | ((plane + 1) << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | ((row + h) << 18) | ((plane + 1) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | (row << 18)       | ((plane + 1) << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | (row << 18)       | ((plane + 1) << 12) | (w << 6) | h);
                    ++vertex_data_index;
                } else { // if (direction == 5)
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | (row << 18)       | (plane << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | (row << 18)       | (plane << 12) | (w << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | ((row + h) << 18) | (plane << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | (plane << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                }
                
            }

            bit += w;
        }
    }

    sector_vertices_size[direction * CHUNK_DIRECTION_SIZE + plane + 1] = vertex_data_index - (direction * CHUNK_DIRECTION_SIZE + plane) * (VERTICES_COUNT_PER_SQUARE * CHUNK_DIRECTION_SIZE_P2);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (direction == 0 && plane == 0) {
        for (w = 2; w < FACES_COUNT_PER_CUBE * CHUNK_DIRECTION_SIZE + 1; ++w) {
            sector_vertices_size[w] += sector_vertices_size[w - 1];
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    if (direction != 0 || plane != 0) {
        for (w = (direction * CHUNK_DIRECTION_SIZE + plane) * (VERTICES_COUNT_PER_SQUARE * CHUNK_DIRECTION_SIZE_P2), h = 0; w < vertex_data_index; ++w, ++h) {
            vertex_data[sector_vertices_size[direction * CHUNK_DIRECTION_SIZE + plane] + h] = vertex_data[w];
        }

        if (direction == 5 && plane == 15) {
            vertex_data_size[0] = sector_vertices_size[direction * CHUNK_DIRECTION_SIZE + plane] + vertex_data_index - (direction * CHUNK_DIRECTION_SIZE + plane) * (VERTICES_COUNT_PER_SQUARE * CHUNK_DIRECTION_SIZE_P2);
        }
    }

}

#define SUB_PLANES_COUNT_PER_PLANE 4
#define VERTEX_OFFSET 256

__kernel void GreedyMesh8bit(__global const uchar* voxels,
                             __global const ushort* face_planes,
                             __global ulong* vertex_data,
                             __global uint* vertex_data_size) {

    __local uint sector_vertices_size[FACES_COUNT_PER_CUBE * CHUNK_DIRECTION_SIZE * SUB_PLANES_COUNT_PER_PLANE + 1];

    uint direction = get_global_id(0);
    uint plane = get_global_id(1);
    uint sub_plane = get_global_id(2);

    uint row_start, row_end;
    uint bit_start, bit_end;

    if (sub_plane == 0) {
        row_start = 0;
        row_end = CHUNK_DIRECTION_SIZE / 2;
        bit_start = 0;
        bit_end = CHUNK_DIRECTION_SIZE / 2;
    } else if (sub_plane == 1) {
        row_start = CHUNK_DIRECTION_SIZE / 2;
        row_end = CHUNK_DIRECTION_SIZE;
        bit_start = 0;
        bit_end = CHUNK_DIRECTION_SIZE / 2;
    } else if (sub_plane == 2) {
        row_start = 0;
        row_end = CHUNK_DIRECTION_SIZE / 2;
        bit_start = CHUNK_DIRECTION_SIZE / 2;
        bit_end = CHUNK_DIRECTION_SIZE;
    } else { // if (sub_plane == 3)
        row_start = CHUNK_DIRECTION_SIZE / 2;
        row_end = CHUNK_DIRECTION_SIZE;
        bit_start = CHUNK_DIRECTION_SIZE / 2;
        bit_end = CHUNK_DIRECTION_SIZE;
    }


    uint vertex_data_index = ((direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane) * VERTEX_OFFSET;

    uchar is_processed[CHUNK_DIRECTION_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0};


    uchar bit_mask;
    uchar next_bit_mask;

    uchar row_mask;
    uchar current_row;

    ulong current_type;
    ulong next_type;

    uint brithness;
    uint w, h;
    uint vertexAO[VERTICES_COUNT_PER_SQUARE];

    uint row;
    uint bit;
    uint next_bit;
    uint next_row;


    if (direction < 2) {
        brithness = 13;
    } else if (direction == 2) {
        brithness = 15;
    } else if (direction == 3) {
        brithness = 9;
    } else {
        brithness = 11;
    }


    bool flag;
    for (row = row_start; row < row_end; ++row) {
        if (row_start == 0) {
            current_row = face_planes[direction * CHUNK_DIRECTION_SIZE_P2 + plane * CHUNK_DIRECTION_SIZE + row];
        } else {
            current_row = (face_planes[direction * CHUNK_DIRECTION_SIZE_P2 + plane * CHUNK_DIRECTION_SIZE + row] >> 8);
        }

        if (is_processed[row] == 0xFF || !current_row) {
            continue;
        }

        for (bit = bit_start; bit < bit_end; ) {
            if (bit_start == 0) {
                bit_mask = (1 << bit);
            } else {
                bit_mask = (1 << (bit - 8));
            }

            if ((is_processed[row] & bit_mask) || !(current_row & bit_mask)) {
                ++bit;
                continue;
            }

            if (direction < 2) {
                current_type = ((AmbientOcclusion(voxels, plane, bit, row, direction) << 24) |
                                (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                voxels[(bit * CHUNK_DEPTH + row) * CHUNK_WIDTH + plane]);
            } else if (direction < 4) {
                current_type = ((AmbientOcclusion(voxels, row, plane, bit, direction) << 24) |
                                (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                voxels[(plane * CHUNK_DEPTH + bit) * CHUNK_WIDTH + row]);
            } else {
                current_type = ((AmbientOcclusion(voxels, bit, row, plane, direction) << 24) |
                                (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                voxels[(row * CHUNK_DEPTH + plane) * CHUNK_WIDTH + bit]);
            }


            w = h = 1;
            row_mask = bit_mask;
            is_processed[row] |= bit_mask;

            for (next_bit = bit + 1; next_bit < bit_end; ++next_bit) {
                if (bit_start == 0) {
                    next_bit_mask = (1 << next_bit);
                } else {
                    next_bit_mask = (1 << (next_bit - 8));
                }

                if ((is_processed[row] & next_bit_mask) || !(current_row & next_bit_mask)) {
                    break;
                }

                if (direction < 2) {
                    next_type = ((AmbientOcclusion(voxels, plane, next_bit, row, direction) << 24) |
                                    (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                    voxels[(next_bit * CHUNK_DEPTH + row) * CHUNK_WIDTH + plane]);
                } else if (direction < 4) {
                    next_type = ((AmbientOcclusion(voxels, row, plane, next_bit, direction) << 24) |
                                    (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                    voxels[(plane * CHUNK_DEPTH + next_bit) * CHUNK_WIDTH + row]);
                } else {
                    next_type = ((AmbientOcclusion(voxels, next_bit, row, plane, direction) << 24) |
                                    (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                    voxels[(row * CHUNK_DEPTH + plane) * CHUNK_WIDTH + next_bit]);
                }

                if (current_type != next_type) {
                    break;
                }

                ++w;
                row_mask |= next_bit_mask;
                is_processed[row] |= next_bit_mask;
            }


            flag = false;
            for (next_row = row + 1; next_row < row_end; ++next_row) {
                

                if (row_start == 0) {
                    if ((row_mask & face_planes[direction * CHUNK_DIRECTION_SIZE_P2 + plane * CHUNK_DIRECTION_SIZE + next_row]) != row_mask) {
                        break;
                    }
                } else {
                    if ((row_mask & (face_planes[direction * CHUNK_DIRECTION_SIZE_P2 + plane * CHUNK_DIRECTION_SIZE + next_row] >> 8)) != row_mask) {
                        break;
                    }
                }

                for (next_bit = bit; next_bit < bit + w; ++next_bit) {

                    if (direction < 2) {
                        next_type = ((AmbientOcclusion(voxels, plane, next_bit, next_row, direction) << 24) |
                                        (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                        voxels[(next_bit * CHUNK_DEPTH + next_row) * CHUNK_WIDTH + plane]);
                    } else if (direction < 4) {
                        next_type = ((AmbientOcclusion(voxels, next_row, plane, next_bit, direction) << 24) |
                                        (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                        voxels[(plane * CHUNK_DEPTH + next_bit) * CHUNK_WIDTH + next_row]);
                    } else {
                        next_type = ((AmbientOcclusion(voxels, next_bit, next_row, plane, direction) << 24) |
                                        (brithness << 20) | (brithness << 16) | (brithness << 12) | (15u << 8) |
                                        voxels[(next_row * CHUNK_DEPTH + plane) * CHUNK_WIDTH + next_bit]);
                    }

                    if (current_type != next_type) {
                        flag = true;
                        break;
                    }
                }
                if (flag) {
                    break;
                }

                ++h;
                is_processed[next_row] |= row_mask;
            }

            vertexAO[0] = (current_type >> 24) & 3;
            vertexAO[1] = (current_type >> 26) & 3;
            vertexAO[2] = (current_type >> 28) & 3;
            vertexAO[3] = (current_type >> 30) & 3;

            if (vertexAO[0] + vertexAO[2] < vertexAO[1] + vertexAO[3]) {
                
                if (direction == 0) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | (row << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | (row << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | ((row + h) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | ((row + h) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 1) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (plane << 24) | (bit << 18)       | ((row + h) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (plane << 24) | ((bit + w) << 18) | ((row + h) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (plane << 24) | ((bit + w) << 18) | (row << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (plane << 24) | (bit << 18)       | (row << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 2) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | (bit << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | ((bit + w) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | ((bit + w) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | (bit << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 3) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((row + h) << 24) | (plane << 18) | (bit << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((row + h) << 24) | (plane << 18) | ((bit + w) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (row << 24)       | (plane << 18) | ((bit + w) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (row << 24)       | (plane << 18) | (bit << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 4) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | ((row + h) << 18) | ((plane + 1) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | (row << 18)       | ((plane + 1) << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | (row << 18)       | ((plane + 1) << 12) | (w << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | ((plane + 1) << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                } else {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | (row << 18)       | (plane << 12) | (w << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | ((row + h) << 18) | (plane << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | (plane << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | (row << 18)       | (plane << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                }
                
            } else {
                
                if (direction == 0) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | ((row + h) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((plane + 1) << 24) | (bit << 18)       | (row << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | (row << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((plane + 1) << 24) | ((bit + w) << 18) | ((row + h) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 1) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (plane << 24) | (bit << 18)       | (row << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (plane << 24) | (bit << 18)       | ((row + h) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (plane << 24) | ((bit + w) << 18) | ((row + h) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (plane << 24) | ((bit + w) << 18) | (row << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 2) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | (bit << 12)       | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | (bit << 12)       | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             (row << 24)       | ((plane + 1) << 18) | ((bit + w) << 12) | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((row + h) << 24) | ((plane + 1) << 18) | ((bit + w) << 12) | (h << 6) | w);
                    ++vertex_data_index;
                } else if (direction == 3) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (row << 24)       | (plane << 18) | (bit << 12)       | (0 << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             ((row + h) << 24) | (plane << 18) | (bit << 12)       | (h << 6) | w);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((row + h) << 24) | (plane << 18) | ((bit + w) << 12) | (h << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             (row << 24)       | (plane << 18) | ((bit + w) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                } else if (direction == 4) {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | ((plane + 1) << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | ((row + h) << 18) | ((plane + 1) << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | (row << 18)       | ((plane + 1) << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | (row << 18)       | ((plane + 1) << 12) | (w << 6) | h);
                    ++vertex_data_index;
                } else {
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                             ((bit + w) << 24) | (row << 18)       | (plane << 12) | (w << 6) | 0);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                             (bit << 24)       | (row << 18)       | (plane << 12) | (w << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                             (bit << 24)       | ((row + h) << 18) | (plane << 12) | (0 << 6) | h);
                    ++vertex_data_index;
                    vertex_data[vertex_data_index] = (((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                             ((bit + w) << 24) | ((row + h) << 18) | (plane << 12) | (0 << 6) | 0);
                    ++vertex_data_index;
                }
                
            }

            bit += w;
        }
    }

    sector_vertices_size[(direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane + 1] = vertex_data_index - ((direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane) * VERTEX_OFFSET;
    barrier(CLK_LOCAL_MEM_FENCE);

    if (direction == 0 && plane == 0 && sub_plane == 0) {
        for (w = 2; w < FACES_COUNT_PER_CUBE * CHUNK_DIRECTION_SIZE * SUB_PLANES_COUNT_PER_PLANE + 1; ++w) {
            sector_vertices_size[w] += sector_vertices_size[w - 1];
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    if (direction != 0 || plane != 0 || sub_plane != 0)  {
        for (w = ((direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane) * VERTEX_OFFSET, h = 0; w < vertex_data_index; ++w, ++h) {
            vertex_data[sector_vertices_size[(direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane] + h] = vertex_data[w];
        }

        if (direction == 5 && plane == 15 && sub_plane == 3) {
            vertex_data_size[0] = sector_vertices_size[(direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane] + vertex_data_index - ((direction * CHUNK_DIRECTION_SIZE + plane) * SUB_PLANES_COUNT_PER_PLANE + sub_plane) * VERTEX_OFFSET;
        }
    }

}
