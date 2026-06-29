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

    eprintf("[Scene." SCENE_STR "] Prepared\n");
}

void SFunc (OnInput)(Scene *s, int e) {}

const SceneResult SFunc (WorkEarly)(Scene *s, double deltaTime) {
    int height = GetScreenHeight();
    FontManager_DrawText("MxPlus_IBM_VGA_8x16.ttf", height/24, "Welcome to the 'Demo' scene!", Vector2_new(20, 20), WHITE);

    return (SceneResult){ SCENE_NONE, NULL };
}

const SceneResult SFunc (WorkLate)(Scene *s, double deltaTime) {
    int height = GetScreenHeight();
    FontManager_DrawText("MxPlus_IBM_VGA_8x16.ttf", height/32, "This text is drawn in WorkLate.", Vector2_new(20, 20+height/22), WHITE);

    return (SceneResult){ SCENE_NONE, NULL };
}

DEFINE_SCENE(SCENE_NAME)