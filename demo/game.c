#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "color.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "quiz_bank.h"


#define LIST_INIT_CAPACITY 4  

//----------------------------------------------------------------------------//
//                             GAME CONSTANTS                                 //
//----------------------------------------------------------------------------//

//Window and Constants
const vector_t MIN = {0, 0};      // Minimum world coordinates
const vector_t MAX = {1000, 500}; // Maximum world coordinates    
const double PANEL_WIDTH = MAX.x * 0.85;
const double PANEL_HEIGHT = MAX.y * 0.75;
const double OFFSCREEN_X_REMOVAL_THRESHOLD = -50.0; // Remove objects past this threshold
vector_t DUMMY_VEC = {0, 0};
const double SCREEN_SCALE_1 = 0.3;
const double SCREEN_SCALE_2 = 0.1;
const size_t PANEL_SCALE = 60;

//Floor
const double FLOOR_SPRITE_HEIGHT = 50.0;

//Physics
static const double DEFAULT_THRUST_ACCEL = 1600; // Upward acceleration from thrust
static const double GRAVITY_ACCEL = -1000; // Downward acceleration due to gravity
const double UNIT_WEIGHT = 1.0;           // Default weight for some bodies

//Character
const double CHARACTER_HEIGHT = 70;
const double CHARACTER_WIDTH = 45;
static const double GROUND_Y = FLOOR_SPRITE_HEIGHT + CHARACTER_HEIGHT / 2.0;
const vector_t RESET_POS = {150, GROUND_Y}; // Character's starting and reset position
const color_t CHARACTER_COLOR = (color_t){0.1, 0.9, 0.2};

//Coin
const double COIN_RADIUS = 15;
const size_t COIN_NUM_POINTS = 12;
const double COIN_SPAWN_INTERVAL = 3.0;
const size_t COIN_NUM_MAX = 12;
const size_t COIN_NUM_MIN = 4;
const double COIN_SCALE_1=2.0;
const double COIN_SCALE_2=4.0;
const double COIN_SCALE_3=2.5;

//Obstacles
const size_t OBSTACLE_HEIGHT = 70; // Height of obstacles
const size_t OBSTACLE_WIDTH = 100;  // Width of obstacles
const vector_t BASE_OBJ_VEL = {75, 0}; // Base velocity for objects
const color_t OBS_COLOR = (color_t){0.2, 0.2, 0.3};
static const double SHURIKEN_ROT_SPEED = M_PI;
const double SHURIKEN_SIZE = 60.0;

//Lasers
const size_t VERTICAL_LASER_HEIGHT = 200;
const size_t VERTICAL_LASER_WIDTH = 40;
const size_t HORIZONTAL_LASER_HEIGHT = 40;
const size_t HORIZONTAL_LASER_WIDTH = 200;
const size_t LASER_SPAWN_INTERVAL = 6.0;

//Heat Seeking Rocket
const size_t HEAT_SEEKING_ROCKET_HEIGHT = 40;
const size_t HEAT_SEEKING_ROCKET_WIDTH = 80;
const double ROCKET_HORIZONTAL_SPEED = -1200.0;
const double ROCKET_VERTICAL_ADJUST_RATE = 250.0;
const double ROCKET_MAX_VERTICAL_SPEED = 150.0;
const double ROCKET_MAX_TURN_RATE_RADS_PER_SEC = M_PI / 2.0;
const double TWO_PI=2 * M_PI;

//Powerups
const double POWERUP_WIDTH = 60; // Width of power-up objects
const double POWERUP_HEIGHT = 60; // Height of power-up objects
const size_t NUM_POWER_TYPES = 3;
const double POWERUP_VERTICAL_SPEED = 300.0;
const double MULTIPLIER_VEL=0.5;

//Background Velocity
static vector_t BACKGROUND_VEL = {-600, 0}; // Scrolling velocity of the background
const vector_t CONSTANT_VEL_BACKGROUND= {-600,0};

//Timers and Spawning
static const double OBSTACLE_SPAWN_INTERVAL = 7.0; // Interval between obstacle spawns
static const double ALERT_TRIGGER_OFFSET = OBSTACLE_SPAWN_INTERVAL - 2.0; // Time before obstacle spawn to show alert (10s - 2s = 8s)
static const double POWERUP_SPAWN_INTERVAL = 12.0; // Interval between power-up spawns
static const double POWERUP_DURATION = 10.0;
static const double SHURIKEN_SPAWN_INTERVAL = 14.0;
static const char *FONT_PATH = "assets/fonts/Cascadia.ttf";
static const size_t QUIZ_TIME_INIT = 12;
const size_t TIMER_SIZE =32;

static const double ALERT_SPRITE_WIDTH = 40.0;
static const double ALERT_SPRITE_HEIGHT = 40.0;
static state_t *GLOBAL_STATE = NULL; // For music
const color_t PLACEHOLDER_COLOR = (color_t){.red = 1, .blue = 0, .green = 0}; // Color placeholder for body_init_with_info


//Images and Animation Constants
const char *PANEL_PATH = "/assets/images/panel1.png";
const char *ALERT_PATH = "/assets/images/exclamation.png";
const char *MOVING_OBSTACLE_PATH = "/assets/images/bug_obstacle_1.png";
const char *BACKGROUND_PATH = "/assets/images/background.png";
const char *NORMAL_CHARACTER_PATH = "/assets/images/character_normal.png";
const char *SHURIKEN_PATH = "/assets/images/shuriken.png";
const char *FLAME_CHARACTER_PATH = "/assets/images/character_flying.png";
const char *PROTECTIVE_SHIELD = "/assets/images/protective_shield_flying.png";
const char *GENERIC_POWERUP = "/assets/images/powerup.png";
const char *FLOOR_PATH = "/assets/images/floor.png";

const double COIN_ANIMATION_INTERVAL = 0.1;
const double LASER_ANIMATION_INTERVAL = COIN_ANIMATION_INTERVAL;
const double HEAT_SEEKING_ROCKET_ANIMATION_INTERVAL = COIN_ANIMATION_INTERVAL;
const double CHARACTER_RUNNING_ANIMATION_INTERVAL = COIN_ANIMATION_INTERVAL;
const double CHARACTER_SHIELDED_RUNNING_ANIMATION_INTERVAL = COIN_ANIMATION_INTERVAL;

const char *COIN_PATHS[] = {
  "/assets/images/coins/coin_01.png",
  "/assets/images/coins/coin_02.png",
  "/assets/images/coins/coin_03.png",
  "/assets/images/coins/coin_04.png",
  "/assets/images/coins/coin_05.png",
  "/assets/images/coins/coin_06.png",
  "/assets/images/coins/coin_07.png",
  "/assets/images/coins/coin_08.png"
};
const char *LASER_VERTICAL_PATHS[] = {
  "/assets/images/laser_v_1.png",
  "/assets/images/laser_v_2.png",
  "/assets/images/laser_v_3.png"
};
const char *LASER_HORIZONTAL_PATHS[] = {
  "/assets/images/laser_h_1.png",
  "/assets/images/laser_h_2.png",
  "/assets/images/laser_h_3.png"
};
const char *HEAT_SEEKING_ROCKET_PATHS[] = {
  "/assets/images/hs_rocket_1.png",
  "/assets/images/hs_rocket_2.png",
  "/assets/images/hs_rocket_3.png",
  "/assets/images/hs_rocket_4.png"
};
const char *CHARACTER_RUNNING_PATHS[] = {
  "/assets/images/character_normal.png",
  "/assets/images/character_normal_2.png"
};
const char *CHARACTER_SHIELDED_RUNNING_PATHS[] = {
  "/assets/images/protective_shield_normal.png",
  "/assets/images/protective_shield_normal_2.png"
};
const size_t NUM_COIN_ANIM_FRAMES = 8;
const size_t NUM_LASER_ANIM_FRAMES = 3;
const size_t NUM_HEAT_SEEKING_ROCKET_ANIM_FRAMES = 4;
const size_t NUM_CHARACTER_RUNNING_ANIM_FRAMES = 2;
const size_t NUM_CHARACTER_SHIELDED_RUNNING_ANIM_FRAMES = 2;

//Fonts & Quiz
const color_t TEXT_COLOR = (color_t){1, 1, 1};
const double PANEL_WEIGHT = 1.0;
const size_t NUM_OF_QUIZ_QS = 5;
const color_t UI_PANEL_COLOR = {0.05, 0.05, 0.25};
static const double QUIZ_PANEL_W_PCT     = 0.80;   // 80 % of window width
static const double QUIZ_PANEL_H_PCT     = 0.70;   // 70 % of window height
static const int    QUIZ_FONT_SZ_QUESTION = 28;    // px – tweak to taste
static const int    QUIZ_FONT_SZ_OPTION   = 22;    // px
static const double QUIZ_LINE_SPACING_PCT = 1.30;  // 130 % of font height
double panel_w = MAX.x * QUIZ_PANEL_W_PCT;
double panel_h = MAX.y * QUIZ_PANEL_H_PCT;


//Sounds
const size_t SFX_CHANNEL = 0; //All non dedicated SFX to be played thru this channel to prevent overlay
const size_t COIN_SFX_CHANNEL = 1; //Dedicated channel for coin sfx.
const size_t THRUST_SFX_CHANNEL = 2; //Dedicated channel for thrust sfx. Ignores overlays.
const size_t RUNNING_SFX_CHANNEL = 3; //Dedicated channel for running sfx.
const size_t POWERUP_SFX_CHANNEL = 4;
const size_t LASER_SFX_CHANNEL = 5;
const char *MUSIC_PATH = "/assets/sounds/background_music.mp3";
const char *POWER_UP_SPAWN_SOUND_PATH = "/assets/sounds/powerup_spawn_sound.mp3";
const char *POWER_UP_COLLECT_SOUND_PATH = "/assets/sounds/powerup_collect_sound.mp3";
const char *GAME_OVER_SOUND_PATH = "/assets/sounds/gameover_sound.mp3";
const char *ALERT_SPAWN_SOUND_PATH = "/assets/sounds/warning_sound.mp3"; 
const char *COIN_COLLECT_SOUND_ONE_PATH = "/assets/sounds/coin1_sound.mp3";
const char *COIN_COLLECT_SOUND_TWO_PATH = "/assets/sounds/coin2_sound.mp3";
const char *THRUST_SOUND_PATH = "/assets/sounds/thrust_sound.mp3";
const char *RUNNING_SOUND_PATH = "/assets/sounds/running.mp3";
const char *LASER_SOUND_PATH = "/assets/sounds/laser_sound.mp3";


