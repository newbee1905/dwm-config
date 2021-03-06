/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappih    = 5;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 10;       /* vert inner gap between windows */
static const unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 10;       /* vert outer gap between windows and screen edge */
static       int smartgaps          = 1;        /* 1 means no outer gap when there is only one window */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int user_bh            = 24;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const int usealtbar          = 0;        /* 1 means use non-dwm status bar */
static const char *altbarclass      = "Bar";    /* Alternate bar class name */
static const char *altbarcmd        = "$HOME/bar"; /* Alternate bar launch command */
static const char *fonts[]          = { "M+ 2m:size=16:antialias=true:autohint=true", "JoyPixels:pixelsize=16:antialias=true:autohint=true" };

static const char col_gray1[]       = "#1d2021";
static const char col_gray2[]       = "#282828";
static const char col_gray3[]       = "#3c3836";
static const char col_gray4[]       = "#504945";
static const char col_purple[]      = "#83a598";
static const char col_urgborder[]   = "#fb4934";
static const char *colors[][3]      = {
	/*               fg         bg             border   			*/
	[SchemeNorm] = { col_gray4, col_gray1,     col_gray2 			},
	[SchemeSel]  = { col_gray3, col_purple,    col_purple 		},
	[SchemeUrg]  = { col_gray3, col_urgborder, col_urgborder  },
};

/**
* TODO: Think of a way to make all scratchpad only use one tag
*/
typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = {"kitty", "--name", "spterm", "-o", "initial_window_width=1280", "-o", "initial_window_height=720", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"spterm",      spcmd1},
	// {"spranger",    spcmd2},
	// {"keepassxc",   spcmd3},
};

/* tagging */
static const char *tags[] = { "???", "???", "???", "???", "???", "???", "???", "???", "???" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class         instance    title       tags mask     isfloating   isterminal  noswallow   monitor */
	{ "Gimp",        NULL,       NULL,       0,            1,           0,          0,          -1 },
	{ "firefox",     NULL,       NULL,       1 << 2,       0,           0,          -1,         -1 },
	{ "Firefox",     NULL,       NULL,       1 << 2,       0,           0,          -1,         -1 },
	{ "TelegramDesktop", NULL,   NULL,       1 << 1,       0,           0,          -1,         -1 },
	{ "discord",		 NULL,       NULL,       1 << 1,       0,           0,          -1,         -1 },
	{ "st-256color", NULL,       NULL,       0,            0,           1,          0,          -1 },
	{ "kitty",			 NULL,       NULL,       0,            0,           1,          0,          -1 },
	{ "St",					 NULL,       NULL,       0,            0,           1,          0,          -1 },
	{ "lemonbar",    NULL,       NULL,       0,            1,           0,          -1,          -1 },
	{ "Bar",         NULL,       NULL,       0,            1,           0,          -1,          -1 },
	{ NULL,          "spterm",   NULL,       SPTAG(0),     1,           1,          0,          -1 },
	{ "org-tlauncher-tlauncher-rmo-Bootstrapper",     NULL,       NULL,       1 << 3,       0,           0,          -1,         -1 },
	{ "org-tlauncher-tlauncher-rmo-Tlauncher",     NULL,       NULL,       1 << 3,       0,           0,          -1,         -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "|M|",      centeredmaster },
	{ ">M>",      centeredfloatingmaster },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define STACKKEYS(MOD,ACTION) \
	{ MOD, XK_j,     ACTION##stack, {.i = INC(+1) } }, \
	{ MOD, XK_k,     ACTION##stack, {.i = INC(-1) } }, \

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, NULL };
static const char *roficmd[]	= {
    "rofi",  "-modi", "combi", "-combi-modi", "window,drun,run,ssh",
    "-show", "combi", NULL };
static const char *kaomojicmd[]	= { "/home/newbee/Documents/Scripts/kaomoji-rofi/kaomoji.sh", NULL };
static const char *chokermancmd[]	= { "/home/newbee/Documents/Scripts/chokegenerator/chokegenerator", NULL };
static const char *termcmd[]  = { "kitty", NULL };
static const char *syncedtermcmd[]  = { "kitty", "-1", NULL };
static const char *upvol[]    = { "pamixer", "-i", "5",   NULL };
static const char *downvol[]  = { "pamixer", "-d", "5",   NULL };
static const char *mutevol[] =  { "pamixer", "-t",        NULL };

static const char *scrfullcmd[] = { "flameshot", "full", "-c", "-p", "~/Screenshots", NULL };
static const char *scrcmd[]			= { "flameshot", "gui", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = roficmd } },
	{ MODKEY|ControlMask,						XK_p,      spawn,          {.v = kaomojicmd } },
	{ MODKEY|ControlMask,						XK_c,      spawn,          {.v = chokermancmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = syncedtermcmd } },
	{ MODKEY|ControlMask,           XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ControlMask|ShiftMask, XK_p,      spawn,			     {.v = scrfullcmd } },
	{ MODKEY|ShiftMask, XK_p,       spawn,			               {.v = scrcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	STACKKEYS(MODKEY,                          focus)
	STACKKEYS(MODKEY|ShiftMask,                push)
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_h,      setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_l,      setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
	{ MODKEY|Mod4Mask,              XK_h,      incrgaps,       {.i = +1 } },
	{ MODKEY|Mod4Mask,              XK_l,      incrgaps,       {.i = -1 } },
	{ MODKEY|Mod4Mask|ShiftMask,    XK_h,      incrogaps,      {.i = +1 } },
	{ MODKEY|Mod4Mask|ShiftMask,    XK_l,      incrogaps,      {.i = -1 } },
	{ MODKEY|Mod4Mask|ControlMask,  XK_h,      incrigaps,      {.i = +1 } },
	{ MODKEY|Mod4Mask|ControlMask,  XK_l,      incrigaps,      {.i = -1 } },
	{ MODKEY|Mod4Mask,              XK_0,      togglegaps,     {0} },
	{ MODKEY|Mod4Mask|ShiftMask,    XK_0,      defaultgaps,    {0} },
	{ MODKEY,                       XK_y,      incrihgaps,     {.i = +1 } },
	{ MODKEY,                       XK_o,      incrihgaps,     {.i = -1 } },
	{ MODKEY|ControlMask,           XK_y,      incrivgaps,     {.i = +1 } },
	{ MODKEY|ControlMask,           XK_o,      incrivgaps,     {.i = -1 } },
	{ MODKEY|Mod4Mask,              XK_y,      incrohgaps,     {.i = +1 } },
	{ MODKEY|Mod4Mask,              XK_o,      incrohgaps,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_y,      incrovgaps,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,      incrovgaps,     {.i = -1 } },
	{ MODKEY,                       XK_space,  zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_o,      setlayout,      {.v = &layouts[4]} },
	// { MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask,   	        XK_Return, togglescratch,  {.ui = 0 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ 0, XF86XK_AudioLowerVolume,              spawn,          {.v = downvol } },
	{ 0, XF86XK_AudioMute,                     spawn,          {.v = mutevol } },
	{ 0, XF86XK_AudioRaiseVolume,              spawn,          {.v = upvol   } },
	{ MODKEY|ShiftMask,             XK_c,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

