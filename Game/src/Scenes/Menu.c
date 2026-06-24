#define SCENE_NAME Menu
#include "EmiBase.h"

static Shader crt;
static Shader shift;

void SFunc (Init)(Scene *s) {
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

    eprintf("[Scene." SCENE_STR "] Initialized\n");
}

ERect* source = NULL;

void SFunc (Prepare)(Scene *s) {
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

    eprintf("[Scene." SCENE_STR "] Prepared\n");
}

static double time = 0.0;

void SFunc (OnInput)(Scene *s, int e) {}

const SceneResult SFunc (WorkEarly)(Scene *s, double deltaTime) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    FontManager_DrawText("MxPlus_IBM_VGA_8x16.ttf", height/24, "Currently displaying: Scene 'Menu'", Vector2_new(20, 20), WHITE);
    
    source->core->Rotation += deltaTime*180;

    return (SceneResult){ SCENE_NONE, NULL };
}

const SceneResult SFunc (WorkLate)(Scene *s, double deltaTime) {
    return (SceneResult){ SCENE_NONE, NULL };
}

DEFINE_SCENE(SCENE_NAME)