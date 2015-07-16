/* SCHEDULER version 2.5 */

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

//#include "helper.h"
#include <msp430.h>


/*=====================================================================*/
/* 	SCHEDULER SECTION 	*/
/*=====================================================================*/

/* ------------------------ */
/* How to use the scheduler */
/* ------------------------ */
/*
Every app has an unique ID (see definitions) - the lower ID, the higher priority.

Scheduler routines are non-interruptible.

The status of all implemented apps is stored in global variable
  apps

Members:
  apps.active: currently pending apps
  apps.enabled: enabled/disabled apps

For app call handling, apps use proprietary global status variable, holding
caller identification flags. When calling an app, the appropriate call flag must
be set, so the app can identify the call. The status variable must be defined as
in the example below (variable sample_01_calls):

///////////////////////////////////////////////////////////////////
// DECLARATION (in file.h):
union union_sample_01
{
  unsigned int value;
  struct
  {
    unsigned int RTC		:1;
    unsigned int GSM		:1;
    unsigned int GPIO		:1;
    unsigned int DUMMY	:13;	//fill up to 16 bits - integer
  };
};

typedef union union_sample_01 sample_01_type;

extern sample_01_type sample_01_calls;
///////////////////////////////////////////////////////////////////
// INSTANTIATION (in file.c):

sample_01_type sample_01_calls;
///////////////////////////////////////////////////////////////////

The main scheduler routine is
  apps_poll()

It is used in the main infinite loop and performs priority scheduling - it outputs
currently pending active app with highest priority until all pending apps are
processed.

When finished, app is suspended with
  apps_suspend(_int_to_flag(app_ID))

Before suspending a (sub-)app, clear caller flags (triggers).
Suspending an app puts it in waiting mode (when task finished or waiting on sema/irq).
To suspend local sub-apps (app is suspended only when all sub-apps suspend, so
they can not block each other), use
  _subapp_suspend and _local_app_suspend

To use local sub-apps suspend, a local static variable of type "signed int" must
be used as status handler!!!

To enable apps use
  apps_enable(_int_to_flag(app_ID))

To activate/call an app, use
  apps_call(_int_to_flag(app_ID))

Before calling an app, set up the app trigger (similar for sub-apps).
*/
/* ------------------------ */

#define USE_SEMAPHORE	0

#define TASK_LOG	0


/* max flag slots in apps.active */
/* max allowed is 16 */
#define MAX_NR_APPS	16

/* define app priority (ID) here */
/* lowest nr - highest priority */
#define LCD_TEST_ID     	1
#define REFRESH_DIS_ID		2
#define MENU_ID		        3
#define REC_ID		        4
#define SET_GAIN_ID		5
#define WRITE_TO_FLASH_ID	6
#define AUX_TX_ID		7
#define READ_KEY_ID		8
#define tipkeDVE_ID		9
#define lediceToggle_ID		10

#define a6_ID                  11
#define a7_ID		      12
#define a8_ID		      13
#define a9_ID		      14
#define a10_ID		      15

#define a11_ID		      16

/* define app_ID here */
#define	NO_APP		0
#define LCD_TEST_APP    	(unsigned int)(1 << (LCD_TEST_ID - 1))
#define REFRESH_DIS_APP	        (unsigned int)(1 << (REFRESH_DIS_ID - 1))
#define MENU_APP	        (unsigned int)(1 << (MENU_ID - 1))
#define REC_APP	                (unsigned int)(1 << (REC_ID - 1))
#define SET_GAIN_APP		(unsigned int)(1 << (SET_GAIN_ID - 1))
#define WRITE_TO_FLASH_APP      (unsigned int)(1 << (WRITE_TO_FLASH_ID - 1))
#define AUX_TX_APP		(unsigned int)(1 << (AUX_TX_ID - 1))
#define READ_KEY_APP	        (unsigned int)(1 << (READ_KEY_ID - 1))
#define KONTROLA_APP            (unsigned int)(1 << (KONTROLA_ID - 1))
#define lediceToggle_APP        (unsigned int)(1 << (lediceToggle_ID - 1))
#define a6_APP		        (unsigned int)(1 << (a6_ID - 1))

#define a7_APP     (unsigned int)(1 << (a7_ID - 1))
#define a8_APP	  (unsigned int)(1 << (a8_ID - 1))
#define a9_APP	  (unsigned int)(1 << (a9_ID - 1))
#define a10_APP	  (unsigned int)(1 << (a10_ID - 1))
#define a11_APP	  (unsigned int)(1 << (a11_ID - 1))


/* apps definition */
struct struct_sched_apps
{
  unsigned int active;
//  unsigned int wait_mask;
  unsigned int enabled_mask;
};

typedef struct struct_sched_apps apps_type;

/* global extern for apps */
extern volatile apps_type apps;

/* uninterruptible functions - scheduler */
__monitor void scheduler_init(void);
__monitor unsigned int apps_poll(void);
//__monitor void apps_call(unsigned int app_ID);
//__monitor void apps_enable(unsigned int app_ID);
//__monitor void apps_disable(unsigned int app_ID);
//__monitor void apps_suspend(unsigned int app_ID);


#define scheduler_init()	apps.active = 0; apps.enabled_mask = 0


