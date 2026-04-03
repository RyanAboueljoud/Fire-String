#include "../../fire_string.h"

static int32_t usb_worker(void* context) {
    FURI_LOG_I(TAG, "usb_worker");
    furi_check(context);

    FireString* app = context;

    FURI_LOG_I(TAG, "usb_worker %p starting", furi_thread_get_id(app->thread));

    if(furi_hal_usb_is_locked()) {
        app->hid->usb_if_prev = NULL;
    } else {
        app->hid->usb_if_prev = furi_hal_usb_get_config();
        furi_check(furi_hal_usb_set_config(NULL, NULL));
    }

    app->hid->interface = BadUsbHidInterfaceUsb;
    app->hid->api = bad_usb_hid_get_interface(app->hid->interface);
    app->hid->hid_inst = app->hid->api->init(NULL);

    furi_delay_ms(100); // Give USB HID time to settle

    FURI_LOG_I(TAG, "usb_worker %p ended", furi_thread_get_id(app->thread));

    return 0;
}

void fire_string_scene_on_enter_loading_usb(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_loading_usb");
    furi_check(context);

    FireString* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_Loading);

    app->thread = furi_thread_alloc_ex("USBWorker", 2048, usb_worker, app);
    furi_thread_start(app->thread);
}

bool fire_string_scene_on_event_loading_usb(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "fire_string_scene_on_event_loading_usb");
    furi_check(context);

    FireString* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        default:
            break;
        }
        break;

    case SceneManagerEventTypeTick:
        if(furi_thread_get_state(app->thread) == FuriThreadStateStopped) {
            scene_manager_next_scene(app->scene_manager, FireStringScene_USB);
        } else {
            FURI_LOG_I(
                TAG,
                "loading_usb thread id: %p state: %s",
                furi_thread_get_id(app->thread),
                furi_thread_get_state(app->thread) == FuriThreadStateRunning ? "Running" :
                                                                               "Starting");
        }
        break;
    case SceneManagerEventTypeBack:
        app->hid->api->deinit(app->hid->hid_inst);
        if(app->hid->usb_if_prev) {
            furi_check(furi_hal_usb_set_config(app->hid->usb_if_prev, NULL));
        }
        consumed = true;
        scene_manager_previous_scene(app->scene_manager);
        break;
    default:
        break;
    }
    return consumed;
}

void fire_string_scene_on_exit_loading_usb(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_exit_loading_usb");
    furi_check(context);

    FireString* app = context;

    furi_thread_free(app->thread);
    app->thread = NULL;
}
