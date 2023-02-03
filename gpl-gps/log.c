// log.c gpl-gps logging output
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "constants.h"
#include "display.h"
#include "ephemeris.h"
#include "position.h"
#include "pseudorange.h"
#include "serial.h"
#include "switches.h"
#include "time.h"


/******************************************************************************
 * Log the ephemeris from the satellites as it comes in. Called only when
 * ephemeris is processed, which is either right after a new lock or after
 * the IODE/IODC is updated.
 ******************************************************************************/

#ifdef ENABLE_EPHEMERIS_LOG

void
log_ephemeris2( unsigned short ch)
{
    char string[200];
    gpstime_t now;

    now = get_time();
    
    sprintf( string,
             "EPH,%d,%.9f,",
             now.weeks,
             now.seconds);
    uart2_send_string( string);

    sprintf( string,
             "%d,%d,%d,%d,%x,%x,%x,",
             ephemeris[ch].prn,
             ch,
             ephemeris[ch].valid,
             ephemeris[ch].ura,
             ephemeris[ch].health,
             ephemeris[ch].iodc,
             ephemeris[ch].iode);
    uart2_send_string( string);
             
    sprintf( string,
             "%.10e,%.10e,%.10e,%.10e,%.10e,",
             ephemeris[ch].tgd,
             ephemeris[ch].toc,
             ephemeris[ch].af2,
             ephemeris[ch].af1,
             ephemeris[ch].af0);
    uart2_send_string( string);

    sprintf( string,
             "%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,",
             ephemeris[ch].crs,
             ephemeris[ch].dn,
             ephemeris[ch].ma,
             ephemeris[ch].cuc,
             ephemeris[ch].ecc,
             ephemeris[ch].cus,
             ephemeris[ch].sqrtA,
             ephemeris[ch].toe);
    uart2_send_string( string);

    sprintf( string,
             "%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n\r",
             ephemeris[ch].cic,
             ephemeris[ch].w0,
             ephemeris[ch].cis,
             ephemeris[ch].inc0,
             ephemeris[ch].crc,
             ephemeris[ch].w,
             ephemeris[ch].omegadot,
             ephemeris[ch].idot);
    uart2_send_string( string);
}

void
log_ephemeris( void)
{
    unsigned short ch;

    if( display_command == DISPLAY_LOG)
    {
        // print out ephemeris messages

        for(ch = 0; ch < N_CHANNELS; ch++)
        {
            if( log_eph[ch] != 0)
            {
                log_ephemeris2( ch);
                log_eph[ch] = 0;
            }
        }
    }
}
#endif // ENABLE_EPHEMERIS_LOG



/******************************************************************************
 * Print out a one line log.
 *
 * NOTE: Called blindly from position.c, and only should be called when there's
 *       a new position update.
 ******************************************************************************/

#ifdef ENABLE_POSITION_LOG
void
log_position( void)
{
    char      string[200];
    double    x,y,z;
    unsigned short i;
    unsigned short ch;

    if( display_command == DISPLAY_LOG)
    {
        // log the time
        sprintf( string,
                "POS,%d,%.9f,",
                pr2_time.weeks,
                pr2_time.seconds);
        uart2_send_string( string);

        // temporary debug - subtract reference location
        x = receiver_pos.x - HERE_X;
        y = receiver_pos.y - HERE_Y;
        z = receiver_pos.z - HERE_Z;

        // log the ECEF position
        sprintf( string,
                "%d,%d,%d,% 7.1f,% 7.1f,% 7.1f,%e,%e,",
                receiver_pos.positioning,
                receiver_pos.n,
                receiver_pos.valid,
                x,
                y,
                z,
                receiver_pos.b,
               receiver_pos.error);
        uart2_send_string( string);

        // log the pseudoranges and sat positions used
        for( i = 0; i < receiver_pos.n; i++)
        {
            sprintf( string,
                    "%d,%.1f,%.1f,%.1f,%.1f,%e,",
                    sat_position[i].prn,
                    m_rho[i],
                    sat_position[i].x,
                    sat_position[i].y,
                    sat_position[i].z,
                    sat_position[i].tb);
            uart2_send_string( string);

            ch = sat_position[i].channel;

            sprintf( string,
                     "%ld,%d,%d,%d,%d,%.10f,%.1f,",
                     pr2[ch].bit_time,
                     pr2[ch].epoch_20ms,
                     pr2[ch].epoch_1ms,
                     pr2[ch].code_phase,
                     pr2[ch].code_dco_phase,
                     pr2[ch].sat_time,
                     pr2[ch].range);
            uart2_send_string( string);
        }

        sprintf( string,"\n\r");
        uart2_send_string( string);
    }
}
#endif // ENABLE_POSITION_LOG


/******************************************************************************
 * Print a one line, comma delimited output of all the pseudoranges
 *
 * NOTE: Called from pseudorange.c on a each new pseudorange (TIC).
 * WARNING: May require a larger stack size (e.g., 2048) for pseudorange thread
 ******************************************************************************/

#ifdef ENABLE_PSEUDORANGE_LOG
void
log_pseudoranges( void)
{
    char string[120];
    unsigned short ch;

    if( display_command == DISPLAY_LOG)
    {
        sprintf( string,
                 "%.9f,",
                 pr_time.seconds);
        uart2_send_string( string);

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( pr[ch].valid)
            {
                sprintf( string,
                         "%d,%d,%ld,%d,%d,%.1f,",
                         ch,
                         pr[ch].prn,
                         pr[ch].bit_time,
                         pr[ch].epoch_20ms,
                         pr[ch].epoch_1ms,
                         pr[ch].range);
                uart2_send_string( string);
            }
            else
            {
                sprintf( string,
                         "%d,,,,,,",
                         ch);
                uart2_send_string( string);
            }
        }
        sprintf( string,"\n\r");
        uart2_send_string( string);
    }
}

#endif // ENABLE_PSEUDORANGE_LOG
