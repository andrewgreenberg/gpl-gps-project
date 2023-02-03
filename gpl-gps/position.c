// position.c Process pseudoranges and ephemeris into position
// Copyright (C) 2005  Andrew Greenberg
// Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
// See the "COPYING" file distributed with this software for more information.

#include <cyg/kernel/kapi.h>
#include <math.h>
#include "position.h"
#include "constants.h"
#include "display.h"        // logging function
#include "ephemeris.h"
#include "gp4020.h"
#include "gp4020_io.h"
#include "log.h"
#include "pseudorange.h"
#include "switches.h"
#include "time.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

// WGS-84 defines
#define OMEGA_E     7.2921151467E-5  // Earth rotation rate [r/s]
#define WGS84_A         6378137.0    // Earth semimajor axis [m]
#define WGS84_B         6356752.3142 // Earth semiminor axis [m]

// Other defines

#define SQRTMU      19964981.8432    // [(m^3/2)/s] sqrt(G * M(Earth))

#define F_RC -4.442807633E-10 // Relativistic correction: -2*sqrt(mu)/c^2
                              // (BB p133)

/******************************************************************************
 * Globals
 ******************************************************************************/

cyg_sem_t  position_semaphore;

unsigned short pr2_data_fresh;
// unsigned short positioning;

position_t  receiver_pos;
llh_t       receiver_llh;

azel_t  sat_azel[N_CHANNELS];
xyzt_t  sat_pos_by_ch[N_CHANNELS];

// temporarily globals for debugging

satpos_t sat_position[N_CHANNELS];
double   m_rho[N_CHANNELS];

/******************************************************************************
 * Statics (Module variables)
 ******************************************************************************/


//static xyzt_t   rec_pos_xyz;

/******************************************************************************
 * Signal from pseudorange thread that there aren't enough satellites to be
 * calculating position. Clear things as necessary.
 ******************************************************************************/
void
clear_position( void)
{
    receiver_pos.positioning = 0;
}

/******************************************************************************
 * Translate ECEF to LLH coordinates.
 *
 * Based on equations found in Hoffman-Wellinhoff
 ******************************************************************************/

static llh_t
ecef_to_llh( xyz_t pos)
{
    double p,n,thet,esq,epsq;
    double a2,b2,slat,clat,sth,sth3,cth,cth3; // Save some float math.
    llh_t result;

    a2 = WGS84_A * WGS84_A;
    b2 = WGS84_B * WGS84_B;

    p = sqrt( pos.x * pos.x + pos.y * pos.y);
    thet = atan( pos.z * WGS84_A / ( p * WGS84_B));
    esq = 1.0 - b2 / a2;
    epsq = a2 / b2 - 1.0;

    sth = sin( thet);
    cth = cos( thet);

    sth3 = sth * sth * sth;
    cth3 = cth * cth * cth;

    result.lat = atan( (pos.z + epsq * WGS84_B * sth3)
                        / (p - esq * WGS84_A * cth3));
    result.lon = atan2( pos.y, pos.x);
    clat = cos( result.lat);
    slat = sin( result.lat);
    n = a2 / sqrt( a2 * clat * clat + b2 * slat * slat);
    result.hgt = p / clat - n;

    return( result);
}


/******************************************************************************
 * calculate_position
 * Given satellite locations and pseudorange measurements, return an estimated
 * receiver position based purely on the geometry and Earth's rotation rate.
 *
 * The initial estimated position comes from the (rec_pos_xyz) structure, the
 * pseduorange measurements from (m_rho), measured range,
 *
 * If the position cannot be determined, the position returned will be
 * <000>, the center of the Earth.
 *****************************************************************************/
