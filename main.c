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

#define GPIO_BUTTON	2
#define GPIO_LED	3
#define GPIO_CUP	4
#define GPIO_COFFEE	5
#define GPIO_MILK	6

#define CUP_TIME	250
#define COFFEE_TIME	3000
#define MILK_TIME	3000

enum cofm_state {
  COFM_WAITING,
  COFM_CUP,
  COFM_COFFEE,
  COFM_MILK,
};

enum purse_state {
  PURSE_WAITING,
};

extern int money;
extern int change;

static int button = 0;
static void button_isr (void) { button = 1; }
static int button_pressed (fsm_t* this) { return button && money >= 50; }


static int timer = 0;
static void timer_isr (union sigval arg) { timer = 1; }
static void timer_start (int ms)
{
  timer_t timerid;
  struct itimerspec spec;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  spec.it_value.tv_sec = ms / 1000;
  spec.it_value.tv_nsec = (ms % 1000) * 1000000;
  spec.it_interval.tv_sec = 0;
  spec.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &spec, NULL);
  timer = 0;
}
static int timer_finished (fsm_t* this) { return timer; }


static void cup (fsm_t* this)
{
  digitalWrite (GPIO_LED, LOW);
  digitalWrite (GPIO_CUP, HIGH);
  timer_start (CUP_TIME);
  button = 0;
  money -= 50;
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
  change = 1;
}


// Explicit FSM description
static fsm_trans_t cofm[] = {
  { COFM_WAITING, button_pressed, COFM_CUP,     cup    },
  { COFM_CUP,     timer_finished, COFM_COFFEE,  coffee },
  { COFM_COFFEE,  timer_finished, COFM_MILK,    milk   },
  { COFM_MILK,    timer_finished, COFM_WAITING, finish },
  {-1, NULL, -1, NULL },
};


int money = 0; // ¿tendrán que ser externas? 

static int coin;
static int coin_intro (fsm_t* this) { return coin; }


int change = 0; // ¿tendrán que ser externas? 
static int change_read (fsm_t* this) { return change; }

static void money_add (fsm_t* this) { money += coin; coin = 0; }
static void change_return (fsm_t* this) { money = 0; change = 0; }

// Explicit FSM description
static fsm_trans_t purse[] = {
  { PURSE_WAITING, coin_intro,  PURSE_WAITING,   money_add},
  { PURSE_WAITING, change_read, PURSE_WAITING,  change_return },
  {-1, NULL, -1, NULL },
};


// Utility functions, should be elsewhere

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}


/* Coffee machine with credit check and give change activation */
void* main_ec (void* arg)
{
  struct timeval clk_period = { 0, 250 * 1000 };
  struct timeval next_activation;
  fsm_t* cofm_fsm = fsm_new (cofm);
  fsm_t* purse_fsm = fsm_new (purse);

  wiringPiSetup();
  pinMode (GPIO_BUTTON, INPUT);
  wiringPiISR (GPIO_BUTTON, INT_EDGE_FALLING, button_isr);
  pinMode (GPIO_CUP, OUTPUT);
  pinMode (GPIO_COFFEE, OUTPUT);
  pinMode (GPIO_MILK, OUTPUT);
  pinMode (GPIO_LED, OUTPUT);
  digitalWrite (GPIO_LED, HIGH);
  //(scanf("%d %d %d", &coin, &change, &timer) == 3) {
  //(scanf("%d %d", &button, &timer) == 2)
  gettimeofday (&next_activation, NULL);
  while (1) {
    fsm_fire (cofm_fsm);
    fsm_fire (purse_fsm);
    timeval_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);
  }
  return NULL;
}

int main ()
{
	task_new ("ec", main_ec, 0, 0, 1, 1024);
	while (scanf("%d %d %d %d", &button, &coin, &change, &timer) == 4)
		;
	return 0;
}

