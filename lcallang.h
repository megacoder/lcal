/*
 * lcallang.h - language-dependent strings (month and day names, etc.)
 *
 */

char *months[12] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
	};

char *days[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
        "Saturday"
	};


/* size of abbreviations  */

#define MIN_DAY_LEN	2
#define MIN_MONTH_LEN	3


/*
 * Symbolic names for command-line flags.  These may be changed
 * as desired in order to be meaningful in languages other than
 * English.
 */

#define F_DAY_FONT	'd'		/* select alternate day font */
#define F_TITLE_FONT	't'		/* select alternate title font */

#define F_OUT_FILE	'o'		/* select alternate output file */

#define F_LANDSCAPE	'l'		/* landscape mode */
#define F_PORTRAIT	'p'		/* portrait mode */

#define F_SHADING	's'		/* set background/foreground shading */

#define F_INVERT	'i'		/* print white on black background */

#define F_HELP		'h'		/* generate help message */
#define F_USAGE		'u'		/* generate usage message */
#define F_VERSION	'v'		/* generate version stamp */

#define F_WEEKDAYS	'W'		/* weekday names inside moons */

#define F_XOFFSET	'X'		/* fudge factor for X offset (-l) */
#define F_YOFFSET	'Y'		/* fudge factor for Y offset (-p) */

#define F_TIMEZONE	'z'		/* specify alternate time zone */

#define F_SINGLEPAGE	'S'		/* print single-page calendar */

/*
 * Flag usage information - not strictly language-dependent, but here anyway
 * (N.B.: all flags must be represented by an entry in this table!)
 *
 * The table below supplies the following information about each flag:
 *
 *	- Its name (cf. symbolic definitions above)
 *
 *	- Whether or not it can take an (optional) argument
 *
 */

FLAG_USAGE flag_tbl[] = {

/*	flag name	arg? */

	{ F_DAY_FONT,	TRUE },
	{ F_TITLE_FONT,	TRUE },

	{ F_OUT_FILE,	TRUE },

	{ F_LANDSCAPE,	FALSE },
	{ F_PORTRAIT,	FALSE },

	{ F_SHADING,	TRUE },

	{ F_INVERT,	FALSE },

	{ F_HELP,	FALSE },
	{ F_USAGE,	FALSE },
	{ F_VERSION,	FALSE },

	{ F_WEEKDAYS,	FALSE },

	{ F_TIMEZONE,	TRUE },

	{ F_SINGLEPAGE,	FALSE },

	{ F_XOFFSET,	TRUE },
	{ F_YOFFSET,	TRUE },

	{ '-',		FALSE },
	{ '\0',		FALSE },
};


/*
 * Words used in usage() message - translate as necessary
 */

#define W_DEFAULT	"default"		/* translate as required */
#define W_USAGE		"Usage"

#define W_FONT		"<FONT>"		/* names of metavariables */
#define W_FILE		"<FILE>"
#define W_VALUE		"<VALUE>"
#define W_VAL2		"<n>{/<n>}"
#define W_SHADING	"{<b>}{/<f>}"

#define W_YY		"YY"


/* special flag_msg[] entries for end of option group, etc. */

#define END_GROUP	'\n', NULL, NULL, NULL		/* end of option group */
#define END_LIST	'\0', NULL, NULL, NULL		/* end of list */
#define GROUP_DEFAULT	' ', NULL, " "			/* group default */

/*
 * Message strings to be printed by usage() - translate as necessary
 */

FLAG_MSG flag_msg[] = {

/*	flag name	metasyntax	description						default */

	{ F_DAY_FONT,	W_FONT,		"specify alternate font for weekday names",		DATEFONT },
	{ F_TITLE_FONT,	W_FONT,		"specify alternate font for year/months/dates",		TITLEFONT },
	{ END_GROUP },

#ifdef DEFAULT_OUTFILE
	{ F_OUT_FILE,	W_FILE,		"specify alternate output file",			DEFAULT_OUTFILE },
#else
	{ F_OUT_FILE,	W_FILE,		"specify alternate output file",			"stdout" },
#endif
	{ END_GROUP },

	{ F_LANDSCAPE,	NULL,		"generate landscape-style calendar",			NULL },
	{ F_PORTRAIT,	NULL,		"generate portrait-style calendar",			NULL },
#if ROTATE == LANDSCAPE
	{ GROUP_DEFAULT,									"landscape" },
#else	
	{ GROUP_DEFAULT,									"portrait" },
#endif
	{ END_GROUP },
	{ F_SHADING,	W_SHADING,	"specify back/foreground colors (grayscale or r:g:b)",	NULL },
	{ ' ',		NULL,		" ",							SHADING },
	{ END_GROUP },

	{ F_INVERT,	NULL,		"invert colors (white moons on black background)",	NULL },
	{ END_GROUP },

	{ F_TIMEZONE,	W_VALUE,	"specify alternate time zone",				TIMEZONE },
	{ END_GROUP },

	{ F_SINGLEPAGE,	NULL,		"compress output to fit on single page" },
	{ 	0 },
	{ END_GROUP },

	{ F_XOFFSET,	W_VAL2,		"specify X offset (p.1/p.2)" },
	{ 	0 },
	{ GROUP_DEFAULT,									X_OFFSET },

	{ F_YOFFSET,	W_VAL2,		"specify Y offset (p.1/p.2)",				NULL },
	{ GROUP_DEFAULT,									Y_OFFSET },
	{ END_GROUP },

	{ F_WEEKDAYS,	NULL,		"print weekdays inside moons (warning: very slow)",	NULL },
	{ END_GROUP },

	{ F_HELP,	NULL,		"print this help message",				NULL },
	{ F_USAGE,	NULL,		"print usage message",					NULL },
	{ F_VERSION,	NULL,		"print version stamp",					NULL },
	{ END_GROUP },			/* must precede END_LIST */

	{ END_LIST }			/* must be last */
};


/* Numeric parameter descriptions and text */

#if __STDC__
PARAM_MSG param_msg[] = {
	{ W_YY,			"generate calendar for year " W_YY " (CC" W_YY " if " W_YY " < 100)" },
	{ "(" W_DEFAULT ")",	"generate calendar for current year" },
	{ NULL,			NULL }		/* must be last */
};
#else
PARAM_MSG param_msg[] = {
	{ "YY",		"generate calendar for year YY (CCYY if YY < 100)" },
	{ "(default)",	"generate calendar for current year" },
	{ NULL,		NULL }		/* must be last */
};
#endif

#define PARAM_MSGS	1	/* number of above to print in command-line syntax message */

/* format string for short usage() message */
#define USAGE_MSG	"enter \"%s -%c\" for full description of parameters\n"

/* format strings for comment in PostScript output file */
#define VERSION_MSG	"Generated by %s %s"
#define CREATED_MSG     "Generated by"
#define TITLE_MSG       "Lunar phase calendar for"

/* Error and information messages - translate as necessary */

/* program error messages */
#define	E_FOPEN_ERR	"%s: can't open file %s\n"
#define	E_ILL_OPT	"%s: unrecognized flag %s"
#define E_ILL_OPT2	" (%s\"%s\")"
#define	E_ILL_YEAR	"%s: year %d not in range %d .. %d\n"
#define E_FLAG_IGNORED	"%s: -%c flag ignored (%s\"%s\")\n"
#define ENV_VAR		"environment variable "

/* information message (VMS, Amiga only) */
#define	I_OUT_NAME	"%s: output is in file %s\n"

