/*
 * lcaldefs.h - definitions for Lcal (lunar calendar) program
 *
 */

/*
 * System dependencies:
 */

#ifdef VMS		/* VMS oddities isolated here */

#include <ssdef.h>	/* required for trnlog() */
#include <descrip.h>

#define DEFAULT_OUTFILE	"calendar.ps"

#define EXIT_SUCCESS	1
#define EXIT_FAILURE	3

#else
#ifdef AMIGA		/* more oddities for Amiga */

#include <stdlib.h>
#include <string.h>

#define PROTOS				/* compiler accepts prototypes */
#define DEFAULT_OUTFILE	"RAM:calendar.ps"

#else
#ifdef DOS		/* even more oddities for MS-DOS */

#else			/* neither VMS, Amiga, nor MS-DOS - assume Un*x */

#define UNIXX		/* to distinguish from others (not used yet) */
#include <unistd.h>

#define END_PATH	'/'

#define PAGER_ENV	"PAGER"		/* points to help message pager */
#define PAGER_DEFAULT	"more"		/* default pager (NULL = none) */

#endif
#endif
#endif

/* define OUTFILE to DEFAULT_OUTFILE if defined, otherwise to "" (stdout) */

#ifdef DEFAULT_OUTFILE
#define OUTFILE		DEFAULT_OUTFILE
#else
#define OUTFILE		""
#endif

/* PROTOS may be defined independently of __STDC__ for compilers which
 * support function prototypes but are not fully ANSI-compliant
 */

#if defined(__STDC__) && ! defined(PROTOS)
#define PROTOS	
#endif

/* STDLIB may be defined independently of __STDC__ for systems which
 * support <stdlib.h> but are not fully ANSI-compliant
 */

#if defined(__STDC__) && ! defined(STDLIB)
#define STDLIB                  /* cf. {no}protos.h */
#endif

#ifdef STDLIB                   /* include <stdlib.h> if known to exist */
#include <stdlib.h>
#endif

/* EXIT_SUCCESS and EXIT_FAILURE should be defined in <stdlib.h>
 * (or above if non-standard); define here if necessary
 */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS    0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE    1
#endif

/* definition of NULL (if needed) */
 
#ifndef NULL
#define NULL	0
#endif

/*
 * Environment variables (global symbol, logical name on VMS):
 */

#define LCAL_OPTS	"LCAL_OPTS"	/* command-line flags */

/*
 * Function-like macros:
 */

#define FPR		(void)fprintf
#define PRT		(void)printf

#define IS_LEAP(y)	((y) % 4 == 0 && ((y) % 100 != 0 || (y) % 400 == 0))
#define LENGTH_OF(m, y) (month_len[(m)-1] + ((m) == FEB && IS_LEAP(y)))
#define YEAR_LEN(y)	(IS_LEAP(y) ? 366 : 365)
#define DAY_OF_YEAR(m, d, y) ((month_off[(m)-1] + ((m) > FEB && IS_LEAP(y))) + d)
#define OFFSET_OF(m, y) ((month_off[(m)-1] + ((m) > FEB && IS_LEAP(y))) % 7)
#define FIRST_OF(m, y)	calc_weekday(m, 1, y)

#define P_LASTCHAR(p)	((p) && *(p) ? (p) + strlen(p) - 1 : NULL)
#define LASTCHAR(p)	(p)[strlen(p) - 1]

#define IS_NUMERIC(p)	((p)[strspn((p), DIGITS)] == '\0')

#define ERR(errmsg) \
	FPR(stderr, E_ILL_LINE, progname, errmsg, filename, line);

#ifdef __STDC__
#define TOLOWER(c)	tolower(c)
#else
#define TOLOWER(c)	(isupper(c) ? tolower(c) : (c))
#endif


/*
 * Miscellaneous other constants:
 */

