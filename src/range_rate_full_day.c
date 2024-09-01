/*
 *  main.c          April 10  2001
 *
 *  A skeleton main() function to demonstrate the use of
 *  the SGP4() and SDP4() routines in the sgp4sdp4.c file.
 *  The TLE set to be used is read from the file amateur.txt
 *  downloaded from Dr Kelso's website at www.celestrak.com.
 *  Please note that this is a simple application and can only
 *  read the first TLE kep set, including line 0 with the Name.
 */

#define SGP4SDP4_CONSTANTS
#include "include/sgp4sdp4.h"
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <argp.h>

// argp related constants
const char *argp_program_version = "range_rate_full_day 0.8.0";
const char *argp_program_bug_address = "<knud@ualberta.ca>";

// argp documentation
static char doc[] = "A program to generate and record over one day a specified satellite's range rate for a speficied ground station location.";

// argp arguments
static char args_doc[] = "<satellite catalog number> <ground station lat deg> <ground station long deg>";

// argp options
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Verbose output", 0},
    {"to file", 'o', "FILE", 0, "Output to FILE instead of stdout", 0},
    { 0 }
};

struct arguments {
  char *args[3]; // the three args defined in args_doc[]
  int verbose;
  char *output_file;
};

// Parse a single option
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  // Get the input argument from argp_parse, which is a
  // pointer to the arguments structure
  struct arguments *arguments = state->input;

  switch(key) {
  case 'v':
    arguments->verbose = 1;
    break;
  case 'o':
    arguments->output_file = arg;
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
} // parse_opt

// the argp parser
static struct argp argp = {options, parse_opt, args_doc, doc };

