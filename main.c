/*
 * Copyright (C) 1984-2012  Mark Nudelman
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Less License, as specified in the README file.
 *
 * For more information, see the README file.
 */
/*
 * Modified for use with illumos.
 * Copyright 2014 Garrett D'Amore <garrett@damore.org>
 */

/*
 * Entry point, initialization, miscellaneous routines.
 */

#include <libgen.h>
#include <stdarg.h>
#include <sys/types.h>
#include "less.h"

char	*every_first_cmd = NULL;
int	new_file;
int	is_tty;
IFILE	curr_ifile = NULL_IFILE;
IFILE	old_ifile = NULL_IFILE;
struct scrpos initial_scrpos;
int	any_display = FALSE;
off_t	start_attnpos = -1;
off_t	end_attnpos = -1;
int	wscroll;

static char 	*progname;

int	quitting;
int	secure;
int	dohelp;

int logfile = -1;
int force_logfile = FALSE;
char *namelogfile = NULL;
char *editor;
char *editproto;

extern char 	*tags;
extern char 	*tagoption;
extern int	jump_sline;
extern int	less_is_more;
extern int	missing_cap;
extern int	know_dumb;
extern int	quit_if_one_screen;
extern int	quit_at_eof;
extern int	pr_type;
extern int	errmsgs;


/*
 * Entry point.
 */
int
main(int argc, char *argv[])
{
	IFILE ifile;
	char *s;

	progname = basename(argv[0]);
	argv++;
	argc--;

	/*
	 * If the name of the executable program is "more",
	 * act like LESS_IS_MORE is set.  We have to set this as early
	 * as possible for POSIX.
	 */
	if ((strcmp(progname, "more") == 0) ||
	    (strcmp(progname, "page") == 0)) {
		less_is_more = 1;
	} else {
		s = lgetenv("LESS_IS_MORE");
		if (s != NULL && *s != '\0')
			less_is_more = 1;
	}

	secure = 0;
	s = lgetenv("LESSSECURE");
	if (s != NULL && *s != '\0')
		secure = 1;

	/*
	 * Process command line arguments and LESS environment arguments.
	 * Command line arguments override environment arguments.
	 */
	is_tty = isatty(1);
	get_term();
	init_cmds();
	init_charset();
	init_line();
	init_cmdhist();
	init_option();
	init_search();


	init_prompt();

	s = lgetenv(less_is_more ? "MORE" : "LESS");
	if (s != NULL)
		scan_option(save(s));

	if (less_is_more) {
		/* this is specified by XPG */
		quit_at_eof = OPT_ON;

		/* more users don't like the warning */
		know_dumb = OPT_ON;

		/* default prompt is medium */
		pr_type = OPT_ON;
	}

#define	isoptstring(s)	(((s)[0] == '-' || (s)[0] == '+') && (s)[1] != '\0')
	while (argc > 0 && (isoptstring(*argv) || isoptpending())) {
		s = *argv++;
		argc--;
		if (strcmp(s, "--") == 0)
			break;
		scan_option(s);
	}
#undef isoptstring

	if (isoptpending()) {
		/*
		 * Last command line option was a flag requiring a
		 * following string, but there was no following string.
		 */
		nopendopt();
		quit(QUIT_OK);
	}

	if (errmsgs) {
		quit(QUIT_ERROR);
	}
	if (less_is_more && quit_at_eof == OPT_ONPLUS) {
		extern int no_init;
		no_init = OPT_ON;
	}
	if (less_is_more && pr_type == OPT_ONPLUS) {
		extern int quiet;
		quiet = VERY_QUIET;
	}

	editor = lgetenv("VISUAL");
	if (editor == NULL || *editor == '\0') {
		editor = lgetenv("EDITOR");
		if (editor == NULL || *editor == '\0')
			editor = EDIT_PGM;
	}
	editproto = lgetenv("LESSEDIT");
	if (editproto == NULL || *editproto == '\0')
		editproto = "%E ?lm+%lm. %f";

	/*
	 * Call get_ifile with all the command line filenames
	 * to "register" them with the ifile system.
	 */
	ifile = NULL_IFILE;
	if (dohelp)
		ifile = get_ifile(FAKE_HELPFILE, ifile);
	while (argc-- > 0) {
		char *filename;
		filename = shell_quote(*argv);
		if (filename == NULL)
			filename = *argv;
		argv++;
		(void) get_ifile(filename, ifile);
		ifile = prev_ifile(NULL_IFILE);
		free(filename);
	}
	/*
	 * Set up terminal, etc.
	 */
	if (!is_tty) {
		/*
		 * Output is not a tty.
		 * Just copy the input file(s) to output.
		 */
		if (nifile() == 0) {
			if (edit_stdin() == 0)
				cat_file();
		} else if (edit_first() == 0) {
			do {
				cat_file();
			} while (edit_next(1) == 0);
		}
		quit(QUIT_OK);
	}

	if (missing_cap && !know_dumb)
		error("WARNING: terminal is not fully functional", NULL_PARG);
	init_mark();
	open_getchr();
	raw_mode(1);
	init_signals(1);

	/*
	 * Select the first file to examine.
	 */
	if (tagoption != NULL || strcmp(tags, "-") == 0) {
		/*
		 * A -t option was given.
		 * Verify that no filenames were also given.
		 * Edit the file selected by the "tags" search,
		 * and search for the proper line in the file.
		 */
		if (nifile() > 0) {
			error("No filenames allowed with -t option", NULL_PARG);
			quit(QUIT_ERROR);
		}
		findtag(tagoption);
		if (edit_tagfile())  /* Edit file which contains the tag */
			quit(QUIT_ERROR);
		/*
		 * Search for the line which contains the tag.
		 * Set up initial_scrpos so we display that line.
		 */
		initial_scrpos.pos = tagsearch();
		if (initial_scrpos.pos == -1)
			quit(QUIT_ERROR);
		initial_scrpos.ln = jump_sline;
	} else if (nifile() == 0) {
		if (edit_stdin())  /* Edit standard input */
			quit(QUIT_ERROR);
	} else {
		if (edit_first())  /* Edit first valid file in cmd line */
			quit(QUIT_ERROR);
	}

	init();
	commands();
	quit(QUIT_OK);
	/*NOTREACHED*/
	return (0);
}

