// display.c gpl-gps display output
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "display.h"
#include "constants.h"
#include "ephemeris.h"
#include "message.h"
#include "position.h"
#include "pseudorange.h"
#include "serial.h"
#include "switches.h"
#include "threads.h"
#include "time.h"
#include "tracking.h"


unsigned short display_command = DISPLAY_POSITION;
static unsigned short ephemeris_mode;


/******************************************************************************
 * Send out the VT100 series clear-screen command
 ******************************************************************************/
static void
clear_screen( void)
{
    char clear_screen[] = "\033[2J";

    uart2_send_string( clear_screen);
}

/******************************************************************************
 * Display position/clock info
 ******************************************************************************/
static void
display_position( void)
{
#ifdef ENABLE_POSITION_DISPLAY

    static unsigned short  was_positioning;

    char string[120];
    char header[] = "\
Ch: PN C PrV EpV Pseudorange    Elev. Azim.\n\r";
// Ch: PN C PrV EpV U pseudorange   El. Az. ----x---- ----y---- ----z----\n\r";

    time_t          std_time;
    gpstime_t       gps_time;
    unsigned short  clock_state;
    double          lat, lon;
    double          az, el;

    unsigned short  ch;
    unsigned short  channel_state;
//     char            in_use;

    gps_time = get_time();
    std_time = get_standard_time();
    clock_state = get_clock_state();

    lat = receiver_llh.lat * RADTODEG;
    lon = receiver_llh.lon * RADTODEG;

    // Print the Clock/Time info first
    sprintf( string,
             "\033[HTime = %d/%d/%d %d:%d:%2.3f (state:%d)\033[K\n\r\n\r",
             std_time.years,
             std_time.months,
             std_time.days,
             std_time.hours,
             std_time.minutes,
             std_time.seconds,
             clock_state);
    uart2_send_string( string);

    // Print the ECEF position info (even if it hasn't been set yet!)
    sprintf( string,
             "dECEF = (X:% 8.1f Y:% 8.1f Z:% 8.1f) tb:%1.3e\033[K\n\r\n\r",
             receiver_pos.x - HERE_X,
             receiver_pos.y - HERE_Y,
             receiver_pos.z - HERE_Z,
             receiver_pos.b);
    uart2_send_string( string);

    // Print the LLH position info (even if it hasn't been set yet!)
    sprintf( string,
             "LLH  = (Lat:%2.5f Lon:%2.5f Hgt:%6.2f)\033[K\n\r\n\r",
             lat,
             lon,
             receiver_llh.hgt);
    uart2_send_string( string);

    // Print out some position.c info
    sprintf( string, "\
State: positioning = %d, last position valid = %d, pr_rdy = %d\n\r\n\r",
             receiver_pos.positioning,
             receiver_pos.valid,
             pr2_data_fresh);
    uart2_send_string( string);

    // beep the bell if we just got busy in position.
    if( receiver_pos.positioning)
    {
        if( !was_positioning)
        {
            sprintf( string, "\007"); // bell code
            uart2_send_string( string);
            was_positioning = 1;
        }
    }
    else
        was_positioning = 0;

    // Now put out a summary of what the hell is going on in the receiver
    uart2_send_string( header);

    // Send out data on all 12 channels if there's no error
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        switch( CH[ch].state)
        {
            case CHANNEL_ACQUISITION:
                channel_state = 'A';
                break;
            case CHANNEL_CONFIRM:
                channel_state = 'C';
                break;
            case CHANNEL_PULL_IN:
                channel_state = 'P';
                break;
            case CHANNEL_LOCK:
                channel_state = 'L';
                break;
            default:
                channel_state = '-';
                break;
        }

        if( (pr[ch].valid) && ephemeris[ch].valid)
        {
            az = sat_azel[ch].az * RADTODEG;
            el = sat_azel[ch].el * RADTODEG;

            sprintf( string,
//                      "%2d: %2d %c  %d   %d  % e %3.f %3.f %.3e %.3e %.3e\033[K\n\r",
                     "%2d: %2d %c  %d   %d  % e  %5.1f %5.1f\033[K\n\r",
                     ch,
                     CH[ch].prn,
                     channel_state,
                     pr[ch].valid,
                     ephemeris[ch].valid,
                     pr[ch].range,
                     el,
                     az);
//                      sat_pos_by_ch[ch].x,
//                      sat_pos_by_ch[ch].y,
//                      sat_pos_by_ch[ch].z);
            uart2_send_string( string);
        }
        else
        {
            sprintf( string,
                     "%2d: %2d %c  %d   %d\033[K\n\r",
                     ch,
                     CH[ch].prn,
                     channel_state,
                     pr[ch].valid,
                     ephemeris[ch].valid);
            uart2_send_string( string);
        }
    }