#ifdef EPS
#define PS_RELEASE	"PS-Adobe-2.0 EPSF"	/* for comments at top */
#else
#define PS_RELEASE	"PS-Adobe-1.0"		/* for comments at top */
#endif

#define P_ENV		1	/* environment variable / command line pass */
#define P_CMD1		2

#ifndef FALSE
#define FALSE		0	/* pseudo-Booleans */
#define TRUE		1
#endif

#define MIN_YR		1753	/* significant years (calendar limits) */
#define MAX_YR		9999

#define CENTURY		1900	/* base year for struct tm year field */

#define JAN		 1	/* significant months */
#define FEB		 2
#define DEC		12

#define SUN		 0
#define SAT		 6

#define SCREENWIDTH	72	/* command-line message in usage() */

#define STRSIZ		200	/* size of misc. strings */
#define MAXWORD		100
#define LINSIZ		512	/* size of source line buffer */

#define MAXARGS		1	/* numeric command-line args */

#define DIGITS		"0123456789"
#define WHITESPACE	" \t"

/*
 * Defaults for calendar layout:
 */

#ifndef DATEFONT
#define DATEFONT	"Times-Bold"	/* default font names */
#endif
#ifndef TITLEFONT
#define TITLEFONT	"Times-Bold"
#endif

#define	TITLEFONTSIZE	72		/* font size for year...	*/
#define	DATEFONTSIZE	16		/* dates			*/
#define	MONTHFONTSIZE	16		/* month names			*/
#define	WKDFONTSIZE	10		/* weekday names (inside moons) */
#define	SMWKDFONTSIZE	 7		/* weekday names		*/

#define	DATEFONTSIZE_S	18		/* enlarge font sizes slightly	*/
#define	MONTHFONTSIZE_S	18		/* for legibility when output	*/
#define SMWKDFONTSIZE_S	 9		/* is compressed (-S)		*/

/*
 * very few PS printers will actually print to the edges of each page; these
 * site-specific magic numbers (also definable in the Makefile) compensate
 * for this
 */
#ifndef X_OFFSET
#define X_OFFSET	"0/0"		/* X origin offset (p.1, p.2) */
#endif
#ifndef Y_OFFSET
#define Y_OFFSET	"0/0"		/* Y origin offset (p.1, p.2) */
#endif

#define LANDSCAPE  	90		/* degrees to rotate */
#define PORTRAIT    	 0
#define ROTATE	   	LANDSCAPE	/* default */

#define WEEKDAYS	FALSE		/* weekdays inside moons? */

#define FULL_SIZE	1.0		/* scale factor (normal)      */
#define HALF_SIZE	0.5		/* scale factor (single page) */

#define PAGEBREAK	15		/* days printed on first page   */
#define PAGEBREAK_S	33		/* no page break if single page */

/*
 * default time zone is UTC; site-specific time zone may be defined here
 * or in the Makefile
 */
#ifndef TIMEZONE
#define TIMEZONE	"0"		/* hours west of UTC */
#endif

/*
 * background/foreground colors (one grayscale value or three r:g:b values)
 */
#define SHADING		"1/0"		/* default: black-on-white */
#define INVERT		"0/1"		/* -i flag: white-on-black */
#define RGB_CHAR	':'		/* r:g:b value separator */

/* 
 * Global typedef declaration for flag usage struct (cf. lcallang.h, get_args())
 */

typedef struct {
	char flag;		/* name of flag */
	char has_arg;		/* TRUE if flag takes (optional) arg */
	} FLAG_USAGE;

/*
 * Global typedef declarations for usage message (cf. lcallang.h, usage())
 */

typedef struct {
	char flag;		/* name of flag */
	char *meta;		/* metaname for argument (if any) */
	char *text;		/* associated text */
	char *def;		/* default value (if any) */
	} FLAG_MSG;

typedef struct {
	char *desc;		/* description of param */
	char *text;		/* associated text */
	} PARAM_MSG;


