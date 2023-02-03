// ephemeris.c gpl-gps Satellite navigation message to ephemeris processing
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <math.h>
#include "constants.h"
#include "ephemeris.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "message.h"
#include "switches.h"
#include "time.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

// c_2pX == 2^+X, c_2mX == 2^-X
#define c_2p4     16.0
#define c_2m5     0.03125
#define c_2m11    4.8828125e-4
#define c_2m19    1.9073486328125e-6
#define c_2m20    9.5367431640625e-7
#define c_2m21    4.76837158203125e-7
#define c_2m23    1.19209289550781e-7   // NaN
#define c_2m24    5.96046447753906e-8   // Nan
#define c_2m27    7.45058059692383e-9
#define c_2m29    1.86264514923096e-9
#define c_2m30    9.31322574615479e-10
#define c_2m31    4.65661287307739e-10  // NaN
#define c_2m33    1.16415321826935E-10
#define c_2m38    3.63797880709171e-12  // NaN
#define c_2m43    1.13686837721616e-13  // NaN
#define c_2m50    8.881784197e-16       // NaN
#define c_2m55    2.77555756156289e-17  // NaN

// Maximum allowable user range allowable is 8 TODO how man m is this? see BB.
#define MAX_URA 8

/******************************************************************************
 * Globals
 ******************************************************************************/

// Right now we're declaring a message structure per channel, since the
// messages come out of locked channels_ready.. but you could argue they should
// be in a per-satellite array.
ephemeris_t  ephemeris[N_CHANNELS];

// Use an array to communicate which subframes we just received to the log
#ifdef ENABLE_EPHEMERIS_LOG
unsigned char log_eph[N_CHANNELS];
#endif

cyg_flag_t  ephemeris_channel_flag;
cyg_flag_t  ephemeris_subframe_flags[N_CHANNELS];

/******************************************************************************
 * Statics
 ******************************************************************************/

static msg_copy_t local_msg[N_CHANNELS];        // local copy of the messages.

/******************************************************************************
 * If the channel is reallocated, then clear the ephemeris data for that
 * channel. Called from allocate.c
 ******************************************************************************/
void
clear_ephemeris( unsigned short ch)
{
    // Clear the ephemeris structure
    ephemeris[ch].valid = 0;

    // Clear local copies of the nav message
    local_msg[ch].have_sf = 0;
}


/******************************************************************************
 * Convert subframe bits to ephemeris values.
 *
 * Note that subframes aren't passed up from the message_thread unless they're
 * already considered valid (all parity checks have passed and been removed)
 ******************************************************************************/
static void
process_subframe1( unsigned short ch)
{
    signed   long temp;
    unsigned long utemp;

    // Map the messages structure to OSGPS's "sf" and make this whole thing
    // a bit more readable. TODO remove this eventually.
    sf_copy_t * sf = local_msg[ch].sf;

    // Update the time with the week number (only done once, further calls
    // just return).
    utemp = (sf[1-1].word[3-1] >> 14);
    set_time_with_weeks( (unsigned short)utemp);

    // Get the rest of the ephemerides
    utemp = sf[1-1].word[8-1] & 0xffff;
    ephemeris[ch].toc = (double)utemp * c_2p4;

    // The following variables are signed integers so if the MSB is set,
    // sign extend it to 32bits. Yes, this sucks but the if makes it
    // faster than another multiply, even by -1.
    // TODO try (-((1<<n) - x)) for (x) of (n)bits.
    temp = sf[1-1].word[7-1] & 0xff;
    if( temp & (1 << 7))
        temp |= ~0xff;
    ephemeris[ch].tgd = (double)temp * c_2m31;

    temp = sf[1-1].word[9-1] >> 16;
    if( temp & (1 << 7))
        temp |= ~0xff;
        ephemeris[ch].af2 = (double)temp * c_2m55;

    temp = sf[1-1].word[9-1] & 0xffff;
    if( temp & (1 << 15))
        temp |= ~0xFFFF;
        ephemeris[ch].af1 = (double)temp * c_2m43;

    temp = sf[1-1].word[10-1] >> 2;
    if( temp & (1 << 21))
        temp |= ~0x3FFFFF;
    ephemeris[ch].af0 = (double)temp * c_2m31;
}


