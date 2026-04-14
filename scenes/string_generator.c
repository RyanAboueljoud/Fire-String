#include "../fire_string.h"
#include "furi_hal_random.h"

#define ALPH_ROWS 2
#define NUMS_ROWS 1
#define SYMB_ROWS 4
#define BIN_ROWS  1
#define COLS      2

void build_string_generator_widget(FireString* app);

#define DEFAULT_DELAY          250
#define TEXT_SCROLL_CHAR_LIMIT 256
uint8_t delay_ms = DEFAULT_DELAY;
uint16_t fire_string_len = -1;

void get_char_list(FireString* app) {
    FURI_LOG_T(TAG, "get_char_list");

    if(app->dict->char_list != NULL) {
        furi_string_free(app->dict->char_list);
        app->dict->char_list = NULL;
    }
    app->dict->char_list = furi_string_alloc();

    // Alphabet: Uppercase (65-90) and Lowercase (97-122)
    const int alphabet_ranges[ALPH_ROWS][COLS] = {
        {65, 90}, // Uppercase A-Z
        {97, 122} // Lowercase a-z
    };

    // Numbers: Digits (48-57)
    int numbers_ranges[NUMS_ROWS][COLS] = {
        {48, 57} // Digits 0-9
    };

    // Symbols: Multiple ranges
    int symbols_ranges[SYMB_ROWS][COLS] = {
        {33, 47}, // Exclamation mark to Slash (! to /)
        {58, 64}, // Colon to At Symbol (: to @)
        {91, 96}, // Opening Bracket to Backtick ([ to `)
        {123, 126} // Opening Curly Brace to Tilde ({ to ~)
    };

    // Binary: (48-49)
    int binary_range[BIN_ROWS][COLS] = {
        {48, 49} // Zero to One (0 to 1)
    };

    switch(app->settings->str_type) {
    case(StrType_AlphaNumSymb):
        // cat alphabetic chars
        for(uint8_t i = 0; i < ALPH_ROWS; i++) {
            for(uint8_t j = alphabet_ranges[i][0]; j <= alphabet_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        // cat numerical chars
        for(uint8_t i = 0; i < NUMS_ROWS; i++) {
            for(uint8_t j = numbers_ranges[i][0]; j <= numbers_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        // cat symbol chars
        for(uint8_t i = 0; i < SYMB_ROWS; i++) {
            for(uint8_t j = symbols_ranges[i][0]; j <= symbols_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        break;
    case(StrType_AlphaNum):
        // cat alphabetic chars
        for(uint8_t i = 0; i < ALPH_ROWS; i++) {
            for(uint8_t j = alphabet_ranges[i][0]; j <= alphabet_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        // cat numerical chars
        for(uint8_t i = 0; i < NUMS_ROWS; i++) {
            for(uint8_t j = numbers_ranges[i][0]; j <= numbers_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        break;
    case(StrType_Alpha):
        // cat alphabetic chars
        for(uint8_t i = 0; i < ALPH_ROWS; i++) {
            for(uint8_t j = alphabet_ranges[i][0]; j <= alphabet_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        break;
    case(StrType_Symb):
        // cat symbol chars
        for(uint8_t i = 0; i < SYMB_ROWS; i++) {
            for(uint8_t j = symbols_ranges[i][0]; j <= symbols_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        break;
    case(StrType_Num):
        // cat numerical chars
        for(uint8_t i = 0; i < NUMS_ROWS; i++) {
            for(uint8_t j = numbers_ranges[i][0]; j <= numbers_ranges[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        break;
    case(StrType_Bin):
        // cat binary chars
        for(uint8_t i = 0; i < BIN_ROWS; i++) {
            for(uint8_t j = binary_range[i][0]; j <= binary_range[i][COLS - 1]; j++) {
                furi_string_push_back(app->dict->char_list, j);
            }
        }
        break;
    }
}

static inline void string_builder(FireString* app, FuriString* str) {
    if(fire_string_len < 1) {
        furi_string_set(app->fire_string, str);
    } else {
        furi_string_push_back(app->fire_string, '-');

        uint8_t str_len = furi_string_size(str);
        for(uint8_t i = 0; i < str_len; i++) {
            furi_string_push_back(app->fire_string, furi_string_get_char(str, i));
        }
    }
    fire_string_len++;
}

void get_dict_len(FireString* app) {
    FURI_LOG_T(TAG, "get_dict_len");

    app->dict->len = 0;
    if(app->settings->str_type == StrType_Passphrase) {
        size_t i = 0;
        while(app->dict->word_list[i] != NULL && !furi_string_empty(app->dict->word_list[i])) {
            i++;
        }
        app->dict->len = i;
    } else {
        app->dict->len = furi_string_size(app->dict->char_list);
    }
}

// Get string length or word count if phrase is enabled
static inline uint16_t get_str_len(FireString* app) {
    FURI_LOG_T(TAG, "get_str_len");

    if(app->settings->str_type == StrType_Passphrase) {
        uint16_t string_size = furi_string_size(app->fire_string);

        if(string_size == 0) {
            return 0; // Return zero if string is empty
        }
        uint16_t word_count = 1;
        for(uint16_t i = 0; i < string_size; i++) {
            if(furi_string_get_char(app->fire_string, i) == '-') {
                word_count++;
            }
        }
        return word_count;
    } else {
        return furi_string_size(app->fire_string);
    }
}

void vibro(FireString* app) {
    if(fire_string_len >= app->settings->str_len && app->settings->file_loaded == false) {
        furi_hal_vibro_on(true);
        furi_delay_ms(30);
        furi_hal_vibro_on(false);
    }
}

// get word using internal rng
const char* get_rnd_word(FireString* app, bool save) {
    // FURI_LOG_T(TAG, "get_rnd_word");

    uint16_t rnd_buffer = 0;
    rnd_buffer = furi_hal_random_get() & 0xFFF; // Bit mask for 12 bit

    while(rnd_buffer >= app->dict->len) {
        rnd_buffer = furi_hal_random_get() & 0xFFF;
    }
    if(save) {
        string_builder(app, app->dict->word_list[rnd_buffer]);
    } else {
        return furi_string_get_cstr(app->dict->word_list[rnd_buffer]);
    }

    return 0;
}

// get character using internal rng
char get_rnd_char(FireString* app, bool save) {
    // FURI_LOG_T(TAG, "get_rnd_char");

    uint8_t rnd_byte = 0b00000000;
    furi_hal_random_fill_buf(&rnd_byte, sizeof(rnd_byte));
    rnd_byte &= 0b01111111; // Bit mask for char
    while(rnd_byte > app->dict->len - 1) {
        rnd_byte = 0;
        furi_hal_random_fill_buf(&rnd_byte, sizeof(rnd_byte));
        rnd_byte &= 0b01111111;
    }
    if(save) {
        furi_string_push_back(
            app->fire_string, furi_string_get_char(app->dict->char_list, rnd_byte));
        fire_string_len++;
    } else {
        return furi_string_get_char(app->dict->char_list, rnd_byte);
    }
    return 0;
}

// use internal rng to generate fire_string of app->settings->str_len
void get_random_str(FireString* app) {
    FURI_LOG_T(TAG, "get_random_str");

    uint16_t str_len = app->settings->str_len - fire_string_len;

    for(uint16_t i = 0; i < str_len; i++) {
        if(app->settings->str_type == StrType_Passphrase) {
            get_rnd_word(app, true);
        } else {
            get_rnd_char(app, true);
        }
    }
}

void string_generator_btn_callback(GuiButtonType result, InputType type, void* context) {
    FURI_LOG_T(TAG, "string_generator_btn_callback");
    furi_assert(context);

    FireString* app = context;

    switch(type) {
    case InputTypeShort:
        switch(result) {
        case GuiButtonTypeRight:
            scene_manager_next_scene(app->scene_manager, FireStringScene_GenerateStepTwo);
            break;
        case GuiButtonTypeLeft:
            scene_manager_next_scene(app->scene_manager, FireStringScene_Settings);
            break;
        case GuiButtonTypeCenter:
            furi_string_reset(app->fire_string);
            furi_string_reserve(app->fire_string, STR_RESERVE_LEN);
            fire_string_len = 0;
            delay_ms = DEFAULT_DELAY;
            app->settings->file_loaded = false;
            if(app->settings->use_ir == true) {
                build_string_generator_widget(app);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void build_string_generator_widget(FireString* app) {
    FURI_LOG_T(TAG, "build_string_generator_widget");

    FuriString* progress = furi_string_alloc();
    furi_string_printf(progress, "%d/%ld", fire_string_len, app->settings->str_len);

    widget_reset(app->widget);

    // Limit size of displayed string to avoid memory issues associated with widget_add_text_scroll_element()
    if(furi_string_size(app->fire_string) > TEXT_SCROLL_CHAR_LIMIT) {
        FuriString* short_fire_string = furi_string_alloc();

        for(uint16_t i = 0; i < TEXT_SCROLL_CHAR_LIMIT - 3; i++) {
            furi_string_push_back(short_fire_string, furi_string_get_char(app->fire_string, i));
        }
        furi_string_cat_str(short_fire_string, "...");

        widget_add_text_scroll_element(
            app->widget, 0, 1, 128, 45, furi_string_get_cstr(short_fire_string));

        furi_string_free(short_fire_string);
        short_fire_string = NULL;
    } else {
        widget_add_text_scroll_element(
            app->widget, 0, 1, 128, 45, furi_string_get_cstr(app->fire_string));
    }

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Config", string_generator_btn_callback, app);

    if(fire_string_len > 0) {
        widget_add_button_element(
            app->widget, GuiButtonTypeCenter, "Reset", string_generator_btn_callback, app);
    }

    if(fire_string_len >= app->settings->str_len || app->settings->file_loaded == true) {
        widget_add_button_element(
            app->widget, GuiButtonTypeRight, "Next", string_generator_btn_callback, app);
    } else {
        widget_add_string_element(
            app->widget,
            85,
            56,
            AlignLeft,
            AlignTop,
            FontSecondary,
            furi_string_get_cstr(progress));
    }

    furi_string_free(progress);
    progress = NULL;
}

// rng using IR input
static void ir_received_callback(void* context, InfraredWorkerSignal* signal) {
    FURI_LOG_T(TAG, "ir_received_callback");
    furi_assert(context);

    FireString* app = context;

    const uint32_t* timings;
    size_t timings_size;

    if(app->ir_worker) {
        infrared_worker_get_raw_signal(signal, &timings, &timings_size);

        uint32_t i = 0;
        if(app->settings->str_type == StrType_Passphrase) {
            while(fire_string_len < app->settings->str_len && i < timings_size) {
                string_builder(app, app->dict->word_list[timings[i] % app->dict->len]);
                i++;
            }
        } else {
            while(fire_string_len < app->settings->str_len && i < timings_size) {
                furi_string_push_back(
                    app->fire_string,
                    furi_string_get_char(app->dict->char_list, timings[i] % app->dict->len));
                fire_string_len++;
                i++;
            }
        }
    }

    build_string_generator_widget(app);
}

void infrared_rx_start(FireString* app) {
    FURI_LOG_T(TAG, "infrared_rx_start");
    furi_check(app);

    app->ir_worker = infrared_worker_alloc();

    infrared_worker_rx_enable_blink_on_receiving(app->ir_worker, true);
    infrared_worker_rx_set_received_signal_callback(app->ir_worker, ir_received_callback, app);
    infrared_worker_rx_start(app->ir_worker);
}

void infrared_rx_stop(FireString* app) {
    FURI_LOG_T(TAG, "infrared_rx_stop");
    furi_check(app);

    infrared_worker_rx_stop(app->ir_worker);
    infrared_worker_free(app->ir_worker);
    app->ir_worker = NULL;
}

void fire_string_scene_on_enter_string_generator(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_string_generator");
    furi_check(context);

    FireString* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_Widget);

    if(app->settings->str_type != StrType_Passphrase && app->dict->char_list == NULL) {
        get_char_list(app);
    }

    if(app->dict->char_list != NULL || app->dict->word_list != NULL) {
        get_dict_len(app);
    }

    if(app->fire_string == NULL) {
        furi_string_reserve(app->fire_string, STR_RESERVE_LEN);
    }

    fire_string_len = get_str_len(app);
    build_string_generator_widget(app);
}

bool fire_string_scene_on_event_string_generator(void* context, SceneManagerEvent event) {
    // FURI_LOG_T(TAG, "fire_string_scene_on_event_string_generator");
    furi_check(context);

    FireString* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        break;
    case SceneManagerEventTypeBack:
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, FireStringScene_MainMenu);
        consumed = true;
        break;
    case SceneManagerEventTypeTick:
        // Toggle infrared_worker if needed
        if(fire_string_len < app->settings->str_len && !app->ir_worker && app->settings->use_ir &&
           app->settings->file_loaded == false) {
            infrared_rx_start(app);
        }
        if(fire_string_len >= app->settings->str_len && app->ir_worker) {
            infrared_rx_stop(app);
            vibro(app);
        }
        // animate automatic string generation
        if(fire_string_len < app->settings->str_len && !app->settings->use_ir &&
           app->settings->file_loaded == false) {
            if(fire_string_len > 30) { // arbitrarily skip animation at certain length
                get_random_str(app);
            } else {
                if(app->settings->str_type == StrType_Passphrase) {
                    get_rnd_word(app, true);
                } else {
                    get_rnd_char(app, true);
                }
                furi_delay_ms(delay_ms);
                if(delay_ms > 1) {
                    delay_ms /= 1.5;
                } else {
                    delay_ms = 0;
                }
            }
            build_string_generator_widget(app);
            vibro(app);
        }
        break;
    }

    return consumed;
}

void fire_string_scene_on_exit_string_generator(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_exit_string_generator");
    furi_check(context);

    FireString* app = context;

    if(app->ir_worker != NULL) {
        infrared_rx_stop(app);
    }

    widget_reset(app->widget);
}
