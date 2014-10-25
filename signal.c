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
 * Routines dealing with signals.
 *
 * A signal usually merely causes a bit to be set in the "signals" word.
 * At some convenient time, the mainline code checks to see if any
 * signals need processing by calling psignal().
 * If we happen to be reading from a file [in iread()] at the time
 * the signal is received, we call intread to interrupt the iread.
 */

#include "less.h"
#include <signal.h>

/*
 * "sigs" contains bits indicating signals which need to be processed.
 */
int sigs;

extern int sc_width, sc_height;
extern int screen_trashed;
extern int linenums;
extern int wscroll;
extern int reading;
extern int quit_on_intr;
extern long jump_sline_fraction;

/*
 * Interrupt signal handler.
 */
/* ARGSUSED */
static void
u_interrupt(int type)
{
	ring_bell();
	LSIGNAL(SIGINT, u_interrupt);
	sigs |= S_INTERRUPT;
	if (reading)
		intread(); /* May longjmp */
}

/*
 * "Stop" (^Z) signal handler.
 */
/* ARGSUSED */
static void
stop(int type)
{
	LSIGNAL(SIGTSTP, stop);
	sigs |= S_STOP;
	if (reading)
		intread();
}

/*
 * "Window" change handler
 */
/* ARGSUSED */
void
sigwinch(int type)
{
	LSIGNAL(SIGWINCH, sigwinch);
	sigs |= S_WINCH;
	if (reading)
		intread();
}

/*
 * Set up the signal handlers.
 */
void
init_signals(int on)
{
	if (on) {
		/*
		 * Set signal handlers.
		 */
		(void) LSIGNAL(SIGINT, u_interrupt);
		(void) LSIGNAL(SIGTSTP, stop);
		(void) LSIGNAL(SIGWINCH, sigwinch);
		(void) LSIGNAL(SIGQUIT, SIG_IGN);
	} else {
		/*
		 * Restore signals to defaults.
		 */
		(void) LSIGNAL(SIGINT, SIG_DFL);
		(void) LSIGNAL(SIGTSTP, SIG_DFL);
		(void) LSIGNAL(SIGWINCH, SIG_IGN);
		(void) LSIGNAL(SIGQUIT, SIG_DFL);
	}
}

/*
 * Process any signals we have received.
 * A received signal cause a bit to be set in "sigs".
 */
void
psignals(void)
{
	register int tsignals;

	if ((tsignals = sigs) == 0)
		return;
	sigs = 0;

	if (tsignals & S_STOP) {
		/*
		 * Clean up the terminal.
		 */
		LSIGNAL(SIGTTOU, SIG_IGN);
		clear_bot();
		deinit();
		flush();
		raw_mode(0);
		LSIGNAL(SIGTTOU, SIG_DFL);
		LSIGNAL(SIGTSTP, SIG_DFL);
		kill(getpid(), SIGTSTP);
		/*
		 * ... Bye bye. ...
		 * Hopefully we'll be back later and resume here...
		 * Reset the terminal and arrange to repaint the
		 * screen when we get back to the main command loop.
		 */
		LSIGNAL(SIGTSTP, stop);
		raw_mode(1);
		init();
		screen_trashed = 1;
		tsignals |= S_WINCH;
	}
	if (tsignals & S_WINCH) {
		int old_width, old_height;
		/*
		 * Re-execute scrsize() to read the new window size.
		 */
		old_width = sc_width;
		old_height = sc_height;
		get_term();
		if (sc_width != old_width || sc_height != old_height) {
			wscroll = (sc_height + 1) / 2;
			calc_jump_sline();
			calc_shift_count();
			screen_trashed = 1;
		}
	}
	if (tsignals & S_INTERRUPT) {
		if (quit_on_intr)
			quit(QUIT_INTERRUPT);
	}
}