static void
process_subframe2( unsigned short ch)
{
    unsigned long   ultemp;
    long            temp;

    // Map the messages structure to OSGPS's "sf" and make this whole thing
    // a bit more readable. TODO remove this eventually.
    sf_copy_t * sf = local_msg[ch].sf;

    temp = sf[2-1].word[3-1] & 0xFFFF;
    if( temp & (1 << 15))
        temp |= ~0xFFFF;
    ephemeris[ch].crs = (double)temp * c_2m5;

    temp = sf[2-1].word[4-1] >> 8;
    if( temp & (1 << 15))
        temp |= ~0xFFFF;
    ephemeris[ch].dn = (double)temp * (c_2m43 * PI);

    temp = ((sf[2-1].word[4-1] & 0xFF) << 24) | sf[2-1].word[5-1];
    ephemeris[ch].ma = (double)temp * (c_2m31 * PI);

    temp = sf[2-1].word[6-1] >> 8;
    if( temp & (1 << 15))
        temp |= ~0xFFFF;
    ephemeris[ch].cuc = (double)temp * c_2m29;

    temp = ((sf[2-1].word[6-1] & 0xFF) << 24) | sf[2-1].word[7-1];
    ephemeris[ch].ecc = (double)temp * c_2m33;

    temp = sf[2-1].word[8-1] >> 8;
    if( temp & (1 << 15))
        temp |= ~0xFFFF;
    ephemeris[ch].cus = (double)temp * c_2m29;

    ultemp = (((sf[2-1].word[8-1] & 0xFF) << 24) | sf[2-1].word[9-1]);
    ephemeris[ch].sqrtA = (double)ultemp * c_2m19;

    ultemp = (sf[2-1].word[10-1] >> 8);
    ephemeris[ch].toe = (double)ultemp * c_2p4;
}


static void
process_subframe3( unsigned short ch)
{
    long temp;

    // Map the messages structure to OSGPS's "sf" and make this whole thing
    // a bit more readable. TODO remove this eventually.
    sf_copy_t * sf = local_msg[ch].sf;

    // Some of these data words are signed; check their sign bit and extend
    // as appropriate

    temp = sf[3-1].word[3-1] >> 8;
    if( temp & (1 << 15))
        temp |=  ~0xFFFF;
    ephemeris[ch].cic = (double)temp * c_2m29;

    temp = ((sf[3-1].word[3-1] & 0xFF) << 24) | sf[3-1].word[4-1];
    ephemeris[ch].w0 = (double)temp * (c_2m31 * PI);

    temp = sf[3-1].word[5-1] >> 8;
    if( temp & (1 << 15))
        temp |=  ~0xFFFF;
    ephemeris[ch].cis = (double)temp * c_2m29;

    temp = ((sf[3-1].word[5-1] & 0xFF) << 24) | sf[3-1].word[6-1];
    ephemeris[ch].inc0 = (double)temp * (c_2m31 * PI);

    temp = sf[3-1].word[7-1] >> 8;
    if( temp & (1 << 15))
        temp |=  ~0xFFFF;
    ephemeris[ch].crc = (double)temp * c_2m5;

    temp = ((sf[3-1].word[7-1] & 0xFF) << 24) | sf[3-1].word[8-1];
    ephemeris[ch].w = (double)temp * (c_2m31 * PI);

    temp = sf[3-1].word[9-1];
    if( temp & (1 << 23))
        temp |=  ~0xFFFFFF;
    ephemeris[ch].omegadot = (double)temp * (c_2m43 * PI);

    temp = (sf[3-1].word[10-1] >> 2) & 0x3FFF;
    if( temp & (1 << 13))
        temp |=  ~0x3FFF;
    ephemeris[ch].idot = (double)temp * (c_2m43 * PI);
}

static void
process_subframe4( unsigned short ch)
{
    // Sure, you got subframe 4, why not?
}

static void
process_subframe5( unsigned short ch)
{
    // Sure, you got subframe 5, why not?
}


/******************************************************************************
 * According to the GPS documentation, the ephemerides from subframes 1,2,3
 * are to be used as a complete set. This function tests the navigation
 * messages to see if we can get a valid ephemeris from them.
 ******************************************************************************/