position_t
calculate_position( unsigned short nsats_used)
{
    position_t result;
    double x, y, z, b;  // estimated receiver position and clock bias
    double alpha;       // Angle of Earth rotation during signal transit
    //double sa,ca;       // Sin( alpha), Cos( alpha)
    double e_rho;       // Estimated Range

    unsigned short i, j, k; // indices

    // matrix declarations
    // (Is it smart to reserve N_CHANNELS when we need (nsats_used)?
    //  Since there's only one instance maybe these should be globals?)
    double delrho[N_CHANNELS]; // vector of per-sat. estimated range error
    double A[N_CHANNELS][3];   // linearized relation of position to pseudorange
    double inv[4][4];          // inverse of matrix (a[i,j] == Transpose[A].A)
    double delx[4];            // incremental position correction
    double  a00, a01, a02, a03,  // a[i,j], intermediate matrix for
                 a11, a12, a13,  // generalized inverse calculation.
                      a22, a23,  // Only 10 elements due to symmetry.
                           a33;  // Don't loop, so no indexing.
    double det; // matrix determinate

#if 0
    const char idx[4][4] = // For symmetric, indexed matrices such as inv[]
        {   {0,1,2,3},     //  we store only their upper triangle.
            {1,4,5,6},     // Normal indexed access is possible like so:
            {2,5,7,8},     //   a[i][j] => a[idx[i][j]]
            {3,6,8,9}
        }
    // This is not clearly better. It's probably faster to copy the 6 elements
    // over and save the indirect look-up in the inner loop
    // (about (16 * nsats_used) look-ups).
#endif

    double dx, dy, dz;          // vector from receiver to satellite (delx)
    xyz_t  satxyz[N_CHANNELS];  // sat position in ECEF at time of reception

    unsigned short nits = 0;    // Number of ITerationS

    double         error = 1000;        // residual calculation error
    unsigned short singular = 0;        // flag for matrix inversion
    result.valid = 0;

    b = 0.0;  // [m] local estimate of clock bias, initially zero
             // Note, clock bias is defined so that:
            //     measured_time == true_time + clock_bias

//    nsats_used  = 4;
    // make local copy of current position estimate (a speed optimization?)
    x = HERE_X;
    y = HERE_Y;
    z = HERE_Z;
//     x = rec_pos_xyz.x;
//     y = rec_pos_xyz.y;
//     z = rec_pos_xyz.z;

    for( i = 0; i < nsats_used; i++)
    {
        // The instantaneous origin of a satellite's transmission, though fixed
        // in inertial space, nevertheless varies with time in ECEF coordinates
        // due to Earth's rotation. Here we find the position of each satellite
        // in ECEF at the moment our receiver latched its signal.

        // The time-delay used here is based directly on the measured
        // time-difference. Perhaps some sort of filtering should be used.
        // If a variation of this calculation were included in the iteration,
        // more accuracy could be gained at some added computational expense.

        // Note that we may want to correct for the clock bias here, but
        // perhaps it's better if that's already folded into m_rho by
        // the time we get here.

        // alpha is the angular rotation of the earth since the signal left
        // satellite[i]. [radians]
        alpha = m_rho[i] * OMEGA_E / SPEED_OF_LIGHT;

        // Compute the change in satellite position during signal propagation.
        // Exact relations:
        //   sa = sin( alpha);
        //   ca = cos( alpha);
        //   dx = (sat_position[i].x * ca - sat_position[i].y * sa) - x;
        //   dy = (sat_position[i].y * ca + sat_position[i].x * sa) - y;
        //   dz = sat_position[i].z - z;
        // Make the small angle approximation
        //   cos( alpha) ~= 1 and sin( alpha) ~= alpha.
        satxyz[i].x = sat_position[i].x + sat_position[i].y * alpha;
        satxyz[i].y = sat_position[i].y - sat_position[i].x * alpha;
        satxyz[i].z = sat_position[i].z;
        // TODO, check the error in this approximation.
    }


/**************************************
 * Explanation of the iterative method:
 *
 * We make measurements of the time of flight of satellite signals to the
 * receiver (delta-t). From this we calculate a distance known as a pseudo-
 * range. The pseudorange differs from the true distance because of atmospheric
 * delay, receiver clock error, and other smaller effects. The error sources
 * other than clock error are dealt with elsewhere. The precise clock error can
 * only be found during this position calculation unless an external and very
 * accurate time source is used, or the carrier "integer ambiguity" can be
 * resolved. Since neither of the latter is usually the case, we solve for the
 * receiver "clock bias" as part of this position calculation. To be precise,
 * We define our pseudorange to be:
 *
 *   rho[i] == sqrt( (sx[i]-x)^2 + (sy[i]-y)^2 + (sz[i]-z)^2) + b
 *
 * Where (rho[i]) is the pseudorange to the ith satellite, (<sx,sy,sz>[i]) are
 * the ECEF (Earth Centered Earth Fixed) rectangular coordinates of the ith
 * satellite which are known from orbital calculations based on the local time
 * and ephemeris data, and (<xyz>) is the current estimate of the receiver
 * position in ECEF coordinates, (b)[meters] is the remaining clock-error
 * (bias) after performing all relevant corrections, and (^2) means to square
 * the quantity in parenthesis.
 *
 * By differentiation of (rho), a linear relation can be found between a change
 * in receiver position (delx), and the change in pseudorange (delrho).
 * Written as a matrix, this relation can be inverted and the receiver position
 * correction (delx) found.  Though approximate, the linearized process can
 * be iterated until a sufficiently accurate position is found.
 *
 * The linear relation in matrix form can be written:
 *
 *   delrho == A . delx
 *
 * Where (delrho) is a column vector representing the change in pseudoranges
 * resulting from a change in receiver position vector (delx), and (A) is
 * the matrix expressing the linear relation.
 *
 * To calculate a receiver position correction, first calculate the
 * pseudoranges based on the estimated receiver position and the known satellite
 * positions, call this (e_rho), and subtract this from the measured
 * pseudoranges:
 *
 *   delrho = m_rho - e_rho
 *
 * Knowing (delrho) and (A) the correction (delx) can be solved for, and
 * the corrected estimate of position can then be found:
 *
 *   (corrected-x) = (previous-x) + delx
 *
 * In the iterative method, (corrected-x) is used as the new value of the
 * estimated receiver position, and the whole correction process repeated.
 * Iteration is successful when the size of the correction is sufficiently small.
 *
 **************************************/

    do
    {   // Iterate until an accurate solution is found

        // Use some locals to avoid re-computing terms.
        // The names somewhat indicate the terms contained.
        double a01a33, a02a02, a12a23, a13a13a22,
            a03a03_a00a33, a02a12_a01a22, a02a13_a01a23, a12a13_a11a23;

        for( i = 0; i < nsats_used; i++)
        {
            // Working in the ECEF (Earth Centered Earth Fixed) frame, find the
            // difference between the position of satellite[i] and the
            // receiver's estimated position at the time the signal was
            // received.
            dx = x - satxyz[i].x;
            dy = y - satxyz[i].y;
            dz = z - satxyz[i].z;

            e_rho = sqrt( dx*dx + dy*dy + dz*dz); // estimated range

            // range error == ( (estimated range) - (measured range) ).
            delrho[i] = m_rho[i] - e_rho - b;

            // Now find the relation (delrho = A . delx). The full relation
            // given above is
            //   rho == sqrt((sx - x)^2 + (sy - y)^2 + (sz - z)^2) + b
            //
            // Taking the derivative gets
            //
            //   delrho = (( dx(x - sx) + dy(y -sy) + dz(z - sz) ) / rho) + b
            //
            // Using the basis <dx,dy,dz,b> a row of the matrix (A) referred to
            // previously is
            //   A[i] = <dx/e_rho, dy/e_rho, dz/e_rho, 1>
            //
            // Where [i] refers to the ith satellite, and the matrix (A) has
            // size (n_sats_used x 4). In practice we store only the first
            // three elements of each row, the last element being always (1).
            A[i][0] = dx / e_rho;
            A[i][1] = dy / e_rho;
            A[i][2] = dz / e_rho;
        }

        // Compute the generalized inverse (GI) of (A) in three steps:

        // First:
        //   a00 a01 a02 a03   Form the square matrix:
        //       a11 a12 a13     a[i,j] == Transpose[A].A
        //           a22 a23  (Since this matrix is symmetric,
        //               a33   compute only the upper triangle.)
        //
        // Initialize with values from the first satellite
        a03 = A[0][0];
        a13 = A[0][1];
        a23 = A[0][2];
        a33 = nsats_used; // This is actually the final value
        a00 = a03 * a03; a01 = a03 * a13; a02 = a03 * a23;
        a11 = a13 * a13; a12 = a13 * a23;
        a22 = a23 * a23;

        // fill in the data from the remaining satellites
        for( i=1; i < nsats_used; i++)
        {
            a00 += A[i][0] * A[i][0];
            a01 += A[i][0] * A[i][1];
            a02 += A[i][0] * A[i][2];
            a03 += A[i][0];
            a11 += A[i][1] * A[i][1];
            a12 += A[i][1] * A[i][2];
            a13 += A[i][1];
            a22 += A[i][2] * A[i][2];
            a23 += A[i][2];
        }

        // Second:
        // Solve for the inverse of the square matrix
        // The inverse is also symmetric, so again find only the upper triangle.
        //
        // The algorithm used is supposed to be efficient. It's rather hard
        // to derive, but fairly easy to show correct. It has been checked
        // and appears correct.
        // It uses some temporary variables, some of which could be re-used in
        // place it the optimizer is smart enough.

        inv[0][0]      = a12 * a13;
        a12a13_a11a23  = inv[0][0] - a11 * a23; // done
        inv[0][0]      = a23 * (inv[0][0] + a12a13_a11a23);
        inv[2][2]      = a01 * a03;
        inv[1][2]      = inv[2][2] - a00 * a13;
        inv[1][3]      = a22 * inv[1][2];
        inv[2][2]     += inv[1][2];
        inv[1][2]     *= -a23;
        inv[2][2]     *=  a13;
         det           = a01 * a12;
        a02a13_a01a23  = a02 * a13;
        inv[0][1]      = a03 * a12;
        inv[1][3]     += a02 * (a02a13_a01a23 - a03 * a12) -
                          a23 * (a01 * a02 - a00 * a12); // done
        a02a13_a01a23 -= a01 * a23; // done
        inv[0][1]     += a02a13_a01a23;
        inv[2][3]      = inv[0][1];
        inv[0][1]     *= -a23;
        inv[2][3]     *= -a01;
        inv[0][2]      = a02 * a11 - det;
         det           = a02 * (det - inv[0][2]);
        inv[3][3]      = a02 * a12;
        a02a12_a01a22  = - a01 * a22;
         det          += a01 * a02a12_a01a22;
         det          *= a33;
        a02a12_a01a22 += inv[3][3]; // done
        inv[3][3]     += a02a12_a01a22;
        inv[3][3]     *= a01;
        a13a13a22      = a13 * a22;
        inv[0][1]     += a03 * a13a13a22 + a33 * a02a12_a01a22;
        a12a23         = a12 * a23;
         det          -= (a03 + a03) * (a02 * a12a13_a11a23 -
                            a01 * (a13a13a22 - a12a23));
        a13a13a22     *= a13; // done
        a03a03_a00a33  = a03 * a03;
        inv[0][3]      = a12 * a12;
         det          -= a00 * (a13a13a22 - (a13 + a13) * a12a23 +
                          inv[0][3] * a33 + a11 * (a23 * a23 - a22 * a33));
        inv[0][3]     -= a11 * a22;
        inv[0][0]     -= a13a13a22 + a33 * inv[0][3]; // done
        inv[3][3]     -= a00 * inv[0][3];
         det          += a03a03_a00a33 * inv[0][3] +
                            a02a13_a01a23 * a02a13_a01a23; // done
        inv[0][3]     *= a03;
        inv[0][3]     += a23 * inv[0][2] - a13 * a02a12_a01a22; // done
        inv[0][2]     *= -a33;
        inv[0][2]     += a13 * a02a13_a01a23 - a03 * a12a13_a11a23; // done
        a03a03_a00a33 -= a00 * a33; // done
        inv[1][1]      = a02 * a03;
        inv[2][3]     += a11 * inv[1][1] + a00 * a12a13_a11a23; // done
        inv[1][1]     += inv[1][1];
        inv[1][1]      = a23 * (inv[1][1] - a00 * a23) -
                           a22 * a03a03_a00a33;
        a02a02         = a02 * a02;
        inv[1][1]     -= a33 * a02a02; // done
        inv[3][3]     -= a11 * a02a02; //done
        a01a33         = a01 * a33;
        inv[1][2]     += a12 * a03a03_a00a33 -
                           a02 * (a03 * a13 - a01a33); // done
        inv[2][2]     -= a11 * a03a03_a00a33 + a01 * a01a33; // done
        // 66 multiplies.

        if( det <= 0) // det ought not be less than zero because the matrix
                     // (Transpose[A].A) is the square of a real matrix,
                    // therefore its determinate is the square of a real number.
                   // If det is negative it means the numeric errors are large.
        {
            singular = 1;
            break;
        }
        else
        {
            // Third and final step, (generalized inverse) = (inv.Transpose[A])
            // go ahead and multiply (GI).delrho => (delx)
            // Note that (inv[]) as calculated is actually the matrix of
            // cofactors, so we divide by the determinate (det).

            // Copy over the symmetric elements if (inv[]) this should improve
            // the overall speed of the loop, we're guessing.
            inv[1][0] = inv[0][1];
            inv[2][0] = inv[0][2]; inv[2][1] = inv[1][2];
            inv[3][0] = inv[0][3]; inv[3][1] = inv[1][3]; inv[3][2] = inv[2][3];

            for( i = 0; i < 4; i++) // Over (delx) & rows of (inv[])
            {
                double acc;   // accumulator
                delx[i] = 0;

                for( j = 0; j < nsats_used; j++) // (delrho) & columns of (A)
                {
                    acc = 0;
                    for( k = 0; k < 3; k++) // rows of (A) & columns of (inv[])
                        acc += A[j][k] * inv[i][k];
                    acc += inv[i][3]; // Last column of (A) always = 1
                    delx[i] += acc * delrho[j];
                }
                delx[i] /= det;
            }


            nits++;     // bump number of iterations counter

            x += delx[0]; // correction to position estimates
            y += delx[1];
            z += delx[2];
            b += delx[3]; // correction to time bias

            error = sqrt( delx[0]*delx[0] + delx[1]*delx[1] +
		          delx[2]*delx[2] + delx[3]*delx[3]);
        }
    }
    while( (error > 0.1) && (nits < 10) && (error < 1.e8));
    // TODO, justify these numbers.

    // FIXME WHOAH Nelly, errors can creep out of that while loop, let's catch
    // them here and stop them from affecting the clock bias. If the solution
    // doens't converge to less than 0.1 meters, toss it.
    if( !singular && (error < 0.1))
    {
        result.valid = 1;

# if 0 // NOT CURRENTLY USED ANYWHERE - should it be?
        // Range residuals after position fix.
        for( i = 0; i < nsats_used; i++)
            pr2[sat_position[i].channel].residual = delrho[i];
#endif
    }
    else
    {
        result.valid = 0;
#if 0
        result.x = rec_pos_xyz.x;
        result.y = rec_pos_xyz.y;
        result.z = rec_pos_xyz.z;
#endif
    }

    result.x = x;
    result.y = y;
    result.z = z;
    result.b = b;
    result.n = nsats_used;
    result.error = error;
    result.positioning = 1;

    return ( result);
}


