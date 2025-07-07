#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const size_t WINDOW_WIDTH = 1000;
const int8_t FONT_HEIGHT_SCALE = 2;
const size_t WINDOW_HEIGHT = 500;
const SDL_Color SDL_BLACK = {0, 0, 0};
const double MS_PER_S = 1000.0;

typedef enum {
  CHARACTER,
  OBSTACLE,
  SHURIKEN,
  POWERUP,
  COIN,
  HEAT_SEEK_ROCKET,
  BACKGROUND,
  UI
} body_info_type_t;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;

/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}
SDL_Renderer *sdl_get_renderer(void) {
    return renderer;
}

bool sdl_is_done(state_t *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state);
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_body(body_t *body) {
  void *raw_info = body_get_info(body);
  if (raw_info != NULL) {
    body_info_type_t *maybe_type = (body_info_type_t *)raw_info;
    if (*maybe_type == SHURIKEN) {
      return;
    }
  }

  list_t *points = body_get_shape(body);
  size_t n = list_size(points);
  assert(n >= 3);

  color_t color = body_get_color(body);
  double r = color.red, g = color.green, b = color.blue;
  assert(0 <= r && r <= 1);
  assert(0 <= g && g <= 1);
  assert(0 <= b && b <= 1);

  vector_t window_center = get_window_center();

  int16_t *x_points = malloc(sizeof(*x_points) * n);
  int16_t *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL && y_points != NULL);

  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = (int16_t)pixel.x;
    y_points[i] = (int16_t)pixel.y;
  }

  filledPolygonRGBA(renderer,
                    x_points,
                    y_points,
                    (int)n,
                    (Uint8)(r * 255),
                    (Uint8)(g * 255),
                    (Uint8)(b * 255),
                    255);

  free(x_points);
  free(y_points);
  list_free(points);
}

SDL_Texture *sdl_get_image_texture(const char *image_path) {
  SDL_Texture *img = IMG_LoadTexture(renderer, image_path);
  return img;
}

SDL_Rect *sdl_get_rect(double x, double y, double w, double h) {
  SDL_Rect *rect = malloc(sizeof(SDL_Rect));
  rect->x = x;
  rect->y = y;
  rect->w = w;
  rect->h = h;
  return rect;
}

void sdl_render_image(SDL_Texture *image_texture, SDL_Rect *rect) {
  SDL_SetTextureBlendMode(image_texture, SDL_BLENDMODE_BLEND);
  SDL_RenderCopy(renderer, image_texture, NULL, rect);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    sdl_draw_body(body);
  }
  // sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}
void sdl_render_text(TTF_Font *font, const char *text, SDL_Rect *rect,
                     SDL_Color color) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_RenderCopy(renderer, texture, NULL, rect);

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

SDL_Rect sdl_get_body_bounding_box(body_t *body) {
  list_t *verts = body_get_shape(body);
  vector_t window_center = get_window_center();

  double min_x = __DBL_MAX__, min_y = __DBL_MAX__;
  double max_x = -__DBL_MAX__, max_y = -__DBL_MAX__;

  for (size_t i = 0; i < list_size(verts); i++) {
    vector_t scene_v = *(vector_t *)list_get(verts, i);
    vector_t pixel_v = get_window_position(scene_v, window_center);

    if (pixel_v.x < min_x)
      min_x = pixel_v.x;
    if (pixel_v.x > max_x)
      max_x = pixel_v.x;
    if (pixel_v.y < min_y)
      min_y = pixel_v.y;
    if (pixel_v.y > max_y)
      max_y = pixel_v.y;
  }

  SDL_Rect box;
  box.x = (int)round(min_x);
  box.y = (int)round(min_y);
  box.w = (int)round(max_x - min_x);
  box.h = (int)round(max_y - min_y);

  return box;
}