//UI Elements
const color_t UI_TEXT_COLOR = {1, 1, 1}; 

const double SCORE_COIN_SPRITE_SIZE = 20;
const vector_t SCORE_COIN_SPRITE_POS = (vector_t){.x = MAX.x - SCORE_COIN_SPRITE_SIZE - 70, .y = MAX.y - SCORE_COIN_SPRITE_SIZE - 10};
const vector_t SCORE_TEXT_POS = (vector_t){.x = MAX.x - 40, .y = MAX.y - 20};

const double METERS_PER_SECOND_TRAVEL_SPEED = 12.42; // Usain Bolt sprint speed
const vector_t DISTANCE_TEXT_POS = {40, MAX.y - 20};
const vector_t TIME_TEXT_POS = {40, MAX.y - 45};
const size_t RECTANGLE_POINTS=4;
const size_t SCORE_FONT_SIZE = 32;
const size_t DIST_FONT_SIZE = 32;
const size_t TIME_FONT_SIZE = 32;


//----------------------------------------------------------------------------//
//                               GAME STATE                                   //
//----------------------------------------------------------------------------//

typedef enum {
  GAME_MODE_PLAYING,
  GAME_MODE_QUIZ,
  GAME_MODE_GAMEOVER 
} game_mode_t;

typedef enum {
  CHARACTER,
  OBSTACLE,
  SHURIKEN,
  POWERUP,
  COIN,
  HEAT_SEEK_ROCKET,
  BACKGROUND,
  VERTICAL_LASER,
  HORIZONTAL_LASER,
  UI
} body_info_type_t;

typedef enum {
  POWER_SHIELD,
  POWER_SPEED,
  POWER_DISTANCE
} power_type_t;

struct state {
  body_t *character;
  body_t *background_body1;
  body_t *background_body2;
  body_t *floor_body_1;
  body_t *floor_body_2;
  scene_t *scene;

  double time_since_last;
  double time_since_last_powerup_spawn;
  bool   alert_shown;          
  double next_bug_y;          
  body_t *alert_body;
  double shield_timer;
  double time_since_last_shuriken;
  
  //Coin animation and spawning
  size_t coin_frame_index;
  double last_coin_animation_change;
  double last_coin_spawn_time;

  //Laser animation and spawning
  size_t laser_frame_index;
  double last_laser_spawn_time;
  double last_laser_animation_change;

  //Heat seeking laser animation
  size_t hs_rocket_frame_index;
  double last_hs_rocket_animation_change;

  //Character running animation
  size_t character_running_frame_index;
  double last_character_running_animation_change;
  size_t character_shielded_running_frame_index;
  double last_character_shielded_running_animation_change;

  //Sound and music
  Mix_Music *music;
  Mix_Chunk *sfx_powerup_spawn;
  Mix_Chunk *sfx_powerup_collect;
  Mix_Chunk *sfx_gameover;  
  Mix_Chunk *sfx_alert;
  Mix_Chunk *sfx_coin_collect_one;
  Mix_Chunk *sfx_coin_collect_two;
  Mix_Chunk *sfx_thrust_loop;
  Mix_Chunk *sfx_running_loop;
  Mix_Chunk *sfx_laser_loop;
  bool is_running_sfx;

  //Character state
  vector_t character_velocity;
  bool is_flame; 
  bool thrusting;
  
  //Quiz mode variables
  game_mode_t current_game_mode;
  const quiz_question_t *current_question_data;
  double quiz_time_remaining;
  body_t *quiz_panel_body;
  body_t *quiz_question_text_body;
  list_t *quiz_option_text_bodies; 
  body_t *quiz_timer_text_body;
  bool quiz_requested_hide;
  bool game_over_shown; 

  //Powerups
  bool   shielded;           
  bool   speed_boost_active; 
  double speed_boost_timer;   
  double thrust_accel;         
  bool   obstacle_slow_active; 
  double obstacle_slow_timer;  
  power_type_t pending_power;
  bool has_pending_power;
  double bg_vel_mult; 

  //UI
  size_t score;
  body_t *score_text_ui_body;
  double distance_traveled_meters;
  double total_game_time_seconds;
  body_t *distance_text_ui_body;
  body_t *time_text_ui_body;
  
  size_t score_str_len;
  size_t dist_str_len;
  size_t time_str_len;
};

//----------------------------------------------------------------------------//
//                              UTILITY FUNCTIONS                             //
//----------------------------------------------------------------------------//

/**
 * @brief Generates a random double within a specified range.
 * @return A random double value between low and high.
 */
double rand_double(double low, double high) {
  return (high - low) * rand() / RAND_MAX + low;
}

/**
 * @brief Removes a body from the scene by its pointer.
 * @param scene The scene from which to remove the body.
 * @param body_to_remove A pointer to the body to remove.
 */
void scene_remove_body_by_ptr(scene_t *scene, body_t *body) {
  size_t n = scene_bodies(scene);
  for (size_t i = 0; i < n; i++) {
    if (scene_get_body(scene, i) == body) {
      scene_remove_body(scene, i);
      return;
    }
  }
}

/**
 * @brief Removes body from scene and asset list
 */
void remove_body_and_asset(scene_t *scene, body_t *body) {
  asset_remove_body(body);
  scene_remove_body_by_ptr(scene, body);
}

/**
 * @brief General abstracted function for creating all game objects with rectangular bodies. Called by object specific functions.
 */
body_t *make_rectangle_body(double width, double height, body_info_type_t type) {
  list_t *shape = list_init(RECTANGLE_POINTS, free);

  vector_t *v1 = malloc(sizeof(vector_t));
  *v1 = (vector_t){0,0};
  list_add(shape, v1);
  vector_t *v2 = malloc(sizeof(vector_t));
  *v2 = (vector_t){width,0};
  list_add(shape, v2);
  vector_t *v3 = malloc(sizeof(vector_t));
  *v3 = (vector_t){width, height};
  list_add(shape, v3);
  vector_t *v4 = malloc(sizeof(vector_t));
  *v4 = (vector_t){0,height};
  list_add(shape, v4);

  body_info_type_t *info = malloc(sizeof(body_info_type_t));
  *info = type;

  body_t *body = body_init_with_info(shape, UNIT_WEIGHT, PLACEHOLDER_COLOR, info, free);
  return body;
}

body_t *make_character_body(double width, double height) {
  return make_rectangle_body(width, height, CHARACTER);
}

body_t *make_obstacle_body(size_t w, size_t h, vector_t center) {
  body_t *obstacle = make_rectangle_body(w, h, OBSTACLE);
  body_set_centroid(obstacle, center);
  return obstacle;
}

body_t *make_vertical_laser_body(vector_t center) {
  body_t *laser_v = make_rectangle_body(VERTICAL_LASER_WIDTH, VERTICAL_LASER_HEIGHT, VERTICAL_LASER);
  body_set_centroid(laser_v, center);
  return laser_v;
}

body_t *make_horizontal_laser_body(vector_t center) {
 body_t *laser_h = make_rectangle_body(HORIZONTAL_LASER_WIDTH, HORIZONTAL_LASER_HEIGHT, HORIZONTAL_LASER);
  body_set_centroid(laser_h, center);
  return laser_h;
}

body_t *make_heat_seeking_rocket_body(vector_t center) {
  body_t *hs_rocket = make_rectangle_body(HEAT_SEEKING_ROCKET_WIDTH, HEAT_SEEKING_ROCKET_HEIGHT, HEAT_SEEK_ROCKET);
  body_set_centroid(hs_rocket, center);
  return hs_rocket;
}

body_t *make_powerup_body(vector_t center) {
  body_t *pu = make_rectangle_body(POWERUP_WIDTH, POWERUP_HEIGHT, POWERUP);
  body_set_centroid(pu, center);
  return pu;
}

body_t *make_shuriken_body(vector_t center) {
  list_t *pts = list_init(RECTANGLE_POINTS, free);
  vector_t corners[RECTANGLE_POINTS] = {
    { -SHURIKEN_SIZE/2, -SHURIKEN_SIZE/2 },
    {  SHURIKEN_SIZE/2, -SHURIKEN_SIZE/2 },
    {  SHURIKEN_SIZE/2,  SHURIKEN_SIZE/2 },
    { -SHURIKEN_SIZE/2,  SHURIKEN_SIZE/2 }
  };
  for (int i = 0; i < RECTANGLE_POINTS; i++) {
    vector_t *p = malloc(sizeof(*p));
    *p = corners[i];
    list_add(pts, p);
  }
  body_info_type_t *info = malloc(sizeof(body_info_type_t));
  *info = SHURIKEN;
  body_t *shuriken = body_init_with_info(pts, UNIT_WEIGHT, (color_t){1,1,1}, info, free);
  body_set_centroid(shuriken, center);
  return shuriken;
}

body_t *make_background_body(scene_t *scene, const char *img_path, vector_t center, double width, double height) {
  list_t *body_list = list_init(RECTANGLE_POINTS, free);

  vector_t *v1 = malloc(sizeof(vector_t)); 
  *v1 = (vector_t){-width / 2.0, -height / 2.0}; 
  list_add(body_list, v1);
  vector_t *v2 = malloc(sizeof(vector_t)); 
  *v2 = (vector_t){ width / 2.0, -height / 2.0}; 
  list_add(body_list, v2);
  vector_t *v3 = malloc(sizeof(vector_t)); 
  *v3 = (vector_t){ width / 2.0,  height / 2.0}; 
  list_add(body_list, v3);
  vector_t *v4 = malloc(sizeof(vector_t)); 
  *v4 = (vector_t){-width / 2.0,  height / 2.0}; 
  list_add(body_list, v4);

  body_info_type_t *info = malloc(sizeof(body_info_type_t));
  *info = BACKGROUND;
  body_t *background_body = body_init_with_info(body_list, UNIT_WEIGHT, (color_t){0,0,0}, info, free);
  
  body_set_centroid(background_body, center);
  body_set_velocity(background_body, BACKGROUND_VEL);
  scene_add_body(scene, background_body);
  asset_make_image_with_body(img_path, background_body);

  return background_body;
}