/******************************************************************************
 * Calculate a az/el for each satellite given a reference position
 * (Reference taken from "here.h".)
 ******************************************************************************/
static azel_t
satellite_azel( xyzt_t satpos)
{
    double xls, yls, zls, tdot, satang, xaz, yaz, az;
    azel_t result;

// What are these?
#define north_x 0.385002966431406
#define north_y 0.60143634005935
#define north_z 0.70003360254707
#define east_x  0.842218174688889
#define east_y -0.539136852963804
#define east_z  0
#define up_x -0.377413913446142
#define up_y -0.589581022958081
#define up_z 0.71410990421991

    // DETERMINE IF A CLEAR LINE OF SIGHT EXISTS
    xls = satpos.x - HERE_X;
    yls = satpos.y - HERE_Y;
    zls = satpos.z - HERE_Z;

    tdot = (up_x * xls + up_y * yls + up_z * zls) /
            sqrt( xls * xls + yls * yls + zls * zls);

    if ( tdot >= 1.0 )
        satang = PI_OVER_2;
    else if ( tdot <= -1.0 )
        satang = - PI_OVER_2;
    else
        satang = asin( tdot);

    xaz = east_x * xls + east_y * yls;
    yaz = north_x * xls + north_y * yls + north_z * zls;

    if (xaz != 0.0 || yaz != 0.0)
    {
        az = atan2( xaz, yaz);
        if( az < 0)
            az += TWO_PI;
    }
    else
        az = 0.0;

    result.el = satang;
    result.az = az;

    return (result);
}


