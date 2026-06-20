#pragma once

#define MAX_SCENES 16

typedef enum {
    SCENE_NONE,
    SCENE_PUSH,
    SCENE_REPLACE
} SceneAction;

typedef struct {
    SceneAction action;
    const char* name;
} SceneResult;

typedef struct Scene {
    const char *name;
    void (*Init)(struct Scene *s);
    void (*Prepare)(struct Scene *s);
    void (*OnInput)(struct Scene *s, int e);
    const SceneResult (*WorkEarly)(struct Scene *s, double deltaTime); // return name of next scene, or NULL
    const SceneResult (*WorkLate)(struct Scene *s, double deltaTime); // return name of next scene, or NULL
    void *data;
} Scene;

typedef struct SceneStack {
    Scene *scenes[MAX_SCENES];
    int top;
} SceneStack;

// --- scene stack ---
extern SceneStack scene_stack;

// --- registered scenes ---
extern Scene *registered_scenes[MAX_SCENES];
extern int scene_count;

// --- scene management ---
void register_scene(Scene *s);
Scene* find_scene(const char* name);

void PushScene(Scene *s);
void PopScene();
Scene* TopScene();