static char VERSION_STRING[] = "@(#)lcal v1.2 - print Postscript lunar calendars (moon phase for full year)";
/*
 * lcal.c - generate PostScript file to print lunar calendar for any year
 *
 *	Parameters:
 *
 *		lcal [opts]		generate calendar for current year
 *
 *		lcal [opts] yy		generate calendar for entire year yy
 *
 *	Output:
 *
 *		PostScript file to print moon phase calendar for given year
 *
 *	Options:
 *
 *		-d <FONT>	specify alternate font for weekday names
 *				(default: Helvetica-Bold)
 *
 *		-t <FONT>	specify alternate font for year/months/dates
 *				(default: Helvetica-Bold)
 *
 *		-o <FILE>	specify alternate output file (default:
 *				stdout on Un*x, CALENDAR.PS on VMS, 
 *				RAM:calendar.ps on Amiga)
 *
 *		-l		generate landscape-mode calendars
 *		-p		generate portrait-mode calendars
 *				(default: landscape-mode)
 *
 *		-z		specify alternate time zone (hours west
 *				of UTC)
 *
 *		-S		compress output to fit on single page
 *
 *		-X <n>{/<n>}	specify X origin fudge factor (p.1/p.2) for
 *				landscape-mode calendars
 *		-Y <n>{/<n>}	specify Y origin fudge factor (p.1/p.2) for
 *				portrait-mode calendars
 *
 *		-W		draw weekday names inside moons (warning:
 *				this feature is *very* slow)
 *
 *		-s <b>{/<f>}	specify background/foreground colors (single
 *				grayscale value or red:green:blue)
 *
 *		-S		print small (single-page) calendar
 *
 *		-i		print white moons against black background
 *
 *		-h		write "help" message to stdout
 *		-u		write "usage" message to stdout
 *		-v		write version stamp to stdout
 *
 *
 *	If the environment variable (global symbol on VMS) LCAL_OPTS is
 *	present, its value will be parsed as if it were a command line.
 *	Any options specified will override the program defaults.
 *
 *	Any flag which normally takes an argument may also be specified without
 *	an argument; this resets the corresponding option to its default.
 *
 *	Parameters and flags may be mixed on the command line.  In some cases
 *	(e.g., when a parameter follows a flag without its optional argument)
 *	this may lead to ambiguity; the dummy flag '-' (or '--') may be used
 *	to separate them, i.e. "lcal -t - 1996".
 *	
 *	Lcal was inspired by "Moonlight 1997", a 16" x 36" full-color (silver
 *	moons against a midnight blue background) lunar phase calendar marketed
 *	by Celestial Products, Inc., P.O. Box 801, Middleburg VA 22117.
 *	Send for their catalog or visit their site:
 *
 *	  http://www.celestialproducts.com
 *
 *	to see (and, hopefully, order) this as well as some even more amazing
 *	stuff - particularly "21st Century Luna", a lunar phase calendar for
 *	*every day* of the upcoming century.
 *
 */


/*
 * Standard headers:
 */

#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#ifdef UN_X
#include <pwd.h>		/* for user name in PostScript comments */
#define NOBODY	"nobody"	/* anonymous account */
#endif

/*
 * Lcal-specific definitions:
 */

#include "lcaldefs.h"
#include "lcallang.h"
#include "lcal_l.h"		/* PostScript boilerplate (landscape) */
#include "lcal_p.h"		/* PostScript boilerplate (portrait)  */
#include "protos.h"

/* fmod() may be absent from pre-ANSI math libraries; the macro version
 * supplied below isn't of production quality, but is acceptable for the
 * range of numbers encountered here
 */

#ifdef __STDC__
#define FMOD(x, y)	fmod(x, y)
#else
#define FMOD(x, y)	((x) - ((long) ((x)/(y)) * (y)))
#endif

/* split string into two substrings at separator 'c' */
#define SPLIT(str, str1, str2, c) \
	do { char *p; \
	str2 = ((p = strrchr(str, c))) ? (*p = '\0', ++p) : ""; \
	if ((p = strchr(str1 = str, c))) *p = '\0'; } while (0)

/*
 * Globals:
 */

static int init_year;

char *words[MAXWORD];		/* maximum number of words per date file line */
char lbuf[LINSIZ];		/* date file source line buffer */
char progname[STRSIZ];		/* program name (for error messages) */
char version[20];		/* program version (for info messages) */

