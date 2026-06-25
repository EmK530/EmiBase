#ifndef RELEASE

#include <float.h>
#include <limits.h>
#include <string.h>

#include "EmiBase.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

struct nk_context *ctx = NULL;
EObject* nk_selected_object = NULL;

bool nk_workEarly = 0;
bool nk_emiObject = 0;
bool nk_workLate = 0;
bool nk_postProcess = 0;
bool nk_overlay = 0;

bool _nk_window_workspace = 0;

int NuklearUI_Init()
{
    ctx = InitNuklear(10);

    if (!IsNuklearValid(ctx))
        return 0;

    // ===== GLOBAL STYLE TWEAKS =====
    ctx->style.window.padding = nk_vec2(0, 0);
    ctx->style.window.spacing = nk_vec2(0, 0);

    ctx->style.button.padding = nk_vec2(0, 0);
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
    ctx->style.window.border = 1.0f;
    ctx->style.window.border_color = nk_rgb(60, 60, 60);

    ctx->style.menu_button.text_alignment = NK_TEXT_CENTERED;
    ctx->style.button.text_alignment = NK_TEXT_CENTERED;

    ctx->style.selectable.normal         = nk_style_item_color(nk_rgba(37, 37, 37, 255));
    ctx->style.selectable.hover          = nk_style_item_color(nk_rgba(60, 60, 60, 255));
    ctx->style.selectable.pressed_active = nk_style_item_color(nk_rgba(32, 68, 136, 255));
    ctx->style.selectable.normal_active  = nk_style_item_color(nk_rgba(42, 87, 178, 255));
    ctx->style.selectable.hover_active   = nk_style_item_color(nk_rgba(60, 110, 210, 255));

    return 1;
}

static void Workspace_DrawHierarchyNode(EObject* object, int depth)
{
    bool has_children = object->Children != NULL && object->Children->size > 0;
    bool is_selected = (nk_selected_object == object);

    nk_layout_row_begin(ctx, NK_STATIC, 18, 3);
    nk_layout_row_push(ctx, depth * 12);
    nk_label(ctx, "", NK_TEXT_LEFT);

    nk_layout_row_push(ctx, 18);
    if (has_children)
    {
        const char* toggle = object->_nk_expanded ? "-" : "+";
        if (nk_button_label(ctx, toggle))
            object->_nk_expanded = !object->_nk_expanded;
    }
    else
    {
        nk_label(ctx, " ", NK_TEXT_LEFT);
    }

    struct nk_rect bounds = nk_window_get_content_region(ctx);
    nk_layout_row_push(ctx, bounds.w - (depth * 12) - 18);
    if (nk_select_label(ctx, object->Name ? object->Name : "(null)", NK_TEXT_LEFT, is_selected))
        nk_selected_object = object;

    nk_layout_row_end(ctx);

    if (has_children && object->_nk_expanded)
    {
        LinkedList_foreach(object->Children, node)
        {
            EObject* child = (EObject*)node->item;
            Workspace_DrawHierarchyNode(child, depth + 1);
        }
    }
}

// Tracks which object the name buffer belongs to so we can reset it on selection change
static EObject*  nk_name_buf_owner = NULL;
static char      nk_name_buf[128]  = {0};
static int       nk_name_buf_len   = 0;
static bool      nk_name_editing   = false;

static void Workspace_DrawRenamePopup(EObject* object)
{
    if (!nk_name_editing || object == NULL)
        return;

    // Draw dim overlay over the workspace content
    struct nk_rect content = nk_window_get_content_region(ctx);
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    nk_fill_rect(canvas, content, 0.0f, nk_rgba(0, 0, 0, 120));

    struct nk_rect popup_rect = nk_rect(10, 10, 200, 105);

    if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Rename",
        NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR,
        popup_rect))
    {
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_flags event = nk_edit_string(ctx,
            NK_EDIT_FIELD | NK_EDIT_SIG_ENTER,
            nk_name_buf, &nk_name_buf_len, 128, nk_filter_default);

        nk_layout_row_dynamic(ctx, 22, 2);
        if (nk_button_label(ctx, "OK") || (event & NK_EDIT_COMMITED))
        {
            nk_name_buf[nk_name_buf_len] = '\0';
            object->SetName(object, nk_name_buf);
            nk_name_editing = false;
            nk_popup_close(ctx);
        }
        if (nk_button_label(ctx, "Cancel"))
        {
            nk_name_editing = false;
            nk_popup_close(ctx);
        }

        nk_popup_end(ctx);
    }
    else
    {
        // Popup was closed via the X button
        nk_name_editing = false;
    }
}

