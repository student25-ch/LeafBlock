/*
 * LeafBlock - Pure C Visual Programming Engine
 * Copyright (C) 2026 杨子桐 (Yang Zitong)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef LEAF_API_H
#define LEAF_API_H
#include "raylib.h"

typedef struct {
    float sprite_x; float sprite_y; float direction;
    int is_running; int skin;
    char message[256]; float message_timer;
} LeafState;
extern LeafState g_leaf;

void leaf_move(float steps);
void leaf_turn(float degrees);
void leaf_say(const char* text, float seconds);
void leaf_play_sound(float freq, float duration);
void leaf_gravity();

typedef enum { 
    BLOCK_MOVE, BLOCK_TURN_RIGHT, BLOCK_TURN_LEFT, BLOCK_SAY, BLOCK_WAIT, 
    BLOCK_REPEAT, BLOCK_CUSTOM, BLOCK_KEY_UP, BLOCK_KEY_DOWN, BLOCK_FOREVER, 
    BLOCK_PLAY_SOUND, BLOCK_GRAVITY
} BlockType;

typedef struct BlockNode {
    BlockType type;
    float param;             
    char text[64];           
    Vector2 pos;             
    Vector2 size;            
    int is_dragging;         
    int z_index;             
    struct BlockNode* next;  
    struct BlockNode* child; 
    struct BlockNode* parent;
} BlockNode;
#endif
