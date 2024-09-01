#include "Block.h"

namespace Blocks {
Block blocks[256] = {
    {.texture_id = {0, 0, 0, 0, 0, 0}, .r = 0,  .g = 0,  .b = 0,  .is_transparent = true},
    {.texture_id = {1, 1, 1, 1, 1, 1}, .r = 0,  .g = 0,  .b = 0,  .is_transparent = false},
    {.texture_id = {3, 3, 4, 1, 3, 3}, .r = 0,  .g = 0,  .b = 0,  .is_transparent = false},
    {.texture_id = {5, 5, 5, 5, 5, 5}, .r = 0,  .g = 0,  .b = 0,  .is_transparent = false},
    {.texture_id = {6, 6, 6, 6, 6, 6}, .r = 15, .g = 0,  .b = 0,  .is_transparent = false},
    {.texture_id = {6, 6, 6, 6, 6, 6}, .r = 0,  .g = 15, .b = 0,  .is_transparent = false},
    {.texture_id = {6, 6, 6, 6, 6, 6}, .r = 0,  .g = 0,  .b = 15, .is_transparent = false},
};
}