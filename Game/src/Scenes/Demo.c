#define SCENE_NAME Demo
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
    EmiObject_Deserialize("object/output.eobj");

    ERect* container = ERect_Create(NULL);
    container->core->Position = UDim2_fromScale(0, 0);
    container->core->Size = UDim2_fromScale(1, 1);
    container->Color = Color32_new(0, 0, 0, 0);
    container->core->SetName(container->core, "Background");

    eprintf("[Scene." SCENE_STR "] Prepared\n");
}

void SFunc (OnInput)(Scene *s, int e) {}

const SceneResult SFunc (WorkEarly)(Scene *s, double deltaTime) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    FontManager_DrawText("MxPlus_IBM_VGA_8x16.ttf", height/24, "Currently displaying: Scene 'Menu'", Vector2_new(20, 20), WHITE);

    return (SceneResult){ SCENE_NONE, NULL };
}

const SceneResult SFunc (WorkLate)(Scene *s, double deltaTime) {
    return (SceneResult){ SCENE_NONE, NULL };
}

DEFINE_SCENE(SCENE_NAME)