#include "../fire_string.h"
#include "string_generator.h"

static void usb_scene_builder(FireString* app);

#define SPAM_UNLOCK 5

uint8_t right_btn_clk_cnt = 0;
bool usb_state = false;

bool ducky_string(FireString* app) {
    if(furi_string_size(app->fire_string) == 0) {
        return false;
    }
    const char* param = furi_string_get_cstr(app->fire_string);
    uint16_t keycode = HID_KEYBOARD_NONE;
    uint32_t i = 0;
    while(param[i] != '\0') {
        keycode = ASCII_TO_KEY(app, param[i]);
        if(keycode != HID_KEYBOARD_NONE) {
            app->hid->api->kb_press(app->hid->hid_inst, keycode);
            app->hid->api->kb_release(app->hid->hid_inst, keycode);
        }
        i++;
    }
    return true;
}

void usb_btn_callback(GuiButtonType result, InputType type, void* context) {
    FURI_LOG_T(TAG, "usb_btn_callback");
    furi_check(context);

    FireString* app = context;

    if(type == InputTypeShort) {
        switch(result) {
        case GuiButtonTypeCenter:
            if(app->hid->api->is_connected) {
                ducky_string(app);
            }
            break;
        case GuiButtonTypeLeft:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, FireStringScene_GenerateStepTwo);
            break;
        case GuiButtonTypeRight:
            if(right_btn_clk_cnt < SPAM_UNLOCK) {
                right_btn_clk_cnt++;
                if(right_btn_clk_cnt == SPAM_UNLOCK) {
                    usb_scene_builder(app);
                    furi_hal_vibro_on(true);
                    furi_delay_ms(30);
                    furi_hal_vibro_on(false);
                }
            }
            break;
        }
    }
    if(type == InputTypeRepeat && right_btn_clk_cnt == SPAM_UNLOCK) {
        if(app->hid->api->is_connected) {
            uint16_t keycode = HID_KEYBOARD_NONE;
            if(app->settings->str_type == StrType_Passphrase) {
                const char* param = get_rnd_word(app, false);
                uint32_t i = 0;
                while(param[i] != '\0') {
                    keycode = ASCII_TO_KEY(app, param[i]);
                    if(keycode != HID_KEYBOARD_NONE) {
                        app->hid->api->kb_press(app->hid->hid_inst, keycode);
                        app->hid->api->kb_release(app->hid->hid_inst, keycode);
                    }
                    i++;
                }
            } else {
                char rnd_char = get_rnd_char(app, false);
                keycode = ASCII_TO_KEY(app, rnd_char);
                if(keycode != HID_KEYBOARD_NONE) {
                    app->hid->api->kb_press(app->hid->hid_inst, keycode);
                    app->hid->api->kb_release(app->hid->hid_inst, keycode);
                }
            }
        }
    }
}

static void usb_scene_builder(FireString* app) {
    FURI_LOG_T(TAG, "usb_scene_builder");

    widget_reset(app->widget);

    widget_add_icon_element(app->widget, 80, 20, &I_UsbTree_48x22);

    if(app->hid->api->is_connected(app->hid->hid_inst)) {
        widget_add_string_element(
            app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Ready to send Fire String");
        widget_add_button_element(app->widget, GuiButtonTypeCenter, "Send", usb_btn_callback, app);
        if(right_btn_clk_cnt < SPAM_UNLOCK) {
            widget_add_icon_element(app->widget, 0, 20, &I_Connected_62x31);
            widget_add_icon_element(app->widget, 62, 22, &I_Smile_18x18);
            widget_add_button_element(app->widget, GuiButtonTypeRight, "", usb_btn_callback, app);
        } else {
            widget_add_icon_element(app->widget, 0, 20, &I_WarningDolphin_45x42);
            widget_add_icon_element(app->widget, 62, 20, &I_EviSmile2_18x21);
            widget_add_button_element(
                app->widget, GuiButtonTypeRight, "Spam", usb_btn_callback, app);
        }
    } else {
        widget_add_string_element(
            app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Waiting for connection...");
        widget_add_icon_element(app->widget, 62, 22, &I_Error_18x18);
        widget_add_icon_element(app->widget, 0, 20, &I_Connect_me_62x31);
    }
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", usb_btn_callback, app);
}

void fire_string_scene_on_enter_usb(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_usb");
    furi_check(context);

    FireString* app = context;

    right_btn_clk_cnt = 0;
    usb_state = app->hid->api->is_connected(app->hid->hid_inst);
    usb_scene_builder(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_Widget);
}

bool fire_string_scene_on_event_usb(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "fire_string_scene_on_event_usb");
    furi_check(context);

    FireString* app = context;
    bool is_connected = app->hid->api->is_connected(app->hid->hid_inst);
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        default:
            break;
        }
        break;
    case SceneManagerEventTypeTick:
        if(usb_state != is_connected) {
            usb_state = is_connected;
            usb_scene_builder(app);
        }
        break;
    case SceneManagerEventTypeBack:
        consumed = true;
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, FireStringScene_GenerateStepTwo);
        break;
    default:
        break;
    }
    return consumed;
}

void fire_string_scene_on_exit_usb(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_exit_usb");
    furi_check(context);

    FireString* app = context;

    app->hid->api->deinit(app->hid->hid_inst);
    if(app->hid->usb_if_prev) {
        furi_check(furi_hal_usb_set_config(app->hid->usb_if_prev, NULL));
    }
    widget_reset(app->widget);
}