static void
update_ephemeris( unsigned short ch)
{
    unsigned short iodc;
    unsigned short iode_2;
    unsigned short iode_3;
    unsigned short ura;
    unsigned short health;

    // Calculate the `Issue Of Data' Clock (IODC), ephemeris (IODE), user range
    // error (URA), and satellite health.
    iodc =   (unsigned short)(((local_msg[ch].sf[0].word[2] & 0x3) << 8)
                | (local_msg[ch].sf[0].word[7] >> 16));
    ura =    (unsigned short)((local_msg[ch].sf[0].word[2] & 0xf00) >> 8);
    health = (unsigned short)((local_msg[ch].sf[0].word[2] & 0xfc) >> 2);
    iode_2 = (unsigned short)(local_msg[ch].sf[1].word[2] >> 16);
    iode_3 = (unsigned short)(local_msg[ch].sf[2].word[9] >> 16);

    if( (ura < MAX_URA) && !(health & (1 << 5)) )
    {
        // The LS 8 bits of the IODC and the two IODE's should match. Although
        // it doesn't explicitly say this anywhere, this seems to be the case
        // (ICD 20.3.4.4).
        if( ((iodc & 0xff) == iode_2) && (iode_2 == iode_3))
        {
            // Skiddadle if we have valid message data, we've already parsed
            // it, and nothing has changed.
            if( ephemeris[ch].valid && (ephemeris[ch].iode == iode_2))
                return;

            ephemeris[ch].prn = messages[ch].prn;
            ephemeris[ch].ura = ura;
            ephemeris[ch].health = health;
            ephemeris[ch].iodc = iodc;
            ephemeris[ch].iode = iode_2;

            process_subframe1( ch);
            process_subframe2( ch);
            process_subframe3( ch);
            ephemeris[ch].valid = 1;

            // If the switch is on, log the ephemeris to the serial port
#ifdef ENABLE_EPHEMERIS_LOG
            log_eph[ch] = 1;
#endif
        }
    }
    // If the MSB of the 6 bit health is set, the satellite's nav message is
    // toast - see (ICD 20.3.3.3.1.4). Bad satellite! We'll also call it bad if
    // the URA is ever huge. Eventually the allocation thread will notice all
    // of this from the almanac data and stop using this satellite.
    else
        clear_ephemeris( ch);
}



/******************************************************************************
 * Stuff incoming bits from the tracking interrupt into words and subframes in
 * the messages structure.
 ******************************************************************************/
void
ephemeris_thread( CYG_ADDRWORD data) // input 'data' not used
{
    cyg_flag_value_t channels_ready;
    cyg_flag_value_t subframes;
    unsigned short   ch;
    unsigned short   i;
    unsigned short   j;

    // There's no way that we're going to get a subframe before this thread
    // is first executed, so it's ok to run the flag inits here.
    cyg_flag_init( &ephemeris_channel_flag);

    for( ch = 0; ch < N_CHANNELS; ch++)
        cyg_flag_init( &ephemeris_subframe_flags[0]);

    while(1)
    {
        // Sleep here until the message thread signals there is at least one
        // subframe ready.  Clear the ephemeris_channel_flag(s) on wake up, but
        // save the value in channels_ready.
        channels_ready = cyg_flag_wait( &ephemeris_channel_flag,
                         (1 << N_CHANNELS) - 1,   // 0xfff (all channels)
                         CYG_FLAG_WAITMODE_OR |   // any flag
                         CYG_FLAG_WAITMODE_CLR);  // clear flags

        setbits32( GPS4020_GPIO_WRITE, LED5); // DEBUG

        // OK we're awake, process the messages
        for( ch = 0; ch < N_CHANNELS; ++ch)
        {
            if (channels_ready & (1 << ch))
            {
                // Find out which subframes are new (nonblocking)
                // TODO Don't clear bits here, do it at end.
                subframes = cyg_flag_poll( &ephemeris_subframe_flags[ch],
                                           0x01f, // 5 possible subframes
                                           CYG_FLAG_WAITMODE_OR
                                           | CYG_FLAG_WAITMODE_CLR);

                // Grab local copies of all incoming subframes
                for( i = 0; i < NUM_SUBFRAMES; ++i)
                {
                    if( subframes & (1 << i))
                    {
                        // If we have all 10 words, copy over the message
                        if( messages[ch].sf[i].valid == 0x3ff)
                        {
                            for( j = 0; j < NUM_WORDS; ++j)
                            {
                                local_msg[ch].sf[i].word[j] =
                                        messages[ch].sf[i].word[j];
                            }
                            local_msg[ch].have_sf |= (1 << i);
                        }
                    }
                }

                // If we have all of the necessary message subframes, parse
                // the messages for ephemeris. Subframe 4 and 5 are handled
                // seperately.
                if( (local_msg[ch].have_sf & 7) == 7)
                    update_ephemeris( ch);

                // TODO implement parsing of subframes 4
                if( local_msg[ch].have_sf & (1 << 3))
                    process_subframe4( ch);

                // TODO implement parsing of subframes 5
                if( local_msg[ch].have_sf & (1 << 4))
                   process_subframe5( ch);
            }
        }
        clearbits32( GPS4020_GPIO_WRITE, LED5); // DEBUG
    }
}