body_t *make_coin_body(vector_t center) {
  list_t *coin_points = list_init(COIN_NUM_POINTS, free);
  
  for (size_t i=0; i<COIN_NUM_POINTS; i++) {
    double angle = TWO_PI * i / COIN_NUM_POINTS;
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t) {COIN_RADIUS * cos(angle), COIN_RADIUS * sin(angle)};
    list_add(coin_points, v);
  }

  body_info_type_t *info = malloc(sizeof(body_info_type_t));
  *info = COIN;
  
  body_t *coin_body = body_init_with_info(coin_points, UNIT_WEIGHT, PLACEHOLDER_COLOR, info, free);
  body_set_centroid(coin_body, center);
  
  return coin_body;
}

//----------------------------------------------------------------------------//
//                             QUIZ AND UI FUNCTIONS                          //
//----------------------------------------------------------------------------//

void queue_body_for_removal(scene_t *scene, body_t *b) {
  asset_remove_body(b); 
  body_remove(b);                       
}

void remove_all_text_assets_for_body(body_t *b) {
  list_t *assets = asset_get_asset_list();
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_t *a = list_get(assets, i);
    if (a->type == ASSET_TEXT) {
      text_asset_t *txt = (text_asset_t *)a;
      if (txt->body == b) {
        free(txt->text);                   
        asset_t *removed = list_remove(assets, i);
        asset_destroy(removed);            
        continue;
      }
    }
  }
}

list_t *rect_for_text(const char *font_path, int pt_size, char *text) {
  TTF_Font *font = (TTF_Font *) asset_cache_obj_get_or_create(ASSET_TEXT, font_path);
  int w, h;
  TTF_SizeText(font, text, &w, &h);        // <- real pixel size

  list_t *pts = list_init(RECTANGLE_POINTS, free);
  vector_t *v;

  v = malloc(sizeof *v); *v = (vector_t){-w/2.0, -h/2.0}; list_add(pts, v);
  v = malloc(sizeof *v); *v = (vector_t){ w/2.0, -h/2.0}; list_add(pts, v);
  v = malloc(sizeof *v); *v = (vector_t){ w/2.0,  h/2.0}; list_add(pts, v);
  v = malloc(sizeof *v); *v = (vector_t){-w/2.0,  h/2.0}; list_add(pts, v);
  return pts;
}

void hide_quiz(state_t *state) {
  if (state->quiz_question_text_body) {
    remove_all_text_assets_for_body(state->quiz_question_text_body);
    queue_body_for_removal(state->scene, state->quiz_question_text_body);
    state->quiz_question_text_body = NULL;
  }

  if (state->quiz_panel_body) {
    queue_body_for_removal(state->scene, state->quiz_panel_body);
    state->quiz_panel_body = NULL;
  }

  if (state->quiz_option_text_bodies) {
    for (size_t i = 0; i < list_size(state->quiz_option_text_bodies); i++) {
      body_t *curr_body = (body_t *)list_get(state->quiz_option_text_bodies, i);
      queue_body_for_removal(state->scene, curr_body);
    }
    while(list_size(state->quiz_option_text_bodies) > 0) {
      list_remove(state->quiz_option_text_bodies, 0);
    }
  }
  if (state->quiz_timer_text_body) {
    queue_body_for_removal(state->scene, state->quiz_timer_text_body);
    state->quiz_timer_text_body = NULL;
  }
}


void display_current_question(state_t *state) {
  if (state->current_question_data == NULL) {
    return;
  }

  const quiz_question_t *quiz = state->current_question_data;
  hide_quiz(state);
  
  //panel 
  double panel_w = MAX.x * QUIZ_PANEL_W_PCT;
  double panel_h = MAX.y * QUIZ_PANEL_H_PCT;
  vector_t panel_ctr = { MAX.x / 2.0, MAX.y / 2.0 };
  

  list_t *panel_pts = list_init(RECTANGLE_POINTS, free);
  vector_t *p[RECTANGLE_POINTS];
  for (int i = 0; i < 4; i++) p[i] = malloc(sizeof *p[i]);
  *p[0] = (vector_t){-panel_w/2, -panel_h/2};
  *p[1] = (vector_t){ -panel_w/2, panel_h/2};
  *p[2] = (vector_t){ panel_w/2,  panel_h/2};
  *p[3] = (vector_t){panel_w/2,  -panel_h/2};
  for (int i = 0; i <RECTANGLE_POINTS; i++) list_add(panel_pts, p[i]);

  state->quiz_panel_body = body_init(panel_pts, PANEL_WEIGHT, UI_PANEL_COLOR);
  asset_make_image_with_body(PANEL_PATH, state->quiz_panel_body);
  body_set_centroid(state->quiz_panel_body, panel_ctr);
  scene_add_body(state->scene, state->quiz_panel_body);
  
  // quiz question
  list_t *qshape= rect_for_text(quiz->font_path, QUIZ_FONT_SZ_QUESTION, quiz->question_text);
  state->quiz_question_text_body = body_init(qshape, PANEL_WEIGHT, UI_TEXT_COLOR);

  
  body_set_centroid(state->quiz_question_text_body,(vector_t){ panel_ctr.x, panel_ctr.y + panel_h*SCREEN_SCALE_1});
  scene_add_body(state->scene, state->quiz_question_text_body);
  char *heap_copy = strdup(quiz->question_text);
  asset_make_text_with_body(quiz->font_path, state->quiz_question_text_body, heap_copy, UI_TEXT_COLOR);

  // answer options
  const double option_step = QUIZ_FONT_SZ_OPTION * QUIZ_LINE_SPACING_PCT;
  double start_y = panel_ctr.y + panel_h*SCREEN_SCALE_2;
  state->quiz_option_text_bodies = list_init(quiz->num_options, NULL);
  
  for (size_t i = 0; i < quiz->num_options; i++) {
    size_t len = strlen(quiz->answer_choices[i]) + 8;
    char *opt_text = malloc(len);
    snprintf(opt_text, len, "%zu. %s", i + 1, quiz->answer_choices[i]);
    list_t *opt_pts = rect_for_text(quiz->font_path, QUIZ_FONT_SZ_OPTION, opt_text);
    body_t *opt_body = body_init(opt_pts, PANEL_WEIGHT, UI_TEXT_COLOR);
    body_set_centroid(opt_body,(vector_t){ panel_ctr.x,start_y - i*option_step });
    scene_add_body(state->scene, opt_body);
    asset_make_text_with_body(quiz->font_path,opt_body,opt_text,UI_TEXT_COLOR);
    list_add(state->quiz_option_text_bodies, opt_body);
  }

  //timer
  char *timer_str = malloc(TIMER_SIZE);
  snprintf(timer_str, TIMER_SIZE, "Time: %.0f", state->quiz_time_remaining);
  list_t *tshape = rect_for_text(FONT_PATH, QUIZ_FONT_SZ_OPTION, timer_str);
  vector_t timer_pos = (vector_t){ panel_ctr.x + panel_w/2 - PANEL_SCALE, panel_ctr.y + panel_h/2 - (PANEL_SCALE/2) };
  state->quiz_timer_text_body = body_init(tshape, PANEL_WEIGHT, UI_TEXT_COLOR);
  body_set_centroid(state->quiz_timer_text_body, timer_pos);
  scene_add_body(state->scene, state->quiz_timer_text_body);
  asset_make_text_with_body(FONT_PATH, state->quiz_timer_text_body, timer_str, UI_TEXT_COLOR);

}

//----------------------------------------------------------------------------//
//                             GAME OVER FUNCTIONALITY                        //
//----------------------------------------------------------------------------//

void display_game_over(state_t *state) {
  scene_free(state->scene);
  state->scene = scene_init();

  // clear every asset so nothing refers to a freed body
  list_t *assets = asset_get_asset_list();
  while (list_size(assets) > 0) {
    asset_t *a = list_remove(assets, 0);
    asset_destroy(a);
  }
  
  // full‐screen 8-bit background
  body_t *bg = make_rectangle_body(MAX.x, MAX.y, UI);
  body_set_centroid(bg, (vector_t){MAX.x/2, MAX.y/2});
  scene_add_body(state->scene, bg);
  asset_make_image_with_body("assets/images/game_over_screen.png", bg);

  // title
  const char *msg = "Game Over";
  list_t *pts = rect_for_text(FONT_PATH, 48, (char *)msg);
  body_t *b1 = body_init_with_info(pts, UNIT_WEIGHT, PLACEHOLDER_COLOR,
                                   malloc(sizeof(body_info_type_t)), free);
  *(body_info_type_t *)body_get_info(b1) = UI;
  body_set_centroid(b1, (vector_t){ MAX.x/2, MAX.y * 0.30 });
  scene_add_body(state->scene, b1);
  asset_make_text_with_body(FONT_PATH, b1, strdup(msg), UI_TEXT_COLOR);

  // stats
  const char *fmt = "Score: %zu   Time: %.1lfs   Dist: %.0fm";
  size_t len = snprintf(NULL, 0, fmt, state->score, state->total_game_time_seconds, state->distance_traveled_meters) + 1;
  char *stats = malloc(len);
  snprintf(stats, len, fmt, state->score, state->total_game_time_seconds, state->distance_traveled_meters);
  list_t *pts2 = rect_for_text(FONT_PATH, 24, stats);
  body_t *b2 = body_init_with_info(pts2, UNIT_WEIGHT, PLACEHOLDER_COLOR, malloc(sizeof(body_info_type_t)), free);
  *(body_info_type_t *)body_get_info(b2) = UI;
  body_set_centroid(b2, (vector_t){ MAX.x/2, MAX.y * 0.45 });
  scene_add_body(state->scene, b2);
  asset_make_text_with_body(FONT_PATH, b2, stats, UI_TEXT_COLOR);

  // restart prompt
  const char *prompt = "Press 1 to Restart";
  list_t *pts3 = rect_for_text(FONT_PATH, 20, (char *)prompt);
  body_t *b3 = body_init_with_info(pts3, UNIT_WEIGHT, PLACEHOLDER_COLOR, malloc(sizeof(body_info_type_t)), free);
  *(body_info_type_t *)body_get_info(b3) = UI;
  body_set_centroid(b3, (vector_t){ MAX.x/2, MAX.y * 0.60 });
  scene_add_body(state->scene, b3);
  asset_make_text_with_body(FONT_PATH, b3, strdup(prompt), UI_TEXT_COLOR);

  //stop all sounds
  for (size_t i=1; i<5; i++) { //1 to 5 are the SFX channels, 0 is reserved for gameover which needs to play
    Mix_HaltChannel(i);
  }
}

/**
 * @brief Makes and initializes UI component bodies in state and adds them to scene
 */
