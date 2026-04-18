#include "../../fire_string.h"

typedef struct __attribute__((packed)) {
    uint16_t offset;
    uint8_t length;
} WordSpan;

static int32_t word_list_worker(void* context) {
    FURI_LOG_T(TAG, "word_list_worker");
    furi_check(context);

    FireString* app = context;

    FURI_LOG_I(TAG, "word_list_worker %p starting", furi_thread_get_id(app->thread));

    if(app->dict->word_list != NULL) {
        for(uint16_t i = 0; i < app->dict->len; i++) {
            furi_string_free(app->dict->word_list[i]);
            app->dict->word_list[i] = NULL;
        }
        free(app->dict->word_list);
        app->dict->word_list = NULL;
    }

    File* word_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    uint16_t word_count = 0;
    size_t buf_size = 0;

    // Open wordlist file
    if(storage_file_open(word_file, APP_ASSETS_PATH(DICT_FILE), FSAM_READ, FSOM_OPEN_EXISTING)) {
        buf_size = storage_file_size(word_file);

        if(!(buf_size > 0)) { // error check
            furi_assert(buf_size);
            FURI_LOG_E(TAG, "File read error");
        }
        if(buf_size > memmgr_get_free_heap()) { // Check if memory is available to read file
            FURI_LOG_E(TAG, "File too large");
        } else { // read file and build string
            uint8_t* file_buf = malloc(buf_size);
            uint16_t read_count = storage_file_read(word_file, file_buf, buf_size);
            uint16_t char_index = 0;
            uint16_t span_index = 0;
            uint16_t word_start = 0;

            while(char_index < read_count) { // Get word count for word_list malloc
                if(file_buf[char_index] == '\n') {
                    word_count++;
                }
                char_index++;
            }
            char_index = 0;

            // Build index dictionary
            WordSpan* spans = malloc(sizeof(WordSpan) * word_count);
            if(spans == NULL) { // error check
                FURI_LOG_E(TAG, "could not allocate spans");
                furi_assert(spans);
            }
            for(uint16_t i = 0; i < read_count; i++) {
                if(file_buf[i] == '\n') {
                    spans[span_index].offset = word_start;
                    spans[span_index].length =
                        i - word_start; // distance from word_start to newline
                    span_index++;
                    word_start = i + 1; // next word starts after this newline
                }
            }

            // Partial Fisher-Yates shuffle
            for(uint16_t i = 0; i < DICT_MAX_SIZE; i++) {
                uint16_t j = i + (furi_hal_random_get() % (word_count - i));

                WordSpan tmp = spans[i];
                spans[i] = spans[j];
                spans[j] = tmp;
            }

            // allocate and build word_list
            app->dict->word_list = (FuriString**)calloc(DICT_MAX_SIZE, sizeof(FuriString*));
            if(app->dict->word_list == NULL) { // error check
                FURI_LOG_E(TAG, "Failed to allocate word list");
                furi_assert(app->dict->word_list);
            }
            for(uint16_t i = 0; i < DICT_MAX_SIZE; i++) {
                FuriString* s = furi_string_alloc();
                furi_string_set_strn(
                    s, (const char*)(file_buf + spans[i].offset), spans[i].length);
                app->dict->word_list[i] = s;
            }

            app->dict->len = DICT_MAX_SIZE;

            free(spans);
            spans = NULL;
            free(file_buf);
            file_buf = NULL;
        }
    } else {
        FURI_LOG_E(TAG, "File open error");
    }

    // close file and free memory
    storage_file_close(word_file);
    storage_file_free(word_file);
    word_file = NULL;
    furi_record_close(RECORD_STORAGE);

    FURI_LOG_I(TAG, "word_list_worker %p stopping", furi_thread_get_id(app->thread));

    return 0;
}

void fire_string_scene_on_enter_loading_word_list(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_loading_word_list");
    furi_check(context);

    FireString* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_Loading);

    app->thread = furi_thread_alloc_ex("WordListWorker", 2048, word_list_worker, app);

    furi_thread_start(app->thread);
}

bool fire_string_scene_on_event_loading_word_list(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "fire_string_scene_on_event_loading_word_list");

    FireString* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeTick:
        if(furi_thread_get_state(app->thread) == FuriThreadStateStopped) {
            if(scene_manager_search_and_switch_to_previous_scene(
                   app->scene_manager, FireStringScene_Generate)) {
            } else {
                scene_manager_next_scene(app->scene_manager, FireStringScene_Generate);
            }
        }
        break;
    default:
        break;
    }
    return consumed;
}

void fire_string_scene_on_exit_loading_word_list(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_exit_loading_word_list");
    furi_check(context);

    FireString* app = context;

    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
    app->thread = NULL;
}
