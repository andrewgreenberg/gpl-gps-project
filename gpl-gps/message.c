// message.c Navigation Message Processing
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include "message.h"
#include "constants.h"
#include "ephemeris.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "time.h"
#include "tracking.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

#define PREAMBLE  (0x8b << (30-8)) // preamble equals 0x8b, but it's
                                  // located in the MSBits of a 30 bit word.

// GPS parity bit-vectors
// The last 6 bits of a 30bit GPS word are parity check bits.
// Each parity bit is computed from the XOR of a selection of bits from the
// 1st 24 bits of the current GPS word, and the last 2 bits of the _previous_
// GPS word.
// These parity bit-vectors are used to select which message bits will be used
// for computing each of the 6 parity check bits.
// We assume the two bits from the previous message (b29, b30) and the 24 bits
// from the current message, are packed into a 32bit word in this order:
//   < b29, b30, b1, b2, b3, ... b23, b24, X, X, X, X, X, X > (X = don't care)
// Example: if PBn = 0x40000080,
// The parity check bit "n" would be computed from the expression (b30 XOR b23).
#define PB1     0xbb1f3480
#define PB2     0x5d8f9a40
#define PB3     0xaec7cd00
#define PB4     0x5763e680
#define PB5     0x6bb1f340
#define PB6     0x8b7a89c0

// Define how many words can have bad parity in a row before we axe the sync
#define MAX_BAD_PARITY  5

/******************************************************************************
 * Globals
 ******************************************************************************/

// Right now we're declaring a message structure per channel, since the
// messages come out of locked channels.. but you could argue they should be
// in a per satellite array.
message_t  messages[N_CHANNELS];
cyg_flag_t  message_flag;


/******************************************************************************
 * Statics
 ******************************************************************************/

// None

/******************************************************************************
 * Count the number of bits set in the input and return (1) if this count is
 * odd (0) otherwise.
 * This is used in the navigation message parity check algorithm.
 *
 * Note, on the ARM there is probably a more efficient way to do this.
 ******************************************************************************/
static int
parity( unsigned long word)
{
    int count = 0;

    while( word)
    {
        if( (long)word < 0) count++;
        word <<= 1; // Want to go this way because we typically ignore some
                   // LSBits
    }
    return( count & 1);
}


/******************************************************************************
 * Return 1 if and only if input 30bit word passes GPS parity check, otherwise
 * return 0.
 *
 * The input word is expected to be 32bits, with the 30bit word right justified.
 * The two most significant bits (b30 and b31) should contain the last two bits
 * of the _previous_ GPS word.
 ******************************************************************************/
static int
ParityCheck( unsigned long word)
{
    return( !(
                (word & 0x3f) ^ // Last 6 bits are the message parity bits
               ((parity( word & PB1) << 5) | (parity( word & PB2) << 4) |
                (parity( word & PB3) << 3) | (parity( word & PB4) << 2) |
                (parity( word & PB5) << 1) |  parity( word & PB6))
             )
          );
}

/******************************************************************************
 * New satellite in a channel; clear the message. For now that means just
 * clearing the valid flags (and the current subframe for display purposes)
 *****************************************************************************/
void
clear_messages( unsigned short ch)
{
    unsigned short i;

    messages[ch].frame_sync = 0;
    messages[ch].subframe = 0;
    messages[ch].set_ch_time = 0;
    for( i = 0; i < 5; ++i)
        messages[ch].sf[i].valid = 0;
}


/******************************************************************************
 * Load bits into wordbuf0 and wordbuf1. Flip the incoming bit if we're sync'd
 * onto a subframe and the bits are inverted.
 *
 * Note, see coments about weird 2+ 30 bit pattern below in the words below.
 *****************************************************************************/
