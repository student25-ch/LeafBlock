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
