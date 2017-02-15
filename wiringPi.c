#include <stdio.h>
#include <time.h>
#include "task.h"
#include "interp.h"

#define FREQ 9
#define NUM_CHAR 5
#define CHAR_W 8
#define CHAR_H 8

extern int ir;

void delay_until (struct timespec* next_activation);

static volatile int pendulum_col = 0;
static char screen[CHAR_H][NUM_CHAR * CHAR_W + 1];

static void 
screen_dump (void)
{
  int i;
  printf ("\e7\e[?25l\e[s\e[1;1f"); // save cursor pos, goto (1,1)
  for (i = 0; i < CHAR_H; ++i)
    puts (screen[i]);
  printf ("\e[u\e8\e[?25h"); // restore cursor pos
}

void 
digitalWrite(int gpio, int value)
{
  screen[gpio][pendulum_col] = value? '*' : ' ';
}

static void* 
main_pendulum (void* arg)
{
  const struct timespec tcol = { 0, 1000000000L / (2 * FREQ * NUM_CHAR * CHAR_W) };
  struct timespec next_activation;

  clock_gettime (CLOCK_MONOTONIC, &next_activation);
  while (1) {
    for (pendulum_col = 0; pendulum_col < NUM_CHAR * CHAR_W; ++pendulum_col) {
      timespec_add (&next_activation, &next_activation, &tcol);
      delay_until (&next_activation);
    }
    for (pendulum_col = NUM_CHAR * CHAR_W - 2; pendulum_col > 0; --pendulum_col) {
      timespec_add (&next_activation, &next_activation, &tcol);
      delay_until (&next_activation);
    }
    ir = 1;
  }
  return NULL;
}

static void* 
main_refresh (void* arg)
{
  const struct timespec period = { 0, 250 * 1000000L };
  struct timespec next_activation;

  clock_gettime (CLOCK_MONOTONIC, &next_activation);
  while (1) {
    screen_dump ();
    timespec_add (&next_activation, &next_activation, &period);
    delay_until (&next_activation);
  }
  return NULL;
}

static int
screen_cmd (char *arg)
{
  screen_dump ();
  return 0;
}

void 
pendulum_setup (void)
{
  int i;
  for (i = 0; i < CHAR_H; ++i)
    screen[i][NUM_CHAR * CHAR_W] = '\0';

  interp_addcmd ("screen", screen_cmd, "Screen dump");
  task_new ("pendulum", main_pendulum, 0, 0, 1, 1024);
  task_new ("refresh", main_refresh, 0, 0, 1, 1024);
}
