#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include "task.h"
#include "interp.h"

#define GPIO_IR 1

#define FREQ 9
#define NUM_CHAR 5
#define CHAR_W 8
#define CHAR_H 8

static char buf[NUM_CHAR * CHAR_W];
int ir = 0;

void render ();

static void ir_isr (void) { ir = 1; }

// Wait until next_activation (clock_gettime and nanosleep)
void delay_until (struct timespec* next_activation)
{
  struct timespec now, timeout;
  clock_gettime (CLOCK_MONOTONIC, &now);
  timespec_sub (&timeout, next_activation, &now);
  nanosleep (&timeout, NULL);
}

void active_delay_until (struct timespec* next_activation)
{
  struct timespec now;
  while (1) {  
    clock_gettime (CLOCK_MONOTONIC, &now);
    if (timespec_less (next_activation, &now))
      return;
  }
}

/* Clock gets time from system, calculates buffer and paints time */
void* main_clock (void* arg)
{
  struct timespec tcol = { 0, 1000000000L / (2 * 9 * NUM_CHAR * CHAR_W) };
  struct timespec next_activation;
  time_t t;
  struct tm* lt;
  const int gpio[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

  /* Colon between Hours and Minutes */
  buf[16] = buf[17] = buf[18] = buf[21] = buf[22] = buf[23] = 0;
  buf[19] = buf[20] = 0x66;

  wiringPiISR (GPIO_IR, INT_EDGE_FALLING, ir_isr);
  
  while (1) {
    /* Light Time (paint) */
    int i;

    clock_gettime (CLOCK_MONOTONIC, &next_activation);
    if (! ir) {
      timespec_add (&next_activation, &next_activation, &tcol);
      active_delay_until (&next_activation);
      continue;
    }

    ir = 0;

    for (i = 0; i < NUM_CHAR * CHAR_W; ++i) {
      char col = buf[i];
      int led;
      for (led = 0; led < 8; ++led) {
        digitalWrite (gpio[led], (col & (1 << led)) ? HIGH : LOW);
      }
      timespec_add (&next_activation, &next_activation, &tcol);
      active_delay_until (&next_activation);
    }

    /* Get time from system */
    time (&t);
    lt = localtime (&t);
    printf ("Hours: %d - Minutes: %d\n", lt->tm_hour, lt->tm_min);

    /* Calculate char buffer */
    render (buf + CHAR_W * 0, lt->tm_hour / 10);
    render (buf + CHAR_W * 1, lt->tm_hour % 10);
    render (buf + CHAR_W * 3, lt->tm_min / 10);
    render (buf + CHAR_W * 4, lt->tm_min % 10);
    /* Check buffer
    int i;
    for (test = 0; test < NUM_CHAR * CHAR_W; ++test) {
      printf ("%d", buf[test]);
    }
    printf ("\n");
    */
  }
  
  

/*
  struct timespec clk_period = { 0, 250 * 1000000L };
  struct timespec next_activation;

  fsm_t* cofm_fsm = cofm_fsm_new ();
  fsm_t* purse_fsm = purse_fsm_new ();


  clock_gettime (CLOCK_MONOTONIC, &next_activation);
  while (1) {
    fsm_fire (cofm_fsm);
    fsm_fire (purse_fsm);
    timespec_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);
  }
*/  
  return NULL;
}

int main ()
{
  wiringPiSetup();

  task_new ("clock", main_clock, 0, 0, 1, 1024);
  interp_run ();

  return 0;
}

