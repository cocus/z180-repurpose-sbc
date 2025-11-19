/*
 * Copyright (C) 2025 Phillip Stevens  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * Note: This code comes from z88dk, which supports RomWBW, YAZ180, etc.
 * This file has been modified to make the port tailor my custom Z180.
 */

#include <stdlib.h>

#include "include/FreeRTOS.h"
#include "include/task.h"

#include <z180.h>
#include "z180_internal.h"

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
/* Make unitialised in BSS (to ensure above 0x8000) */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/*
 * Macros to set up, restart (reload), and stop the PRTx Timer used for
 * the System Tick.
 */
#if (configTICK_TIMER == 0)
#define TIMER_TIE TCR_TIE0
#define TIMER_TDE TCR_TDE0
#define TIMER_RLDRL "_RLDR0L"
#define TIMER_RLDRH "_RLDR0H"
#define TIMER_TMDRL "_TMDR0L"
#define TIMER_IVT_OFF 4
#else
#define TIMER_TIE TCR_TIE1
#define TIMER_TDE TCR_TDE1
#define TIMER_RLDRL "_RLDR1L"
#define TIMER_RLDRH "_RLDR1H"
#define TIMER_TMDRL "_TMDR1L"
#define TIMER_IVT_OFF 6
#endif

/*
 * Set up the timer reload count, and unmask the selected timer's interrupts.
 */
#define configSETUP_TIMER_RATE_AND_INT()                                        \
    do{                                                                         \
        __asm__(                                                                \
            "; we do configTICK_RATE_HZ ticks per second                    \n" \
            "ld hl,#"string(__CPU_CLOCK)"/"string(configTICK_RATE_HZ)"/20-1 \n" \
            "out0("TIMER_RLDRL"),l                                          \n" \
            "out0("TIMER_RLDRH"),h                                          \n" \
            "in0 a,(_TCR)                                                   \n" \
            "or #"string(TIMER_TIE)"|#"string(TIMER_TDE)"                   \n" \
            "out0 (_TCR),a                                                  \n" \
            );                                                                  \
    }while(0)

/*
 * If the IVT is in RAM, compute the appropriate offset for the timer vector
 * by reading the I and the IL registers, and set the vector value to the
 * "timer_isr" function. Note that "I" can only be read through "ld a, i",
 * while IL is part of the ICR.
 */
#if (configISR_IVT_IN_RAM == 1)
#define configSETUP_TIMER_INTERRUPT()           \
    do{                                         \
        __asm__(                                \
            "ld de,#_timer_isr              \n" \
            "ld a, i                        \n" \
            "ld h, a                        \n" \
            "in0 a,(_IL)                    \n" \
            "and a, #0xf0                   \n" \
            "or a, #"string(TIMER_IVT_OFF)" \n" \
            "ld l, a                        \n" \
            "ld (hl),e                      \n" \
            "inc hl                         \n" \
            "ld (hl),d                      \n" \
            );                                  \
    }while(0);                                  \
    configSETUP_TIMER_RATE_AND_INT()
#else
/*
 * However, if the IVT is NOT in RAM (i.e. it's in ROM), there's no need to
 * update any vector tables. This requires that the vector table in ROM contains
 * an entry for the "timer_isr" at the appropriate position, depeding on the
 * desired timer (see configTICK_TIMER).
 */
#define configSETUP_TIMER_INTERRUPT configSETUP_TIMER_RATE_AND_INT
#endif

#define configRESET_TIMER_INTERRUPT()                           \
    do{                                                         \
        __asm__(                                                \
            "in0 a,(_TCR)                                   \n" \
            "in0 a,("TIMER_TMDRL")                          \n" \
            );                                                  \
    }while(0)

#define configSTOP_TIMER_INTERRUPT()                            \
    do{                                                         \
        __asm__(                                                \
            "; disable down counting and interrupts for PRTx\n" \
            "in0 a,(_TCR)                                   \n" \
            "xor #"string(TIMER_TIE)"|#"string(TIMER_TDE)"  \n" \
            "out0 (_TCR),a                                  \n" \
            );                                                  \
    }while(0)