/* lengths and offsets of months in common year */
char month_len[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
short month_off[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

/*
 * Default values for command-line options:
 */

int rotate = ROTATE;			/* -l, -p */

char dayfont[STRSIZ] = DATEFONT;	/* -d, -t, -n */
char titlefont[STRSIZ] = TITLEFONT;

char outfile[STRSIZ] = OUTFILE;		/* -o */

char x_offset[STRSIZ] = X_OFFSET;	/* -X, -Y */
char y_offset[STRSIZ] = Y_OFFSET;

int weekdays = WEEKDAYS;		/* -W */

char shading[STRSIZ] = SHADING;		/* -i, -s */

char time_zone[STRSIZ] = TIMEZONE;	/* -z */

int singlepage = FALSE;			/* -S */

/*
 * Main program - parse and validate command-line arguments, open files,
 * generate PostScript boilerplate and code to generate calendars.
 *
 * Program structure:
 *
 * main() looks for the environment variable (global symbol on VMS) LCAL_OPTS
 * and, if present, calls get_args() to parse it.  It then calls get_args()
 * again to parse the command line.
 */
int
#ifdef PROTOS
main(int argc, char *argv[])
#else
main(argc, argv)
	int argc;
	char *argv[];
#endif
{
	char *p, tmp[STRSIZ];

	/* extract root program name and program path */

	strcpy(progname, **argv ? *argv : "lcal");

	if ((p = strrchr(progname, END_PATH)) != NULL)
		strcpy(progname, ++p);
	if ((p = strchr(progname, '.')) != NULL)
		*p = '\0';

	/* get version from VERSION_STRING (for use in PostScript comment) */
	strcpy(tmp, VERSION_STRING + 4);
	p = strchr(tmp, ' ') + 1;	/* skip program name */
	*strchr(p, ' ') = '\0';		/* terminate after version */
	strcpy(version, p);

	/*
	 * Get the arguments from a) the environment variable, b) the command
	 * line
	 */

	/* parse environment variable LCAL_OPTS as a command line */

	if ((p = getenv(LCAL_OPTS)) != NULL) {
		strcpy(lbuf, "lcal ");		/* dummy program name */
		strcat(lbuf, p);
		(void) loadwords(words, lbuf);	/* split string into words */
		if (! get_args(words, P_ENV, LCAL_OPTS)) {
			usage(stderr, FALSE);
			exit(EXIT_FAILURE);
		}
	}

	/* parse command-line arguments once to find name of moon file, etc. */

	if (!get_args(argv, P_CMD1, NULL)) {
		usage(stderr, FALSE);
		exit(EXIT_FAILURE);
	}

	/* done with the arguments and flags - try to open the output file */

	if (*outfile && freopen(outfile, "w", stdout) == (FILE *) NULL) {
		FPR(stderr, E_FOPEN_ERR, progname, outfile);
		exit(EXIT_FAILURE);
	}

	/* generate the PostScript code */

	write_psfile(init_year);
	
	/* if output was written to a non-obvious location, tell user where */

#ifdef DEFAULT_OUTFILE
	FPR(stderr, I_OUT_NAME, progname, outfile);
#endif

	exit(EXIT_SUCCESS);
}




/*
 * get_flag() - look up flag in flag_tbl; return pointer to its entry
 * (NULL if not found)
 */
FLAG_USAGE *
#ifdef PROTOS
get_flag(char flag)
#else
get_flag(flag)
	char flag;
#endif
{
	FLAG_USAGE *pflag;

	for (pflag = flag_tbl; pflag->flag; pflag++)
		if (flag == pflag->flag)
			return pflag;

	return flag ? NULL : pflag;		/* '\0' is a valid flag */
}


/*
 * get_args - walk the argument list
 */
int
#ifdef PROTOS
get_args(char **argv,
	 int curr_pass,
	 char *where)
#else
get_args(argv, curr_pass, where)
	char **argv;		/* argument list */
	int curr_pass;		/* current pass */
	char *where;		/* for error messages */
#endif
{
	char *parg, *opt;
	FLAG_USAGE *pflag;
	int flag;
	long tmp;			/* for getting current month/year */
	struct tm *p_tm;
	int badopt = FALSE;		/* flag set if bad param   */
	int nargs = 0;			/* count of non-flag args  */
	int numargs[MAXARGS];		/* non-flag (numeric) args */
	FILE *fp = stdout;		/* for piping "help" message */

/*
 * If argument follows flag (immediately or as next parameter), return
 * pointer to it (and bump argv if necessary); else return NULL
 */
#define GETARG() (*(*argv + 2) ? *argv + 2 : \
		  (*(argv+1) && **(argv+1) != '-' ? *++argv : NULL))

	/* Walk argument list, ignoring first element (program name) */

 	while ((opt = *++argv) != NULL) {

		/* Assume that any non-flag argument is a numeric argument */
		if (*opt != '-') {
		    	if (curr_pass == P_CMD1 && nargs < MAXARGS) {
				if (! IS_NUMERIC(opt))
					goto bad_par;
				numargs[nargs++] = atoi(opt);
			}
			continue;
		}

		/* Check that flag is legal */

		if (! (pflag = get_flag(flag = opt[1])) )
			goto bad_par;

		/* get optional argument */

		parg = pflag->has_arg ? GETARG() : NULL;

		switch (flag) {

		case F_SINGLEPAGE:
			singlepage = TRUE;
			break;

 		case F_DAY_FONT:	/* specify alternate day font */
			strcpy(dayfont, parg ? parg : DATEFONT);
 			break;

 		case F_TITLE_FONT:	/* specify alternate title font */
			strcpy(titlefont, parg ? parg : TITLEFONT);
 			break;

		case F_OUT_FILE:	/* specify alternate output file */
			strcpy(outfile, parg ? parg : OUTFILE);
			break;

		case F_LANDSCAPE:	/* generate landscape calendar */
 			rotate = LANDSCAPE;
 			break;
 
		case F_PORTRAIT:	/* generate portrait calendar */
 			rotate = PORTRAIT;
 			break;

		case F_SHADING:		/* set background/foreground shading */
			define_shading(shading, parg, SHADING);
			break;

		case F_HELP:		/* request "help" message */
		case F_USAGE:		/* request "usage" message */
		case F_VERSION:		/* request version stamp */
			/* PAGER_ENV (cf. lcaldefs.h) defines the name of an
			 * environment variable which, if set, points to the
			 * appropriate pager (e.g., "more", "less", "pg")
			 * for piping the "help" message (Un*x systems only)
			 */
#ifdef PAGER_ENV
			if (flag == F_HELP) {
				FILE *pfp;
				char *pager, *p;

				pager = (p = getenv(PAGER_ENV)) ? p :
					PAGER_DEFAULT;
				/* null pointer or string: no paging */
				if (pager && *pager &&
				    (pfp = popen(pager, "w")) != (FILE *)NULL)
					fp = pfp;
			}
#endif

			FPR(fp, "%s\n", VERSION_STRING + 4);
			if (flag != F_VERSION)
				usage(fp, flag == F_HELP);
			fflush(fp);
#ifdef PAGER_ENV
			if (fp != stdout)
				pclose(fp);
#endif
			exit(EXIT_SUCCESS);
			break;

		case F_WEEKDAYS:	/* draw weekday names inside moons */
			weekdays = !(WEEKDAYS);
			break;

		case F_INVERT:		/* print white-on-black */
			strcpy(shading, INVERT);
			break;

		case F_XOFFSET:		/* X offset fudge factors */
			strcpy(x_offset, parg ? parg : X_OFFSET);
			break;

		case F_YOFFSET:		/* Y offset fudge factors */
			strcpy(y_offset, parg ? parg : Y_OFFSET);
			break;

		case F_TIMEZONE:	/* alternate time zone */
			strcpy(time_zone, parg ? parg : TIMEZONE);
			break;

		case '-' :		/* accept - and -- as dummy flags */
		case '\0':
			break;

		default:		/* missing case label if reached!!! */

bad_par:				/* unrecognized parameter */

			FPR(stderr, E_ILL_OPT, progname, opt);
			if (where)
				FPR(stderr, E_ILL_OPT2,
					curr_pass == P_ENV ? ENV_VAR : "",
					where);
			FPR(stderr, "\n");
			badopt = TRUE;
			break;
		}
        }

	if (curr_pass != P_CMD1)
		return !badopt;		/* return TRUE if OK, FALSE if error */

	/* Validate non-flag (numeric) parameters */

	time(&tmp);
	p_tm = localtime(&tmp);

	if (nargs == 0) {
		/* assume tm_year represents years elapsed since 1900 */
		init_year = CENTURY + p_tm->tm_year;
	} else {
		init_year = numargs[0];
	}

	if (init_year > 0 && init_year < 100)	/* treat nn as CCnn */
		init_year += 100 * ((CENTURY + p_tm->tm_year) / 100);
	
	if (init_year < MIN_YR || init_year > MAX_YR) {
		FPR(stderr, E_ILL_YEAR, progname, init_year, MIN_YR, MAX_YR);
		badopt = TRUE;
	}

	return !badopt;		/* return TRUE if OK, FALSE if error */
}


/*
 * usage - print message explaining correct usage of the command-line
 * arguments and flags.  If "fullmsg" is true, print associated text
 */
void
#ifdef PROTOS
usage(FILE *fp,
      int fullmsg)
#else
usage(fp, fullmsg)
	FILE *fp;	/* destination of usage message */
	int fullmsg;	/* print complete message? */
#endif
{
	FLAG_MSG *pflag;
	PARAM_MSG *ppar;
	char buf[30], *p, flag, *meta;
	int nchars, first, i, indent, n;

	sprintf(buf, "%s: %s", W_USAGE, progname);
	nchars = indent = strlen(buf);
	first = TRUE;
	meta = p = NULL;
	FPR(fp, "\n%s", buf);

	/* loop to print command-line syntax message (by group of flags) */

	for (pflag = flag_msg; (flag = pflag->flag) != '\0'; pflag++) {
		if (flag == '\n') {		/* end of group? */
			if (p)
				*p = '\0';
			if (meta) {		/* append metavariable name */
				strcat(buf, " ");
				strcat(buf, meta);
			}
			strcat(buf, "]");
			n = strlen(buf);
			if (nchars + n > SCREENWIDTH) {	/* does it fit on line? */
				FPR(fp, "\n");		/* no - start new one */
				for (nchars = 0; nchars < indent; nchars++)
					FPR(fp, " ");
			}
			FPR(fp, "%s", buf);
			nchars += n;
			first = TRUE;
		}
		else if (flag != ' ') {		/* accumulate flags for group */
			if (first) {
				sprintf(buf, " [");
				p = buf + strlen(buf);
			}
			else
				*p++ = '|';
			*p++ = '-';
			*p++ = flag;
			meta = pflag->meta;	/* save metavariable name */
			first = FALSE;
		}
	}

	/* loop to print selected numeric parameter descriptions */

	for (i = 0; i < PARAM_MSGS; i++) {
		sprintf(buf, " [%s]%s", param_msg[i].desc,
			i < PARAM_MSGS - 1 ? " |" : "");
		n = strlen(buf);
		if (nchars + n > SCREENWIDTH) {	/* does it fit on line? */
			FPR(fp, "\n");		/* no - start new one */
			for (nchars = 0; nchars < indent; nchars++)
				FPR(fp, " ");
		}
		FPR(fp, "%s", buf);
		nchars += n;
	}

	FPR(fp, "\n\n");
	if (! fullmsg) {
		FPR(fp, USAGE_MSG, progname, F_HELP);
		return;
	}
	
	/* loop to print the full flag descriptions */

	for (pflag = flag_msg; (flag = pflag->flag) != '\0'; pflag++) {
		if (flag == '\n') {	/* newline?  print and quit */
			FPR(fp, "\n");
			continue;
		}
		p = buf;		/* copy flag and metavariable to buffer */
		if (flag != ' ')
			*p++ = '-';
	/* special hack for VMS - surround upper-case flags in quotes */
#ifdef VMS
		if (isupper(flag)) {
			*p++ = '"';
			*p++ = flag;
			*p++ = '"';
		}
		else
			*p++ = flag;
#else
		*p++ = flag;
#endif
		*p = '\0';
		if (pflag->meta)
			sprintf(p, " %s", pflag->meta);
		FPR(fp, "\t%-16.16s", buf);
		if (pflag->text)
			FPR(fp, "%s", pflag->text);

		/* print default value if specified */
		if (pflag->def)
			FPR(fp, " (%s: %s)", W_DEFAULT, pflag->def[0] ? pflag->def : "\"\"" );
		FPR(fp, "\n");


	}
	
	/* now print the information about the numeric parameters */

	for (ppar = param_msg; ppar->desc; ppar++)
		FPR(fp, "\t%-16.16s%s\n", ppar->desc, ppar->text);
	
	FPR(fp, "\n");

}


/*
 * write_psfile - write PostScript code to stdout
 *
 * The actual output of the PostScript code is straightforward.  This routine
 * writes a PostScript header followed by declarations of all the PostScript
 * variables affected by command-line flags and/or language dependencies.  It
 * then generates the PostScript boilerplate generated from lcal_[lp].ps, and
 * finally prints the moon phase information for the year.
 */
void
#ifdef PROTOS
write_psfile(int year)
#else
write_psfile(year)
	int year;			/* starting year    */
#endif
{
	int month, day, fudge1, fudge2, invert_moons;
	double phase;
	char **ap, *off, *p, tmp[STRSIZ];
	static char *cond[2] = {"false", "true"};
	struct tm *p_tm;
	time_t t;
#ifdef UN_X
	struct passwd *pw;
#endif

/* make sure printf() doesn't round "ph" up to 1.0 when printing it */
#define PRT_TWEAK(ph)		((ph) >= 0.9995 ? 0.0 : (ph))

	/*
	 * Write out PostScript prolog
	 */

	/* comment block at top */

 	PRT("%%!%s\n", PS_RELEASE);		/* PostScript release */

	PRT("%%%%CreationDate: ");		/* date/time stamp */
	p_tm = localtime((time(&t), &t));
	PRT("%d/%d/", p_tm->tm_mon + 1, p_tm->tm_mday);
	PRT("%02d %02d:%02d:%02d\n", p_tm->tm_year % 100, p_tm->tm_hour,
		p_tm->tm_min, p_tm->tm_sec);
	PRT("%%%%Creator: %s %s %s\n", CREATED_MSG, progname, version);

#ifdef UN_X
	if ((pw = getpwuid(getuid())) != NULL &&
	    strcmp(pw->pw_name, NOBODY) != 0) {
		strcpy(tmp, pw->pw_gecos);
		if ((p = strchr(tmp, ',')) != NULL)
			*p = '\0';
		PRT("%%%%For: %s\n", pw->pw_name);
		PRT("%%%%Routing: %s\n", tmp);
	}
#endif

	/* Miscellaneous other identification */

	PRT("%%%%Title: %s %d\n", TITLE_MSG, init_year);
	PRT("%%%%Pages: %d\n", singlepage ? 1 : 2);
	PRT("%%%%PageOrder: Ascend\n");
	PRT("%%%%Orientation: %s\n", rotate == LANDSCAPE ? "Landscape" :
							   "Portrait");
	PRT("%%%%BoundingBox: 0 0 612 792\n");
	PRT("%%%%ProofMode: NotifyMe\n");
	PRT("%%%%EndComments\n");

	/* advertisement for original inspiration */

	PRT("%%\n");
	PRT("%% Lcal was inspired by \"Moonlight 1996\", a 16\" x 36\" full-color (silver\n");
	PRT("%% moons against a midnight blue background) lunar phase calendar marketed\n");
	PRT("%% by Celestial Products, Inc., P.O. Box 801, Middleburg VA  22117.  Send\n");
	PRT("%% for their catalog to see (and, hopefully, order) this as well as some\n");
	PRT("%% even more amazing stuff - particularly \"21st Century Luna\", a lunar\n");
	PRT("%% phase calendar for *every day* of the upcoming century.\n");
	PRT("%%\n");
	PRT("%% Or visit Celestial Products' site:\n");
	PRT("%%\n");
	PRT("%%   http://www.celestialproducts.com\n");
	PRT("%%\n\n");

	/* font names and sizes */

	PRT("/titlefont /%s def\n/dayfont /%s def\n",
	    titlefont, dayfont);

	PRT("/titlefontsize %d def\n", TITLEFONTSIZE);
	PRT("/datefontsize  %d def\n",
	     singlepage ? DATEFONTSIZE_S : DATEFONTSIZE);
	PRT("/monthfontsize %d def\n",
	     singlepage ? MONTHFONTSIZE_S : MONTHFONTSIZE);
	PRT("/weekdayfontsize     %d def\n", WKDFONTSIZE);
	PRT("/sm_weekdayfontsize  %d def\n",
	     singlepage ? SMWKDFONTSIZE_S : SMWKDFONTSIZE);

	/* month names */

	PRT("/month_names [");
	for (month = JAN; month <= DEC; month++) 
		PRT(" (%-3.3s)", months[month-JAN]);
	PRT(" ] def\n");

	/* day names - abbreviate if printing entire year on page */

	PRT("/day_names [");
	for (day = SUN; day <= SAT; day++)
		PRT(" (%-2.2s)", days[day-SUN]);
	PRT(" ] def\n");

	/* weekday flag */

	PRT("/draw_weekdays %s def\n", cond[weekdays]);

	/* fudge factors for X origin (landscape), Y origin (portrait) -
	 * theoretically unnecessary, but useful if your printer isn't
	 * aligned quite right and clips the edges
	 */
	off = rotate == LANDSCAPE ? x_offset : y_offset;
	fudge1 = atoi(off);
	fudge2 = (p = strchr(off, '/')) ? atoi(++p) : fudge1;
	PRT("/fudge1 %d def\n", singlepage ? 0 : fudge1);
	PRT("/fudge2 %d def\n", singlepage ? 0 : fudge2);

	/* misc. constants */
	PRT("/xsval %.1f def\n", singlepage ? HALF_SIZE : FULL_SIZE);
	PRT("/ysval %.1f def\n", singlepage ? HALF_SIZE : FULL_SIZE);
	PRT("/pagebreak %d def\n", singlepage ? PAGEBREAK_S : PAGEBREAK);

	/* background and foreground colors */

	strcpy(tmp, shading);
	*(p = strchr(tmp, '/')) = '\0';
	PRT("/setbackground { %s } def\n", set_rgb(tmp));
	PRT("/setforeground { %s } def\n", set_rgb(++p));

	/* disable duplex mode (if supported) */

	PRT("statusdict (duplexmode) known {\n");
	PRT("statusdict begin false setduplexmode end\n");
	PRT("} if\n");

	/* PostScript boilerplate (part 1 of 1) */

	for (ap = rotate == LANDSCAPE ? header_l : header_p; *ap; ap++)
		PRT("%s\n", *ap);
	PRT("\n");

	/*
	 * Write out PostScript code to print lunar calendar
	 */

	PRT("/year %d def\n", year);
	PRT("/startday [");
	for (month = JAN; month <= DEC; month++)
		PRT("%2d", FIRST_OF(month, year));
	PRT(" ] def\n");

	/* invert moon phases (actually shift by 1/2 cycle, i.e., print first
	 * quarter as third quarter, etc.) if background color is darker than
	 * foreground color - thus ensuring that the visible portion of the
	 * moon is printed in the lighter color
	 */
	invert_moons = is_bg_darker();

	PRT("/moon_phases [\n");
	for (day = 1; day <= 31; day++) {
		for (month = JAN; month <= DEC; month++) {
			phase = calc_phase(month, day, year);
			if (invert_moons)
				phase = FMOD(phase + 0.5, 1.0);
			if (day <= LENGTH_OF(month, year))
				PRT("%.3f ", PRT_TWEAK(phase));
			else
				PRT(" -1   ");
		}
		PRT("\n");
	}
	PRT("] def\n");

	PRT("\ncalendar\n");	/* start printing it... */
}

/*
 * is_bg_darker - return 1 if the shading string indicates that the background
 * is darker than the foreground (requiring that light/dark portions of moons
 * be reversed by shifting the phase by 1/2 cycle), 0 if not
 */
int
#ifdef PROTOS
is_bg_darker(void)
#else
is_bg_darker()
#endif
{
	static char tmp[STRSIZ];
	char *p, *p1, *p2;
	double bval, fval;
	int n;

	bval = fval = 0.0;

	strcpy(tmp, shading);
	*(p = strchr(tmp, '/')) = '\0';

	/* get average of 1 - 3 floating-point background values */
	for (n = 1, p1 = tmp; n <= 3; n++, p1 = p2 + 1) {
		bval += atof(p1);
		if ((p2 = strchr(p1, RGB_CHAR)) == NULL)
			break;
	}
	bval /= (n > 3 ? 3 : n);

	/* get average of 1 - 3 floating-point foreground values */
	for (n = 1, p1 = ++p; n <= 3; n++, p1 = p2 + 1) {
		fval += atof(p1);
		if ((p2 = strchr(p1, RGB_CHAR)) == NULL)
			break;
	}
	fval /= (n > 3 ? 3 : n);

	/* return 1 if background is darker (lower value; 0:0:0 is white,
	 * 1:1:1 is black) than foreground
	 */
	return bval < fval;
}


/* Misc. utility routines borrowed from Pcal */

/*
 * set_rgb - convert "<r>:<g>:<b>" to "r g b setrgbcolor" or "<gray>" to
 * "gray setgray"; return pointer to static buffer containing converted string
 */
char *
#ifdef PROTOS
set_rgb(char *s)
#else
set_rgb(s)
	char *s;
#endif
{
	static char buf[STRSIZ];
	char *p1, *p2;
	double val[3];
	int n;

	val[0] = val[1] = val[2] = 0;		/* defaults */

	/* extract 1 - 3 floating-point values from string */
	for (n = 1, p1 = s; n <= 3; n++, p1 = p2 + 1) {
		val[n-1] = atof(p1);
		if ((p2 = strchr(p1, RGB_CHAR)) == NULL)
			break;
	}

	/* single value is gray scale; assume anything else is [r g b] */
	if (n > 1)
		sprintf(buf, "%.3f %.3f %.3f setrgbcolor", val[0], val[1],
			val[2]);
	else
		sprintf(buf, "%.3f setgray", val[0]);

	return buf; 
}

void
#ifdef PROTOS
define_shading(register char *orig_shading,
	       register char *new_shading,
	       register char *dflt_shading)
#else
define_shading(orig_shading, new_shading, dflt_shading)
	register char *orig_shading;
	register char *new_shading;
	register char *dflt_shading;
#endif
{
	char *orig_bg, *new_bg, *dflt_bg, *orig_fg, *new_fg, *dflt_fg;
	char tmp1[STRSIZ], tmp2[STRSIZ], tmp3[STRSIZ];
	double v;

	/* use default date/fill if new _shading is null */
	if (new_shading == NULL || new_shading[0] == '\0') {
		strcpy(orig_shading, dflt_shading);
		return;
	}

	/* split old, new, and default shadings into background/foreground
	 * components
	 */
	SPLIT(orig_shading, orig_bg, orig_fg, '/');

	new_shading = strcpy(tmp1, new_shading);
	SPLIT(new_shading, new_bg, new_fg, '/');

	dflt_shading = strcpy(tmp2, dflt_shading);
	SPLIT(dflt_shading, dflt_bg, dflt_fg, '/');

	/* replace invalid fields from new shading with default values */
	if (new_fg[0] && strchr(new_fg, RGB_CHAR) == NULL &&
	    ((v = atof(new_fg)) <= 0.0 || v > 1.0))
		new_fg = dflt_fg;
	if (new_bg[0] && strchr(new_bg, RGB_CHAR) == NULL &&
	    ((v = atof(new_bg)) <= 0.0 || v > 1.0))
		new_bg = dflt_bg;

	/* replace fields of old shading with specified fields from new */

	strcpy(tmp3, new_bg[0] ? new_bg : orig_bg);
	strcat(tmp3, "/");
	strcat(tmp3, new_fg[0] ? new_fg : orig_fg);

	strcpy(orig_shading, tmp3);
}

/*
 * calc_weekday - return the weekday (0-6) of mm/dd/yy (mm: 1-12)
 */
int
#ifdef PROTOS
calc_weekday(int mm,
	     int dd,
	     int yy)
#else
calc_weekday(mm, dd, yy)
	int mm;
	int dd;
	int yy;
#endif
{
	return (yy + (yy-1)/4 - (yy-1)/100 + (yy-1)/400 + OFFSET_OF(mm, yy) +
		(dd-1)) % 7;
}

/*
 * loadwords - tokenize buffer 'buf' into array 'words' and return word count;
 * differs from old loadwords() in that it handles quoted (" or ') strings
 * and removes escaped quotes
 */
int
#ifdef PROTOS
loadwords(char **words,
	  char *buf)
#else
loadwords(words, buf)
	char **words;
	char *buf;
#endif
{
	register char *ptok;
	char *delim, **ap, *p1, *p2, c;
	int nwords;

	for (ptok = buf, ap = words; TRUE; ap++) {

		ptok += strspn(ptok, WHITESPACE); /* find next token */

		if (! *ptok) {			/* end of buf? */
			*ap = NULL;		/* add null ptr at end */
			nwords = ap - words;	/* number of non-null ptrs */
			break;			/* exit loop */
			}

		delim =	*ptok == '"'  ? "\"" :	/* set closing delimiter */
			*ptok == '\'' ? "'"  :
			WHITESPACE;

		/* split flag followed by quoted string into two words */
		if (*ptok == '-' && isalpha(ptok[1]) &&
		    ((c = ptok[2]) == '"' || c == '\'')) {
			delim = c == '"' ? "\"" : "'";
			*ap++ = ptok;
			ptok[2] = '\0';		/* terminate first token */
			ptok += 3;		/* start second token */
		}
		else if (*ptok == *delim)
			ptok++;			/* skip opening quote */

		*ap = ptok;			/* save token ptr */

		/* find first unescaped string delimiter - handle special
		 * case where preceding backslash is itself escaped
		 */
		do {
			ptok += strcspn(ptok, delim);
			if ((c = ptok[-1]) == '\\')	{
				if (ptok[-2] == '\\')	{
					break;	/* stop on \\" or \\' */
				} else	{
					ptok++;
				}
			}
		} while (c == '\\');

		if (*ptok)			/* terminate token */
			*ptok++ = '\0';
	}

	/* now reprocess the word list, removing remaining escapes */
	for (ap = words; *ap; ap++)
		for (p1 = p2 = *ap; (c = *p2 = *p1++); p2++)
			if (c == '\\')
				*p2 = *p1++;

	return nwords;				/* return word count */

}

/*
 *  The following suite of routines are to calculate the phase of the moon
 *  for a given month, day, year.  They compute the phase of the moon for
 *  noon (UT) on the day requested, the start of the Julian day.
 *
 *  Revision history:
 *
 *	1.2	AWR	01/25/00	fix calculation of default year
 *
 *	1.0	AWR	01/26/92	from Pcal v4.4
 *
 */

/*  Astronomical constants. */

#define epoch	    2444238.5	   /* 1980 January 0.0 */

/*  Constants defining the Sun's apparent orbit. */

#define elonge	    278.833540	   /* ecliptic longitude of the Sun
				        at epoch 1980.0 */
#define elongp	    282.596403	   /* ecliptic longitude of the Sun at
				        perigee */
#define eccent      0.016718       /* eccentricity of Earth's orbit */

/*  Elements of the Moon's orbit, epoch 1980.0. */

#define mmlong      64.975464      /* moon's mean lonigitude at the epoch */
#define mmlongp     349.383063	   /* mean longitude of the perigee at the
                                        epoch */
#define mlnode	    151.950429	   /* mean longitude of the node at the
				        epoch */
#define synmonth    29.53058868    /* synodic month (new Moon to new Moon) */

#define PI 3.14159265358979323846  /* assume not near black hole nor in
				        Tennessee ;) */