void make_ui_component_bodies(state_t *state) {
  char time_text[TIMER_SIZE];
  snprintf(time_text, sizeof(time_text), "%.1lfs", state->total_game_time_seconds);
  char dist_text[TIMER_SIZE];
  snprintf(dist_text, sizeof(dist_text), "%.0fm", state->distance_traveled_meters);
  char score_text[TIMER_SIZE];
  snprintf(score_text, sizeof(score_text), "%04zu", state->score);

  list_t *time_pts = rect_for_text(FONT_PATH, SCORE_FONT_SIZE, time_text);
  body_info_type_t *info_time = malloc(sizeof(body_info_type_t));
  *info_time = UI;
  state->time_text_ui_body = body_init_with_info(time_pts, UNIT_WEIGHT, PLACEHOLDER_COLOR, info_time, free);
  body_set_centroid(state->time_text_ui_body, TIME_TEXT_POS);

  list_t *dist_pts = rect_for_text(FONT_PATH, DIST_FONT_SIZE, dist_text);
  body_info_type_t *info_dist = malloc(sizeof(body_info_type_t));
  *info_dist = UI;
  state->distance_text_ui_body = body_init_with_info(dist_pts, UNIT_WEIGHT, PLACEHOLDER_COLOR, info_dist, free);
  body_set_centroid(state->distance_text_ui_body, DISTANCE_TEXT_POS);

  list_t *score_pts = rect_for_text(FONT_PATH, SCORE_FONT_SIZE, score_text);
  body_info_type_t *info_score = malloc(sizeof(body_info_type_t));
  *info_score = UI;
  state->score_text_ui_body = body_init_with_info(score_pts, UNIT_WEIGHT, PLACEHOLDER_COLOR, info_score, free);
  body_set_centroid(state->score_text_ui_body, SCORE_TEXT_POS);

  scene_add_body(state->scene, state->score_text_ui_body);
  scene_add_body(state->scene, state->distance_text_ui_body);
  scene_add_body(state->scene, state->time_text_ui_body);
  asset_make_text_with_body(FONT_PATH, state->time_text_ui_body, strdup(time_text), UI_TEXT_COLOR);
  asset_make_text_with_body(FONT_PATH, state->distance_text_ui_body, strdup(dist_text), UI_TEXT_COLOR);
  asset_make_text_with_body(FONT_PATH, state->score_text_ui_body, strdup(score_text), UI_TEXT_COLOR);

  //make coin icon
  SDL_Rect coin_ui_rect = {
      (int)SCORE_COIN_SPRITE_POS.x,
      (int)(MAX.y - SCORE_COIN_SPRITE_POS.y - SCORE_COIN_SPRITE_SIZE),
      (int)SCORE_COIN_SPRITE_SIZE,
      (int)SCORE_COIN_SPRITE_SIZE
    };
  asset_make_image(COIN_PATHS[0], coin_ui_rect);
} 

void reset_game(state_t *state) {
  asset_cache_destroy();
  asset_cache_init();
  /* clear scene */
  for (size_t i = scene_bodies(state->scene); i-- > 0; ) {
    body_t *b = scene_get_body(state->scene, i);
    remove_body_and_asset(state->scene, b);
  }

  /* reset timers, flags & counters */
  state->score = 0;
  state->distance_traveled_meters = 0;
  state->total_game_time_seconds = 0;
  state->time_since_last = 0;
  state->time_since_last_powerup_spawn = 0;
  state->alert_shown = false;
  state->alert_body = NULL;
  state->shield_timer = 0;
  state->time_since_last_shuriken = 0;
  state->coin_frame_index = 0;
  state->laser_frame_index = 0;
  state->hs_rocket_frame_index = 0;
  state->character_running_frame_index = 0;
  state->last_coin_animation_change = 0;
  state->last_laser_animation_change = 0;
  state->last_hs_rocket_animation_change = 0;
  state->last_character_running_animation_change = 0;
  state->last_coin_spawn_time = 0;
  state->last_laser_spawn_time = 0;
  state->character_shielded_running_frame_index = 0;
  state->last_character_shielded_running_animation_change = 0;
  state->is_running_sfx = false;
  state->bg_vel_mult = 1;
  state->dist_str_len = 2;
  state->time_str_len = 3;
  state->score_str_len = 4;
  state->next_bug_y = 0;
  state->quiz_requested_hide = false;
  state->speed_boost_timer = 0.0;
  state->obstacle_slow_timer = 0.0;
  state->pending_power = POWER_SHIELD;

  /* reset quiz state */
  state->current_question_data = NULL;
  state->quiz_time_remaining = 0;
  list_free(state->quiz_option_text_bodies);
  state->quiz_option_text_bodies = list_init(LIST_INIT_CAPACITY, NULL);
  state->quiz_timer_text_body = NULL;
  state->has_pending_power = false;

  /* reset character state */
  state->is_flame = false;
  state->thrusting = false;
  state->shielded = false;
  state->speed_boost_active = false;
  state->obstacle_slow_active = false;
  state->character_velocity = (vector_t){0, 0};
  state->current_game_mode = GAME_MODE_PLAYING;
  state->thrust_accel = DEFAULT_THRUST_ACCEL;

  state->character = NULL;
  state->background_body1 = NULL;
  state->background_body2 = NULL;
  state->floor_body_1 = NULL;
  state->floor_body_2 = NULL;
  state->alert_body = NULL;
  state->quiz_panel_body = NULL;
  state->quiz_question_text_body = NULL;
  state->quiz_timer_text_body = NULL;
  state->score_text_ui_body = NULL;
  state->distance_text_ui_body = NULL;
  state->time_text_ui_body = NULL;
  BACKGROUND_VEL = CONSTANT_VEL_BACKGROUND;

  /* recreate character */
  body_t *character = make_character_body(CHARACTER_WIDTH, CHARACTER_HEIGHT);
  body_set_centroid(character, RESET_POS);
  state->character = character;
  scene_add_body(state->scene, character);
  asset_make_image_with_body(NORMAL_CHARACTER_PATH, character);

  /* recreate background & floor */
  vector_t bg1_ctr = { MAX.x/2, MAX.y/2 };
  state->background_body1 = make_background_body(
    state->scene, BACKGROUND_PATH, bg1_ctr, MAX.x, MAX.y
  );
  vector_t bg2_ctr = { MAX.x/2 + MAX.x, MAX.y/2 };
  state->background_body2 = make_background_body(
    state->scene, BACKGROUND_PATH, bg2_ctr, MAX.x, MAX.y
  );
  vector_t fl1_ctr = { MAX.x/2, FLOOR_SPRITE_HEIGHT/2 };
  state->floor_body_1 = make_background_body(
    state->scene, FLOOR_PATH, fl1_ctr, MAX.x, FLOOR_SPRITE_HEIGHT
  );
  vector_t fl2_ctr = { MAX.x/2 + MAX.x, FLOOR_SPRITE_HEIGHT/2 };
  state->floor_body_2 = make_background_body(
    state->scene, FLOOR_PATH, fl2_ctr, MAX.x, FLOOR_SPRITE_HEIGHT
  );

  make_ui_component_bodies(state);
}

//----------------------------------------------------------------------------//
//                                EVENT HANDLERS                              //
//----------------------------------------------------------------------------//

/**
 * @brief Loops background music indefinitely
 */
void on_music_finished(void) {
  if (GLOBAL_STATE && GLOBAL_STATE->music) {
    Mix_PlayMusic(GLOBAL_STATE->music, -1);
  }
}

/**
 * @brief Dedicated helper function to play a SFX on a specific channel. If a SFX
 * is already playing in the channel, the new SFX will not be played except coins
 * @param sfx_chunk the Mix_Chunk * to be played
 */
void play_sfx(Mix_Chunk *sfx_chunk, bool isCoin) {
  if (isCoin) {
    Mix_Volume(COIN_SFX_CHANNEL, MIX_MAX_VOLUME / 6); 
    Mix_PlayChannel(COIN_SFX_CHANNEL, sfx_chunk, 0);
  } else {
    if (!Mix_Playing(SFX_CHANNEL)) {
      Mix_PlayChannel(SFX_CHANNEL, sfx_chunk, 0);
    }
  }
}
/**
 * @brief Resets the player character's position to a predefined RESET_POS.
 * @param character_body The player character's body.
 */
void reset_user(body_t *body) { 
  body_set_centroid(body, RESET_POS); 
}

 // if speed-boost is active, slow rockets by the same multiplier
static double rocket_horiz_speed_for(state_t *state) {
  return ROCKET_HORIZONTAL_SPEED * (state->speed_boost_active ? MULTIPLIER_VEL : 1.0);
}

void player_obstacle_collision_handler(body_t *body1, body_t *body2, vector_t axis, void *aux,
                        double force_const) {
  state_t *state = (state_t *)aux;
  if(state->shielded){
    scene_remove_body_by_ptr(state->scene, body2);
    asset_remove_body(body2);
    return;
  } else {
    if (state->sfx_gameover) {
      play_sfx(state->sfx_gameover, false);
    }
    state->current_game_mode = GAME_MODE_GAMEOVER;
  }
  
  reset_user(body1);
}

void power_up_collected_handler(body_t *character, body_t *powerup_item, vector_t axis, void *aux, double force_const) {
  state_t *state = (state_t *)aux;
  if(state->sfx_powerup_collect) {
    play_sfx(state->sfx_powerup_collect, false);
  }

  int choice = rand() % NUM_POWER_TYPES;
  state->pending_power = (power_type_t)choice;
  state->has_pending_power = true;
  asset_remove_body(powerup_item);
  scene_remove_body_by_ptr(state->scene, powerup_item);
  state->current_game_mode = GAME_MODE_QUIZ;
  state->quiz_time_remaining = QUIZ_TIME_INIT;
  int question_idx = rand() % (int)QUIZ_BANK_LEN;
  state->current_question_data = &QUIZ_BANK[question_idx];
  display_current_question(state);
}

void coin_collected_handler(body_t *character, body_t *coin, vector_t axis, void *aux, double force_const) {
  state_t *state = (state_t *)aux;
  state->score += 1;
  //Alternate between two coin sounds
  if (rand() % 2 == 0) {
    play_sfx(state->sfx_coin_collect_one, true);
  }else{
    play_sfx(state->sfx_coin_collect_two, true);
  }
  remove_body_and_asset(state->scene, coin);
}

//----------------------------------------------------------------------------//
//                                SPAWNING                                    //
//----------------------------------------------------------------------------//

