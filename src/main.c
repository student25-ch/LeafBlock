/*
 * LeafBlock - Pure C Visual Programming Engine
 * Copyright (C) 2026 Ñî×ÓÍ© (Yang Zitong)
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
#include "ui.h"
#include <stdlib.h>
extern void init_blocks();
int main(void) {
    InitWindow(800, 600, "LeafBlock 9.0");
    SetTargetFPS(60);
    InitAudioDevice();
    init_blocks();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_ui();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
