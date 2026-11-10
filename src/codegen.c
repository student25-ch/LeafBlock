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
