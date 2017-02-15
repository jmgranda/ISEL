#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "task.h"
#include "interp.h"

#define GPIO_IR 1

#define FREQ 9
#define NUM_CHAR 5
#define CHAR_W 8
#define CHAR_H 8

void pendulum_setup (void);
void render (char* buf, int n);

static char buf[NUM_CHAR * CHAR_W];
int ir = 0;

static void ir_isr (void) { ir = 1; }



/* Wait until next_activation (based on clock_gettime and nanosleep) */
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


static void
pintar (void)
{
    const struct timespec tcol = { 0, 1000000000L / (2 * FREQ * NUM_CHAR * CHAR_W) };
    struct timespec next_activation;

    /* Light Time (paint) */
    int i;
    const int gpio[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    /* wiringPi GPIO paint closer to real final ver */
    clock_gettime (CLOCK_MONOTONIC, &next_activation);
    for (i = 0; i < NUM_CHAR * CHAR_W; ++i) {
      char col = buf[i];
      int led;
      for (led = 0; led < 8; ++led) {      
        digitalWrite (gpio[led], (col & (1 << led)) ? HIGH : LOW);
      }
      timespec_add (&next_activation, &next_activation, &tcol);
      active_delay_until (&next_activation);
    }
}

static void
calcular (void)
{
    time_t t;
    struct tm* lt;


    /* Get time from system */
    time (&t);
    lt = localtime (&t);

    /* Calculate char buffer */
    render (buf + CHAR_W * 0, lt->tm_hour / 10);
    render (buf + CHAR_W * 1, lt->tm_hour % 10);
    render (buf + CHAR_W * 3, lt->tm_min / 10);
    render (buf + CHAR_W * 4, lt->tm_min % 10);
}

/* Clock gets time from system, calculates buffer and paints time */
void* main_clock (void* arg)
{
  const struct timespec tcol = { 0, 1000000000L / (2 * 9 * NUM_CHAR * CHAR_W) };
  struct timespec next_activation;

  /* Colon between Hours and Minutes */
  buf[16] = buf[17] = buf[18] = buf[21] = buf[22] = buf[23] = 0;
  buf[19] = buf[20] = 0x66;

  wiringPiISR (GPIO_IR, INT_EDGE_FALLING, ir_isr);

  clock_gettime (CLOCK_MONOTONIC, &next_activation);
  while (1) {
    if (ir) {
      ir = 0;
      pintar ();
      calcular ();
    }
    timespec_add (&next_activation, &next_activation, &tcol);
    active_delay_until (&next_activation);
  }
  return NULL;
}

int main ()
{
  wiringPiSetup ();
  pendulum_setup ();

  task_new ("clock", main_clock, 0, 0, 100, 1024);
  interp_run ();

  return 0;
}

