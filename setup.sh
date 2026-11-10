```bash
#!/bin/bash

echo "=== 正在初始化 LeafBlock 9.0 ==="
mkdir -p src include include/tcc lib external

if [ ! -f "lib/libraylib.a" ]; then
    curl -sL -o raylib.zip https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_mingw-w64.zip
    unzip -q raylib.zip -d external/
    mv external/raylib-5.0_win64_mingw-w64/include/* include/
    mv external/raylib-5.0_win64_mingw-w64/lib/* lib/
    rm -rf raylib.zip external/raylib-5.0_win64_mingw-w64
fi

if [ ! -f "lib/libtcc.a" ]; then
    curl -sL -o tcc.zip https://github.com/TinyCC/tinycc/releases/download/tcc-0.9.27/tcc-0.9.27-win64-bin.zip
    unzip -q tcc.zip -d external/
    cp external/tcc/libtcc.h include/tcc/
    cp external/tcc/libtcc.def lib/
    cd lib && dlltool -d libtcc.def -l libtcc.a && cd ..
    rm -rf tcc.zip external/tcc
fi

cat << 'EOF' > Makefile
CC = gcc
CFLAGS = -Iinclude -Isrc -O2 -Wall
LDFLAGS = -Llib -lraylib -ltcc -lws2_32 -lgdi32 -lwinmm -lopengl32
TARGET = LeafBlock.exe
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)
EOF

cat << 'EOF' > .gitignore
*.o
*.exe
lib/
include/raylib.h
include/tcc/
external/
output.txt
.vscode/
EOF

cat << 'EOF' > README.md
# 🌿 LeafBlock - Pure C Visual Programming Engine

**LeafBlock** 是一个完全由纯 C 语言编写的轻量级可视化积木编程引擎。灵感来源于 Scratch，底层采用内置的 Tiny C Compiler (TCC)，让积木逻辑瞬间编译为原生机器码执行。

## ✨ 特性
- **极速运行**：拖拽积木后点击绿旗，内存中直接编译生成原生 C 代码并执行。
- **模块化架构**：UI 渲染、代码生成、运行时库分离。
- **代码嵌套解析**：支持将积木拖入循环积木内部，递归生成 `for` / `while` 代码块。
- **纯代码绘制**：所有角色和音效均通过代码生成，无外部素材。
- **多线程隔离**：UI 主循环与 TCC 虚拟机分离，互不阻塞。

## 🛠️ 编译方式
1. 克隆仓库并进入目录。
2. 执行 `make`。
3. 双击 `LeafBlock.exe` 运行。

## 📄 开源协议
AGPL-3.0
EOF

cat << 'EOF' > include/leaf_api.h
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
EOF

cat << 'EOF' > include/runtime.h
#ifndef RUNTIME_H
#define RUNTIME_H
void start_vm_thread(const char* code);
#endif
EOF

cat << 'EOF' > include/codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H
#include "leaf_api.h"
const char* generate_code(BlockNode* blocks, int count);
#endif
EOF

cat << 'EOF' > include/ui.h
#ifndef UI_H
#define UI_H
void draw_ui();
#endif
EOF

cat << 'EOF' > src/leaf_api.c
#include "leaf_api.h"
#include <string.h>
#include <math.h>

LeafState g_leaf = { 400.0f, 300.0f, 90.0f, 0, 0, "", 0.0f };
Vector2 trail[2000];
int trail_count = 0;

void leaf_move(float steps) {
    float rad = (g_leaf.direction - 90.0f) * (3.14159265f / 180.0f);
    g_leaf.sprite_x += cos(rad) * steps;
    g_leaf.sprite_y += sin(rad) * steps;
    if (trail_count < 2000) {
        trail[trail_count].x = g_leaf.sprite_x; trail[trail_count].y = g_leaf.sprite_y; trail_count++;
    }
}
void leaf_turn(float degrees) { g_leaf.direction += degrees; }
void leaf_say(const char* text, float seconds) {
    strcpy(g_leaf.message, text); g_leaf.message_timer = seconds * 60.0f;
}
void leaf_gravity() {
    g_leaf.sprite_y += 5.0f; 
    if (trail_count < 2000) {
        trail[trail_count].x = g_leaf.sprite_x; trail[trail_count].y = g_leaf.sprite_y; trail_count++;
    }
}
void leaf_play_sound(float freq, float duration) {
    if (freq < 50) freq = 440;
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    if (frameCount > 44100) frameCount = 44100;
    float *samples = (float *)RL_MALLOC(frameCount * sizeof(float));
    for (int i = 0; i < frameCount; i++) {
        float fade = 1.0f - ((float)i / frameCount);
        samples[i] = sinf(2.0f * PI * freq * ((float)i / sampleRate)) * 0.3f * fade;
    }
    Wave wave = { frameCount, sampleRate, 32, 1, samples };
    Sound sound = LoadSoundFromWave(wave);
    PlaySound(sound);
    UnloadSound(sound);
    RL_FREE(samples);
}
EOF

cat << 'EOF' > src/runtime.c
#include <stdio.h>
#include <windows.h>
#include <tcc/libtcc.h>
#include "runtime.h"
#include "leaf_api.h"

DWORD WINAPI vm_thread(LPVOID lpParam) {
    const char* code = (const char*)lpParam;
    TCCState *s = tcc_new();
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    tcc_add_symbol(s, "leaf_move", leaf_move);
    tcc_add_symbol(s, "leaf_say", leaf_say);
    tcc_add_symbol(s, "leaf_turn", leaf_turn);
    tcc_add_symbol(s, "leaf_play_sound", leaf_play_sound);
    tcc_add_symbol(s, "leaf_gravity", leaf_gravity);
    tcc_add_symbol(s, "Sleep", Sleep); 
    if (tcc_compile_string(s, code) == 0 && tcc_relocate(s) >= 0) {
        int (*run_func)(void) = tcc_get_symbol(s, "run");
        if (run_func) { g_leaf.is_running = 1; run_func(); g_leaf.is_running = 0; }
    }
    tcc_delete(s);
    return 0;
}
void start_vm_thread(const char* code) { CreateThread(NULL, 0, vm_thread, (LPVOID)code, 0, NULL); }
EOF

cat << 'EOF' > src/codegen.c
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char code_buffer[8192];

void generate_block_tree(BlockNode* block) {
    BlockNode* curr = block;
    while (curr != NULL) {
        switch (curr->type) {
            case BLOCK_MOVE: sprintf(code_buffer + strlen(code_buffer), "    leaf_move(%f); leaf_play_sound(600, 0.1); Sleep(150);\n", curr->param); break;
            case BLOCK_TURN_RIGHT: sprintf(code_buffer + strlen(code_buffer), "    leaf_turn(%f); Sleep(200);\n", curr->param); break;
            case BLOCK_TURN_LEFT: sprintf(code_buffer + strlen(code_buffer), "    leaf_turn(-%f); Sleep(200);\n", curr->param); break;
            case BLOCK_SAY: sprintf(code_buffer + strlen(code_buffer), "    leaf_say(\"%s\", 1.5);\n", curr->text); break;
            case BLOCK_WAIT: sprintf(code_buffer + strlen(code_buffer), "    Sleep(%d);\n", (int)(curr->param * 1000)); break;
            case BLOCK_CUSTOM: sprintf(code_buffer + strlen(code_buffer), "    for(int _j=0; _j<%d; _j++) { leaf_move(80); leaf_turn(180.0f - (180.0f / %d)); leaf_play_sound(800, 0.05); Sleep(100); }\n", (int)curr->param, (int)curr->param); break;
            case BLOCK_KEY_UP: sprintf(code_buffer + strlen(code_buffer), "    if(IsKeyDown(KEY_UP)) { leaf_move(5); }\n"); break;
            case BLOCK_KEY_DOWN: sprintf(code_buffer + strlen(code_buffer), "    if(IsKeyDown(KEY_DOWN)) { leaf_move(-5); }\n"); break;
            case BLOCK_PLAY_SOUND: sprintf(code_buffer + strlen(code_buffer), "    leaf_play_sound(440, 0.5); Sleep(500);\n"); break;
            case BLOCK_GRAVITY: sprintf(code_buffer + strlen(code_buffer), "    leaf_gravity(); Sleep(30);\n"); break;
            case BLOCK_FOREVER: 
                sprintf(code_buffer + strlen(code_buffer), "    while(1) {\n"); 
                generate_block_tree(curr->child); 
                sprintf(code_buffer + strlen(code_buffer), "    }\n"); 
                return; 
            case BLOCK_REPEAT: 
                sprintf(code_buffer + strlen(code_buffer), "    for(int _i=0; _i<%d; _i++) {\n", (int)curr->param); 
                generate_block_tree(curr->child); 
                sprintf(code_buffer + strlen(code_buffer), "    }\n"); 
                break;
        }
        curr = curr->next;
    }
}

const char* generate_code(BlockNode* blocks, int count) {
    code_buffer[0] = '\0';
    sprintf(code_buffer, "#include \"leaf_api.h\"\n#include \"raylib.h\"\nint run() {\n");
    for(int i=0; i<count; i++) {
        if (blocks[i].pos.x > 240 && blocks[i].parent == NULL) {
            generate_block_tree(&blocks[i]);
        }
    }
    sprintf(code_buffer + strlen(code_buffer), "    return 0;\n}\n");
    return code_buffer;
}
EOF

cat << 'EOF' > src/ui.c
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
EOF

cat << 'EOF' > src/main.c
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
EOF
```