/*  Handy mathematical functions. */

#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))	  /* extract sign */
#ifndef abs
#define abs(x) ((x) < 0 ? (-(x)) : (x)) 		  /* absolute val */
#endif
#define fixangle(a) ((a) - 360.0 * (floor((a) / 360.0)))  /* fix angle	  */
#define torad(d) ((d) * (PI / 180.0))			  /* deg->rad	  */
#define todeg(d) ((d) * (180.0 / PI))			  /* rad->deg	  */
#define dsin(x) (sin(torad((x))))			  /* sin from deg */
#define dcos(x) (cos(torad((x))))			  /* cos from deg */
#define FNITG(x) (sgn (x) * floor (abs (x)))


/*
 *  Routines to accurately calculate the phase of the moon
 *
 *  Originally adapted from "moontool.c" by John Walker, Release 2.0.
 *
 *      This routine (calc_phase) and its support routines were adapted from
 *      phase.c (v 1.2 88/08/26 22:29:42 jef) in the program "xphoon"        
 *      (v 1.9 88/08/26 22:29:47 jef) by Jef Poskanzer and Craig Leres.
 *      The necessary notice follows...
 *
 *      Copyright (C) 1988 by Jef Poskanzer and Craig Leres.
 *
 *      Permission to use, copy, modify, and distribute this software and its
 *      documentation for any purpose and without fee is hereby granted,
 *      provided that the above copyright notice appear in all copies and that
 *      both that copyright notice and this permission notice appear in
 *      supporting documentation.  This software is provided "as is" without
 *      express or implied warranty.
 *
 *      These were added to "pcal" by RLD on 19-MAR-1991
 */


