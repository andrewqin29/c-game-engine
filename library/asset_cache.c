#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

// frees the object at the entry
static void asset_cache_free_entry(entry_t *entry) {
  if (entry->obj != NULL) {
    if (entry->type == ASSET_IMAGE) {
      SDL_DestroyTexture((SDL_Texture *)entry->obj);
    } else if (entry->type == ASSET_TEXT) {
      TTF_CloseFont((TTF_Font *)entry->obj);
    }
  }
  free(entry);
}
/**
 * Initializes the empty, list-based global asset cache. The caller must then
 * destroy the cache with `asset_cache_destroy` when done.
 */
void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}
/**
 * Frees the global asset cache and its owned contents.
 */
void asset_cache_destroy() { list_free(ASSET_CACHE); }

// helper function to get the corresponding entry for a file
static entry_t *asset_entry_correspondence(const char *filepath) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (strcmp(entry->filepath, filepath) == 0) {
      return entry;
    }
  }
  return NULL;
}
// creates an asset if there is not an existing one at the entry or gets the
// existing one
void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  entry_t *entry = asset_entry_correspondence(filepath);
  if (entry != NULL) {
    assert(entry->type == ty);
    return entry->obj;
  }
  void *obj = NULL;

  if (ty == ASSET_IMAGE) {
    obj = sdl_get_image_texture(filepath);
  } else if (ty == ASSET_TEXT) {
    obj = TTF_OpenFont(filepath, FONT_SIZE);
  } else {
    assert(false);
  }

  assert(obj != NULL);

  entry_t *new_entry = malloc(sizeof(entry_t));
  assert(new_entry != NULL);
  new_entry->type = ty;
  new_entry->filepath = strdup(filepath);
  new_entry->obj = obj;

  list_add(ASSET_CACHE, new_entry);
  return obj;
}
