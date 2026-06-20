#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "raylib.h"
#include "rlgl.h"
#include "EmiBase.h"

/* Main execution script for EmiBase, you shouldn't need to modify this beyond defining new scenes. */

extern void Register_MenuScene();

int screenWidth = RES_X;
int screenHeight = RES_Y;

static void DrawOverlay()
{
    DrawFPS(10, screenHeight - 27);
    // Add anything else you may want
}

// Road ends here :)

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Draw EObjects recursively
void Recursive_EObject_Draw(EObject* object, ETransform* parent)
{
    ETransform current;

    object->_render(object, parent, &current);

    LinkedList_foreach(object->Children, child)
    {
        EObject* co = (EObject*)child->item;
        Recursive_EObject_Draw(co, &current);
    }
}

int main()
{
    Register_MenuScene();

    //SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, PROJECT_NAME " " BUILD_IDENT);

    SetDarkTitleBar();

    int initResult = EmiBase_Init(screenWidth, screenHeight);
    if(initResult != 0) {
        printf("Could not initialize EmiBase\n");
        if(initResult == 1) WinMessageBox("Fatal error!", "Could not initialize EmiBase systems!", MB_TOPMOST | MB_ICONERROR);
        return 1;
    }

#if SUPPORTS_POSTPROCESS == 1
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
#endif

    SetTargetFPS(FPS_LIMIT);

    printf("[Main] Ready!\n");

    Scene *start = find_scene("Menu");

    if (start) PushScene(start);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
#if SUPPORTS_POSTPROCESS == 1
        if (IsWindowResized())
        {
            PostProcess_Resize(screenWidth, screenHeight);
            UnloadRenderTexture(target);
            target = LoadRenderTexture(screenWidth, screenHeight);
        }
#endif

        Scene *top = TopScene();

        if (top && top->OnInput) {
            int key;
            while ((key = GetKeyPressed()) != 0)
                top->OnInput(top, key);
        }

#if SUPPORTS_POSTPROCESS == 1
        BeginTextureMode(target);
#else
        BeginDrawing();
#endif
            ClearBackground(BLACK);

            for (int i = 0; i <= scene_stack.top; i++) {
                Scene *s = scene_stack.scenes[i];
                if (s && s->WorkEarly) {

                    SceneResult res = s->WorkEarly(s, deltaTime);

                    if (res.action != SCENE_NONE && res.name) {
                        Scene *new_scene = find_scene(res.name);

                        if (new_scene) {
                            if (res.action == SCENE_PUSH) {
                                PushScene(new_scene);
                            }
                            else if (res.action == SCENE_REPLACE) {
                                PopScene(); // remove current
                                PushScene(new_scene);
                            }
                        } else {
                            printf("[Main] Failed to jump to nonexistent scene: %s\n", res.name);
                        }
                    }
                }
            }

            Vector2 res = { (float)screenWidth, (float)screenHeight };
            ETransform root = {
                .Position = {0.0f, 0.0f},
                .Size     = {res.x, res.y},
                .Rotation = 0.0f,
                .Anchor   = {0.0f, 0.0f}
            };

            LinkedList_foreach(root_objects, obj)
            {
                EObject *object = (EObject *)obj->item;
                Recursive_EObject_Draw(object, &root);
            }

            for (int i = 0; i <= scene_stack.top; i++) {
                Scene *s = scene_stack.scenes[i];
                if (s && s->WorkLate) {

                    SceneResult res = s->WorkLate(s, deltaTime);

                    if (res.action != SCENE_NONE && res.name) {
                        Scene *new_scene = find_scene(res.name);

                        if (new_scene) {
                            if (res.action == SCENE_PUSH) {
                                PushScene(new_scene);
                            }
                            else if (res.action == SCENE_REPLACE) {
                                PopScene();          // remove current
                                PushScene(new_scene);
                            }
                        } else {
                            printf("[Main] Failed to jump to nonexistent scene: %s\n", res.name);
                        }
                    }
                }
            }

#if SUPPORTS_POSTPROCESS == 1
        EndTextureMode();
        PostProcess_Apply(&target, TopScene(), GetTime(), screenWidth, screenHeight, DrawOverlay);
#else
        DrawOverlay();
        EndDrawing();
#endif

        AudioManager_Update();
    }

    PostProcess_Cleanup();
    CloseWindow();
    return 0;
}