#endif // POSITION_DISPLAY
}


/******************************************************************************
 * Display pseudorange info
 ******************************************************************************/
static void
display_pseudorange( void)
{
#ifdef ENABLE_PSEUDORANGE_DISPLAY

    unsigned short ch;
    unsigned char channel_state;
    char header[] =
            "\033[HCH: PN S bit%50 eb ems Pseudorange Average\n\r";
    char string[80];

    uart2_send_string( header);

    // Send out data on all 12 channels if there's no error
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        switch( CH[ch].state)
        {
            case CHANNEL_ACQUISITION:
                channel_state = 'A';
                break;
            case CHANNEL_CONFIRM:
                channel_state = 'C';
                break;
            case CHANNEL_PULL_IN:
                channel_state = 'P';
                break;
            case CHANNEL_LOCK:
                channel_state = 'L';
                break;
            default:
                channel_state = '-';
                break;
        }

        if( pr[ch].valid)
        {
            sprintf( string,
                    "%2d: %2d %c %6ld %2d %2d  %e %5ld\033[K\n\r",
                    ch,
                    pr[ch].prn,
                    channel_state,
                    pr[ch].bit_time,
                    pr[ch].epoch_20ms,
                    pr[ch].epoch_1ms,
                    pr[ch].range,
                    CH[ch].avg);
            uart2_send_string( string);
        }
        else
        {
            sprintf( string,
                    "%2d: %2d %c\033[K\n\r",
                    ch,
                    pr[ch].prn,
                    channel_state);
            uart2_send_string( string);
        }

    }
#endif // PSEUDORANGE_DISPLAY
}


/******************************************************************************
 * Display ephemeris_thread info
 ******************************************************************************/
static void
display_ephemeris( void)
{
#ifdef ENABLE_EPHEMERIS_DISPLAY

    unsigned short ch;
    char string[120];
    char header0[] = "\033[H\
CH: PN V UR HE IODC  ------tgd------ ------toc------\033[K\n\r";
    char header1[] ="\033[H\
CH: ------af2------ ------af1------ ------af0------\033[K\n\r";
    char header2[] ="\033[H\
CH: IODE ------Crs------ ------dn------- ------Mo------- ------Cuc------\033[K\n\r";
    char header3[] = "\033[H\
CH: ------e-------- ------Cus------ ------sqA------ --toe--\033[K\n\r";
    char header4[] = "\033[H\
CH: ------cic------ ------w0------- ------cis------ ------inc0-----\033[K\n\r";
    char header5[] = "\033[H\
CH: ------crc------ ------w-------- ---omegadot---- ------idot-----\033[K\n\r";

    if( ephemeris_mode == 0)
    {
        uart2_send_string( header0);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( ephemeris[ch].valid)
            {
                sprintf( string,
                         "%2d: %2d %d %2d %2x %3x % 15e % 15e\033[K\n\r",
                         ch,
                         ephemeris[ch].prn,
                         ephemeris[ch].valid,
                         ephemeris[ch].ura,
                         ephemeris[ch].health,
                         ephemeris[ch].iodc,
                         ephemeris[ch].tgd,
                         ephemeris[ch].toc);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string,
                         "%2d: %2d %d %2d %2x %3x\033[K\n\r",
                         ch,
                         ephemeris[ch].prn,
                         ephemeris[ch].valid,
                         ephemeris[ch].ura,
                         ephemeris[ch].health,
                        ephemeris[ch].iodc);
                uart2_send_string( string);
            }
        }
    }
    else if( ephemeris_mode == 1)
    {
        uart2_send_string( header1);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( ephemeris[ch].valid)
            {
                sprintf( string,
                         "%2d: % 15e % 15e % 15e\033[K\n\r",
                         ch,
                         ephemeris[ch].af2,
                         ephemeris[ch].af1,
                         ephemeris[ch].af0);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string, "%2d:\033[K\n\r", ch);
                uart2_send_string( string);
            }
        }
    }
    else if( ephemeris_mode == 2)
    {
        uart2_send_string( header2);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( ephemeris[ch].valid)
            {
                sprintf( string,
                         "%2d: %3x  % 15e % 15e % 15e % 15e\033[K\n\r",
                         ch,
                         ephemeris[ch].iode,
                         ephemeris[ch].crs,
                         ephemeris[ch].dn,
                         ephemeris[ch].ma,
                         ephemeris[ch].cuc);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string, "%2d:\033[K\n\r", ch);
                uart2_send_string( string);
            }
        }
    }
    else if( ephemeris_mode == 3)
    {
        uart2_send_string( header3);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( ephemeris[ch].valid)
            {
                sprintf( string,
                         "%2d: % 15e % 15e % 15e %6.1f\033[K\n\r",
                         ch,
                         ephemeris[ch].ecc,
                         ephemeris[ch].cus,
                         ephemeris[ch].sqrtA,
                         ephemeris[ch].toe);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string, "%2d:\033[K\n\r", ch);
                uart2_send_string( string);
            }
        }
    }
    else if( ephemeris_mode == 4)
    {
        uart2_send_string( header4);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( ephemeris[ch].valid)
            {
                sprintf( string,
                         "%2d: % 15e % 15e % 15e % 15e\033[K\n\r",
                         ch,
                         ephemeris[ch].cic,
                         ephemeris[ch].w0,
                         ephemeris[ch].cis,
                         ephemeris[ch].inc0);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string, "%2d:\033[K\n\r", ch);
                uart2_send_string( string);
            }
        }
    }
    else if( ephemeris_mode == 5)
    {
        uart2_send_string( header5);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( ephemeris[ch].valid)
            {
                sprintf( string,
                         "%2d: % 15e % 15e % 15e % 15e\033[K\n\r",
                         ch,
                         ephemeris[ch].crc,
                         ephemeris[ch].w,
                         ephemeris[ch].omegadot,
                         ephemeris[ch].idot);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string, "%2d:\033[K\n\r", ch);
                uart2_send_string( string);
            }
        }
    }
    else
        ephemeris_mode = 0;

