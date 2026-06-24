#include <stdio.h>

#include "EmiBase.h"

typedef struct {
    int frame_count;
} MenuData;

static Shader crt;
static Shader shift;

void Menu_Init(Scene *s) {
    MenuData *data = MemAlloc(sizeof(MenuData));
    data->frame_count = 0;
    s->data = data;

    crt = ContentManager_LoadShader(NULL, "shader/crt.glsl");
    shift = ContentManager_LoadShader(NULL, "shader/shift.glsl");

    PostProcess_RegisterScene(s);
    if(IsShaderValid(crt)) PostProcess_AddShader(s, &crt);
    if(IsShaderValid(shift)) {
        PostProcess_AddShader(s, &shift);
        int intLoc = GetShaderLocation(shift, "intensity");
        float intensity = 0.001;
        if (intLoc != -1) SetShaderValue(shift, intLoc, &intensity, SHADER_UNIFORM_FLOAT);
    }

    eprintf("[MenuScene] Initialized\n");
}

ERect* source = NULL;

void Menu_Prepare(Scene *s) {
    MenuData *data = (MenuData*)s->data;
    data->frame_count = 0;

    source = ERect_Create(NULL);
    source->core->Position = UDim2_fromScale(0.5, 0.5);
    source->core->Anchor = Vector2_new(0.5, 0.5);

    ERect* o1 = ERect_Create(source->core);
    o1->core->Position = UDim2_fromScale(-1.0, 0.5);
    o1->core->Anchor = Vector2_new(0.5, 0.5);
    o1->core->Size = UDim2_fromOffset(50, 50);
    o1->Color = RED;

    ERect* o2 = ERect_Create(source->core);
    o2->core->Position = UDim2_fromScale(0.5, -1.0);
    o2->core->Anchor = Vector2_new(0.5, 0.5);
    o2->core->Size = UDim2_fromOffset(50, 50);
    o2->Color = ORANGE;

    ERect* o3 = ERect_Create(source->core);
    o3->core->Position = UDim2_fromScale(1.0, 0.5);
    o3->core->Anchor = Vector2_new(-1.5, 0.5);
    o3->core->Size = UDim2_fromOffset(50, 50);
    o3->Color = YELLOW;

    ERect* o4 = ERect_Create(source->core);
    o4->core->Position = UDim2_fromScale(0.5, 1.0);
    o4->core->Anchor = Vector2_new(0.5, -1.5);
    o4->core->Size = UDim2_fromOffset(50, 50);
    o4->Color = GREEN;

    EmiObject_Serialize();

    eprintf("[MenuScene] Prepared\n");
}

static double time = 0.0;

void Menu_OnInput(Scene *s, int e) {
    MenuData *data = (MenuData*)s->data;
}

const SceneResult Menu_WorkEarly(Scene *s, double deltaTime) {
    MenuData *data = (MenuData*)s->data;
    data->frame_count++;
    if(data->frame_count > 1)
        time += deltaTime;

    int width = GetScreenWidth();
    int height = GetScreenHeight();

    FontManager_DrawText("MxPlus_IBM_VGA_8x16.ttf", height/24, "Currently displaying: Scene 'Menu'", Vector2_new(20, 20), WHITE);
    
    source->core->Rotation += deltaTime*180;

    return (SceneResult){ SCENE_NONE, NULL };
}

const SceneResult Menu_WorkLate(Scene *s, double deltaTime) {
    MenuData *data = (MenuData*)s->data;

    (void)data;
    (void)deltaTime;

    return (SceneResult){ SCENE_NONE, NULL };
}

Scene MenuScene = {
    .name = "Menu",
    .Init = Menu_Init,
    .Prepare = Menu_Prepare,
    .OnInput = Menu_OnInput,
    .WorkEarly = Menu_WorkEarly,
    .WorkLate = Menu_WorkLate,
    .data = NULL
};

void Register_MenuScene() {
    register_scene(&MenuScene);
}