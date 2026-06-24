#ifndef RELEASE

#include "EmiBase.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

struct nk_context *ctx = NULL;

bool nk_workEarly = 0;
bool nk_emiObject = 0;
bool nk_workLate = 0;
bool nk_postProcess = 0;
bool nk_overlay = 0;

int NuklearUI_Init()
{
    ctx = InitNuklear(10);

    if (!IsNuklearValid(ctx))
        return 0;

    // ===== GLOBAL STYLE TWEAKS =====
    ctx->style.window.padding = nk_vec2(0, 0);
    ctx->style.window.spacing = nk_vec2(0, 0);

    ctx->style.button.padding = nk_vec2(4, 0);
    ctx->style.menu_button.padding = nk_vec2(4, 0);
    ctx->style.text.padding = nk_vec2(0, 0);
    ctx->style.edit.padding = nk_vec2(0, 0);

    // Make buttons match dark theme
    ctx->style.button.normal  = nk_style_item_color(nk_rgb(28, 28, 28));
    ctx->style.button.hover   = nk_style_item_color(nk_rgb(55, 55, 55));
    ctx->style.button.active  = nk_style_item_color(nk_rgb(70, 70, 70));

    ctx->style.menu_button.normal = nk_style_item_color(nk_rgb(28, 28, 28));
    ctx->style.menu_button.hover  = nk_style_item_color(nk_rgb(55, 55, 55));
    ctx->style.menu_button.active = nk_style_item_color(nk_rgb(70, 70, 70));

    ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(28, 28, 28));
    ctx->style.window.border = 12.0f;
    ctx->style.window.border_color = nk_rgb(60, 60, 60);

    ctx->style.menu_button.text_alignment = NK_TEXT_CENTERED;
    ctx->style.button.text_alignment = NK_TEXT_CENTERED;

    return 1;
}

void NuklearUI_Draw()
{
    nk_input_begin(ctx);
    UpdateNuklear(ctx);

    int w = GetScreenWidth();

    if (nk_begin(ctx, "Menubar",
        nk_rect(0, 0, w, 20),
        NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);

        nk_layout_row_push(ctx, 28);
        if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(140, 200)))
        {
            nk_layout_row_dynamic(ctx, 18, 1);

            if(nk_menu_item_label(ctx, "Save Workspace To File", NK_TEXT_LEFT))
                EmiObject_Serialize();

            nk_layout_row_dynamic(ctx, 4, 1);
            nk_label(ctx, "", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }

        nk_layout_row_push(ctx, 50);
        if (nk_menu_begin_label(ctx, "Toggles", NK_TEXT_LEFT, nk_vec2(100, 200)))
        {
            nk_layout_row_dynamic(ctx, 18, 1);

            nk_checkbox_label(ctx, "WorkEarly", &nk_workEarly);
            nk_checkbox_label(ctx, "EmiObject", &nk_emiObject);
            nk_checkbox_label(ctx, "WorkLate", &nk_workLate);
#if SUPPORTS_POSTPROCESS == 1
            nk_checkbox_label(ctx, "PostProcess", &nk_postProcess);
#endif
            nk_checkbox_label(ctx, "Overlay", &nk_overlay);

            nk_layout_row_dynamic(ctx, 4, 1);
            nk_label(ctx, "", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }

        nk_layout_row_push(ctx, 46);
        if (nk_menu_begin_label(ctx, "Scenes", NK_TEXT_LEFT, nk_vec2(150, 200)))
        {
            nk_layout_row_dynamic(ctx, 18, 1);

            for(int i = 0; i < MAX_SCENES; i++)
            {
                Scene* scene = registered_scenes[i];
                if(scene == NULL)
                    break;
                bool btn = !scene->active;
                bool btn_og = btn;
                nk_checkbox_label(ctx, scene->name, &btn);
                if(!btn && btn_og)
                {
                    _emibase_internal_replacescene(scene);
                }
            }

            nk_layout_row_dynamic(ctx, 4, 1);
            nk_label(ctx, "", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }

        /* RIGHT SIDE */
        nk_layout_row_push(ctx, w - 128);

        nk_label(ctx, "EmiBase " EMIBASE_VER " (" GIT_HASH GIT_DIRTY ") | " PROJECT_NAME " " PROJECT_VER, NK_TEXT_RIGHT);

        nk_layout_row_end(ctx);
    }
    nk_end(ctx);

    DrawNuklear(ctx);
    nk_input_end(ctx);
}

#endif