/*
 * protos.h - ANSI-style function prototypes for Lcal sources
 *
 * Revision history:
 *
 *	1.1	AWR	12/23/92	scrapped "noprotos.h" in favor of
 *					using PROTO() macro (below)
 *
 *	1.0	AWR	01/26/92	original version
 *
 */

/* macro to translate parenthesized argument list into ANSI- or K&R-style
 * function declaration, depending on definition of PROTOS - obviates need
 * for separate "noprotos.h" (now gone) containing K&R declarations
 */
#ifdef PROTOS
#define PROTO(args)     args            /* ANSI */
#else
#define PROTO(args)     ()              /* K&R  */
#endif

/*
 * Prototypes for functions defined in lcal.c:
 */
double	calc_phase PROTO((int month, int day, int year));
int	calc_weekday PROTO((int mm, int dd, int yy));
void	define_shading PROTO((char *orig_shading, char *new_shading,
			      char *dflt_shading));
int	get_args PROTO((char **argv, int curr_pass, char *where));
FLAG_USAGE *get_flag PROTO((char flag));
int	is_bg_darker PROTO((void));
int	loadwords PROTO((char **words, char *buf));
int	main PROTO((int argc, char **argv));
void	usage PROTO((FILE *fp, int fullmsg));
void	write_psfile PROTO((int year));
char	*set_rgb PROTO((char *s));

/*
 * Prototypes for miscellaneous library routines (if not already included
 * via <stdlib.h> - cf. lcaldefs.h):
 */
#ifndef STDLIB
extern double	atof PROTO((char *));
extern int	atoi PROTO((char *));
extern int	exit PROTO((int));
extern char	*getenv PROTO((char *));
#endif
