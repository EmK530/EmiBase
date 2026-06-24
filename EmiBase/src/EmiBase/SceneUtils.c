#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "EmiBase/SceneUtils.h"

// --- stack ---
SceneStack scene_stack = { .top = -1 };

// --- registered scenes ---
Scene *registered_scenes[MAX_SCENES] = {0};
int scene_count = 0;

// --- registration ---
void register_scene(Scene *s) {
    if (scene_count < MAX_SCENES) {
        registered_scenes[scene_count++] = s;
        if(s->Init) s->Init(s);
    } else {
        fprintf(stderr, "Error: max scenes reached\n");
    }
}

Scene* find_scene(const char* name) {
    for(int i = 0; i < scene_count; ++i) {
        if(strcmp(registered_scenes[i]->name, name) == 0)
            return registered_scenes[i];
    }
    return NULL; // not found
}

// --- stack operations ---
void PushScene(Scene *s) {
    if(scene_stack.top + 1 < MAX_SCENES) {
        s->active = true;
        scene_stack.scenes[++scene_stack.top] = s;
        if(s->Prepare) s->Prepare(s);
    } else {
        fprintf(stderr, "Error: scene stack overflow\n");
    }
}

void PopScene() {
    if(scene_stack.top >= 0) {
        Scene* tgt = scene_stack.scenes[scene_stack.top--];
        tgt->active = false;
        scene_stack.scenes[scene_stack.top--] = NULL;
    } else {
        fprintf(stderr, "Error: scene stack underflow\n");
    }
}

Scene* TopScene() {
    if(scene_stack.top >= 0) return scene_stack.scenes[scene_stack.top];
    return NULL;
}