static void Workspace_DrawProperties(EObject* object, float panel_height)
{
    // Reset name buffer when selection changes
    if (object != nk_name_buf_owner)
    {
        nk_name_buf_owner = object;
        nk_name_editing = false;
        if (object->Name)
        {
            nk_name_buf_len = (int)strlen(object->Name);
            if (nk_name_buf_len > 127) nk_name_buf_len = 127;
            memcpy(nk_name_buf, object->Name, nk_name_buf_len);
            nk_name_buf[nk_name_buf_len] = '\0';
        }
        else
        {
            nk_name_buf[0]  = '\0';
            nk_name_buf_len = 0;
        }
    }

    // --- Name ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_label(ctx, "Name", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 22, 2);
    nk_label(ctx, object->Name ? object->Name : "(unnamed)", NK_TEXT_LEFT);
    if (nk_button_label(ctx, "Rename"))
    {
        nk_name_editing = true;
    }

    // --- Position ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_label(ctx, "Position", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 22, 2);
    nk_property_float(ctx, "#X.Scale",  -FLT_MAX, &object->Position.X.Scale,  FLT_MAX, 0.01f, 0.005f);
    nk_property_int  (ctx, "#X.Offset", INT_MIN,  (int*)&object->Position.X.Offset, INT_MAX, 1, 1);

    nk_layout_row_dynamic(ctx, 22, 2);
    nk_property_float(ctx, "#Y.Scale",  -FLT_MAX, &object->Position.Y.Scale,  FLT_MAX, 0.01f, 0.005f);
    nk_property_int  (ctx, "#Y.Offset", INT_MIN,  (int*)&object->Position.Y.Offset, INT_MAX, 1, 1);

    // --- Size ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_label(ctx, "Size", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 22, 2);
    nk_property_float(ctx, "X.Scale",  0.0f, &object->Size.X.Scale,  FLT_MAX, 0.01f, 0.005f);
    nk_property_int  (ctx, "X.Offset", 0,    (int*)&object->Size.X.Offset, INT_MAX, 1, 1);

    nk_layout_row_dynamic(ctx, 22, 2);
    nk_property_float(ctx, "Y.Scale",  0.0f, &object->Size.Y.Scale,  FLT_MAX, 0.01f, 0.005f);
    nk_property_int  (ctx, "Y.Offset", 0,    (int*)&object->Size.Y.Offset, INT_MAX, 1, 1);

    // --- Rotation ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_label(ctx, "Rotation", NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 22, 1);
    nk_property_float(ctx, "°", 0.0f, &object->Rotation, 360.0f, 0.5f, 0.5f);

    // --- Anchor ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_label(ctx, "Anchor", NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 22, 2);
    nk_property_float(ctx, "X", -FLT_MAX, &object->Anchor.x, FLT_MAX, 0.01f, 0.005f);
    nk_property_float(ctx, "Y", -FLT_MAX, &object->Anchor.y, FLT_MAX, 0.01f, 0.005f);

    // --- Visible, renamed to Hidden because of checkbox inversion ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_checkbox_label(ctx, "Hidden", (nk_bool*)&object->Visible);

    // --- ZIndex ---
    nk_layout_row_dynamic(ctx, 18, 1);
    nk_label(ctx, "ZIndex", NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 22, 1);
    nk_property_int(ctx, "#", 0, (int*)&object->ZIndex, 255, 1, 1);

    // --- ERect specific (type ID 1) ---
    if (object->_item != NULL && object->_item->innerType == 1)
    {
        ERect* rect = (ERect*)object->_item;

        nk_layout_row_dynamic(ctx, 8, 1);
        nk_label(ctx, "", NK_TEXT_LEFT); // small spacer

        nk_layout_row_dynamic(ctx, 18, 1);
        nk_label(ctx, "-- ERect --", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 18, 1);
        nk_label(ctx, "Color", NK_TEXT_LEFT);

        // Convert Raylib Color to nk_colorf for the picker
        struct nk_colorf nk_col = {
            rect->Color.r / 255.0f,
            rect->Color.g / 255.0f,
            rect->Color.b / 255.0f,
            rect->Color.a / 255.0f
        };

        nk_layout_row_dynamic(ctx, 200, 1);
        nk_col = nk_color_picker(ctx, nk_col, NK_RGBA);

        // Write back
        rect->Color.r = (uint8_t)(nk_col.r * 255.0f);
        rect->Color.g = (uint8_t)(nk_col.g * 255.0f);
        rect->Color.b = (uint8_t)(nk_col.b * 255.0f);
        rect->Color.a = (uint8_t)(nk_col.a * 255.0f);
    }
}

