#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snake_utils.h"
#include "state.h"

/* Helper function definitions */
static char get_board_at(game_state_t* state, int x, int y);
static void set_board_at(game_state_t* state, int x, int y, char ch);
static bool is_tail(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static int incr_x(char c);
static int incr_y(char c);
static void find_head(game_state_t* state, int snum);
static char next_square(game_state_t* state, int snum);
static void update_tail(game_state_t* state, int snum);
static void update_head(game_state_t* state, int snum);

/* Helper function to get a character from the board (already implemented for you). */
static char get_board_at(game_state_t* state, int x, int y) {
  return state->board[y][x];
}

/* Helper function to set a character on the board (already implemented for you). */
static void set_board_at(game_state_t* state, int x, int y, char ch) {
  state->board[y][x] = ch;
}

/* Task 1 */
game_state_t* create_default_state() {
  /* Construct the game stgate */
  game_state_t *game = malloc(sizeof(game_state_t));

  game->x_size = 14;
  game->y_size = 10;

  /* Construct the game board pointer */
  game->board = malloc(sizeof(char*) * game->y_size);

  /* Construct the game board */
  static char *board[] = {
    "##############",
    "#            #",
    "#        *   #",
    "#            #",
    "#   d>       #",
    "#            #",
    "#            #",
    "#            #",
    "#            #",
    "##############"
  };

  game->board[0] = malloc(sizeof(char) * (game->x_size + 1) * game->y_size);
  for(unsigned int i = 0; i < game->y_size; ++i) {
    game->board[i] = game->board[0] + (game->x_size + 1) * i;
    strcpy(game->board[i], board[i]);
  }

  game->num_snakes = 1;

  /* Construct the snakes */
  game->snakes = malloc(sizeof(snake_t) * game->num_snakes);
  game->snakes[0].tail_x = 4;
  game->snakes[0].tail_y = 4;
  game->snakes[0].head_x = 5;
  game->snakes[0].head_y = 4;
  game->snakes[0].live = true;

  return game;
}

/* Task 2 */
void free_state(game_state_t* state) {
  free(state->snakes);
  free(state->board[0]);
  free(state->board);
  free(state);
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  for (size_t i = 0; i < state->y_size; ++i) {
    if (state->board[i]) {
      fprintf(fp, "%s\n", state->board[i]);
    }
  }
}

/* Saves the current state into filename (already implemented for you). */
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */
static bool is_tail(char c) {
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') { return true; }
  return false;
}

static bool is_snake(char c) {
  if (is_tail(c) || c == 'x') { return true; }
  if (c == '^' || c == '<' || c == 'v' || c == '>') { return true; }
  return false;
}

static char body_to_tail(char c) {
  if (c == '^') { return 'w'; }
  if (c == '<') { return 'a'; }
  if (c == 'v') { return 's'; }
  if (c == '>') { return 'd'; }
  return '?';
}

static int incr_x(char c) {
  if (c == '>' || c == 'd') { return 1; }
  else if (c == '<' || c == 'a') { return -1; }
  return 0;
}

static int incr_y(char c) {
  if (c == 'v' || c == 's') { return 1; }
  else if (c == '^' || c == 'w') { return -1; }
  return 0;
}

/* Task 4.2 */
static char next_square(game_state_t* state, int snum) {
  unsigned int x = state->snakes[snum].head_x;
  unsigned int y = state->snakes[snum].head_y;
  char ch = get_board_at(state, x, y);
  return get_board_at(state, x + incr_x(ch), y + incr_y(ch));
}

/* Task 4.3 */
static void update_head(game_state_t* state, int snum) {
  // get prev head location
  unsigned int x = state->snakes[snum].head_x;
  unsigned int y = state->snakes[snum].head_y;
  // get prev head body
  char ch = get_board_at(state, x, y);

  // get curr head location
  state->snakes[snum].head_x = incr_x(ch) + x;
  state->snakes[snum].head_y = incr_y(ch) + y;

  // set curr head body
  set_board_at(state, incr_x(ch) + x, incr_y(ch) + y, ch);
}

/* Task 4.4 */
static void update_tail(game_state_t* state, int snum) {
  // get prev tail location
  unsigned int prev_x = state->snakes[snum].tail_x;
  unsigned int prev_y = state->snakes[snum].tail_y;

  // get prev tail
  char ch = get_board_at(state, prev_x, prev_y);

  // determine curr tail location
  unsigned int curr_x = incr_x(ch) + prev_x;
  unsigned int curr_y = incr_y(ch) + prev_y;

  // get curr tail
  ch = get_board_at(state, curr_x, curr_y);
  // update curr tail location in snake struct
  state->snakes[snum].tail_x = curr_x;
  state->snakes[snum].tail_y = curr_y;

  // update curr tail in board
  set_board_at(state, curr_x, curr_y, body_to_tail(ch));

  // update prev tail in board
  set_board_at(state, prev_x, prev_y, ' ');
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for (unsigned int i = 0; i < state->num_snakes; ++i) {
    char ch = next_square(state, i);
    if (ch == '#' || is_snake(ch)) {
      state->snakes[i].live = false;
      set_board_at(state, state->snakes[i].head_x, state->snakes[i].head_y, 'x');
    } else if (ch == '*') {
      update_head(state, i);
      add_food(state);
    } else {
      update_head(state, i);
      update_tail(state, i);
    }
  }
  return;
}

/* Task 5 */
game_state_t* load_board(char* filename) {
  FILE *fp = fopen(filename, "r");
  unsigned int x_size = 0;
  unsigned int y_size = 0;

  // determinte the x_size
  while (fgetc(fp) != '\n') {
    ++x_size;
  }
  rewind(fp);

  // load board
  unsigned int buffer_size = x_size;
  char *board = malloc(sizeof(char) * (x_size + 1) * buffer_size);

  char *buffer = board;
  while (fgets(buffer, x_size + 2, fp) != NULL) {
    buffer[x_size] = '\0';
    ++y_size;

    if (buffer_size == y_size + 1) {
      board = realloc(board, sizeof(char) * (x_size + 1) * buffer_size * 2);
      buffer_size *= 2;
    }
    buffer = board + (x_size + 1) * y_size;
  }

  board = realloc(board, sizeof(char) * (x_size + 1) * y_size);
  fclose(fp);

  // Construct the game state
  game_state_t *state = malloc(sizeof(game_state_t));

  state->x_size = x_size;
  state->y_size = y_size;

  state->board = malloc(sizeof(char*) * state->y_size);
  for (unsigned int i = 0; i < state->y_size; ++i) {
    state->board[i] = board + (state->x_size + 1) * i;
  }

  state->num_snakes = 0;
  state->snakes = NULL;

  return state;
}

/* Task 6.1 */
static void find_head(game_state_t* state, int snum) {
  unsigned int x = state->snakes[snum].tail_x;
  unsigned int y = state->snakes[snum].tail_y;

  while (true) {
    char ch = get_board_at(state, x, y);
    unsigned int next_x = x + incr_x(ch);
    unsigned int next_y = y + incr_y(ch);

    ch = get_board_at(state, next_x, next_y);
    if (!is_snake(ch)) {
      state->snakes[snum].head_x = x;
      state->snakes[snum].head_y = y;
      return ;
    } else {
      x = next_x;
      y = next_y;
    }
  }
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  // find all tails of the state
  unsigned int buffer_size = 2;
  state->snakes = malloc(sizeof(snake_t) * buffer_size);

  for (unsigned int y = 0; y < state->y_size; ++y) {
    for (unsigned int x = 0; x < state->x_size; ++x) {
      if (is_tail(get_board_at(state, x, y))) {
        // realloc memeory
        if (buffer_size == state->num_snakes) {
          buffer_size *= 2;
          state->snakes = realloc(state->snakes, sizeof(snake_t) * buffer_size);
        }

        state->snakes[state->num_snakes].tail_x = x;
        state->snakes[state->num_snakes].tail_y = y;
        find_head(state, state->num_snakes);
        state->snakes[state->num_snakes].live = true;
        state->num_snakes += 1;
      }
    }
  }

  state->snakes = realloc(state->snakes, sizeof(snake_t) * state->num_snakes);

  return state;
}