/*
 * Copy a string to a "safe" place
 * (that is, to a buffer allocated by calloc).
 */
char *
save(const char *s)
{
	char *p;

	if ((p = estrdup(s)) == NULL)
		quit(QUIT_ERROR);
	return (p);
}

/*
 * Allocate memory.
 * Like calloc(), but never returns an error (NULL).
 */
void *
ecalloc(int count, unsigned int size)
{
	void *p;

	p = calloc(count, size);
	if (p != NULL)
		return (p);
	error("Cannot allocate memory", NULL_PARG);
	quit(QUIT_ERROR);
	/*NOTREACHED*/
	return (NULL);
}

#ifndef	HAVE_VASPRINTF
static int
x_vasprintf(char **p, const char *fmt, va_list ap)
{
	size_t sz;
	va_list copy;
	int rv;

	va_copy(copy, ap);
	sz = vsnprintf(NULL, 0, fmt, ap) + 1;
	if ((sz != 0) && ((*p = malloc(sz)) == NULL)) {
		va_end(copy);
		return (-1);
	}
	rv = vsnprintf(*p, sz, fmt, ap);
	va_end(copy);
	return (rv);
}
#else
#define	x_vasprintf	vasprintf
#endif

char *
easprintf(const char *fmt, ...)
{
	char *p = NULL;
	int rv;
	va_list ap;

	va_start(ap, fmt);
	rv = x_vasprintf(&p, fmt, ap);
	va_end(ap);

	if (p == NULL || rv < 0) {
		error("Cannot allocate memory", NULL_PARG);
		quit(QUIT_ERROR);
		/*NOTREACHED*/
	}
	return (p);
}

char *
estrdup(const char *str)
{
	char *n = strdup(str);
	if (n == NULL && str != NULL) {
		error("Cannot allocate memory", NULL_PARG);
	}
	return (n);
}

/*
 * Skip leading spaces in a string.
 */
char *
skipsp(char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	return (s);
}

/*
 * See how many characters of two strings are identical.
 * If uppercase is true, the first string must begin with an uppercase
 * character; the remainder of the first string may be either case.
 */
int
sprefix(char *ps, char *s, int uppercase)
{
	int c;
	int sc;
	int len = 0;

	for (; *s != '\0';  s++, ps++) {
		c = *ps;
		if (uppercase) {
			if (len == 0 && isupper(c))
				return (-1);
			if (isupper(c))
				c = tolower(c);
		}
		sc = *s;
		if (len > 0 && isupper(sc))
			sc = tolower(sc);
		if (c != sc)
			break;
		len++;
	}
	return (len);
}

/*
 * Exit the program.
 */
void
quit(int status)
{
	static int save_status;

	/*
	 * Put cursor at bottom left corner, clear the line,
	 * reset the terminal modes, and exit.
	 */
	if (status < 0)
		status = save_status;
	else
		save_status = status;
	quitting = 1;
	edit(NULL);
	save_cmdhist();
	if (any_display && is_tty)
		clear_bot();
	deinit();
	flush();
	raw_mode(0);
	close_getchr();
	exit(status);
}