/*******************************************************************************
FUNCTION SatPosEphemeris(double t, unsigned short n)
RETURNS  eceft

INPUT   t  double   coarse time of week in seconds
        n  char     satellite prn

PURPOSE

     THIS SUBROUTINE CALCULATES THE SATELLITE POSITION
     BASED ON BROADCAST EPHEMERIS DATA

     R    - RADIUS OF SATELLITE AT TIME T
     Crc  - RADIUS COSINE CORRECTION TERM
     Crs  - RADIUS SINE   CORRECTION TERM
     SLAT - SATELLITE LATITUDE
     SLONG- SATELLITE LONGITUDE
     TOE  - TIME OF EPHEMERIS FROM START OF WEEKLY EPOCH
     ecc  - ORBITAL INITIAL ECCENTRICITY
     TOA  - TIME OF APPLICABILITY FROM START OF WEEKLY EPOCH
     INC  - ORBITAL INCLINATION
     IDOT - RATE OF INCLINATION ANGLE
     CUC  - ARGUMENT OF LATITUDE COSINE CORRECTION TERM
     CUS  - ARGUMENT OF LATITUDE SINE   CORRECTION TERM
     CIC  - INCLINATION COSINE CORRECTION TERM
     CIS  - INCLINATION SINE   CORRECTION TERM
     RRA  - RATE OF RIGHT ASCENSION
     SQA  - SQUARE ROOT OF SEMIMAJOR AXIS
     LAN  - LONGITUDE OF NODE AT WEEKLY EPOCH
     AOP  - ARGUMENT OF PERIGEE
     MA   - MEAN ANOMALY AT TOA
     -> MA IS THE ANGLE FROM PERIGEE AT TOA
     DN   - MEAN MOTION DIFFERENCE

*******************************************************************************/

