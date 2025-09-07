#include "../firestring.h"

const char* str_len[] = {"4", "8", "12", "16", "24", "32", "48", "64", "256", "512"};
const char* type_strings[] = {"AlphNumSymb", "AlphNum", "Alpha", "Symbols", "Numeric", "Binary"};
const char* use_ir_strings[] = {"False", "True"};

uint8_t get_str_len_index(uint32_t num) {
    uint8_t index = 0;
    for(uint8_t i = 0; i < COUNT_OF(str_len); i++) {
        if(num == (uint32_t)atoi(str_len[i])) {
            index = i;
            break;
        }
    }
    return index;
}

void str_type_change_callback(VariableItem* item) {
    FireString* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, type_strings[index]);
    app->settings->str_type = index;
}

void str_len_change_callback(VariableItem* item) {
    FireString* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, str_len[index]);
    app->settings->str_len = atoi(str_len[index]);
}

void use_ir_change_callback(VariableItem* item) {
    FireString* app = variable_item_get_context(item);
    app->settings->use_ir = !app->settings->use_ir;
    variable_item_set_current_value_text(item, use_ir_strings[(uint8_t)app->settings->use_ir]);
}

void settings_enter_callback(void* context, uint32_t index) {
    FURI_LOG_T(TAG, "settings_enter_callback");
    UNUSED(index);
    furi_check(context);

    FireString* app = context;

    if(scene_manager_search_and_switch_to_previous_scene(
           app->scene_manager, FireStringScene_Generate)) {
    } else {
        scene_manager_next_scene(app->scene_manager, FireStringScene_Generate);
    }
}

void fire_string_scene_on_enter_settings(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_settings");
    furi_check(context);

    FireString* app = context;
    const uint8_t curr_str_len_index = get_str_len_index(app->settings->str_len);
    const char* curr_str_str = str_len[curr_str_len_index];

    VariableItem* strLen = variable_item_list_add(
        app->variable_item_list, "String Length", COUNT_OF(str_len), str_len_change_callback, app);
    variable_item_set_current_value_index(strLen, curr_str_len_index);
    variable_item_set_current_value_text(strLen, curr_str_str);

    VariableItem* strType = variable_item_list_add(
        app->variable_item_list,
        "String Type",
        COUNT_OF(type_strings),
        str_type_change_callback,
        app);
    variable_item_set_current_value_index(strType, app->settings->str_type);
    variable_item_set_current_value_text(strType, type_strings[app->settings->str_type]);

    VariableItem* use_ir = variable_item_list_add(
        app->variable_item_list, "Use IR", COUNT_OF(use_ir_strings), use_ir_change_callback, app);
    variable_item_set_current_value_index(use_ir, (uint8_t)app->settings->use_ir);
    variable_item_set_current_value_text(use_ir, use_ir_strings[(uint8_t)app->settings->use_ir]);

    variable_item_list_set_enter_callback(app->variable_item_list, settings_enter_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_VariableItemList);
}

bool fire_string_scene_on_event_settings(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "fire_string_scene_on_event_settings");
    furi_check(context);

    FireString* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void fire_string_scene_on_exit_settings(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_exit_settings");
    furi_check(context);

    FireString* app = context;
    variable_item_list_reset(app->variable_item_list);
}