/*----------------------------------------------------------------------*/
/* Call an application */
/*----------------------------------------------------------------------*/
/* - set apps.active flag to active */
/*----------------------------------------------------------------------*/
//__monitor inline void apps_call(unsigned int app_ID)
//{
//  apps.active |= app_ID;
//}

#define apps_call(_APP)	apps.active |= _APP


/*----------------------------------------------------------------------*/
/* Enable apps */
/*----------------------------------------------------------------------*/
/* - set enable flag (settings) */
/* - warning: before use clear active flag!!! */
/*----------------------------------------------------------------------*/
//__monitor inline void apps_enable(unsigned int app_ID)
//{
//  apps.enabled_mask |= app_ID;
//}

#define apps_enable(_APP)	apps.enabled_mask |= _APP


/*----------------------------------------------------------------------*/
/* Disable apps */
/*----------------------------------------------------------------------*/
/* - reset enable flag (settings) */
/* - warning: before use clear active flag!!! */
/*----------------------------------------------------------------------*/
//__monitor inline void apps_disable(unsigned int app_ID)
//{
//  apps.enabled_mask &= ~app_ID;
//}

#define apps_disable(_APP)	apps.enabled_mask &= ~_APP


/*----------------------------------------------------------------------*/
/* Make app idle */
/*----------------------------------------------------------------------*/
/* - when finished, the polled application clears active function flag */
/*----------------------------------------------------------------------*/
//__monitor inline void apps_suspend(unsigned int app_ID)
//{
//  apps.active &= (~app_ID);
//}

#define apps_suspend(_APP)	__disable_interrupt(); apps.active &= (~_APP); __enable_interrupt()


/* Macros for suspending app with multiple independent sub-apps */
/*
  * app is suspended only when all sub-apps suspend!!!
  * individual sub-app is marked to be suspended with _subapp_suspend()
  * @ app exit apply _local_app_suspend() to suspend
  params:
	_loc: local integer var, initialized to 0
	_NR: number of sub-apps suspending
	_id: app ID to be suspended
*/

/* mark sub-app to be suspended */
#define _subapp_suspend(_loc, _sub)			_loc |= _left_to_flag(_sub)

/* call sub-app */
#define _subapp_call(_loc, _sub)			_loc &= ~(_left_to_flag(_sub))

/* suspend app with multiple sub-apps */
#define _local_app_suspend(_loc, _NR, _id)		if(_loc == _uleft_to_flag(_NR)) apps_suspend(_id)


/*=====================================================================*/
/* 	SEMAPHORE SECTION 	*/
/*=====================================================================*/

#if USE_SEMAPHORE == 1

/* ------------------------ */
/* How to use the semaphore */
/* ------------------------ */
/*
Semaphore declaration:

  sema_type my_sema;

Before accessing a shared resource, which is guarded by semaphore, call

  semaphore_take(&my_sema, app_ID);

This routine reserves semaphore for caller app (app_ID) - semaphore is available
for app_ID if app_ID is destination for guarded data (my_sema.dest == app_ID) or
no destination. If successful, returns caller app ID (0xFFFF if no source app),
otherwise app is put in a sema-waiting queue (my_sema.waiting) and returns 0
(user must suspend the app until reactivated by sema!!!).

When app finishes processing data guarded by semaphore, semaphore must be released:

  semaphore_release(&my_sema, app_ID, dest_ID);

The routine automatically reactivates only destination app (dest_ID), others are
kept in the waiting queue (my_sema.waiting).
To release for all apps, use semaphore_release(&my_sema, 0, 0). All sema-waiting
apps are automatically reactivated.

Tips:
Individual sub-apps can be reactivated with semaphore_release: set destination
to parent app (my_sema.src) of sub-apps and fake the producer (my_sema.src) by
using an app number not in use in parent app (or other sub-apps). In sub-app
take the semaphore and perform logic test to identify if targeted; if not, release
back the semaphore manualy to original state.

Semaphore can also be used for interdependent apps/sub-apps synchronization.
*/
/* ------------------------ */

struct struct_sema
{
  unsigned int waiting;
  union union_sema
  {
    unsigned int value;
    struct
    {
      unsigned char src;
      unsigned char dest;
    };
  };
};

typedef struct struct_sema sema_type;

/* uninterruptible functions - semaphore */
__monitor unsigned int semaphore_take(sema_type *p_sema, unsigned int call_app_ID);
__monitor void semaphore_release(sema_type *p_sema, unsigned int call_app_ID, unsigned int call_dest_ID);

#endif /* USE_SEMAPHORE */

/*=====================================================================*/
/* 	TASK LOG SECTION 	*/
/*=====================================================================*/
#if TASK_LOG == 1

#define TASK_LOG_SIZE		10

extern volatile unsigned int Rtbr, Ttbr, Ptbr, Otbr, Ltbr, Btbr, Gtbr, Atbr, Itbr, Stbr, Wtbr, UAtbr, UGtbr, Tst_ON;

struct struct_task_log
{
  unsigned int index;
  unsigned int id;
  unsigned int start;
  unsigned int stop;
};

typedef struct struct_task_log task_log_type;

extern volatile task_log_type task_log[TASK_LOG_SIZE];

extern volatile unsigned int task_counter;
extern volatile unsigned int log_i;

#endif



#endif /*__SCHEDULER_H__*/
