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