static void
store_bit( unsigned short ch, unsigned short bit)
{
    // If we're synced on the current message and the data is inverted,
    // flip the incoming bit.
    if( messages[ch].frame_sync && messages[ch].data_inverted)
        bit ^= 1;

    // GPS NAV messages come MSBit 1st, so the most recent bit is the LSBit.
    messages[ch].wordbuf0 = (messages[ch].wordbuf0 << 1) | bit;

    // NAV words are 30 bits long and the parity check requires the upper
    // two bits to be the least two bits of the previous word. Note that we
    // use wordbuf1 to look for the preamble (TLM and HOW at the same time)
    if( messages[ch].wordbuf0 & (1 << 30))
        messages[ch].wordbuf1 = (messages[ch].wordbuf1 << 1) | 1;
    else
        messages[ch].wordbuf1 = (messages[ch].wordbuf1 << 1);
}

/******************************************************************************
 * Take the message's buffer of 2 + 30 bits (2 from the previous word) and
 * store it in the subframe's word as 24 bits of data after completing a parity
 * check.
 *
 *****************************************************************************/
static void
store_word( unsigned long ch)
{
    static unsigned short bad_parity_count;

    // If bit 30 is set then flip bits 29 - 6 as per GPS spec.
    if( messages[ch].wordbuf0  & (1 << 30))
        messages[ch].wordbuf0 ^= (((1 << 24) - 1) << 6);

    if( ParityCheck( messages[ch].wordbuf0))
    {
        // Store 24 bits, masking the 6 parity bits and the 2 upper bits.
        // We may want to convert this to pointers for efficiency?
        messages[ch].sf[messages[ch].subframe].word[messages[ch].wordcount]
                = (messages[ch].wordbuf0 >> 6) & ((1 << 24) - 1);
        // Mark it as valid
        messages[ch].sf[messages[ch].subframe].valid
                |= (1 << messages[ch].wordcount);

        // Clear the count of bad parity words
        bad_parity_count = 0;
    }

    // If the parity check fails for more than a few words, our frame sync
    // might have been corrupted for whatever reason.
    else
    {
        bad_parity_count++;
        if( bad_parity_count > MAX_BAD_PARITY)
        {
            messages[ch].frame_sync = 0;
            bad_parity_count = 0;
        }
    }
}


