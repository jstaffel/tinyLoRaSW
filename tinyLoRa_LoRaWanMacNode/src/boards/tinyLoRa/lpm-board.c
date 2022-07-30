/*!
 * \file      lpm-board.c
 *
 * \brief     Target board low power modes management
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 */

#include "utilities.h"
#include "lpm-board.h"



/* Low power mode support is not yet available on tinyLoRa
 * all the implementation below are still for an other microcontroller from this project:
 * https://github.com/ErichStyger/mcuoneclipse/tree/master/Examples/MCUXpresso/LPC55S16-EVK/LPC55S16_LoRaWanMacNode
 */



static uint32_t StopModeDisable = 0;
static uint32_t OffModeDisable = 0;

void LpmSetOffMode( LpmId_t id, LpmSetMode_t mode )
{
    #if 0
    CRITICAL_SECTION_BEGIN( );

    switch( mode )
    {
        case LPM_DISABLE:
        {
            OffModeDisable |= ( uint32_t )id;
            break;
        }
        case LPM_ENABLE:
        {
            OffModeDisable &= ~( uint32_t )id;
            break;
        }
        default:
        {
            break;
        }
    }

    CRITICAL_SECTION_END( );
    return;
    #endif
}

void LpmSetStopMode( LpmId_t id, LpmSetMode_t mode )
{
    #if 0
    CRITICAL_SECTION_BEGIN( );

    switch( mode )
    {
        case LPM_DISABLE:
        {
            StopModeDisable |= ( uint32_t )id;
            break;
        }
        case LPM_ENABLE:
        {
            StopModeDisable &= ~( uint32_t )id;
            break;
        }
        default:
        {
            break;
        }
    }

    CRITICAL_SECTION_END( );
    return;
    #endif
}

void LpmEnterLowPower( void )
{
    #if 0
    if( StopModeDisable != 0 )
    {
        /*!
        * SLEEP mode is required
        */
        LpmEnterSleepMode( );
        LpmExitSleepMode( );
    }
    else
    { 
        if( OffModeDisable != 0 )
        {
            /*!
            * STOP mode is required
            */
            LpmEnterStopMode( );
            LpmExitStopMode( );
        }
        else
        {
            /*!
            * OFF mode is required
            */
            LpmEnterOffMode( );
            LpmExitOffMode( );
        }
    }
    return;
    #endif
}

LpmGetMode_t LpmGetMode(void)
{
    #if 0
    LpmGetMode_t mode;

    CRITICAL_SECTION_BEGIN( );

    if( StopModeDisable != 0 )
    {
        mode = LPM_SLEEP_MODE;
    }
    else
    {
        if( OffModeDisable != 0 )
        {
            mode = LPM_STOP_MODE;
        }
        else
        {
            mode = LPM_OFF_MODE;
        }
    }

    CRITICAL_SECTION_END( );
    return mode;
    #endif
}

__attribute__((weak)) void LpmEnterSleepMode( void )
{
}

__attribute__((weak)) void LpmExitSleepMode( void )
{
}

__attribute__((weak)) void LpmEnterStopMode( void )
{
}

__attribute__((weak)) void LpmExitStopMode( void )
{
}

__attribute__((weak)) void LpmEnterOffMode( void )
{
}

__attribute__((weak)) void LpmExitOffMode( void )
{
}
