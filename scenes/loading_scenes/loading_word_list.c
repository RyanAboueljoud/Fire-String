#include "../../fire_string.h"

static int32_t word_list_worker(void* context) {
    FURI_LOG_T(TAG, "word_list_worker");
    furi_check(context);

    FireString* app = context;

    FURI_LOG_I(TAG, "word_list_worker %p starting", furi_thread_get_id(app->thread));

    File* word_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    FuriString* word_buffer = furi_string_alloc();
    uint16_t word_count = 0;
    size_t buf_size = 0;

    // Open wordlist file
    if(storage_file_open(word_file, APP_ASSETS_PATH(DICT_FILE), FSAM_READ, FSOM_OPEN_EXISTING)) {
        buf_size = storage_file_size(word_file);

        if(!(buf_size > 0)) {
            furi_assert(buf_size);
            FURI_LOG_E(TAG, "File read error");
        }
        if(buf_size > memmgr_get_free_heap()) { // Check if memory is available to read file
            FURI_LOG_E(TAG, "File too large");
        } else { // read file and build string
            uint8_t* file_buf = malloc(buf_size);
            size_t read_count = storage_file_read(word_file, file_buf, buf_size);
            uint16_t char_index = 0;
            uint16_t word_index = 0;

            while(char_index < read_count) { // Get word count for word_list malloc
                if(file_buf[char_index] == '\n') {
                    word_count++;
                }
                char_index++;
            }
            char_index = 0;

            // malloc word_list using word_count
            app->dict->word_list = (FuriString**)malloc(sizeof(FuriString*) * word_count);
            if(app->dict->word_list == NULL) {
                FURI_LOG_E(TAG, "Failed to allocate word list");
                furi_assert(app->dict->word_list);
            }

            // Build word list dictionary
            // Arbitrarily limits size of word list using DICT_MAX_SIZE due to memory limitations
            // TODO: randomly pull words from file_buf so all available words have an equal chance of being included in dictionary
            while(word_index < word_count && word_index < DICT_MAX_SIZE) {
                if(file_buf[char_index] == '\n') {
                    app->dict->word_list[word_index] = furi_string_alloc_set(word_buffer);
                    furi_string_reset(word_buffer);
                    word_index++;
                } else {
                    furi_string_push_back(word_buffer, file_buf[char_index]);
                }
                char_index++;
            }

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
    if(word_buffer != NULL) {
        furi_string_free(word_buffer);
        word_buffer = NULL;
    }

    FURI_LOG_I(TAG, "word_list_worker %p stopping", furi_thread_get_id(app->thread));

    return 0;
}

void fire_string_scene_on_enter_loading_word_list(void* context) {
    FURI_LOG_T(TAG, "fire_string_scene_on_enter_loading_word_list");
    furi_check(context);

    FireString* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FireStringView_Loading);

    app->thread = furi_thread_alloc_ex("WordListWorker", 2048, word_list_worker, app);

    // Skip word_list generation if not NULL
    if(app->dict->word_list == NULL) {
        furi_thread_start(app->thread);
    }
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
