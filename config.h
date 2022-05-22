/* See LICENSE file for copyright and license details. */

#define TERMCLASS "st"

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappih    = 10;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 10;       /* vert inner gap between windows */
static const unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 10;       /* vert outer gap between windows and screen edge */
static       int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int user_bh            = 25;       /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const char *fonts[]          = {
	"JetBrains Mono Bold:size=16:antialias=true:autohint=true",
	"Noto Color Emoji:size=13:antialias=true:autohint=true"
};
static const char dmenufont[]       = "monospace:size=14";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const unsigned int baralpha = 0x50;
static const unsigned int borderalpha = OPAQUE;
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
	[SchemeStatus]  = { col_gray3, col_gray1,  "#000000"  }, // Statusbar right {text,background,not used but cannot be empty}
	[SchemeTagsSel]  = { col_gray4, col_gray1,  "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
	[SchemeTagsNorm]  = { col_gray3, col_gray1,  "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
	[SchemeInfoSel]  = { col_gray4, col_gray1,  "#000000"  }, // infobar middle  selected {text,background,not used but cannot be empty}
	[SchemeInfoNorm]  = { col_gray3, col_gray1,  "#000000"  }, // infobar middle  unselected {text,background,not used but cannot be empty}
};
static const unsigned int alphas[][3]      = {
	/*               fg      bg        border     */
	[SchemeNorm] = { OPAQUE, baralpha, borderalpha },
	[SchemeSel]  = { OPAQUE, baralpha, borderalpha },
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;

const char *sptermcmd[] = {"st", "-n", "spterm", "-t", "Terminal", "-g", "100x14", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"spterm",  sptermcmd}
};

/* status bar */
static const Block blocks[] = {
	/* command   interval  signal */
	{ "kstt b",  5,        1},
	{ "kstt t",  60,       2}
};

/* delimeter between blocks commands. NULL character ('\0') means no delimeter. */
static char delimiter[] = "";
/* max number of character that one block command can output */
#define CMDLENGTH 50

/* tagging */
static const char *tags[] = { "🔴", "🟢", "🔵", "⚪" };

static const unsigned int ulinepad     = 5;    /* horizontal padding between the underline and tag */
static const unsigned int ulinestroke  = 2;    /* thickness / height of the underline */
static const unsigned int ulinevoffset = 0;    /* how far above the bottom of the bar the line should appear */
static const int ulineall              = 0;    /* 1 to show underline on all tags, 0 for just the active ones */

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class         instance   title  tags mask  isfloating  isterminal  noswallow  monitor */
	{ TERMCLASS,     NULL,      NULL,  0,         0,          1,           0,        -1 },
	{ TERMCLASS,     "spterm",  NULL,  SPTAG(0),  1,          1,           0,        -1 }
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

#include "vanitygaps.c"

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "🔳",      centeredmaster },
	{ "🔲",      centeredfloatingmaster },
	{ "⬜",      NULL }
};

/* key definitions */
#define MODKEY Mod1Mask
#define ALTMOD Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* commands */
static char runmon[2] = "0"; /* component of runcmd, manipulated in spawn() */
static const char *runcmd[] = { "krun", "-m", runmon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", "-t", "Terminal", NULL };
static const char *browsercmd[]  = { "kffx", NULL };
static const char *dmonitorcmd[]  = { "kdmo", NULL };
static const char *smonitorcmd[]  = { "ksmo", NULL };
static const char *shutdowncmd[]  = { "shutdown", "now", NULL };
static const char *restartcmd[]  = { "shutdown", "-r", "now", NULL };

static Key keys[] = {
	{ MODKEY|ControlMask,           XK_comma,        spawn,          {.v = runcmd } },
	{ MODKEY|ControlMask,           XK_h,            spawn,          {.v = termcmd } },
	{ MODKEY|ControlMask,           XK_l,            spawn,          {.v = browsercmd } },
	{ MODKEY|ControlMask,           XK_y,            togglescratch,  {.ui = 0 } },
	{ MODKEY|ShiftMask,             XK_m,            togglefullscr,  {0} },
	{ MODKEY|ShiftMask,             XK_n,            togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_b,            togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_u,            incrgaps,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_i,            incrgaps,       {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_y,            defaultgaps,    {0} },
	{ MODKEY|ShiftMask,             XK_o,            togglegaps,     {0} },
	{ MODKEY|ShiftMask|ControlMask, XK_u,            incrigaps,      {.i = +1 } },
	{ MODKEY|ShiftMask|ControlMask, XK_i,            incrigaps,      {.i = -1 } },
	{ MODKEY|ShiftMask|ControlMask, XK_y,            incrogaps,      {.i = +1 } },
	{ MODKEY|ShiftMask|ControlMask, XK_o,            incrogaps,      {.i = -1 } },
	{ MODKEY,                       XK_m,            zoom,           {0} },
	{ MODKEY,                       XK_j,            focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,            focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_h,            setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,            setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_y,            setmfact,       {.f = +1.00} },
	{ MODKEY|ShiftMask,             XK_k,            incnmaster,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_j,            incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_u,            incnmaster,     {.i = +0 } },
	{ MODKEY|ShiftMask,             XK_bracketleft,  killclient,     {0} },
	{ MODKEY|ShiftMask|ControlMask, XK_m,            setlayout,      {.v = &layouts[0]} },
	{ MODKEY|ShiftMask|ControlMask, XK_n,            setlayout,      {.v = &layouts[1]} },
	{ MODKEY|ShiftMask|ControlMask, XK_b,            setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_6,            view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_6,            tag,            {.ui = ~0 } },
	{ ALTMOD,                       XK_m,            focusmon,       {.i = +1 } },
	{ ALTMOD|ShiftMask,             XK_m,            spawn,          {.v = dmonitorcmd } },
	{ ALTMOD|ShiftMask|ControlMask, XK_m,            spawn,          {.v = smonitorcmd } },
	{ MODKEY|ShiftMask,             XK_bracketright, quit,           {0} },
	{ MODKEY|ShiftMask|ControlMask, XK_bracketleft,  spawn,          {.v = shutdowncmd } },
	{ MODKEY|ShiftMask|ControlMask, XK_bracketright, spawn,          {.v = restartcmd } },
	TAGKEYS(                        XK_7,                      0)
	TAGKEYS(                        XK_8,                      1)
	TAGKEYS(                        XK_9,                      2)
	TAGKEYS(                        XK_0,                      3)
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
};

