#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

static list_t *ASSET_LIST = NULL;
const size_t INIT_CAPACITY = 5;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  // This is a fancy way of malloc'ing space for an `image_asset_t` if `ty` is
  // ASSET_IMAGE, and `text_asset_t` otherwise.
  if (ASSET_LIST == NULL) {
    ASSET_LIST = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
  }
  asset_t *new =
      malloc(ty == ASSET_IMAGE ? sizeof(image_asset_t) : sizeof(text_asset_t));
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

void asset_make_image_with_body(const char *filepath, body_t *body) {
  SDL_Texture *tex = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  SDL_Rect dummy_bb = {0, 0, 0, 0};
  image_asset_t *img = (image_asset_t *)asset_init(ASSET_IMAGE, dummy_bb);
  img->texture = tex;
  img->body = body;
  list_add(ASSET_LIST, (asset_t *)img);
}

void asset_make_text_with_body(const char *fontpath, body_t *body, char *text, color_t color) {
  TTF_Font *font = asset_cache_obj_get_or_create(ASSET_TEXT, fontpath);
  assert(font);
  SDL_Rect dummy_bb = {0, 0, 0, 0};
  text_asset_t *txt = (text_asset_t *)asset_init(ASSET_TEXT, dummy_bb);
  txt->font = font;
  txt->color = color;
  txt->text = text;
  txt->body = body;

  list_add(ASSET_LIST, (asset_t *)txt);
}

void asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  SDL_Texture *texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  if (texture != NULL) {
    image_asset_t *image =
        (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);
    image->texture = texture;
    image->body = NULL;
    list_add(ASSET_LIST, (asset_t *)image);
  }
}

void asset_make_text(const char *filepath, SDL_Rect bounding_box,
                     char *text, color_t color) {
  TTF_Font *font = asset_cache_obj_get_or_create(ASSET_TEXT, filepath);
  text_asset_t *txt = (text_asset_t *)asset_init(ASSET_TEXT, bounding_box);
  txt->font = font;
  txt->color = color;
  txt->text = text;
  list_add(ASSET_LIST, (asset_t *)txt);
}

void asset_reset_asset_list() {
  if (ASSET_LIST != NULL) {
    list_free(ASSET_LIST);
  }
  ASSET_LIST = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
}

list_t *asset_get_asset_list() { return ASSET_LIST; }




void asset_remove_body(body_t *body) {
  if (ASSET_LIST == NULL || body == NULL) {
    return;
  }

  for (size_t i = list_size(ASSET_LIST); i-- > 0; ) {
    asset_t *asset = list_get(ASSET_LIST, i);
    
    bool found_match = false;
    if (asset->type == ASSET_IMAGE) {
      image_asset_t *img_asset = (image_asset_t *)asset;
      if (img_asset->body == body) {
        found_match = true;
      }
    } else if (asset->type == ASSET_TEXT) {
      text_asset_t *txt_asset = (text_asset_t *)asset;
      if (txt_asset->body == body) {
        found_match = true;
      }
    }

    if (found_match) {
      asset_t *removed = list_remove(ASSET_LIST, i);
      asset_destroy(removed);
    }
  }
}

void sdl_render_image_ex(SDL_Texture *texture, SDL_Rect *dest, double angle_degrees) {
  SDL_Renderer *renderer = sdl_get_renderer();
  SDL_Point center = { dest->w / 2, dest->h / 2 };
  SDL_RenderCopyEx(renderer,
                   texture,
                   NULL,
                   dest,
                   angle_degrees,
                   &center,
                   SDL_FLIP_NONE);
}

void asset_render(asset_t *asset) {
  if (asset->type == ASSET_IMAGE) {
    image_asset_t *img = (image_asset_t *)asset;

    if (img->body != NULL) {
      asset->bounding_box = sdl_get_body_bounding_box(img->body);

      double angle_rad = body_get_rotation(img->body);
      double angle_deg = angle_rad * 180.0 / M_PI;

      sdl_render_image_ex(img->texture, &asset->bounding_box, angle_deg);
    }else{sdl_render_image(img->texture, &asset->bounding_box);}

    
  } else if (asset->type == ASSET_TEXT) {
    text_asset_t *txt = (text_asset_t *)asset;
    if (txt->body != NULL) {
      asset->bounding_box = sdl_get_body_bounding_box(txt->body);
    }
    SDL_Color sdl_color = {.r = (Uint8)(txt->color.red * 255),
                           .g = (Uint8)(txt->color.green * 255),
                           .b = (Uint8)(txt->color.blue * 255),
                           .a = 255};
    sdl_render_text(txt->font, txt->text, &asset->bounding_box, sdl_color);
  }
}


void asset_destroy(asset_t *asset) { free(asset); }
