#include "../fire_string.h"

typedef enum {
    FireStringSaveResult_Success,
    FireStringSaveResult_FileExists,
    FireStringSaveResult_WriteFailed,
} FireStringSaveResult;

static FireStringSaveResult save_result = FireStringSaveResult_Success;

void text_input_callback(void* context) {
    FURI_LOG_T(TAG, "text_input_callback");
    furi_check(context);
    FireString* app = context;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    FuriString* file_name = furi_string_alloc();
    furi_string_printf(file_name, "%s%s%s", DEFAULT_PATH, app->text_buffer, FILE_EXT);

    bool file_opened =
        storage_file_open(file, furi_string_get_cstr(file_name), FSAM_WRITE, FSOM_CREATE_NEW);

    if(!file_opened) {
        FURI_LOG_E(TAG, "File already exists: %s", furi_string_get_cstr(file_name));
        save_result = FireStringSaveResult_FileExists;
    } else {
        size_t expected = furi_string_size(app->fire_string);
        size_t written =
            storage_file_write(file, furi_string_get_cstr(app->fire_string), expected);

        if(written != expected) {
            FURI_LOG_E(TAG, "Write failed: wrote %zu of %zu bytes", written, expected);
            save_result = FireStringSaveResult_WriteFailed;
        } else {
            save_result = FireStringSaveResult_Success;
        }

        storage_file_close(file);
    }

    furi_string_free(file_name);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    view_dispatcher_send_custom_event(app->view_dispatcher, FireStringCustomEvent_SaveDone);
}

void fire_string_scene_on_enter_save_string(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_save_string");
    furi_check(context);
    FireString* app = context;
    text_input_set_result_callback(
        app->text_input, text_input_callback, app, app->text_buffer, TEXT_INPUT_BUF_SIZE, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_TextInput);
}

bool fire_string_scene_on_event_save_string(void* context, SceneManagerEvent event) {
    FireString* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == FireStringCustomEvent_SaveDone) {
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_buttons(message, NULL, "ok!", NULL);

        if(save_result == FireStringSaveResult_FileExists) {
            FURI_LOG_E(TAG, "Save failed: file exists");
            dialog_message_set_header(message, "File Exists...", 82, 17, AlignCenter, AlignTop);
            dialog_message_set_icon(message, &I_WarningDolphin_45x42, 0, 2);
        } else if(save_result == FireStringSaveResult_WriteFailed) {
            FURI_LOG_E(TAG, "Save failed: write error");
            dialog_message_set_header(
                message, "Failed to write to file...", 82, 17, AlignCenter, AlignTop);
            dialog_message_set_icon(message, &I_WarningDolphin_45x42, 0, 2);
        } else {
            dialog_message_set_header(message, "Success!", 98, 17, AlignCenter, AlignTop);
            dialog_message_set_icon(message, &I_DolphinMafia_119x62, 0, 2);
        }

        dialog_message_show(dialogs, message);
        dialog_message_free(message);
        furi_record_close(RECORD_DIALOGS);

        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, FireStringScene_GenerateStepTwo);

        consumed = true;
    }

    return consumed;
}

void fire_string_scene_on_exit_save_string(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_exit_save_string");
    furi_check(context);
    FireString* app = context;
    text_input_reset(app->text_input);
}