static void Workspace_DrawWorkspace()
{
    static float split_y = 0.0f; // 0 = uninitialized
    static bool  dragging = false;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // Default window dimensions
    float win_w = 400.0f, win_h = (float)(sh - 20);
    float win_x = sw-win_w, win_y = 20.0f;

    if (nk_begin(ctx, "Workspace",
        nk_rect(win_x, win_y, win_w, win_h),
        NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
    {
        struct nk_rect content = nk_window_get_content_region(ctx);

        // Initialize split_y on first run to 60% of content height
        if (split_y == 0.0f)
            split_y = content.h * 0.6f;

        // Clamp split so neither panel collapses entirely
        float min_pane = 50.0f;
        if (split_y < min_pane)              split_y = min_pane;
        if (split_y > content.h - min_pane)  split_y = content.h - min_pane;

        float divider_h = 6.0f;
        float hier_h    = split_y - (divider_h / 2.0f);
        float props_h   = content.h - split_y - (divider_h / 2.0f);

        // ---- Hierarchy pane ----
        nk_layout_row_dynamic(ctx, hier_h, 1);
        if (nk_group_begin(ctx, "HierarchyPane", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER))
        {
            LinkedList_foreach(root_objects, node)
            {
                EObject* object = (EObject*)node->item;
                Workspace_DrawHierarchyNode(object, 0);
            }
            nk_group_end(ctx);
        }

        // ---- Divider / drag handle ----
        nk_layout_row_dynamic(ctx, divider_h, 1);
        struct nk_rect divider_rect = nk_widget_bounds(ctx);
        nk_label(ctx, "", NK_TEXT_LEFT);

        // Detect drag on divider
        struct nk_mouse* mouse = &ctx->input.mouse;
        bool hovering = nk_input_is_mouse_hovering_rect(&ctx->input, divider_rect);

        if (hovering && nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT))
            dragging = true;
        if (!nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT))
            dragging = false;
        if (dragging)
            split_y += mouse->delta.y;

        // ---- Properties pane ----
        nk_layout_row_dynamic(ctx, props_h, 1);
        if (nk_group_begin(ctx, "PropertiesPane", NK_WINDOW_BORDER))
        {
            if (nk_selected_object != NULL)
                Workspace_DrawProperties(nk_selected_object, props_h);
            else
            {
                nk_layout_row_dynamic(ctx, 18, 1);
                nk_label(ctx, "No object selected.", NK_TEXT_CENTERED);
            }
            nk_group_end(ctx);
        }
    }

    struct nk_rect bounds = nk_window_get_bounds(ctx);

    bounds.w = NK_MIN(bounds.w, sw);
    bounds.h = NK_MIN(bounds.h, sh - 20); // 20 = menubar height

    bounds.x = NK_CLAMP(0, bounds.x, sw - bounds.w);
    bounds.y = NK_CLAMP(20, bounds.y, sh - bounds.h);

    nk_window_set_bounds(ctx, "Workspace", bounds);

    if(nk_name_editing)
        Workspace_DrawRenamePopup(nk_name_buf_owner);

    nk_end(ctx);
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
        nk_layout_row_begin(ctx, NK_STATIC, 20, 5);

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
                    nk_selected_object = NULL;
                    nk_name_buf_owner = NULL;
                    nk_name_buf[0]  = '\0';
                    nk_name_buf_len = 0;
                }
            }

            nk_layout_row_dynamic(ctx, 4, 1);
            nk_label(ctx, "", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }

        nk_layout_row_push(ctx, 43);
        if (nk_menu_begin_label(ctx, "Panels", NK_TEXT_LEFT, nk_vec2(100, 200)))
        {
            nk_layout_row_dynamic(ctx, 18, 1);

            nk_checkbox_label(ctx, "Workspace", &_nk_window_workspace);

            nk_layout_row_dynamic(ctx, 4, 1);
            nk_label(ctx, "", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }

        /* RIGHT SIDE */
        nk_layout_row_push(ctx, w - 171);

        nk_label(ctx, "EmiBase " EMIBASE_VER " (" GIT_HASH GIT_DIRTY ") | " PROJECT_NAME " " PROJECT_VER, NK_TEXT_RIGHT);

        nk_layout_row_end(ctx);
    }
    nk_end(ctx);

    if(_nk_window_workspace == 0)
    {
        Workspace_DrawWorkspace();
        //Window_Workspace();
    }

    DrawNuklear(ctx);
    nk_input_end(ctx);
}

#endif