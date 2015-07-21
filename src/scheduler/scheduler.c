/* SCHEDULER version 2.5 */

#include "scheduler.h"
#include "intrinsics.h"
#include  "msp430x24x.h"
//#include "msp430x54x.h"

///*=====================================================================*/
///* 	TASK LOG SECTION 	*/
///*=====================================================================*/
//#if TASK_LOG == 1
//
//volatile task_log_type task_log[TASK_LOG_SIZE];
//volatile unsigned int task_counter;
//volatile unsigned int log_i;
//volatile unsigned int Rtbr, Ttbr, Ptbr, Otbr, Ltbr, Btbr, Gtbr, Atbr, Itbr, Stbr, Wtbr, UAtbr, UGtbr, Tst_ON;

//#pragma inline
//void capture_log(unsigned int task)
//{
//  task_counter++;
//  log_i++;
//
//  if (log_i > TASK_LOG_SIZE-1)
//    log_i = 0;
//
//  task_log[log_i].index = task_counter;
//  task_log[log_i].id = task;
//  task_log[log_i].start = TBR;
//  task_log[log_i].stop = 0;
//}
//
//#endif

/*=====================================================================*/
/* 	SCHEDULER SECTION 	*/
/*=====================================================================*/

__no_init volatile apps_type apps;


/*----------------------------------------------------------------------*/
/* Init scheduler */
/*----------------------------------------------------------------------*/
/* - reset/re-init apps */
/*----------------------------------------------------------------------*/

//__monitor void scheduler_init(void)
//{
//  apps.active = 0;
//  apps.enabled_mask = 0;
//}


/*----------------------------------------------------------------------*/
/* Main scheduler routine - POLLING */
/*----------------------------------------------------------------------*/
/* Returns ID of a highest priority running application */
/* Priority scheduling - highest priority app gets polled until cleared */
/*----------------------------------------------------------------------*/
__monitor unsigned int apps_poll(void)
{
  unsigned int t_apps;



  /* return if no apps pending */
  t_apps = apps.active;
  t_apps &= apps.enabled_mask;
  if (t_apps == 0)
  {



//    __disable_interrupt();
    //__bic_SR_register_on_exit(__SR_GIE);
    return 0;
  }


  /* priority scheduling - get active app_ID */
  unsigned int t_active_flags;// = t_apps;
  int i = 0;

  do {
    /* return if loop overflows */
    if (i > MAX_NR_APPS)
      return 0;
    /* find highest priority active flag - first LSB */
    t_active_flags = t_apps & (1 << (i++));
  } while(!t_active_flags);

#if TASK_LOG == 1
  capture_log(t_active_flags);
#endif

  return t_active_flags;
}



//
///*=====================================================================*/
///* 	SEMAPHORE SECTION 	*/
///*=====================================================================*/
//
//#if USE_SEMAPHORE == 1
//
///*----------------------------------------------------------------------*/
///* Take the semaphore */
///*----------------------------------------------------------------------*/
//__monitor unsigned int semaphore_take(sema_type *p_sema, unsigned int call_app_ID)
//{
//  unsigned int t_app_ID = call_app_ID;
//  unsigned int t_src;
//
//  /* if successful */
//  if (p_sema->value == 0 || p_sema->dest == t_app_ID)
//  {
//    /* clear app from semaphore waiting queue */
//    p_sema->waiting &= ~(_int_to_flag(t_app_ID));
//
//    /* set src/dest */
//    t_src = (unsigned int) p_sema->src;
//    p_sema->src = (unsigned char) t_app_ID;
//    p_sema->dest = 0;
//
//    if (t_src > 0)
//      return t_src;
//    else
//      return 0xffff;
//  }
//
//  /* if unsuccessful: set app in semaphore waiting queue */
//  p_sema->waiting |= _int_to_flag(t_app_ID);
//
//  return 0;
//}
//
///*----------------------------------------------------------------------*/
///* Release the semaphore */
///*----------------------------------------------------------------------*/
//__monitor void semaphore_release(sema_type *p_sema, unsigned int call_app_ID, unsigned int call_dest_ID)
//{
//  unsigned int t_app_ID = call_app_ID;
//  unsigned int t_dest_ID = call_dest_ID;
//
//  /* release if not already released */
//  if( t_dest_ID > 0 )
//  {
//    /* copy src/dest */
//    p_sema->src = t_app_ID;
//    p_sema->dest = t_dest_ID;
//  }
//  /* if already released reactivate all sema-waiting apps */
//  else
//  {
//    /* clear just in case */
//    p_sema->value = 0;
//
//    /* reactivate apps and clear waiting resgister */
//    apps_call(p_sema->waiting);
//    p_sema->waiting = 0;
//  }
//}
//
//#endif /* USE_SEMAPHORE */