/*-----------------------------------------------------------*/

/*
 * Perform hardware setup to enable ticks from Timer.
 */
static void prvSetupTimerInterrupt( void ) __preserves_regs(iyh,iyl);

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    /* Place the parameter on the stack in the expected location. */
    *pxTopOfStack-- = ( StackType_t ) pvParameters;

    /* Place the task return address on stack. Not used */
    *pxTopOfStack-- = ( StackType_t ) 0;

    /* The start of the task code will be popped off the stack last, so place
    it on first. */
    *pxTopOfStack-- = ( StackType_t ) pxCode;

    /* Now the registers. */
    *pxTopOfStack-- = ( StackType_t ) 0xAFAF;   /* AF  */
    *pxTopOfStack-- = ( StackType_t ) 0x0404;   /* IF  */
    *pxTopOfStack-- = ( StackType_t ) 0xBCBC;   /* BC  */
    *pxTopOfStack-- = ( StackType_t ) 0xDEDE;   /* DE  */
    *pxTopOfStack-- = ( StackType_t ) 0xEFEF;   /* HL  */
    *pxTopOfStack-- = ( StackType_t ) 0xFAFA;   /* AF' */
    *pxTopOfStack-- = ( StackType_t ) 0xCBCB;   /* BC' */
    *pxTopOfStack-- = ( StackType_t ) 0xEDED;   /* DE' */
    *pxTopOfStack-- = ( StackType_t ) 0xFEFE;   /* HL' */
    *pxTopOfStack-- = ( StackType_t ) 0xCEFA;   /* IX  */
    *pxTopOfStack   = ( StackType_t ) 0xADDE;   /* IY  */

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void ) __preserves_regs(a,b,c,d,e,iyh,iyl) __naked
{
    /* Setup the relevant timer hardware to generate the tick. */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    /* Should not get here. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void ) __preserves_regs(b,c,d,e,h,l,iyh,iyl)
{
    /*
     * It is unlikely that the Z80 port will get stopped.
     * If required simply disable the tick interrupt here.
     */
    configSTOP_TIMER_INTERRUPT();
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute. This is called by the application, so we don't have
 * to check which bank is loaded.
 */
void vPortYield( void ) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch callable from ISRs. The first thing we do is save
 * the registers so we can use a naked attribute.
 */
void vPortYieldFromISR(void)  __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
    portSAVE_CONTEXT_IN_ISR();
    vTaskSwitchContext();
    portRESTORE_CONTEXT_IN_ISR();
}
/*-----------------------------------------------------------*/

/*
 * Initialize Timer (PRT1 for YAZ180, and SCZ180 HBIOS).
 */
void prvSetupTimerInterrupt( void ) __preserves_regs(iyh,iyl)
{
    configSETUP_TIMER_INTERRUPT();
}
/*-----------------------------------------------------------*/

void timer_isr(void) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
#if configUSE_PREEMPTION == 1
    /*
     * Tick ISR for preemptive scheduler.  We can use a naked attribute as
     * the context is saved at the start of timer_isr().  The tick
     * count is incremented after the context is saved.
     *
     * Context switch function used by the tick.  This must be identical to
     * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
     * difference from vPortYield() is the tick count is incremented as the
     * call comes from the tick ISR.
     */
    portSAVE_CONTEXT_IN_ISR();
    configRESET_TIMER_INTERRUPT();
    xTaskIncrementTick();
    vTaskSwitchContext();
    portRESTORE_CONTEXT_IN_ISR();
#else
    /*
     * Tick ISR for the cooperative scheduler.  All this does is increment the
     * tick count.  We don't need to switch context, this can only be done by
     * manual calls to taskYIELD();
     */
    portSAVE_CONTEXT_IN_ISR();
    configRESET_TIMER_INTERRUPT();
    xTaskIncrementTick();
    portRESTORE_CONTEXT_IN_ISR();
#endif
} // configUSE_PREEMPTION