static xyzt_t
SatPosEphemeris( unsigned short ch)
{
    xyzt_t result;

    unsigned short i;

    double del_toc; // Time until/since clock correction reference time
    double del_toe; // Time until/since ephemeris correction reference time
    double del_t;   // Clock correction term
    double tc;      // Corrected time of satellite transmission
    
    /* Temporaries to save some float math. */
    double ea,ma,diff,ta,aol,delr,delal,delinc,r,inc,la,xp,yp;
    double mn;
    double s, c, sin_ea, cos_ea, e2, A, s_la, c_la;

    // Find the time difference in seconds for the clock correction. This must
    // be adjusted for wrapping at the end-of-week: see (BB p138) and (ICD
    // 20.3.3.3.3.1).
    del_toc = pr2[ch].sat_time - ephemeris[ch].toc;
    
    if( del_toc > SECONDS_IN_WEEK / 2)
        del_toc -= SECONDS_IN_WEEK;
    else if( del_toc < -SECONDS_IN_WEEK / 2)
        del_toc += SECONDS_IN_WEEK;

    // Calculate the time correction except for relativistic effects -- need
    // to calculate Ek first before we get those (BB p133).
    del_t =                ephemeris[ch].af0
              + del_toc * (ephemeris[ch].af1
              + del_toc *  ephemeris[ch].af2); // af2 is often zero

    // L1 - L2 Correction (group delay) for single freq. users: see (BB p134)
    // and (ICD 20.3.3.3.3.2)
    del_t -= ephemeris[ch].tgd;

    // Tsv corrected time (except relativistic effects)
    tc = pr2[ch].sat_time - del_t;

    // Find the time since orbital reference in seconds, adjusted for wrapping
    // at end-of-week. See: (ICD 20.3.3.4.3 & Table 20-IV) and (BB p138).
    // Note the time used is corrected EXCEPT for relativistic effects
    del_toe = tc - ephemeris[ch].toe;

    if( del_toe > SECONDS_IN_WEEK / 2)
        del_toe -= SECONDS_IN_WEEK;
    else if( del_toe < -SECONDS_IN_WEEK / 2)
        del_toe += SECONDS_IN_WEEK;

    /* Solve for the eccentric anomaly ("anomaly" means "angle" in old-speak)
     * From Webster's 1913:
     *    The angular distance of a planet from its perihelion,
     *    as seen from the sun. This is the true anomaly. The
     *    eccentric anomaly is a corresponding angle at the
     *    center of the elliptic orbit of the planet. The mean
     *    anomaly is what the anomaly would be if the planet's
     *    angular motion were uniform.
     *
     * ea is the eccentric anomaly to be found,
     * ma is the mean anomaly measured at the satellite's time of transmission,
     * ecc is the orbital eccentricity,
     * mn is the mean motion (proportional to the average sat.ang.velocity)
     * dn is the ephemeris correction to mn,
     */
    // The eccentric anomaly is a solution to the non-linear equation
    //   ea = ma + ecc * sin[ea]
    // Solve this equation by Newton's method
    // Set the initial value to the mean anomaly,
    // might do better with a better initial value (BBp164)
    // TODO: set the initial value using an extrapolation of the old ea

    // A = sqrt(Orbital major axis)^2 (we use this to find r also)
    A = ephemeris[ch].sqrtA * ephemeris[ch].sqrtA;

    // calculate the "mean motion" === (2pi / orbital period), i.e. the average
    // sat.ang.vel.  (mean motion) = sqrt(mu / A^3)
    mn = SQRTMU / (ephemeris[ch].sqrtA * A);

    // Prep the loop
    ma = ephemeris[ch].ma + del_toe * (mn + ephemeris[ch].dn);
    ea = ma;

    for( i = 0; i < 10; i++) /* Avoid possible endless loop. */
    {
        sin_ea = sin( ea);
        cos_ea = cos( ea);

        // Is it really better to use Newton's method as opposed to finding
        // the fixed point of Kepler's equation directly? The direct method
        // saves a cos() per iteration, so to be worth it Newton must be twice
        // as fast.

        diff = (ea - ephemeris[ch].ecc * sin_ea - ma) /
               (1 - ephemeris[ch].ecc * cos_ea);

        ea -= diff;
        if( fabs( diff) < 1.0e-12) // TODO justify this constant
            break;
    }

    /* ea may not converge. No error handling. */
    // It would be interesting to tag non-convergence, probably never happens.

    sin_ea = sin( ea); // Save some trig calculation
    cos_ea = cos( ea);

    // Now make the relativistic correction to the coarse time (BBp133)
    del_t += (F_RC * ephemeris[ch].ecc * ephemeris[ch].sqrtA * sin_ea);

    // Store the final clock corrections (Used for m_rho in position_thread.)
    result.tb = del_t;

    /* ta is the true anomaly (angle from perigee) */
    // This is clever because it magically gets the sign right
    // It can be done with an ArcCos and a simple sign fix-up, not sure what
    // is most efficient. (GBp48)
    e2 = ephemeris[ch].ecc * ephemeris[ch].ecc;
    ta = atan2( (sqrt( 1 - e2) * sin_ea), (cos_ea - ephemeris[ch].ecc));

    /* aol is the argument of latitude of the satellite */
    // w is the argument of perigee (GBp59)
    aol = ta + ephemeris[ch].w;

    /* Calculate the second harmonic perturbations of the orbit */
    c = cos( aol + aol);
    s = sin( aol + aol);
    delr   = ephemeris[ch].crc * c + ephemeris[ch].crs * s; // radius
    delal  = ephemeris[ch].cuc * c + ephemeris[ch].cus * s; // arg. of latitude
    delinc = ephemeris[ch].cic * c + ephemeris[ch].cis * s; // inclination

    /* r is the radius of satellite orbit at time pr2[ch].sat_time */
    r = A * (1 - ephemeris[ch].ecc * cos_ea) + delr;
    aol += delal;
    inc = ephemeris[ch].inc0 + delinc + ephemeris[ch].idot * del_toe;

    /* la is the corrected longitude of the ascending node */
    la = ephemeris[ch].w0 + del_toe * (ephemeris[ch].omegadot - OMEGA_E) -
            OMEGA_E * ephemeris[ch].toe;

    // xy position in orbital plane
    xp = r * cos( aol);
    yp = r * sin( aol);

    // transform to ECEF
    s_la = sin( la);
    c_la = cos( la);
    s = sin( inc); // This reuse may not be clever with optimization turned on
    c = cos( inc);

    result.x = xp * c_la - yp * c * s_la;
    result.y = xp * s_la + yp * c * c_la;
    result.z = yp * s;

    return( result);
}