/**
 * @brief Spawns a visual alert for an upcoming obstacle and plays an alert sound.
 * The alert is positioned based on the character's current y-position.
 * @param state A pointer to the current game state.
 */
void spawn_alert(state_t *state) {
  vector_t character_pos = body_get_centroid(state->character);
  vector_t alert_initial_center = {MAX.x - 20, character_pos.y};
  state->alert_body = make_obstacle_body(ALERT_SPRITE_WIDTH, ALERT_SPRITE_HEIGHT, alert_initial_center);
  scene_add_body(state->scene, state->alert_body);
  asset_make_image_with_body(ALERT_PATH, state->alert_body);
  if (state && state->sfx_alert) {
    play_sfx(state->sfx_alert, false);
  }
}

/**
 * @brief Spawns a moving obstacle at a predetermined y-coordinate.
 * @param state A pointer to the current game state, containing next_bug_y for positioning.
 */
void spawn_obstacle(state_t *state) {
  double w = OBSTACLE_WIDTH; 
  double h = OBSTACLE_HEIGHT;
  vector_t center = { MAX.x + w/2.0, state->next_bug_y };
  body_t *ob = make_obstacle_body(w, h, center);
  body_set_velocity(ob, (vector_t){ -BASE_OBJ_VEL.x*20, 0 });
  scene_add_body(state->scene, ob);
  //upon collision we need to make a game over screen instead of just moving to start pos
  create_collision(state->scene,
                   state->character,
                   ob,
                   player_obstacle_collision_handler,
                   state,
                   0, 
                   NULL);

  asset_make_image_with_body(MOVING_OBSTACLE_PATH, ob);
}

void spawn_vertical_laser(state_t *state, double y_pos) {
  vector_t center = {MAX.x + VERTICAL_LASER_WIDTH/2.0, y_pos};
  body_t *vl = make_vertical_laser_body(center);
  body_set_velocity(vl, BACKGROUND_VEL);
  scene_add_body(state->scene, vl);

  create_collision(state->scene,
                   state->character,
                   vl,
                   player_obstacle_collision_handler,
                   state,
                   0, 
                   NULL);
  asset_make_image_with_body(LASER_VERTICAL_PATHS[0], vl);
}

void spawn_horizontal_laser(state_t *state, double y_pos) {
  vector_t center = {MAX.x + HORIZONTAL_LASER_WIDTH/2.0, y_pos};
  body_t *vh = make_horizontal_laser_body(center);
  body_set_velocity(vh, BACKGROUND_VEL);
  scene_add_body(state->scene, vh);

  create_collision(state->scene,
                   state->character,
                   vh,
                   player_obstacle_collision_handler,
                   state,
                   0, 
                   NULL);
                   
  asset_make_image_with_body(LASER_HORIZONTAL_PATHS[0], vh);
}
void spawn_shuriken(state_t *state) {
  double y = rand_double(
    MIN.y + FLOOR_SPRITE_HEIGHT + 30,
    MAX.y - 30
  );
  vector_t center = { MAX.x + 30, y };
  body_t *sh = make_shuriken_body(center);
  body_set_velocity(sh, BACKGROUND_VEL);
  scene_add_body(state->scene, sh);
  create_collision(state->scene,
                   state->character,
                   sh,
                   player_obstacle_collision_handler,
                   state,
                   0, NULL);
  asset_make_image_with_body(SHURIKEN_PATH, sh);
}


void spawn_heat_seeking_rocket(state_t *state) {
  vector_t spawn_pos = (vector_t){.x = MAX.x + HEAT_SEEKING_ROCKET_WIDTH, state->next_bug_y};
  body_t *rocket = make_heat_seeking_rocket_body(spawn_pos);

  body_set_velocity(rocket, (vector_t){rocket_horiz_speed_for(state), 0});
  scene_add_body(state->scene, rocket);

  create_collision(state->scene,
                   state->character,
                   rocket,
                   player_obstacle_collision_handler,
                   state,
                   0, 
                   NULL);

  asset_make_image_with_body(HEAT_SEEKING_ROCKET_PATHS[0], rocket);
}

/**
 * @brief Spawns a power-up at a random y-position, moving with the background.
 * @param state A pointer to the current game state.
 */
void spawn_powerup(state_t *state) {
  double y = rand_double(MIN.y + FLOOR_SPRITE_HEIGHT + POWERUP_HEIGHT/2 , 
                        MAX.y - POWERUP_HEIGHT/2);
  vector_t center = { MAX.x + POWERUP_WIDTH/2, y };
  
  body_t *pu = make_powerup_body(center); 
  double sign = (rand() % 2 == 0) ? +1.0 : -1.0;
  vector_t initial_vel = (vector_t){
    .x = BACKGROUND_VEL.x, 
    .y = sign * POWERUP_VERTICAL_SPEED 
  };
  body_set_velocity(pu, initial_vel);
  scene_add_body(state->scene, pu);

  asset_make_image_with_body(GENERIC_POWERUP, pu);
  create_collision(state->scene,
                     state->character,  
                     pu,            
                     (collision_handler_t) power_up_collected_handler,
                     state,
                     0, NULL);

  if (state->sfx_powerup_spawn) {
    Mix_Volume(POWERUP_SFX_CHANNEL, MIX_MAX_VOLUME / 20);
    Mix_PlayChannel(POWERUP_SFX_CHANNEL, state->sfx_powerup_spawn, 0);
  }
}

/**
 * @brief Spawns a coin at position vector_t (handled in main)
 * @param position position to spawn coin
 */
void spawn_coin(state_t *state, vector_t position) {
  body_t *coin = make_coin_body(position);
  body_set_velocity(coin, BACKGROUND_VEL);
  scene_add_body(state->scene, coin);
  asset_make_image_with_body(COIN_PATHS[state->coin_frame_index], coin);
  
  create_collision(state->scene, state->character, coin, coin_collected_handler, state, 0, NULL);
}

//----------------------------------------------------------------------------//
//                                GAME LOGIC                                  //
//----------------------------------------------------------------------------//

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {

  if (state->current_game_mode == GAME_MODE_GAMEOVER &&
    type == KEY_PRESSED && key == '1') {
    reset_game(state);
    state->game_over_shown = false;
    return;
  }
  if (state->current_game_mode == GAME_MODE_PLAYING) {
    if (key != SPACE_BAR) {
      return;
    }
  
    body_t *character = scene_get_body(state->scene, 0) ;
    if (type == KEY_PRESSED) {
      state->thrusting = true;
      if (!state->is_flame) {
         asset_remove_body(character);
        if (state->shielded) {
          asset_make_image_with_body(PROTECTIVE_SHIELD, character);
        } else {
          asset_make_image_with_body(FLAME_CHARACTER_PATH, character);
        }
        state->is_flame = true;
      }
      if (!Mix_Playing(THRUST_SFX_CHANNEL)) {
        Mix_Volume(THRUST_SFX_CHANNEL, MIX_MAX_VOLUME / 6); 
        Mix_PlayChannel(THRUST_SFX_CHANNEL, state->sfx_thrust_loop, -1);
      }
      if (state->is_running_sfx) {
        Mix_HaltChannel(RUNNING_SFX_CHANNEL);
        state->is_running_sfx = false;
      }
      return;
    }
    if (type == KEY_RELEASED) {
      state->thrusting = false;
      Mix_HaltChannel(THRUST_SFX_CHANNEL);
      return;
    }
  } else if (state->current_game_mode == GAME_MODE_QUIZ) {
    if (type == KEY_PRESSED) {
      int selected_option_idx = -1;
      if (key >= '1' && key < ('1' + state->current_question_data->num_options)) {
        selected_option_idx = key - '1';
      }
      
      if (selected_option_idx != -1) {
        bool correct = (selected_option_idx == state->current_question_data->correct_option_index);
      if (correct && state->has_pending_power) {
        // Apply the stored power-up
        switch (state->pending_power) {
          case POWER_SHIELD:
            state->shielded = true;
            printf("%s\n", "Correct. You received shield powerup!");
            state->shield_timer = 0.0;
            asset_make_image_with_body(PROTECTIVE_SHIELD, state->character);
            break;
          case POWER_SPEED:
          printf("%s\n", "Correct. You received a slow down powerup!");
            state->obstacle_slow_active = true;
            state->obstacle_slow_timer = 0.0;
            BACKGROUND_VEL.x*=MULTIPLIER_VEL;
            body_set_velocity(state->background_body1, BACKGROUND_VEL);
            body_set_velocity(state->background_body2, BACKGROUND_VEL);
            body_set_velocity(state->floor_body_1, BACKGROUND_VEL);
            body_set_velocity(state->floor_body_2, BACKGROUND_VEL);
            break;
          case POWER_DISTANCE:
          printf("%s\n", "Correct. You received distance powerup!");
            state->distance_traveled_meters+=100;
            break;
         }
        
      } else {
        printf("%s\n", "Wrong. Reporting you to Professor Blank.");
      }
      state->has_pending_power = false;
      hide_quiz(state);
      state->thrusting = false;
      Mix_HaltChannel(THRUST_SFX_CHANNEL);
      state->current_question_data = NULL;
      state->current_game_mode = GAME_MODE_PLAYING;
      }
    }
  }
}