/*******************************************************************************
 * This function finds the preamble, TLM and HOW in the navigation message and
 * synchronizes to the nav message.
*******************************************************************************/
static void
look_for_preamble( unsigned short ch)
{
    unsigned long   TLM, HOW;           // TLeMetry, HandOffWord
    unsigned short  current_sf;
    unsigned short  previous_sf;
    unsigned short  data_inverted;

    // Note: Bits are stored in wordbuf0/1 in store_bits()

    /* Save local copies of the wordbuf's for local checks of TLM and HOW */
    TLM = messages[ch].wordbuf1;
    HOW = messages[ch].wordbuf0;

    /* Test for inverted data. Bit 0 and 1 of HOW must be zero. */
    if( HOW & 1)        // Test for inverted data
    {
        TLM = ~TLM;
        HOW = ~HOW;

        data_inverted = 1;
    }
    else
        data_inverted = 0;

    // Flip bits 29 - 6 if the previous word's LSB is 1
    if( TLM  & (1 << 30))
        TLM ^= (((1 << 24) - 1) << 6);
    if( HOW  & (1 << 30))
        HOW ^= (((1 << 24) - 1) << 6);

    if( (TLM & (0xff << (30 - 8))) != PREAMBLE) // Test for 8-bit preamble
        return;

    current_sf = (int)((HOW >> 8) & 7);             // Subframe ID

    if( (current_sf < 1) || (current_sf > 5)) // subframe range test
        return;

    if( HOW & 3) // Confirm zeros
        return;

    // Preliminary checks passed, now final check of parity
    if( !ParityCheck( TLM))
        return;

    if( !ParityCheck( HOW))
        return;

    // Hooray! We found a valid preamble and a sane HOW word, so for now
    // we'll assume we're synced. We won't really know until we get the next
    // subframe and check that the TOW has incremented by exactly 1.
    messages[ch].frame_sync = 1;

    // Hand off the current satellite to the message structure
    messages[ch].prn = CH[ch].prn;

    // Record the current subframe number (from zero)
    messages[ch].subframe = --current_sf;

    // Flag whether the bits are inverted, and if so invert wordbuf0 so we
    // don't lose the next incoming word.
    if( data_inverted)
    {
        messages[ch].data_inverted = 1;
        messages[ch].wordbuf0 = ~messages[ch].wordbuf0;
    }
    else
        messages[ch].data_inverted = 0;

    messages[ch].sf[current_sf].word[0] = TLM;
    messages[ch].sf[current_sf].word[1] = HOW;

    // We've just stored two words into the current subframe
    messages[ch].wordcount = 2;
    // Flag Words 0 and 1 as valid words
    messages[ch].sf[current_sf].valid = 3;

    // Extract and store the TOW from the HOW so we can easily compare it to
    // future TOWs to verify our frame sync. (TOW == bits 1-17 of 30 bit word)
    // Maybe this should be a macro. Could be done faster if we assumed 32bits.
    messages[ch].sf[current_sf].TOW = (HOW >> (30 - 17))
                                                   & ((1 << 17) - 1);

    if( current_sf > 0)
        previous_sf = current_sf - 1;
    else
        previous_sf = 4;

    // Even if the previous subframe had valid TLM and HOW words, kill both the
    // current and previous subframes if their TOW's are not incrementally
    // different.
    if( messages[ch].sf[previous_sf].valid & 3)
    {
        if( messages[ch].sf[current_sf].TOW
            != (messages[ch].sf[previous_sf].TOW + 1))
        {
            // We're not actually synced. Kill everything and start
            // the sync search over again.
            clear_messages( ch);
            return;
        }

        // We confirmed that we have sync since the TOW's are incremental. And
        // now that we have a valid TLM/HOW, we know the actual time of week.
        // We don't want to keep syncing the time in bits or keep setting the
        // epoch counters, so do this only once, after we confirm sync for
        // this channel. We'll do this again only if we clear the message.
        if( !messages[ch].set_ch_time)
        {
            // Don't run this code again until we've lost frame sync.
            messages[ch].set_ch_time = 1;

            // Update the gps "time_in_bits". Given that we know the current bit
            // is the last bit of the HOW word (the 60th bit of the subframe),
            // we can calculate the gps time in bit-counts (1/50 seconds). The
            // TOW in the HOW is actually the time at the start of the NEXT
            // subframe (see the ICD for the clearest documentation on this).
            // Note, time_in_bits is incremented in the tracking.c lock()
            // function, so TODO guarantee that this is set before tracking()
            // increments it, or compensate if it does.
            if( messages[ch].sf[current_sf].TOW)
                CH[ch].time_in_bits =
                    messages[ch].sf[current_sf].TOW * 300 - 240;
            // The TOW can be zero so handle this case.
            else
                CH[ch].time_in_bits = BITS_IN_WEEK - 240;

            // Update the gps time in seconds (the receiver's main clock) if we
            // haven't already.
            set_time_with_tow( messages[ch].sf[current_sf].TOW);

            // Finally, flag the tracking loop that the next bit (20ms epoch)
            // is the beginning of a new word. TODO Like above, guarantee
            // that this happens before the next bit arrives from the
            // tracking loops.
            CH[ch].sync_20ms_epoch_count = 1;
        }

        // Do a sanity check on the time_in_bits; it should always agree with
        // the TOW bits. Handles the case where TOW is zero at the start of
        // a GPS week. TODO Like above, guarantee this happens before the next
        // bit arrives from the tracking loops.
        else
        {
            if( messages[ch].sf[current_sf].TOW)
            {
                if( CH[ch].time_in_bits !=
                     (messages[ch].sf[current_sf].TOW * 300 - 240))
                    diag_printf( "MESSAGE.C: TOW HAS BECOME UNSYNCHRONIZED.");
            }
            else
                if( CH[ch].time_in_bits != (BITS_IN_WEEK - 240))
                    diag_printf( "MESSAGE.C: TOW HAS BECOME UNSYNCHRONIZED.");
        }
    }
}

