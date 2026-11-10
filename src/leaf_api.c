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
