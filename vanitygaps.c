/* Key binding functions */
static void defaultgaps(const Arg *arg);
static void incrgaps(const Arg *arg);
static void incrigaps(const Arg *arg);
static void incrogaps(const Arg *arg);
static void togglegaps(const Arg *arg);
/* Layouts */
static void centeredmaster(Monitor *m);
static void centeredfloatingmaster(Monitor *m);
/* Internals */
static void getgaps(Monitor *m, int *og, int *ig, unsigned int *nc);
static void getfacts(Monitor *m, int msize, int ssize, float *mf, float *sf, int *mr, int *sr);
static void setgaps(int og, int ig);

/* Settings */
static int enablegaps = 1;

void
setgaps(int og, int ig)
{
    Monitor *m;
    for (m = mons; m; m = m->next) {
        if (og < 0) og = 0;
        if (ig < 0) ig = 0;
        m->gappo = og;
        m->gappi = ig;
        arrange(m);
    }
}

void
togglegaps(const Arg *arg)
{
	enablegaps = !enablegaps;
	arrange(NULL);
}

void
defaultgaps(const Arg *arg)
{
	setgaps(gappo, gappi);
}

void
incrgaps(const Arg *arg)
{
	setgaps(
		selmon->gappo + arg->i,
		selmon->gappi + arg->i
	);
}

void
incrigaps(const Arg *arg)
{
	setgaps(
		selmon->gappo,
		selmon->gappi + arg->i
	);
}

void
incrogaps(const Arg *arg)
{
	setgaps(
		selmon->gappo + arg->i,
		selmon->gappi
	);
}

void
getgaps(Monitor *m, int *og, int *ig, unsigned int *nc)
{
	unsigned int n, oe, ie;
	oe = ie = enablegaps;
	Client *c;

	for (n = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), n++);
	if (smartgaps && n == 1) {
		oe = 0; // outer gaps disabled when only one client
	}

	*og = m->gappo*oe; // outer gap
	*ig = m->gappi*ie; // inner gap
	*nc = n;            // number of clients
}

void
getfacts(Monitor *m, int msize, int ssize, float *mf, float *sf, int *mr, int *sr)
{
	unsigned int n;
	float mfacts, sfacts;
	int mtotal = 0, stotal = 0;
	Client *c;

	for (n = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), n++);
	mfacts = MIN(n, m->nmaster);
	sfacts = n - m->nmaster;

	for (n = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), n++)
		if (n < m->nmaster)
			mtotal += msize / mfacts;
		else
			stotal += ssize / sfacts;

	*mf = mfacts; // total factor of master area
	*sf = sfacts; // total factor of stack area
	*mr = msize - mtotal; // the remainder (rest) of pixels after an even master split
	*sr = ssize - stotal; // the remainder (rest) of pixels after an even stack split
}

/***
 * Layouts
 */

/*
 * Centred master layout + gaps
 * https://dwm.suckless.org/patches/centeredmaster/
 */
