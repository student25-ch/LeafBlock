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
