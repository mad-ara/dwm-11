#include <X11/XF86keysym.h>

#include "gaplessgrid.c"
#include "movestack.c"
#include "horizgrid.c"

/* appearance */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const unsigned int borderpx  = 2;  /* border pixel of windows */
static const unsigned int snap      = 32; /* snap pixel */
static const unsigned int minwsz    = 0;  /* Minimal seigt of a client for smfact */
static const int showbar            = 1;  /* 0 means no bar */
static const int topbar             = 1;  /* 0 means bottom bar */
static const char *fonts[]          = { "Source Code Pro:style=Thin:pixelsize=14" };
static const char dmenufont[]       = "Source Code Pro:style=Thin:size=10";
static const char col_darkgray[]    = "#282828";
static const char col_lightgray[]   = "#d8d8d8";
static const char col_background[]  = "#181818";
static const char col_foreground[]  = "#585858";
static const char *colors[][3]      = {
    /*               fg              bg              border   */
    [SchemeNorm] = { col_lightgray, col_background, col_background },
    [SchemeSel]  = { col_lightgray, col_darkgray,   col_foreground },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
    /* class                instance    title       tags mask     iscentered     isfloating   monitor */
    { "Mpv",                NULL,       NULL,       NULL,         1,             1,           -1 },
    { "Sxiv",               NULL,       NULL,       NULL,         1,             1,           -1 },
    { "Chromium",           NULL,       NULL,       1 << 1,       0,             0,           -1 },
    { "Qemu-system-x86_64", NULL,       NULL,       1 << 5,       1,             1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.60; /* factor of master area size [0.05..0.95] */
static const float smfact    = 0.00; /* factor of tiled clients [0.00..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
    /* symbol     arrange function */
    { "tile",     tile },
    { "monocle",  monocle },
    { "stack",    bstack },
    { "grid",     horizgrid },
    { "float",    NULL },
    { NULL,       NULL },
};

#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },


static const char terminal[] = "kitty";
static const char browser[] = "chromium";

#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

static char dmenumon[2] = "0";

static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_background, "-nf", col_foreground, "-sb", col_lightgray, "-sf", col_background, NULL };
static const char *clipcmd[]  = { "clipmenu",  "-m", dmenumon, "-fn", dmenufont, "-nb", col_background, "-nf", col_foreground, "-sb", col_lightgray, "-sf", col_background, NULL };

static const char scratchpadname[] = "scratchpad";
static const char *scratchpadcmd[] = { terminal, "-title", scratchpadname, "-g", "120x34", "-e", "R", "-q", NULL };

static const char *lockcmd[]    = { "slock",  NULL };
static const char *wwwcmd[]     = { browser,  NULL };
static const char *termcmd[]    = { terminal, NULL };

/* media keys */
static const char *volup[]      = { "pactl", "set-sink-volume", "0", "+5%",    NULL };
static const char *voldown[]    = { "pactl", "set-sink-volume", "0", "-5%",    NULL };
static const char *volmute[]    = { "pactl", "set-sink-mute",   "0", "toggle", NULL };

static const char *audioplay[]  = { "cmus-remote", "-p", NULL };
static const char *audiostop[]  = { "cmus-remote", "-s", NULL };
static const char *audiopause[] = { "cmus-remote", "-u", NULL };
static const char *audioback[]  = { "cmus-remote", "-r", NULL };
static const char *audionext[]  = { "cmus-remote", "-n", NULL };

static const int numpad = 1;

static Key keys[] = {
    /* modifier                      key   function        argument */

    { MODKEY,                       XK_b,  togglebar,      {0} },

    { MODKEY,                       XK_a,  spawn,          {.v = wwwcmd } },
    { MODKEY,                       XK_l,  spawn,          {.v = dmenucmd } },
    { MODKEY|ShiftMask,             XK_l,  spawn,          {.v = clipcmd } },
    { MODKEY,                       XK_s,  togglescratch,  {.v = scratchpadcmd } },

    { MODKEY,                  XK_Return,  spawn,          {.v = termcmd } },
    { MODKEY,               XK_semicolon,  killclient,     {0} },

    { MODKEY,                       XK_0,  view,           {.ui = ~0 } },
    { MODKEY,                     XK_Tab,  view,           {0} },

    { MODKEY|ShiftMask,         XK_space,  togglefloating, {0} },

    { MODKEY,                       XK_d,  setmfact,       {.f = -0.00625 } },
    { MODKEY,                       XK_n,  setmfact,       {.f = +0.00625 } },
    { MODKEY|ShiftMask,             XK_d,  setsmfact,      {.f = -0.05 } },
    { MODKEY|ShiftMask,             XK_n,  setsmfact,      {.f = +0.05 } },

    { MODKEY,                       XK_h,  focusstack,     {.i = +1 } },
    { MODKEY,                       XK_t,  focusstack,     {.i = -1 } },
    { MODKEY|ShiftMask,             XK_h,  movestack,      {.i = +1 } },
    { MODKEY|ShiftMask,             XK_t,  movestack,      {.i = -1 } },

    { MODKEY,                       XK_m,  cyclelayout,    {.i = -1 } },
    { MODKEY,                       XK_w,  cyclelayout,    {.i = +1 } },

    { MODKEY,                       XK_r,  focusmon,       {.i = -1 } },
    { MODKEY,                       XK_l,  focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_r,  tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_l,  tagmon,         {.i = +1 } },

    { 0,         XF86XK_AudioRaiseVolume,  spawn,          {.v = volup } },
    { 0,         XF86XK_AudioLowerVolume,  spawn,          {.v = voldown } },
    { 0,                XF86XK_AudioMute,  spawn,          {.v = volmute } },
    { 0,                XF86XK_AudioPlay,  spawn,          {.v = audioplay } },
    { 0,                XF86XK_AudioStop,  spawn,          {.v = audiostop } },
    { 0,               XF86XK_AudioPause,  spawn,          {.v = audiopause } },
    { 0,                XF86XK_AudioPrev,  spawn,          {.v = audioback } },
    { 0,                XF86XK_AudioNext,  spawn,          {.v = audionext } },

    { 0,                  XF86XK_Launch1,  spawn,          {.v = lockcmd } },

    TAGKEYS(numpad ? XK_1 : XK_ampersand,    0)
    TAGKEYS(numpad ? XK_2 : XK_bracketleft,  1)
    TAGKEYS(numpad ? XK_3 : XK_braceleft,    2)
    TAGKEYS(numpad ? XK_4 : XK_braceright,   3)
    TAGKEYS(numpad ? XK_5 : XK_parenleft,    4)
    TAGKEYS(numpad ? XK_6 : XK_equal,        5)
    TAGKEYS(numpad ? XK_7 : XK_asterisk,     6)
    TAGKEYS(numpad ? XK_8 : XK_parenright,   7)
    TAGKEYS(numpad ? XK_9 : XK_plus,         8)

};

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