/******************************************************************************
 * Wake up on valid measurements and produce pseudoranges. Flag the navigation
 * thread if we have four or more valid pseudoranges
 ******************************************************************************/
void
position_thread( CYG_ADDRWORD data) // input 'data' not used
{
    unsigned short  ch;
    unsigned short  satnum;
    xyz_t           ecef_temp;

    // There's no way that we're going to get a bit before this thread
    // is first executed, so it's ok to run the flag init here.
    cyg_semaphore_init( &position_semaphore, 0);

    while(1)
    {
        // Block until there's a valid set of measurements posted from the
        // measurement thread.
        cyg_semaphore_wait( &position_semaphore);

        setbits32( GPS4020_GPIO_WRITE, LED7); // DEBUG

        // OK we're awake: use a COPY of the PRs/measurements, because they're
        // going to change in about 100ms and there's NO WAY IN HELL we're
        // going to be done by then. And check that the ephemeris is still
        // good while we're at it.
        satnum = 0;
        for( ch = 0; ch < N_CHANNELS; ++ch)
        {
            if( ephemeris[ch].valid && pr2[ch].valid)
            {
                // get the satellite ECEF XYZ + time (correction?) of the
                // satellite. Note that sat_position is ECEF*T* type.
                sat_pos_by_ch[ch] = SatPosEphemeris( ch);

                if (display_command == DISPLAY_POSITION)
                   sat_azel[ch] = satellite_azel( sat_pos_by_ch[ch]);

                // Pack the satellite positions into an array for efficiency
                // in the calculate_position function.
		// Probably should not be doing this copy???
                sat_position[satnum].x = sat_pos_by_ch[ch].x;
                sat_position[satnum].y = sat_pos_by_ch[ch].y;
                sat_position[satnum].z = sat_pos_by_ch[ch].z;
                sat_position[satnum].tb = sat_pos_by_ch[ch].tb;
                sat_position[satnum].channel = ch;
                sat_position[satnum].prn = pr2[ch].prn;

                // Doppler measurement is not supported.
                // SHOCK!! SHOCK, I TELL YOU.

                // Pseudorange measurement. No ionospheric delay correction.
                m_rho[satnum] = pr2[ch].range +
                    (SPEED_OF_LIGHT * sat_pos_by_ch[ch].tb);
                // (.tb) is the per-satellite clock correction calculated from
                // ephemeris data.

                satnum++; // Number of valid satellites
            }
        }

        // If we've got 4 or more satellites, position!
        if( satnum >= 4)
        {
            receiver_pos = calculate_position( satnum);

            if( receiver_pos.valid)
            {
                // Move from meters to seconds
                receiver_pos.b /= SPEED_OF_LIGHT;

                // Correct the clock with the latest bias. But note that the
                // clock correction function may be smoothing the correction
                // and doing other funky things.
//                 set_clock_correction( receiver_pos.b);

                // Put receiver position in LLH
                if (display_command == DISPLAY_POSITION)
                {
                    ecef_temp.x = receiver_pos.x;
                    ecef_temp.y = receiver_pos.y;
                    ecef_temp.z = receiver_pos.z;
                    receiver_llh = ecef_to_llh( ecef_temp);
                }
            }
            // Log position, if enabled
#ifdef ENABLE_POSITION_LOG
            log_position();
#endif
        }
        else
        {
            // Not enough sats to calc position, but let the world know how
            // many satellites we do currently have.
            receiver_pos.positioning = 0;
            receiver_pos.n = satnum;
        }

        // FIXME: shouldn't be called from this thread. HACK!
#ifdef ENABLE_EPHEMERIS_LOG
        log_ephemeris();
#endif

        // Flag pseudorange.c that we've used the pseudorange data and are
	// ready for a new set in pr2.
        pr2_data_fresh = 0;
        clearbits32( GPS4020_GPIO_WRITE, LED7); // DEBUG
    }
}