/******************************************************************************
 * Stuff incoming bits from the tracking interrupt into words and subframes in
 * the messages structure.
 ******************************************************************************/
void
message_thread( CYG_ADDRWORD data) // input 'data' not used
{
    cyg_flag_value_t channels_with_bits;
    cyg_flag_value_t channels_with_subframes;
    unsigned short   ch;

    // There's no way that we're going to get a bit before this thread
    // is first executed, so it's ok to run the flag init here.
    cyg_flag_init( &message_flag);

    while(1)
    {
        // Sleep here until tracking() signals there are channels with new
        // navigation bits ready. Clear the message_flag and save the active
        // channels in channels_with_bits.
        channels_with_bits = cyg_flag_wait( &message_flag,
                             (1 << N_CHANNELS) - 1,   // 0xfff (any channel)
                             CYG_FLAG_WAITMODE_OR |   // any flag
                             CYG_FLAG_WAITMODE_CLR);  // clear flags

        // OK we're awake, process the messages

        setbits32( GPS4020_GPIO_WRITE, LED3); // DEBUG:

        // Clear the flag IPC shadow (see below)
        channels_with_subframes = 0;

        for( ch = 0; ch < N_CHANNELS; ch++)
        {
            if( channels_with_bits & (1 << ch))
            {
                // This channel has a bit to process:
                // store the bit in the word buffers
                store_bit( ch, CH[ch].bit);

                // If the channel isn't sync'd to the message frame,
                // look for the preamble
                if( !messages[ch].frame_sync)
                {
                    look_for_preamble( ch);

                    // If we just found sync, then reset the counters
                    if( messages[ch].frame_sync)
                    {
                        messages[ch].bitcount = 0;
                    }
                }
                // Frame is sync'd, so get bits and words.
                else /* Frame is sync'd */
                {
                    // If we have 30 bits, that's a word so store it
                    messages[ch].bitcount++;
                    if( messages[ch].bitcount >= 30)
                    {
                        messages[ch].bitcount = 0;

                        // Store the word in the subframes array
                        store_word( ch);

                        messages[ch].wordcount++;
                        if( messages[ch].wordcount >= 10)
                        {
                            // We've got 10 words so we have a subframe. Use
                            // frame_sync to restart the subframe parsing.
                            // Note that preamble will reset wordcount.
                            messages[ch].frame_sync = 0;

                            // we assume in the preamble that the bit stream
                            // hasn't been inverted when we check the TLM/HOW.
                            // so if the channel IS inverted, make it non-
                            // inverted as we check the TLM/HOW.
                            if( messages[ch].data_inverted)
                                messages[ch].wordbuf0 = ~messages[ch].wordbuf0;

                            // Only send along complete, error free subframes
                            if( messages[ch].sf[messages[ch].subframe].valid
                                 == 0x3ff) // 0x3ff === 10 good words
                            {
                                // Set the flags to wake up the ephemeris thread
                                cyg_flag_value_t which_subframe =
                                    (1 << messages[ch].subframe);
                                channels_with_subframes |= (1 << ch);

                                // Set the subframe flag so we know which
                                // subframe to process. We don't need a
                                // shadow register here because we're only
                                // going to set one subframe per channel
                                // at a time.
                                cyg_flag_setbits( &ephemeris_subframe_flags[ch],
                                                  which_subframe);
                            }
                        }
                    }
                }
            }
        }

        // Wake up the ephemeris thread if there are subframes ready
        if( channels_with_subframes)
            cyg_flag_setbits( &ephemeris_channel_flag, channels_with_subframes);

        clearbits32( GPS4020_GPIO_WRITE, LED3); // DEBUG:
    }
}
