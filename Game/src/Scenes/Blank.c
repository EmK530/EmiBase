#include <stdio.h>

#define SCENE_NAME Blank
#include "EmiBase.h"

void SFunc (Init)(Scene *s) {
    eprintf("[Scene." SCENE_STR "] Initialized\n");
}

void SFunc (Prepare)(Scene *s) {
    eprintf("[Scene." SCENE_STR "] Prepared\n");
}

void SFunc (OnInput)(Scene *s, int e) {

}

const SceneResult SFunc (WorkEarly)(Scene *s, double deltaTime) {
    return (SceneResult){ SCENE_NONE, NULL };
}

const SceneResult SFunc (WorkLate)(Scene *s, double deltaTime) {
    return (SceneResult){ SCENE_NONE, NULL };
}

DEFINE_SCENE(SCENE_NAME)