state_t *emscripten_init() {
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
    fprintf(stderr, "SDL_mixer could not initialize! Error: %s\n", Mix_GetError());
    exit(1);
  }
  
  asset_cache_init();
  sdl_init(MIN, MAX);
  if (TTF_Init() == -1) {
    fprintf(stderr, "TTF_Init failed.");
    exit(1);
  }

  state_t *state = malloc(sizeof(state_t));
  srand(time(NULL)); // Random seed rand()

  // Quiz state variables
  state->current_game_mode = GAME_MODE_PLAYING;
  state->current_question_data = NULL;
  state->quiz_time_remaining = 0.0;
  state->quiz_panel_body = NULL;
  state->quiz_question_text_body = NULL;
  state->quiz_option_text_bodies = list_init(LIST_INIT_CAPACITY, NULL); 
  state->quiz_timer_text_body = NULL;
  
  state->scene = scene_init();
  state->character_velocity = (vector_t){0,0};

  // Character state variables
  state->is_flame = false;
  state->thrusting = false;
  state->shielded = false;
  state->has_pending_power = false;
  state->pending_power = POWER_SHIELD;
  state->shield_timer = 0.0;
  state->obstacle_slow_active = false;
  state->speed_boost_active = false;
  state->time_since_last_shuriken = 0.0;
  state->speed_boost_timer  = 0.0;
  state->thrust_accel = DEFAULT_THRUST_ACCEL;
  state->obstacle_slow_timer  = 0.0;
  state->current_game_mode = GAME_MODE_PLAYING;
  state->time_since_last = 0.0;
  state->time_since_last_powerup_spawn = 0.0;
  state->bg_vel_mult = 1.0;

  //Alert state variables
  state->alert_shown = false;
  state->alert_body = NULL;

  // Coin state variables
  state->coin_frame_index = 0;
  state->last_coin_animation_change = 0;
  state->last_coin_spawn_time = 0;

  // UI state variables
  state->score = 0;
  state->distance_traveled_meters = 0;
  state->total_game_time_seconds = 0;
  state->time_text_ui_body = NULL;
  state->score_text_ui_body = NULL;
  state->distance_text_ui_body = NULL;
  state->dist_str_len = 2;
  state->time_str_len = 3;
  state->score_str_len = 4;
  state->game_over_shown = false;

  // Laser state variables
  state->laser_frame_index = 0;
  state->last_laser_spawn_time = 0;
  state->last_laser_animation_change = 0;
  state->last_hs_rocket_animation_change = 0;
  state->hs_rocket_frame_index = 0;

  //Character running state variable
  state->character_running_frame_index = 0;
  state->last_character_running_animation_change = 0;
  state->character_shielded_running_frame_index = 0;
  state->last_character_shielded_running_animation_change = 0;
  
  
  // Character
  body_t *character = make_character_body(CHARACTER_WIDTH, CHARACTER_HEIGHT);
  body_set_centroid(character, RESET_POS);
  state->character = character;
  scene_add_body(state->scene, character);

  // Background and Floor
  vector_t background_center_1 = {MAX.x / 2, MAX.y / 2};
  body_t *background_1 = make_background_body(state->scene, BACKGROUND_PATH, background_center_1, MAX.x, MAX.y);
  state->background_body1 = background_1;


  vector_t background_center_2 = {MAX.x / 2 + MAX.x, MAX.y / 2};
  body_t *background_2 = make_background_body(state->scene, BACKGROUND_PATH, background_center_2, MAX.x, MAX.y);
  state->background_body2 = background_2;
  
  vector_t floor_center_1 = {MAX.x / 2, FLOOR_SPRITE_HEIGHT / 2};
  state->floor_body_1 = make_background_body(state->scene, FLOOR_PATH, floor_center_1, MAX.x, FLOOR_SPRITE_HEIGHT);

  vector_t floor_center_2 = {MAX.x / 2 + MAX.x, FLOOR_SPRITE_HEIGHT / 2};
  state->floor_body_2 = make_background_body(state->scene, FLOOR_PATH, floor_center_2, MAX.x, FLOOR_SPRITE_HEIGHT);

  asset_make_image_with_body(NORMAL_CHARACTER_PATH, character);

  // Setup Key Handler
  sdl_on_key((key_handler_t)on_key);

  // Start Background Music
  state->music = Mix_LoadMUS(MUSIC_PATH);
  Mix_PlayMusic(state->music, -1);       
  Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
  GLOBAL_STATE = state;
  Mix_HookMusicFinished(on_music_finished);

  // UI Body Components
  make_ui_component_bodies(state);

  // Load SFX
  state->sfx_powerup_spawn = Mix_LoadWAV(POWER_UP_SPAWN_SOUND_PATH);
  state->sfx_powerup_collect = Mix_LoadWAV(POWER_UP_COLLECT_SOUND_PATH);
  state->sfx_gameover = Mix_LoadWAV(GAME_OVER_SOUND_PATH);
  state->sfx_alert    = Mix_LoadWAV(ALERT_SPAWN_SOUND_PATH);
  state->sfx_coin_collect_one = Mix_LoadWAV(COIN_COLLECT_SOUND_ONE_PATH);
  state->sfx_coin_collect_two = Mix_LoadWAV(COIN_COLLECT_SOUND_TWO_PATH);
  state->sfx_thrust_loop = Mix_LoadWAV(THRUST_SOUND_PATH);
  state->sfx_running_loop = Mix_LoadWAV(RUNNING_SOUND_PATH);
  state->sfx_laser_loop = Mix_LoadWAV(LASER_SOUND_PATH);
  state->is_running_sfx = false;
  
  return state;
}