/*
 *  julday -- calculate the julian date from input month, day, year
 *      N.B. - The Julian date is computed for noon UT.
 *
 *      Adopted from Peter Duffett-Smith's book `Astronomy With Your
 *      Personal Computer' by Rick Dyson 18-MAR-1991
 */
static double
#ifdef PROTOS
julday(int month,
       int day,
       int year)
#else
julday(month, day, year)
	int month, day, year;
#endif
{
        int mn1, yr1;
        double a, b, c, d, djd;

        mn1 = month;
        yr1 = year;
        if ( yr1 < 0 ) yr1 = yr1 + 1;
        if ( month < 3 )
            {
    	        mn1 = month + 12;
    	        yr1 = yr1 - 1;
            }
        if (( year < 1582 ) ||
            ( year == 1582  && month < 10 ) ||
                ( year == 1582  && month == 10 && day < 15.0 ))
                    b = 0;
                else
                {
                    a = floor (yr1 / 100.0);
                    b = 2 - a + floor (a / 4);
                }
        if ( yr1 >= 0 )
            c = floor (365.25 * yr1) - 694025;
        else
            c = FNITG ((365.25 * yr1) - 0.75) - 694025;
        d = floor (30.6001 * (mn1 + 1));
        djd = b + c + d + day + 2415020.0;
        return djd;
}


