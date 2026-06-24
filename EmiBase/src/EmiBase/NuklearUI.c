#ifndef RELEASE

#include "EmiBase.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

struct nk_context *ctx = NULL;

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
        nk_layout_row_begin(ctx, NK_STATIC, 20, 2);

        /* LEFT SIDE (menu) */
        nk_layout_row_push(ctx, 32);

        if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(180, 200)))
        {
            nk_layout_row_dynamic(ctx, 18, 1);

            nk_menu_item_label(ctx, "Test Button 1", NK_TEXT_LEFT);
            nk_menu_item_label(ctx, "Test Button 2", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 4, 1);
            nk_label(ctx, "", NK_TEXT_LEFT);

            nk_menu_end(ctx);
        }

        /* RIGHT SIDE */
        nk_layout_row_push(ctx, w - 36);

        nk_label(ctx, "EmiBase " EMIBASE_VER " (" GIT_HASH GIT_DIRTY ") | " PROJECT_NAME " " PROJECT_VER, NK_TEXT_RIGHT);

        nk_layout_row_end(ctx);
    }
    nk_end(ctx);

    DrawNuklear(ctx);
    nk_input_end(ctx);
}

#endif