bool emscripten_main(state_t *state) {

  if (state->current_game_mode == GAME_MODE_GAMEOVER && !state->game_over_shown) {
    display_game_over(state);   
    state->game_over_shown = true;
  }

  if (state->current_game_mode == GAME_MODE_GAMEOVER) {
    // render only
    sdl_clear();
    sdl_render_scene(state->scene);
    for (size_t i = 0; i < list_size(asset_get_asset_list()); i++)
      asset_render(list_get(asset_get_asset_list(), i));
    sdl_show();
    return false;
  }



  double dt = time_since_last_tick();

  if (state->current_game_mode == GAME_MODE_PLAYING) {

    // ***** SCENE UPDATE *****
    scene_tick(state->scene, dt);
    
    // ***** CHARACTER MOVEMENT *****
    body_t *character = scene_get_body(state->scene, 0);
    vector_t old_center   = body_get_centroid(character);

    if (state->thrusting) {
      state->character_velocity.y += DEFAULT_THRUST_ACCEL * dt;
      
    } else {
      state->character_velocity.y += GRAVITY_ACCEL * dt;
    }

    vector_t displacement = vec_multiply(dt, state->character_velocity);
    vector_t new_center   = vec_add(old_center, displacement);

    //vertical boundaries
    if (new_center.y <= GROUND_Y) {
      new_center.y = GROUND_Y;
      state->character_velocity.y = 0;
    }
    if (new_center.y + CHARACTER_HEIGHT / 2.0 > MAX.y) {
      new_center.y = MAX.y - CHARACTER_HEIGHT / 2.0;
      state->character_velocity.y = 0;
    }

    //horizontal boundaries
    double half_w = CHARACTER_WIDTH / 2.0;
    if (new_center.x < MIN.x + half_w) new_center.x = MIN.x + half_w;
    if (new_center.x > MAX.x - half_w) new_center.x = MAX.x - half_w;

    body_set_centroid(character, new_center);
    
    //character sprite change based on thrust or not
    if (!state->thrusting && state->is_flame && state->character_velocity.y <= 0) {
      asset_remove_body(character);
      if (state->shielded) {
        asset_make_image_with_body(PROTECTIVE_SHIELD, character);
      } else {
        asset_make_image_with_body(NORMAL_CHARACTER_PATH, character);
      }
      state->is_flame = false;
    }

    // ***** RUNNING SOUND LOGIC *****
    bool character_on_ground = (body_get_centroid(state->character).y <= GROUND_Y + 5);
    if (character_on_ground && !state->is_flame) {
      if (!state->is_running_sfx) {
        Mix_Volume(RUNNING_SFX_CHANNEL, MIX_MAX_VOLUME);
        Mix_PlayChannel(RUNNING_SFX_CHANNEL, state->sfx_running_loop, -1);
        state->is_running_sfx = true;
      }
    } else {
      Mix_HaltChannel(RUNNING_SFX_CHANNEL);
      state->is_running_sfx = false;
    }

    // ***** POWERUP LOGIC *****
    size_t n_bodies = scene_bodies(state->scene);
    for (size_t i = 0; i < n_bodies; i++) {
      body_t *b = scene_get_body(state->scene, i);
      if (!b) continue;

      body_info_type_t *info = (body_info_type_t *)body_get_info(b);
      if (info && *info == POWERUP) {
        vector_t pos = body_get_centroid(b);
        vector_t vel = body_get_velocity(b);

        double top_limit = MAX.y - POWERUP_HEIGHT/2.0;
        double bot_limit = MIN.y + FLOOR_SPRITE_HEIGHT + POWERUP_HEIGHT/2.0;

        if (pos.y >= top_limit && vel.y > 0) {
          vel.y = -vel.y;
          body_set_velocity(b, vel);
        }
        else if (pos.y <= bot_limit && vel.y < 0) {
          vel.y = -vel.y;
          body_set_velocity(b, vel);
        }
      }
    }

    if(state->shielded){
      state->shield_timer+=dt;
      if(state->shield_timer>=POWERUP_DURATION){
        state->shielded=false;
        state->shield_timer=0.0;
        asset_remove_body(character);
        if(state->is_flame){
          asset_make_image_with_body(FLAME_CHARACTER_PATH, character);
        }else{
          asset_make_image_with_body(NORMAL_CHARACTER_PATH, character);
        }
      }
    }
    if (state->speed_boost_active) {
      state->speed_boost_timer += dt;
      if (state->speed_boost_timer >= POWERUP_DURATION) {
        state->speed_boost_active = false;
        state->speed_boost_timer = 0.0;
        state->thrust_accel = DEFAULT_THRUST_ACCEL;
      }
    }
    if (state->obstacle_slow_active) {
      state->obstacle_slow_timer += dt;
      if (state->obstacle_slow_timer >= POWERUP_DURATION) {
        BACKGROUND_VEL=CONSTANT_VEL_BACKGROUND;
        state->obstacle_slow_active = false;
        state->obstacle_slow_timer = 0.0;
        body_set_velocity(state->background_body1, BACKGROUND_VEL);
        body_set_velocity(state->background_body2, BACKGROUND_VEL);
        body_set_velocity(state->floor_body_1, BACKGROUND_VEL);
        body_set_velocity(state->floor_body_2, BACKGROUND_VEL);
      }
    }
    if (!state->obstacle_slow_active) {
    state->time_since_last += dt;
    state->time_since_last_powerup_spawn += dt;
    }
   


    // ***** ANIMATIONS *****
    list_t *all_assets = asset_get_asset_list();

    state->last_coin_animation_change += dt;
    state->last_laser_animation_change += dt;
    state->last_hs_rocket_animation_change += dt;
    state->last_character_running_animation_change += dt;

    bool is_powerup = false; // laser will only spawn when there is no powerup asset on the screen to prevent impossible powerups
    bool update_coin_anim = (state->last_coin_animation_change >= COIN_ANIMATION_INTERVAL);
    bool update_laser_anim = (state->last_laser_animation_change >= LASER_ANIMATION_INTERVAL);
    bool update_hs_rocket_anim = (state->last_hs_rocket_animation_change >= HEAT_SEEKING_ROCKET_ANIMATION_INTERVAL);
    bool update_character_anim = (state->last_character_running_animation_change >= CHARACTER_RUNNING_ANIMATION_INTERVAL);
    
    SDL_Texture *next_coin_texture;
    if (update_coin_anim) {
      state->last_coin_animation_change -= COIN_ANIMATION_INTERVAL;
      state->coin_frame_index = (state->coin_frame_index+1) % NUM_COIN_ANIM_FRAMES;
      const char *next_coin_path = COIN_PATHS[state->coin_frame_index];
      next_coin_texture = asset_cache_obj_get_or_create(ASSET_IMAGE, next_coin_path);
    }

    SDL_Texture *next_vl_texture;
    SDL_Texture *next_hl_texture;
    if (update_laser_anim) {
      state->last_laser_animation_change -= LASER_ANIMATION_INTERVAL;
      state->laser_frame_index = (state->laser_frame_index + 1) % NUM_LASER_ANIM_FRAMES;
      const char *next_vertical_laser_anim_path = LASER_VERTICAL_PATHS[state->laser_frame_index];
      const char *next_horizontal_laser_anim_path = LASER_HORIZONTAL_PATHS[state->laser_frame_index];
      next_vl_texture = asset_cache_obj_get_or_create(ASSET_IMAGE, next_vertical_laser_anim_path);
      next_hl_texture = asset_cache_obj_get_or_create(ASSET_IMAGE, next_horizontal_laser_anim_path);
    }

    SDL_Texture *next_hs_rocket_texture;
    if (update_hs_rocket_anim) {
      state->last_hs_rocket_animation_change -= HEAT_SEEKING_ROCKET_ANIMATION_INTERVAL;
      state->hs_rocket_frame_index = (state->hs_rocket_frame_index + 1) % NUM_HEAT_SEEKING_ROCKET_ANIM_FRAMES;
      const char *next_hs_rocket_path = HEAT_SEEKING_ROCKET_PATHS[state->hs_rocket_frame_index];
      next_hs_rocket_texture = asset_cache_obj_get_or_create(ASSET_IMAGE, next_hs_rocket_path);
    }

    SDL_Texture *next_character_running_texture;
    SDL_Texture *next_character_shielded_running_texture;
    if (update_character_anim) {
      state->last_character_running_animation_change -= CHARACTER_RUNNING_ANIMATION_INTERVAL;
      state->character_running_frame_index = (state->character_running_frame_index + 1) % NUM_CHARACTER_RUNNING_ANIM_FRAMES;
      const char *next_character_running_path = CHARACTER_RUNNING_PATHS[state->character_running_frame_index];
      next_character_running_texture = asset_cache_obj_get_or_create(ASSET_IMAGE, next_character_running_path);

      state->last_character_shielded_running_animation_change -= CHARACTER_RUNNING_ANIMATION_INTERVAL;
      state->character_shielded_running_frame_index = 
          (state->character_shielded_running_frame_index + 1) % NUM_CHARACTER_SHIELDED_RUNNING_ANIM_FRAMES;
      const char *next_path = CHARACTER_SHIELDED_RUNNING_PATHS[state->character_shielded_running_frame_index];
      next_character_shielded_running_texture = asset_cache_obj_get_or_create(ASSET_IMAGE, next_path);
    }

    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_t *b = scene_get_body(state->scene, i);
      body_info_type_t *info = body_get_info(b);
      if (info && *info == SHURIKEN) {
        double old_angle = body_get_rotation(b);
        body_set_rotation(b, old_angle + SHURIKEN_ROT_SPEED * dt);
      }
    }

    if (update_coin_anim || update_laser_anim || update_hs_rocket_anim || update_character_anim) {
      for (size_t i=0; i<list_size(all_assets); i++) {
        asset_t *curr = (asset_t *)list_get(all_assets, i);
        if (curr->type == ASSET_IMAGE) {
  
          image_asset_t *img_curr = (image_asset_t *)curr;
          if (img_curr->body == NULL) continue;

          body_info_type_t *body_info = (body_info_type_t *) body_get_info(img_curr->body);

          if (update_laser_anim) {
            if (body_info && *body_info == VERTICAL_LASER) {
              img_curr->texture = next_vl_texture;
            } else if (body_info && *body_info == HORIZONTAL_LASER) {
              img_curr->texture = next_hl_texture;
            }
            if (body_info && *body_info == POWERUP) {
              is_powerup = true;
            }
          }
          if (update_coin_anim) {
            if (body_info && *body_info == COIN) {
              img_curr->texture = next_coin_texture;
            }
          }
          if (update_hs_rocket_anim) {
            if (body_info && *body_info == HEAT_SEEK_ROCKET) {
              img_curr->texture = next_hs_rocket_texture;
            }
          }
          if (update_character_anim) {
            if (body_info && *body_info == CHARACTER) {
              if (state->is_running_sfx) {
                img_curr->texture = state->shielded ? next_character_shielded_running_texture : next_character_running_texture;
              }
            }
          }
        }
      }
    }

    // ***** LASER SPAWNING *****
    state->last_laser_spawn_time += dt;
    if (!is_powerup && state->last_laser_spawn_time >= LASER_SPAWN_INTERVAL + rand_double(0.0, 3.0)) { //some randomness in spawning intervals
      size_t rand_orientation = rand() % 2;
      if (rand_orientation == 0) { //prevent coin collisions
        //spawn vertical laser with bounded ypos
        double y_min = MIN.y + FLOOR_SPRITE_HEIGHT + VERTICAL_LASER_HEIGHT / 2.0;
        double y_max = MAX.y - VERTICAL_LASER_HEIGHT/2.0;
        double y_pos = rand_double(y_min, y_max);
        spawn_vertical_laser(state, y_pos);
      } else {
        //spawn horizontal laser with bounded ypos
        double y_min = MIN.y + FLOOR_SPRITE_HEIGHT + HORIZONTAL_LASER_HEIGHT / 2.0;
        double y_max = MAX.y - HORIZONTAL_LASER_HEIGHT/2.0;
        double y_pos = rand_double(y_min, y_max);
        spawn_horizontal_laser(state, y_pos);
      }
      state->last_laser_spawn_time = 0.0;
      //play laser music if not already playing
      if (!Mix_Playing(LASER_SFX_CHANNEL)) {
        Mix_PlayChannel(LASER_SFX_CHANNEL, state->sfx_laser_loop, -1);
      }
    }

    // ***** COIN SPAWNING *****
    state->last_coin_spawn_time += dt;
    if (state->last_coin_spawn_time >= COIN_SPAWN_INTERVAL + rand_double(0.0, 1.5)) {
      size_t rand_pattern = rand();
      size_t rand_num_coins = (rand() % (COIN_NUM_MAX - COIN_NUM_MIN + 1)) + COIN_NUM_MIN;

      if (rand_pattern % 3 == 0) {
        //LINEAR
        double centerline_y = rand_double(MIN.y + FLOOR_SPRITE_HEIGHT + COIN_RADIUS * 2, MAX.y - COIN_RADIUS * 2);
        double curr_x = MAX.x + COIN_RADIUS;

        for (size_t i=0; i<rand_num_coins; i++) {
          spawn_coin(state, (vector_t){.x = curr_x, .y = centerline_y});
          curr_x += 3 * COIN_RADIUS;
        }
      } else if (rand_pattern % 3 == 1) {
        // RECTANGLE
        size_t num_rows_rect = (rand() % 2) + 2;
        size_t num_cols_rect = (rand() % 3) + 2;
        
        double spacing_rect = COIN_RADIUS * 3;
        double pattern_height = num_rows_rect * spacing_rect;

        double min_start_y_rect = MIN.y + FLOOR_SPRITE_HEIGHT + COIN_RADIUS;
        double max_start_y_rect = MAX.y - COIN_RADIUS - pattern_height;

        double y_start = rand_double(min_start_y_rect, max_start_y_rect);
        double x_start = MAX.x + COIN_RADIUS * 2;

        for (size_t r = 0; r < num_rows_rect; r++) {
          for (size_t c = 0; c < num_cols_rect; c++) {
            double c_x = x_start + c * spacing_rect;
            double c_y = y_start + r * spacing_rect;

            spawn_coin(state, (vector_t){.x = c_x, .y = c_y});
          }
        }
      } else {
        // ZIG ZAG
        size_t num_coins = (rand() % 5) + 6;
        double amplitude = rand_double(COIN_RADIUS * COIN_SCALE_1, COIN_RADIUS * COIN_SCALE_2);
        
        double min_center_y = MIN.y + FLOOR_SPRITE_HEIGHT + COIN_RADIUS * amplitude;
        double max_center_y = MAX.y - COIN_RADIUS * amplitude;

        double centerline_y;
        if (min_center_y >= max_center_y) {
          centerline_y = (MIN.y + MAX.y) / 2.0;
        } else {
          centerline_y = rand_double(min_center_y, max_center_y);
        }

        double x_start = MAX.x + COIN_RADIUS * 2;
        double x_spacing = COIN_RADIUS * COIN_SCALE_3;

        for (size_t i=0; i<num_coins; i++) {
          double curr_x = x_start + i * x_spacing;
          double coin_y;
          if (i % 2 == 0) {
            coin_y = centerline_y + amplitude;
          } else {
            coin_y = centerline_y - amplitude;
          }
          spawn_coin(state, (vector_t){.x = curr_x, .y = coin_y});
        }
      }

      state->last_coin_spawn_time = 0.0;
    }

    // ***** ALERT, POWERUP AND OBSTACLE SPAWNING *****
    if (state->alert_body != NULL) {
      vector_t character_pos = body_get_centroid(character);
      vector_t alert_current_pos = body_get_centroid(state->alert_body);
      body_set_centroid(state->alert_body, (vector_t){alert_current_pos.x, character_pos.y});
    }

    if (!state->alert_shown && state->time_since_last >= ALERT_TRIGGER_OFFSET) {
      double h = OBSTACLE_HEIGHT;
      state->next_bug_y = rand_double(MIN.y + FLOOR_SPRITE_HEIGHT + h/2.0, MAX.y - h/2.0);
      spawn_alert(state);
      state->alert_shown = true;
    }

    if (state->time_since_last >= OBSTACLE_SPAWN_INTERVAL) {
      if (state->alert_body) {
        state->next_bug_y = body_get_centroid(state->alert_body).y;
        scene_remove_body_by_ptr(state->scene, state->alert_body);
        state->alert_body = NULL;
      } 
      if (rand() % 2 == 0) {
        spawn_heat_seeking_rocket(state);
      } else {
        spawn_obstacle(state);
      }

      state->time_since_last = 0.0;
      state->alert_shown = false;
    }


    state->time_since_last_shuriken += dt;
    if (state->time_since_last_shuriken >= SHURIKEN_SPAWN_INTERVAL) {
      spawn_shuriken(state);
      state->time_since_last_shuriken = 0.0;
    }

    if (!state->speed_boost_active &&state->time_since_last_powerup_spawn >= POWERUP_SPAWN_INTERVAL) {
      spawn_powerup(state);
      state->time_since_last_powerup_spawn = 0.0;
    }

    // ***** BACKGROUND AND FLOOR SCROLLING AND WRAP *****
    vector_t background_center_1 = body_get_centroid(state->background_body1);
    vector_t background_center_2 = body_get_centroid(state->background_body2);
    vector_t floor_center_1 = body_get_centroid(state->floor_body_1);
    vector_t floor_center_2 = body_get_centroid(state->floor_body_2);

    if (background_center_1.x + MAX.x / 2 < MIN.x) {
      body_set_centroid(state->background_body1, (vector_t) {background_center_2.x + MAX.x, background_center_1.y});
    }
    if (background_center_2.x + MAX.x / 2 < MIN.x) {
      body_set_centroid(state->background_body2, (vector_t) {background_center_1.x + MAX.x , background_center_2.y});
    }
    if (floor_center_1.x + MAX.x / 2.0 < MIN.x) { 
        body_set_centroid(state->floor_body_1, (vector_t) {floor_center_2.x + MAX.x, floor_center_1.y});
    }
    if (floor_center_2.x + MAX.x / 2.0 < MIN.x) {
        body_set_centroid(state->floor_body_2, (vector_t) {floor_center_1.x + MAX.x , floor_center_2.y});
    }

    // ***** UPDATE UI ELEMENTS *****
    state->distance_traveled_meters += dt * METERS_PER_SECOND_TRAVEL_SPEED;
    state->total_game_time_seconds += dt;
  
    remove_all_text_assets_for_body(state->time_text_ui_body);
    remove_all_text_assets_for_body(state->distance_text_ui_body);
    remove_all_text_assets_for_body(state->score_text_ui_body);
  

    char time_str[TIMER_SIZE];
    snprintf(time_str, sizeof(time_str), "%.1lfs", state->total_game_time_seconds);
    size_t new_time_len = strlen(time_str);
    if (new_time_len > state->time_str_len) {
      remove_all_text_assets_for_body(state->time_text_ui_body);
      scene_remove_body_by_ptr(state->scene, state->time_text_ui_body);
      list_t *time_pts = rect_for_text(FONT_PATH, TIME_FONT_SIZE, time_str);
      body_info_type_t *info_time = malloc(sizeof(body_info_type_t)); *info_time = UI;
      state->time_text_ui_body = body_init_with_info(time_pts, UNIT_WEIGHT, PLACEHOLDER_COLOR, info_time, free);
      body_set_centroid(state->time_text_ui_body, TIME_TEXT_POS);
      scene_add_body(state->scene, state->time_text_ui_body);
      state->time_str_len = new_time_len;
    }
    asset_make_text_with_body(FONT_PATH, state->time_text_ui_body, strdup(time_str), UI_TEXT_COLOR);

    char dist_str[TIMER_SIZE];
    snprintf(dist_str, sizeof(dist_str), "%.0fm", state->distance_traveled_meters);
    size_t new_dist_len = strlen(dist_str);
    if (new_dist_len > state->dist_str_len) {
      remove_all_text_assets_for_body(state->distance_text_ui_body);
      scene_remove_body_by_ptr(state->scene, state->distance_text_ui_body);
      list_t *dist_pts = rect_for_text(FONT_PATH, DIST_FONT_SIZE, dist_str);
      body_info_type_t *info_dist = malloc(sizeof(body_info_type_t)); *info_dist = UI;
      state->distance_text_ui_body = body_init_with_info(dist_pts, UNIT_WEIGHT, PLACEHOLDER_COLOR, info_dist, free);
      body_set_centroid(state->distance_text_ui_body, DISTANCE_TEXT_POS);
      scene_add_body(state->scene, state->distance_text_ui_body);
      state->dist_str_len = new_dist_len;
    }
    asset_make_text_with_body(FONT_PATH, state->distance_text_ui_body, strdup(dist_str), UI_TEXT_COLOR);

    char score_str[TIMER_SIZE];
    snprintf(score_str, sizeof(score_str), "%04zu", state->score);
    size_t new_score_len = strlen(score_str);
    if (new_score_len > state->score_str_len) {
      remove_all_text_assets_for_body(state->score_text_ui_body);
      scene_remove_body_by_ptr(state->scene, state->score_text_ui_body);    
      list_t *score_pts = rect_for_text(FONT_PATH, SCORE_FONT_SIZE, score_str);
      body_info_type_t *info_score = malloc(sizeof(body_info_type_t)); *info_score = UI;
      state->score_text_ui_body = body_init_with_info(score_pts, UNIT_WEIGHT, PLACEHOLDER_COLOR, info_score, free);
      body_set_centroid(state->score_text_ui_body, SCORE_TEXT_POS);
      scene_add_body(state->scene, state->score_text_ui_body);
      state->score_str_len = new_score_len;
    }
    asset_make_text_with_body(FONT_PATH, state->score_text_ui_body, strdup(score_str), UI_TEXT_COLOR);

    // ***** HEAT SEEKING ROCKET TRACKING *****
    vector_t player_center_pos = body_get_centroid(state->character);
    for (size_t i=0; i<scene_bodies(state->scene); i++) {
      body_t *curr_body = scene_get_body(state->scene, i);
      if (curr_body) {
        body_info_type_t *info = (body_info_type_t *)body_get_info(curr_body);
        if (info && *info == HEAT_SEEK_ROCKET) {
          vector_t rocket_pos = body_get_centroid(curr_body);
          vector_t current_rocket_vel = body_get_velocity(curr_body);
          double new_vy = current_rocket_vel.y;
          if (rocket_pos.x > player_center_pos.x) { 
            double dy_to_player = player_center_pos.y - rocket_pos.y;

            // Adjust vertical velocity towards the player
            if (dy_to_player > 0) {
                new_vy += ROCKET_VERTICAL_ADJUST_RATE * dt;
            } else if (dy_to_player < 0) {
                new_vy -= ROCKET_VERTICAL_ADJUST_RATE * dt;
            }

            // Clamp vertical speed
            if (new_vy > ROCKET_MAX_VERTICAL_SPEED) {
                new_vy = ROCKET_MAX_VERTICAL_SPEED;
            } else if (new_vy < -ROCKET_MAX_VERTICAL_SPEED) {
                new_vy = -ROCKET_MAX_VERTICAL_SPEED;
            }
          }

          body_set_velocity(curr_body, (vector_t){rocket_horiz_speed_for(state), new_vy});
        }
      }
    }

    // ***** CLEANUP OFF SCREEN OBJECTS *****
    bool laser_on_screen = false; //for stopping laser sfx
    size_t num_bodies = scene_bodies(state->scene);
    if (num_bodies > 0) {
      for (size_t i = num_bodies; i-- > 0; ) { // PREVENT UNDERFLOW
        
        body_t *curr_body = scene_get_body(state->scene, i);

        if (!curr_body) continue;
        if (curr_body == state->character ||
            curr_body == state->background_body1 ||
            curr_body == state->background_body2 ||
            curr_body == state->alert_body) {
              continue;
            }
        
        body_info_type_t *info = (body_info_type_t *)body_get_info(curr_body);
        //stop vertical and horizontal laser sounds
        if (info && (*info == VERTICAL_LASER || *info == HORIZONTAL_LASER)) {
          laser_on_screen = true;
        }

        if (info && (*info == COIN || *info == OBSTACLE || *info == POWERUP || *info == HEAT_SEEK_ROCKET 
            || *info == HORIZONTAL_LASER || *info == VERTICAL_LASER || *info == SHURIKEN)) {
          vector_t center = body_get_centroid(curr_body);
          if (center.x < OFFSCREEN_X_REMOVAL_THRESHOLD) {
            remove_body_and_asset(state->scene, curr_body);
          }
        }
      }
    }

    if (!laser_on_screen && Mix_Playing(LASER_SFX_CHANNEL)) {
      Mix_HaltChannel(LASER_SFX_CHANNEL);
    }

  } else if (state->current_game_mode == GAME_MODE_QUIZ) {
    state->quiz_time_remaining -= dt;
    if (state->quiz_timer_text_body) {
      asset_remove_body(state->quiz_timer_text_body);
      char time_str[TIMER_SIZE];
      snprintf(time_str, sizeof(time_str), "Time: %.0f", fmax(0, state->quiz_time_remaining));
      asset_make_text_with_body(FONT_PATH, state->quiz_timer_text_body, time_str, UI_TEXT_COLOR);
    } 
    if (state->quiz_time_remaining <= 0) {
      hide_quiz(state);
      state->thrusting = false;
      Mix_HaltChannel(THRUST_SFX_CHANNEL);
      state->current_question_data = NULL;
      state->current_game_mode = GAME_MODE_PLAYING;
    }
  }

  // ***** RENDERING *****
  sdl_clear();
  sdl_render_scene(state->scene);

  list_t *assets = asset_get_asset_list();
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_render(list_get(assets, i));
  }
  
  sdl_show();
  return false;
}

void emscripten_free(state_t *state) {
  if (state->music) {
    Mix_HaltMusic();
    Mix_FreeMusic(state->music);
  }
  if (state->sfx_powerup_spawn)  Mix_FreeChunk(state->sfx_powerup_spawn);
  if (state->sfx_powerup_collect)  Mix_FreeChunk(state->sfx_powerup_collect);
  if (state->sfx_gameover) Mix_FreeChunk(state->sfx_gameover);
  if (state->sfx_alert)    Mix_FreeChunk(state->sfx_alert);
  if (state->sfx_coin_collect_one) Mix_FreeChunk(state->sfx_coin_collect_one);
  if (state->sfx_coin_collect_two) Mix_FreeChunk(state->sfx_coin_collect_two);
  if (state->sfx_thrust_loop) Mix_FreeChunk(state->sfx_thrust_loop);
  if(state->sfx_running_loop) Mix_FreeChunk(state->sfx_running_loop);
  if (state->sfx_laser_loop) Mix_FreeChunk(state->sfx_laser_loop);


  Mix_CloseAudio();
  list_free(asset_get_asset_list());
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}
