

__kernel void Culling(__global const float* input_data, __global bool* output_data) {
    int chunk_local_x = get_global_id(0);
    int chunk_local_z = get_global_id(1);
	int chunk_local_y = get_global_id(2);

    float vec1x = input_data[0];
    float vec1y = input_data[1];
    float vec1z = input_data[2];

    float vec2x = input_data[3];
    float vec2y = input_data[4];
    float vec2z = input_data[5];

    float vec3x = input_data[6];
    float vec3y = input_data[7];
    float vec3z = input_data[8];

    float camera_position_x = input_data[9];
    float camera_position_y = input_data[10];
    float camera_position_z = input_data[11];

	int storage_rendering_center_x = (int) input_data[12];
	int storage_rendering_center_z = (int) input_data[13];
	int storage_sizes_x = (int) input_data[14];
    int storage_sizes_y = 16;
	int storage_sizes_z = (int) input_data[15];

	float k = input_data[16];
	float frustum_length = input_data[17];

	float point_x = (float) (chunk_local_x + storage_rendering_center_x - storage_sizes_x / 2) - camera_position_x;
	float point_y = (float) (chunk_local_y) - camera_position_y;
	float point_z = (float) (chunk_local_z + storage_rendering_center_z - storage_sizes_z / 2) - camera_position_z;

	float rotated_point_x = vec1x * point_x + vec1y * point_y + vec1z * point_z;
    float rotated_point_y = vec2x * point_x + vec2y * point_y + vec2z * point_z;
    float rotated_point_z = vec3x * point_x + vec3y * point_y + vec3z * point_z;

	if (rotated_point_z >= -frustum_length &&
        k * rotated_point_z <= -max(fabs(rotated_point_x), fabs(rotated_point_y))) {
		
		if (chunk_local_x - 1 >= 0 && chunk_local_y - 1 >= 0 && chunk_local_z - 1 >= 0) {
            output_data[((chunk_local_y - 1) * storage_sizes_z + (chunk_local_z - 1)) * storage_sizes_x + (chunk_local_x - 1)] = true;
        }
        if (chunk_local_x - 1 >= 0 && chunk_local_y - 1 >= 0 && chunk_local_z < storage_sizes_z) {
            output_data[((chunk_local_y - 1) * storage_sizes_z + chunk_local_z) * storage_sizes_x + (chunk_local_x - 1)] = true;
        }
        if (chunk_local_x - 1 >= 0 && chunk_local_y < storage_sizes_y && chunk_local_z - 1 >= 0) {
            output_data[(chunk_local_y * storage_sizes_z + (chunk_local_z - 1)) * storage_sizes_x + (chunk_local_x - 1)] = true;
        }
        if (chunk_local_x - 1 >= 0 && chunk_local_y < storage_sizes_y && chunk_local_z < storage_sizes_z) {
            output_data[(chunk_local_y * storage_sizes_z + chunk_local_z) * storage_sizes_x + (chunk_local_x - 1)] = true;
        }
        if (chunk_local_x < storage_sizes_x && chunk_local_y - 1 >= 0 && chunk_local_z - 1 >= 0) {
            output_data[((chunk_local_y - 1) * storage_sizes_z + (chunk_local_z - 1)) * storage_sizes_x + chunk_local_x] = true;
        }
        if (chunk_local_x < storage_sizes_x && chunk_local_y - 1 >= 0 && chunk_local_z < storage_sizes_z) {
            output_data[((chunk_local_y - 1) * storage_sizes_z + chunk_local_z) * storage_sizes_x + chunk_local_x] = true;
        }
        if (chunk_local_x < storage_sizes_x && chunk_local_y < storage_sizes_y && chunk_local_z - 1 >= 0) {
            output_data[(chunk_local_y * storage_sizes_z + (chunk_local_z - 1)) * storage_sizes_x + chunk_local_x] = true;
        }
        if (chunk_local_x < storage_sizes_x && chunk_local_y < storage_sizes_y && chunk_local_z < storage_sizes_z) {
            output_data[(chunk_local_y * storage_sizes_z + chunk_local_z) * storage_sizes_x + chunk_local_x] = true;
        }
	}
}