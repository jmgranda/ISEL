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
#include "reactor.h"
#include "interp.h"

void cofm_setup (void);
fsm_t* cofm_fsm_new (void);

void purse_setup (void);
fsm_t* purse_fsm_new (void);

static
void
purse_func (struct event_handler_t* this)
{
  static const struct timeval period = { 0, 250*000 };
  static fsm_t* purse_fsm = NULL;

  if (!purse_fsm) 
	purse_fsm = purse_fsm_new ();

  fsm_fire (purse_fsm);
  
  timeval_add (&this->next_activation, &this->next_activation, &period);
}

static
void
cofm_func (struct event_handler_t* this)
{
  static const struct timeval period = { 0, 250*000 };
  static fsm_t* cofm_fsm = NULL;

  if (!cofm_fsm) 
	cofm_fsm = cofm_fsm_new ();

  fsm_fire (cofm_fsm);
  
  timeval_add (&this->next_activation, &this->next_activation, &period);
}

/* Coffee machine with credit check and give change activation */
void* main_reactor (void* arg)
{
  EventHandler purse_eh, cofm_eh;
  reactor_init ();

  event_handler_init (&purse_eh, 1, purse_func);  
  event_handler_init (&cofm_eh, 1, cofm_func);
  reactor_add_handler (&purse_eh);
  reactor_add_handler (&cofm_eh);

  while (1) {
    reactor_handle_events ();
  }

  return NULL;
}

int main ()
{
  wiringPiSetup();

  purse_setup ();
  cofm_setup ();
  task_new ("reactor", main_reactor, 0, 0, 1, 1024);
  interp_run ();
  return 0;
}



