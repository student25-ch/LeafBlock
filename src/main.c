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
