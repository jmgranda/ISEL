#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include "interp.h"

enum purse_state {
  PURSE_WAITING,
};

int money = 0;   // Tendrá que ser externa para cofm.c y main.c
static int coin = 0;    // Tendrá que ser externa para cofm.c y main.c
int change = 0;  // Tendrá que ser externa para cofm.c y main.c

static int coin_intro (fsm_t* this) { return coin; }
static int change_read (fsm_t* this) { return change; }

static void money_add (fsm_t* this) 
{ 
  money += coin; 
  DEBUG (printf ("money = %d\n", money));
  coin = 0; 
}
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

static int cmd_coin (char *arg)
{
	coin = atoi (arg);
	return 0;
}

static int cmd_money (char *arg)
{
	printf ("Your credit is: %d\n", money);
	return 0;
}

void purse_setup (void)
{
	interp_addcmd ("coin", cmd_coin, "Insert coin of given value");
	interp_addcmd ("money", cmd_money, "This is your credit");
}



