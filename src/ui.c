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
#include "raylib.h"
#include "codegen.h"
#include "runtime.h"
#include "leaf_api.h"
#include "ui.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

extern Vector2 trail[];
extern int trail_count;

BlockNode blockPool[20];
int blockCount = 0;
int global_z_index = 1;

void init_blocks() {
    blockPool[0] = (BlockNode){ BLOCK_MOVE, 50, "移动 50 步", {20, 150}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[1] = (BlockNode){ BLOCK_TURN_RIGHT, 90, "右转 90 度", {20, 200}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[2] = (BlockNode){ BLOCK_TURN_LEFT, 90, "左转 90 度", {20, 250}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[3] = (BlockNode){ BLOCK_SAY, 0, "说 Hello", {20, 300}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[4] = (BlockNode){ BLOCK_WAIT, 1, "等待 1 秒", {20, 350}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[5] = (BlockNode){ BLOCK_REPEAT, 4, "重复 4 次", {20, 400}, {180, 60}, 0, 0, NULL, NULL, NULL };
    blockPool[6] = (BlockNode){ BLOCK_CUSTOM, 5, "自定义: 画星形", {20, 470}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[7] = (BlockNode){ BLOCK_KEY_UP, 0, "当按下 [↑] 键", {20, 520}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[8] = (BlockNode){ BLOCK_KEY_DOWN, 0, "当按下 [↓] 键", {20, 570}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[9] = (BlockNode){ BLOCK_FOREVER, 0, "无限重复执行", {20, 620}, {180, 60}, 0, 0, NULL, NULL, NULL };
    blockPool[10] = (BlockNode){ BLOCK_PLAY_SOUND, 0, "播放电子音", {20, 690}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockPool[11] = (BlockNode){ BLOCK_GRAVITY, 0, "受重力下落", {20, 740}, {180, 40}, 0, 0, NULL, NULL, NULL };
    blockCount = 12;
}

void draw_sprite_code(float x, float y, float rot, int skin) {
    if (skin == 0) { DrawCircle(x, y, 20, ORANGE); DrawTriangle((Vector2){x-15, y-15}, (Vector2){x-5, y-30}, (Vector2){x-25, y-30}, ORANGE); DrawTriangle((Vector2){x+15, y-15}, (Vector2){x+5, y-30}, (Vector2){x+25, y-30}, ORANGE); }
    else if (skin == 1) { DrawCircle(x, y, 20, BROWN); DrawEllipse(x-18, y, 8, 16, BROWN); DrawEllipse(x+18, y, 8, 16, BROWN); }
    else if (skin == 2) { DrawCircle(x, y, 20, RED); DrawCircle(x-5, y-5, 5, WHITE); }
    else { DrawRectanglePro((Rectangle){x, y, 40, 40}, (Vector2){20, 20}, rot, ORANGE); }
}

void draw_ui() {
    DrawFPS(10, 10); 
    Rectangle flagBtn = { 680, 20, 80, 80 };
    if (CheckCollisionPointRec(GetMousePosition(), flagBtn)) {
        DrawRectangleRec(flagBtn, LIME);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !g_leaf.is_running) {
            trail_count = 0; g_leaf.sprite_x = 400; g_leaf.sprite_y = 300; g_leaf.direction = 90;
            start_vm_thread(generate_code(blockPool, blockCount));
        }
    } else DrawRectangleRec(flagBtn, GREEN);
    DrawTriangle((Vector2){712, 40}, (Vector2){712, 60}, (Vector2){735, 50}, WHITE);

    Rectangle clearBtn = { 580, 20, 80, 80 };
    if (CheckCollisionPointRec(GetMousePosition(), clearBtn)) {
        DrawRectangleRec(clearBtn, SKYBLUE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) trail_count = 0;
    } else DrawRectangleRec(clearBtn, BLUE);
    DrawText("清屏", 605, 115, 20, DARKGRAY);

    Rectangle skinBtn = { 480, 20, 80, 80 };
    if (CheckCollisionPointRec(GetMousePosition(), skinBtn)) {
        DrawRectangleRec(skinBtn, GOLD);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) g_leaf.skin = (g_leaf.skin + 1) % 4;
    } else DrawRectangleRec(skinBtn, ORANGE);
    DrawText("换角色", 495, 115, 20, DARKGRAY);

    DrawRectangle(0, 140, 240, 460, LIGHTGRAY);
    DrawRectangleLines(250, 140, 500, 460, DARKGRAY);
    for (int i = 1; i < trail_count; i++) DrawLineEx(trail[i-1], trail[i], 3.0f, RED);
    draw_sprite_code(g_leaf.sprite_x, g_leaf.sprite_y, g_leaf.direction, g_leaf.skin);

    Vector2 mouse = GetMousePosition();
    for (int render_pass = 0; render_pass < 2; render_pass++) {
        for (int i = 0; i < blockCount; i++) {
            BlockNode* b = &blockPool[i];
            if (render_pass == 0 && b->is_dragging) continue;
            if (render_pass == 1 && !b->is_dragging) continue;

            if (b->pos.x < 240 && !b->is_dragging) { b->pos.x = 20; }

            if (render_pass == 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, (Rectangle){b->pos.x, b->pos.y, b->size.x, b->size.y})) {
                b->is_dragging = 1; b->z_index = global_z_index++; b->parent = NULL; 
            }

            if (b->is_dragging) {
                b->pos.x = mouse.x - b->size.x / 2; b->pos.y = mouse.y - b->size.y / 2;
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && b->is_dragging) {
                b->is_dragging = 0;
                if (b->pos.x > 240) {
                    for (int j = 0; j < blockCount; j++) {
                        BlockNode* other = &blockPool[j];
                        if (other == b || other->pos.x < 240) continue;
                        if (abs(b->pos.x - other->pos.x) < 30 && abs(b->pos.y - (other->pos.y + other->size.y)) < 20) {
                            b->pos.x = other->pos.x; b->pos.y = other->pos.y + other->size.y; b->parent = other; break;
                        }
                        if ((other->type == BLOCK_REPEAT || other->type == BLOCK_FOREVER) && b != other) {
                            Rectangle innerRect = { other->pos.x + 15, other->pos.y + 40, other->size.x - 15, 20 };
                            if (CheckCollisionPointRec(mouse, innerRect)) {
                                b->pos.x = other->pos.x + 15; b->pos.y = other->pos.y + 40; b->parent = other; other->child = b; break;
                            }
                        }
                    }
                } else { b->pos.x = 20; }
            }

            Color color = b->is_dragging ? LIGHTGRAY : DARKGRAY;
            if (b->type == BLOCK_CUSTOM) color = DARKPURPLE;
            if (b->type == BLOCK_KEY_UP || b->type == BLOCK_KEY_DOWN) color = DARKBLUE;
            if (b->type == BLOCK_FOREVER) color = DARKGREEN;
            if (b->type == BLOCK_GRAVITY) color = MAROON;
            
            if (b->type == BLOCK_REPEAT || b->type == BLOCK_FOREVER) {
                DrawRectangleRec((Rectangle){b->pos.x, b->pos.y, b->size.x, 30}, color);
                DrawRectangleRec((Rectangle){b->pos.x, b->pos.y + 30, 15, 30}, color);
                DrawRectangleLinesEx((Rectangle){b->pos.x, b->pos.y, b->size.x, b->size.y}, 2.0f, BLACK);
                DrawText(b->text, b->pos.x + 10, b->pos.y + 5, 18, WHITE);
            } else {
                DrawRectangleRec((Rectangle){b->pos.x, b->pos.y, b->size.x, b->size.y}, color);
                DrawRectangleLinesEx((Rectangle){b->pos.x, b->pos.y, b->size.x, b->size.y}, 2.0f, BLACK);
                DrawText(b->text, b->pos.x + 10, b->pos.y + 10, 18, WHITE);
            }
        }
    }
    if (g_leaf.message_timer > 0) {
        g_leaf.message_timer--;
        DrawRectangleRounded((Rectangle){ g_leaf.sprite_x + 30, g_leaf.sprite_y - 40, MeasureText(g_leaf.message, 20) + 20, 40 }, 0.5f, 10, WHITE);
        DrawText(g_leaf.message, g_leaf.sprite_x + 40, g_leaf.sprite_y - 30, 20, BLACK);
    }
}