#endif // EPHEMERIS_DISPLAY
}


/******************************************************************************
 * Display tracking_thread info
 ******************************************************************************/
static void
display_tracking( void)
{
#ifdef ENABLE_TRACKING_DISPLAY

    char header[] =
        "\033[HCh: PN Mis Frq DelCarFreq Iprmt Qprmt RSSIQ State Avg\n\r";
    char string[80];

    unsigned short ch;
    unsigned short channel_state;
    unsigned short channel_bitsync;
    unsigned short channel_framesync;

    // display header line
    uart2_send_string( header);

    // Send out data on all 12 channels if there's no error
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        switch( CH[ch].state)
        {
            case CHANNEL_ACQUISITION:
                channel_state = 'A';
                break;
            case CHANNEL_CONFIRM:
                channel_state = 'C';
                break;
            case CHANNEL_PULL_IN:
                channel_state = 'P';
                break;
            case CHANNEL_LOCK:
                channel_state = 'L';
                break;
            default:
                channel_state = '-';
                break;
        }

        // Is the channel bit sync'ed?
        if( CH[ch].bit_sync == 1)
            channel_bitsync = 'B';
        else
            channel_bitsync = '-';

        // Is the channel frame sync'ed?
        if( messages[ch].frame_sync == 1)
            channel_framesync = 'F';
        else
            channel_framesync = '-';

        sprintf( string,
             "%2d: %2d %3d %3d %10ld %5d %5d %5ld %c(%c%c) %5ld\n\r",
             ch,
             CH[ch].prn,
             CH[ch].missed,
             CH[ch].n_freq,
             CH[ch].carrier_freq - CARRIER_REF,
             CH[ch].i_prompt,
             CH[ch].q_prompt,
             CH[ch].prompt_mag,
             channel_state,
             channel_bitsync,
             channel_framesync,
             CH[ch].avg);

        uart2_send_string( string);
    }
#endif // TRACKING_DISPLAY
}




/******************************************************************************
 * Display debug info
 ******************************************************************************/
