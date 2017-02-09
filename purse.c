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
#include "task.h"
#include "interp.h"

enum purse_state {
  PURSE_WAITING,
};

static int money = 0;   // Tendrá que ser externa para cofm.c y main.c

int money_get (void) { return money; }
int money_add (int value) { return money += value; }

static int change = 0;  // Tendrá que ser externa para cofm.c y main.c
void change_set (void) { change = 1; }

static int coin = 0;   

static int coin_intro (fsm_t* this) { return coin; }
static int change_read (fsm_t* this) { return change; }

static void money_add_coin (fsm_t* this) 
{ 
  money_add (coin); 
  coin = 0; 
}
static void change_return (fsm_t* this) { money = 0; change = 0; }

// Explicit FSM description
static fsm_trans_t purse[] = {
  { PURSE_WAITING, coin_intro,  PURSE_WAITING,  money_add_coin},
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
	printf ("%d\n", money_get());
	return 0;
}

static int cmd_cancel (char *arg)
{
	change_set ();
	return 0;
}

void purse_setup (int prioceiling)
{
	mutex_init (&m_money, prioceiling);
	interp_addcmd ("coin", cmd_coin, "Insert coin of given value");
	interp_addcmd ("money", cmd_money, "This is your credit");
	interp_addcmd ("cancel", cmd_cancel, "Cancel and return credit");
}