/*
 *  kepler - solve the equation of Kepler
 */
static double
#ifdef PROTOS
kepler(double m,
       double ecc)
#else
kepler(m, ecc)
	double m, ecc;
#endif
{
	double e, delta;
#define EPSILON 1E-6

	e = m = torad(m);
	do {
	   delta = e - ecc * sin(e) - m;
	   e -= delta / (1 - ecc * cos(e));
	} while (abs(delta) > EPSILON);
	return e;
}


/*
 *  calc_phase - calculate phase of moon as a fraction:
 *
 *	The argument is the time for which the phase is requested,
 *	expressed as the month, day and year.  It returns the phase of
 *	the moon (0.0 -> 0.99) with the ordering as New Moon, First Quarter,
 *      Full Moon, and Last Quarter.
 *
 *	Converted from the subroutine phase.c used by "xphoon.c" (see
 *      above disclaimer) into calc_phase() for use in "moonphas.c"
 *	by Rick Dyson 18-MAR-1991
 */
double
#ifdef PROTOS
calc_phase(int month,
	   int inday,
	   int year)
#else
calc_phase(month, inday, year)
	int month, inday, year;
#endif
{
	double Day, N, M, Ec, Lambdasun, ml, MM, MN, Ev, Ae, A3, MmP,
	       mEc, A4, lP, V, lPP, MoonAge, pdate;
	static int first = TRUE;
	static double utc_offset;

	/* get offset from UTC first time through; normalize to +- 1/2 day */
	if (first) {
		utc_offset = FMOD(atof(time_zone), 24.0) / 24.0;
		if (utc_offset < 0.0)
			utc_offset++;
		if (utc_offset > 0.5)
			utc_offset -= 0.5;
		first = FALSE;
		}

	/*  need to convert month, day, year into a Julian pdate */
	pdate = julday (month, inday, year) + utc_offset;

        /*  Calculation of the Sun's position. */

	Day = pdate - epoch;			/* date within epoch */
	N = fixangle((360 / 365.2422) * Day);	/* mean anomaly of the Sun */
	M = fixangle(N + elonge - elongp);      /* convert from perigee
					         co-ordinates to epoch 1980.0 */
	Ec = kepler(M, eccent);			/* solve equation of Kepler */
	Ec = sqrt((1 + eccent) / (1 - eccent)) * tan(Ec / 2);
	Ec = 2 * todeg(atan(Ec));		/* true anomaly */
        Lambdasun = fixangle(Ec + elongp);	/* Sun's geocentric ecliptic
					         longitude */

        /*  Calculation of the Moon's position. */

        /*  Moon's mean longitude. */
	ml = fixangle(13.1763966 * Day + mmlong);

        /*  Moon's mean anomaly. */
	MM = fixangle(ml - 0.1114041 * Day - mmlongp);

        /*  Moon's ascending node mean longitude. */
	MN = fixangle(mlnode - 0.0529539 * Day);

	/*  Evection. */
	Ev = 1.2739 * sin(torad(2 * (ml - Lambdasun) - MM));

	/*  Annual equation. */
	Ae = 0.1858 * sin(torad(M));

	/*  Correction term. */
	A3 = 0.37 * sin(torad(M));

	/*  Corrected anomaly. */
	MmP = MM + Ev - Ae - A3;

	/*  Correction for the equation of the centre. */
	mEc = 6.2886 * sin(torad(MmP));

	/*  Another correction term. */
	A4 = 0.214 * sin(torad(2 * MmP));

	/*  Corrected longitude. */
	lP = ml + Ev + mEc - Ae + A4;

	/*  Variation. */
	V = 0.6583 * sin(torad(2 * (lP - Lambdasun)));

	/*  True longitude. */
	lPP = lP + V;

	/*  Calculation of the phase of the Moon. */

	/*  Age of the Moon in degrees. */
	MoonAge = lPP - Lambdasun;

        return (fixangle (MoonAge) / 360.0);
}