static void
display_debug( void)
{
#ifdef ENABLE_DEBUG_DISPLAY

    char string[80];

    sprintf(string, "\033[HAccumulator int.   = %4d\033[K\n\r\n\r",
            cyg_thread_measure_stack_usage( accum_int_handle));
    uart2_send_string( string);

    sprintf(string, "Allocate thread    = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( allocate_thread_handle));
    uart2_send_string( string);

    sprintf(string, "Display thread     = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( display_thread_handle));
    uart2_send_string( string);

    sprintf(string, "Ephemeris thread   = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( ephemeris_thread_handle));
    uart2_send_string( string);

    sprintf(string, "Measure thread     = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( measure_thread_handle));
    uart2_send_string( string);

    sprintf(string, "Message thread     = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( message_thread_handle));
    uart2_send_string( string);

    sprintf(string, "Position thread    = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( position_thread_handle));
    uart2_send_string( string);

    sprintf(string, "Pseudorange thread = %4d\033[K\n\r",
            cyg_thread_measure_stack_usage( pseudorange_thread_handle));
    uart2_send_string( string);

#endif // DEBUG_DISPLAY
}


/******************************************************************************
 * Display message_thread info
 ******************************************************************************/
static void
display_messages( void)
{
#ifdef ENABLE_MESSAGE_DISPLAY

    char header[] = "\033[H\
Ch: PN Mi TOW    SF SF1V SF2V SF3V SF4V SF5V State Avg\n\r";
    char string[80];

    unsigned short ch;
    unsigned short channel_state;
    unsigned short channel_bitsync;
    unsigned short channel_framesync;
    unsigned short TOW;

    uart2_send_string( header);

    // Send out data on all 12 channels
    for( ch = 0; ch < N_CHANNELS; ch++)
    {
        switch( CH[ch].state)
        {
            case CHANNEL_ACQUISITION:
                channel_state = 'A';
                break;
            case CHANNEL_CONFIRM:
                channel_state = 'C';
                break;
            case CHANNEL_PULL_IN:
                channel_state = 'P';
                break;
            case CHANNEL_LOCK:
                channel_state = 'L';
                break;
            default:
                channel_state = '-';
                break;
        }

        // Is the channel bit sync'ed?
        if( CH[ch].bit_sync == 1)
            channel_bitsync = 'B';
        else
            channel_bitsync = '-';

        // Is the channel frame sync'ed?
        if( messages[ch].frame_sync == 1)
            channel_framesync = 'F';
        else
            channel_framesync = '-';

        // Find the TOW for subframe 1 if valid
        if( messages[ch].sf[0].valid)
            TOW = messages[ch].sf[0].TOW;
        else
            TOW = 0;

        sprintf( string, "\
%2d: %2d %2d %5d %2d %4lx %4lx %4lx %4lx %4lx %c(%c%c) %5ld\n\r",
                 ch,
                 CH[ch].prn,
                 CH[ch].missed_message_bit,
                 TOW,
                 messages[ch].subframe + 1,
                 messages[ch].sf[0].valid,
                 messages[ch].sf[1].valid,
                 messages[ch].sf[2].valid,
                 messages[ch].sf[3].valid,
                 messages[ch].sf[4].valid,
                 channel_state,
                 channel_bitsync,
                 channel_framesync,
                 CH[ch].avg);

        uart2_send_string( string);
    }
#endif // MESSAGE_DISPLAY
}


/******************************************************************************
 * Display pages of GPL-GPS info on /dev/ser2.
 ******************************************************************************/
void
display_thread( CYG_ADDRWORD data) // input 'data' not used
{
    unsigned short  current_display = DISPLAY_NOT_USED; // force clear screen
    cyg_bool got_byte;
    char c;

    uart2_initialize();

    while (1)
    {
        // Delay for 1 second
        cyg_thread_delay( 100);

        // setbits32( GPS4020_GPIO_WRITE, LED5); // DEBUG

        // First, check for an input command

        got_byte = uart2_get_char( &c);

        if (got_byte)
        {
            if (c == 't')
                display_command = DISPLAY_TRACKING;

            else if (c == 'm')
                display_command = DISPLAY_MESSAGES;

            else if (c == 's')
                display_command = DISPLAY_STOP;

            else if (c == 'e')
                if (display_command == DISPLAY_EPHEMERIS)
                    ephemeris_mode++;
                else
                    display_command = DISPLAY_EPHEMERIS;

            else if (c == 'r')
                display_command = DISPLAY_PSEUDORANGE;

            else if (c == 'p')
                display_command = DISPLAY_POSITION;

            else if (c == 'd')
                display_command = DISPLAY_DEBUG;

            else if (c == 'l')
                display_command = DISPLAY_LOG;
        }

        // Second, output to the screen UNLESS we've stopped the display or
        // we're logging (in which case position.c will call display_log()
        // directly
        if( (display_command != DISPLAY_STOP)
             && (display_command != DISPLAY_LOG))
        {
            // Clear the screen if we switched displays
            if( current_display != display_command)
            {
                current_display = display_command;
                clear_screen();
            }

            // Choose the page to display based on user input from the
            // input_thread
            if( display_command == DISPLAY_TRACKING)
                display_tracking();

            else if( display_command == DISPLAY_MESSAGES)
                display_messages();

            else if( display_command == DISPLAY_EPHEMERIS)
                display_ephemeris();

            else if( display_command == DISPLAY_PSEUDORANGE)
                display_pseudorange();

            else if( display_command == DISPLAY_POSITION)
                display_position();

            else if( display_command == DISPLAY_DEBUG)
                display_debug();

            else
                diag_printf("INPUT ERROR: bad display_command.\n\r");
        }
        // clearbits32( GPS4020_GPIO_WRITE, LED5); // DEBUG
    }
}