void
centeredmaster(Monitor *m)
{
	unsigned int i, n;
	int og, ig;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int lx = 0, ly = 0, lw = 0, lh = 0;
	int rx = 0, ry = 0, rw = 0, rh = 0;
	float mfacts = 0, lfacts = 0, rfacts = 0;
	int mtotal = 0, ltotal = 0, rtotal = 0;
	int mrest = 0, lrest = 0, rrest = 0;
	Client *c;

	getgaps(m, &og, &ig, &n);
	if (n == 0)
		return;

	/* initialize areas */
	mx = m->wx + og;
	my = m->wy + og;
	mh = m->wh - 2*og - ig * ((!m->nmaster ? n : MIN(n, m->nmaster)) - 1);
	mw = m->ww - 2*og;
	lh = m->wh - 2*og - ig * (((n - m->nmaster) / 2) - 1);
	rh = m->wh - 2*og - ig * (((n - m->nmaster) / 2) - ((n - m->nmaster) % 2 ? 0 : 1));

	if (m->nmaster && n > m->nmaster) {
		/* go mfact box in the center if more than nmaster clients */
		if (n - m->nmaster > 1) {
			/* ||<-S->|<---M--->|<-S->|| */
			mw = (m->ww - 2*og - 2*ig) * m->mfact;
			lw = (m->ww - mw - 2*og - 2*ig) / 2;
			rw = (m->ww - mw - 2*og - 2*ig) - lw;
			mx += lw + ig;
		} else {
			/* ||<---M--->|<-S->|| */
			mw = (mw - ig) * m->mfact;
			lw = 0;
			rw = m->ww - mw - ig - 2*og;
		}
		lx = m->wx + og;
		ly = m->wy + og;
		rx = mx + mw + ig;
		ry = m->wy + og;
	}

	/* calculate facts */
	for (n = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), n++) {
		if (!m->nmaster || n < m->nmaster)
			mfacts += 1;
		else if ((n - m->nmaster) % 2)
			lfacts += 1; // total factor of left hand stack area
		else
			rfacts += 1; // total factor of right hand stack area
	}

	for (n = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), n++)
		if (!m->nmaster || n < m->nmaster)
			mtotal += mh / mfacts;
		else if ((n - m->nmaster) % 2)
			ltotal += lh / lfacts;
		else
			rtotal += rh / rfacts;

	mrest = mh - mtotal;
	lrest = lh - ltotal;
	rrest = rh - rtotal;

	for (i = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), i++) {
		if (!m->nmaster || i < m->nmaster) {
			/* nmaster clients are stacked vertically, in the center of the screen */
			resize(c, mx, my, mw - (2*c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), 0);
			my += HEIGHT(c) + ig;
		} else {
			/* stack clients are stacked vertically */
			if ((i - m->nmaster) % 2 ) {
				resize(c, lx, ly, lw - (2*c->bw), (lh / lfacts) + ((i - 2*m->nmaster) < 2*lrest ? 1 : 0) - (2*c->bw), 0);
				ly += HEIGHT(c) + ig;
			} else {
				resize(c, rx, ry, rw - (2*c->bw), (rh / rfacts) + ((i - 2*m->nmaster) < 2*rrest ? 1 : 0) - (2*c->bw), 0);
				ry += HEIGHT(c) + ig;
			}
		}
	}
}

void
centeredfloatingmaster(Monitor *m)
{
	unsigned int i, n;
	float mfacts, sfacts;
	float msf = 1.0; // master size factor
	int og, ig, mrest, srest;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	Client *c;

	getgaps(m, &og, &ig, &n);
	if (n == 0)
		return;

	sx = mx = m->wx + og;
	sy = my = m->wy + og;
	sh = mh = m->wh - 2*og;
	mw = m->ww - 2*og - ig*(n - 1);
	sw = m->ww - 2*og - ig*(n - m->nmaster - 1);

	if (m->nmaster && n > m->nmaster) {
		msf = m->mfact * 1.1;
		/* go msf sized box in the center if more than nmaster clients */
		mw = (m->ww - 2*og) * msf - ig * msf * (m->nmaster - 1);
		mh = (m->wh - 2*og) * msf;
		mx = sx + (m->ww - 2*og) * (1 - msf) / 2;
		my = sy + (m->wh - 2*og) * (1 - msf) / 2;
	}

	getfacts(m, mw, sw, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->cl->clients, m); c; c = nexttiled(c->next, m), i++)
		if (i < m->nmaster) {
			/* nmaster clients are stacked horizontally, in the center of the screen */
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2*c->bw), mh - (2*c->bw), 0);
			mx += WIDTH(c) + ig * msf;
		} else {
			/* stack clients are stacked horizontally */
			resize(c, sx, sy, (sw / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2*c->bw), sh - (2*c->bw), 0);
			sx += WIDTH(c) + ig;
		}
}
