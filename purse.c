#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"

enum purse_state {
  PURSE_WAITING,
};

int money = 0; // Tendrá que ser externa para cofm.c y main.c
int coin = 0;  // Tendrá que ser externa para cofm.c y main.c

static int coin_intro (fsm_t* this) { return coin; }


int change = 0; // Tendrá que ser externa para cofm.c 
static int change_read (fsm_t* this) { return change; }

static void money_add (fsm_t* this) { money += coin; coin = 0; }
static void change_return (fsm_t* this) { money = 0; change = 0; }

// Explicit FSM description
static fsm_trans_t purse[] = {
  { PURSE_WAITING, coin_intro,  PURSE_WAITING,  money_add},
  { PURSE_WAITING, change_read, PURSE_WAITING,  change_return },
  {-1, NULL, -1, NULL },
};

fsm_t* purse_fsm_new (void)
{
  return fsm_new (purse);
}