/* Main program */
int main(int argc, char **argv)
{
  int parseReturn;
  UNUSED(parseReturn);

  struct arguments arguments;

  // defaults
  arguments.verbose = 0;
  arguments.output_file = "-";

  // Parse away...

  parseReturn = argp_parse (&argp, argc, argv, 0, 0, &arguments);


  uint32_t satCatNum = 25445;

  double obs_lat;  // observer latitude in degrees
  double obs_long; // observer longitude in degrees (-ve west, +ve east)
  double obs_alt; // observer altitude in meters

  obs_lat = 53.7694;
  obs_long = -113.4560;
  obs_alt = 701;

  if (obs_long < 0) {
    obs_long += 360;
  }
  /* Observer's geodetic co-ordinates.      */
  /* Lat North, Lon East in rads, Alt in km */
  geodetic_t obs_geodetic = {obs_lat*M_PI/180.0, obs_long*M_PI/180.0, obs_alt/1000, 0.0};

  /* Two-line Orbital Elements for the satellite */
  tle_t tle ;

  /* Zero vector for initializations */
  vector_t zero_vector = {0,0,0,0};

  /* Satellite position and velocity vectors */
  vector_t vel = zero_vector;
  vector_t pos = zero_vector;
  /* Satellite Az, El, Range, Range rate */
  vector_t obs_set;

  /* Solar ECI position vector  */
  vector_t solar_vector = zero_vector;
  /* Solar obse#include rved azi and ele vector  */
  vector_t solar_set;

  /* Calendar date and time (UTC) */
  struct tm utc;
  struct timeval tv;

  /* Satellite's predicted geodetic position */
  geodetic_t sat_geodetic;

  double
  tsince,            /* Time since epoch (in minutes) */
  jul_epoch,         /* Julian date of epoch          */
  jul_utc,           /* Julian UTC date               */
  eclipse_depth = 0, /* Depth of satellite eclipse    */
  /* Satellite's observed position, range, range rate */
  sat_azi, sat_ele, sat_range, sat_range_rate,
  /* Satellites geodetic position and velocity */
  sat_lat, sat_lon, sat_alt, sat_vel,
  /* Solar azimuth and elevation */
  sun_azi, sun_ele;

  /* Used for storing function return codes */
  int flg;

  char
  ephem[5],       /* Ephemeris in use string  */
  sat_status[12]; /* Satellite eclipse status */

  /* Input one (first!) TLE set from file */
  //  flg = Input_Tle_Set(tle_file, &tle);

  flg = get_current_tle(satCatNum, &tle);

  /* Abort if file open fails */
  if( flg < 0 ) {
    printf(" Fetching current TLE for Satellite Catalog number %d failed - Exiting!\n", satCatNum);
    exit(-1);
  }

  /* Print satellite name and TLE read status */
  printf(" %s: ", tle.sat_name);
  if( flg == -2 )
  {
    printf("TLE set bad - Exiting!\n");
    exit(-2);
  }
  else
    printf("TLE set good - Happy Tracking!\n");

  /** !Clear all flags! **/
  /* Before calling a different ephemeris  */
  /* or changing the TLE set, flow control */
  /* flags must be cleared in main().      */
  ClearFlag(ALL_FLAGS);

  /** Select ephemeris type **/
  /* Will set or clear the DEEP_SPACE_EPHEM_FLAG       */
  /* depending on the TLE parameters of the satellite. */
  /* It will also pre-process tle members for the      */
  /* ephemeris functions SGP4 or SDP4 so this function */
  /* must be called each time a new tle set is used    */
  select_ephemeris(&tle);

  // Set sampling rate aka delay between orbital parameter updates
  struct timespec delay = {.tv_sec = 0, .tv_nsec = 100000000 };

  // Get calendar date and time of day once, then
  // run time from 0 to 86,400 seconds, a full day
  UTC_Calendar_Now(&utc, &tv);

  // tv.tv_sec is not used, and we will just keep tv.tv_usec zero
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  // set to the start of the day
  utc.tm_hour = 0;
  utc.tm_min = 0;
  utc.tm_sec = 0;

  while (1) {
    /* Get UTC calendar and convert to Julian */
    //  UTC_Calendar_Now(&utc, &tv);
    jul_utc = Julian_Date(&utc, &tv);

    /* Convert satellite's epoch time to Julian  */
    /* and calculate time since epoch in minutes */
    jul_epoch = Julian_Date_of_Epoch(tle.epoch);
    tsince = (jul_utc - jul_epoch) * xmnpda;

    /* Copy the ephemeris type in use to ephem string */
    if( isFlagSet(DEEP_SPACE_EPHEM_FLAG) )
      strcpy(ephem,"SDP4");
    else
      strcpy(ephem,"SGP4");

    /* Call NORAD routines according to deep-space flag */
    if( isFlagSet(DEEP_SPACE_EPHEM_FLAG) )
      SDP4(tsince, &tle, &pos, &vel);
    else
      SGP4(tsince, &tle, &pos, &vel);

    /* Scale position and velocity vectors to km and km/sec */
    Convert_Sat_State( &pos, &vel );

    /* Calculate velocity of satellite */
    Magnitude( &vel );
    sat_vel = vel.w;

    /** All angles in rads. Distance in km. Velocity in km/s **/
    /* Calculate satellite Azi, Ele, Range and Range-rate */
    Calculate_Obs(jul_utc, &pos, &vel, &obs_geodetic, &obs_set);

    /* Calculate satellite Lat North, Lon East and Alt. */
    Calculate_LatLonAlt(jul_utc, &pos, &sat_geodetic);

    /* Calculate solar position and satellite eclipse depth */
    /* Also set or clear the satellite eclipsed flag accordingly */
    Calculate_Solar_Position(jul_utc, &solar_vector);
    Calculate_Obs(jul_utc,&solar_vector,&zero_vector,&obs_geodetic,&solar_set);

    if( Sat_Eclipsed(&pos, &solar_vector, &eclipse_depth) )
      SetFlag( SAT_ECLIPSED_FLAG );
    else
      ClearFlag( SAT_ECLIPSED_FLAG );

    /* Copy a satellite eclipse status string in sat_status */
    if( isFlagSet( SAT_ECLIPSED_FLAG ) )
      strcpy( sat_status, "Eclipsed" );
    else
      strcpy( sat_status, "In Sunlight" );

    /* Convert and print satellite and solar data */
    sat_azi = Degrees(obs_set.x);
    sat_ele = Degrees(obs_set.y);
    sat_range = obs_set.z;
    sat_range_rate = obs_set.w;
    sat_lat = Degrees(sat_geodetic.lat);
    sat_lon = Degrees(sat_geodetic.lon);
    sat_alt = sat_geodetic.alt;

    sun_azi = Degrees(solar_set.x);
    sun_ele = Degrees(solar_set.y);

    if (sat_ele > 1) {
      printf("\n Date: %02d/%02d/%04d UTC: %02d:%02d:%02d  Ephemeris: %s"
          "\n Azi=%6.1f Ele=%6.1f Range=%8.1f Range Rate=%6.2f"
          "\n Lat=%6.1f Lon=%6.1f  Alt=%8.1f  Vel=%8.3f"
          "\n Satellite Status: %s - Depth: %2.3f"
          "\n Sun Azi=%6.1f Sun Ele=%6.1f\n",
          utc.tm_mday, utc.tm_mon, utc.tm_year,
          utc.tm_hour, utc.tm_min, utc.tm_sec, ephem,
          sat_azi, sat_ele, sat_range, sat_range_rate,
          sat_lat, sat_lon, sat_alt, sat_vel,
          sat_status, eclipse_depth,
          sun_azi, sun_ele);
      // delay until range rate value needed
      nanosleep(&delay, NULL);

    }

    printf("current time %02d:%02d:%02d\n",utc.tm_hour,utc.tm_min,utc.tm_sec);

    bool min_roll = false;
    // update the time
    if (utc.tm_sec == 59) {
      utc.tm_sec = 0;
    }
    else {
      utc.tm_sec++;
    }

    // if seconds is 0, we must have just exceeded 59 seconds
    if (utc.tm_sec == 0) {
      if (utc.tm_min == 59) {
        utc.tm_min = 0;
        min_roll = true;
      }
      else {
        utc.tm_min++;
      }

    }

    if (min_roll) {
      utc.tm_hour++;
      min_roll = false;
    }

    if (utc.tm_hour >= 24) {
      break;
    }

  }  // while

} /* End of main() */

/*------------------------------------------------------------------*/

