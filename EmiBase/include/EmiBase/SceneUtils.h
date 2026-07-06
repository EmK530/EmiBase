#pragma once

#define MAX_SCENES 64

typedef enum {
    SCENE_NONE,
    SCENE_PUSH,
    SCENE_REPLACE,
    SCENE_POP
} SceneAction;

typedef struct {
    SceneAction action;
    const char* name;
} SceneResult;

typedef struct Scene {
    const char *name;
    void (*Init)(struct Scene *s);
    void (*Prepare)(struct Scene *s);
    void (*Cleanup)(struct Scene *s);
    void (*OnInput)(struct Scene *s, int e);
    const SceneResult (*WorkEarly)(struct Scene *s, double deltaTime); // return name of next scene, or NULL
    const SceneResult (*WorkLate)(struct Scene *s, double deltaTime); // return name of next scene, or NULL
    void *misc; // Store anything you want here, you can allocate a struct on the scene-end.
    bool active;
} Scene;

#define DEFINE_SCENE_INTERNAL(scene_name) \
    Scene scene_name##Scene = { \
        .name = #scene_name, \
        .Init = scene_name##_Init, \
        .Prepare = scene_name##_Prepare, \
        .Cleanup = scene_name##_Cleanup, \
        .OnInput = scene_name##_OnInput, \
        .WorkEarly = scene_name##_WorkEarly, \
        .WorkLate = scene_name##_WorkLate, \
        .misc = NULL, \
        .active = false \
    }; \
    void Register_##scene_name() { \
        register_scene(&scene_name##Scene); \
    }

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)
#define DEFINE_SCENE(name) DEFINE_SCENE_INTERNAL(name)
#define CONCAT_INNER(a, b) a##_##b
#define CONCAT(a, b) CONCAT_INNER(a, b)
#ifndef SCENE_NAME
    // This should never happen, you should have defined SCENE_NAME before requiring EmiBase.h if you are coding in a scene.
    #define SCENE_NAME Unknown
#endif
#define SFunc(suffix) CONCAT(SCENE_NAME, suffix) 
#define SCENE_STR STRINGIFY(SCENE_NAME)

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