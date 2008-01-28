// all resource indices, used by rxvt.h and rxvtperl.xs

 def(loginShell,            1)
 def(iconic,                2)
 def(visualBell,            3)
 def(mapAlert,              4)
 def(reverseVideo,          5)
 def(utmpInhibit,           6)
 def(scrollBar,             7)
 def(scrollBar_right,       8)
 def(scrollBar_floating,    9)
 def(meta8,                10)
 def(scrollTtyOutput,      11)
 def(scrollTtyKeypress,    12)
 def(transparent,          13)
 def(tripleclickwords,     14)
 def(scrollWithBuffer,     15)
 def(jumpScroll,           16)
 def(skipScroll,           17)
 def(mouseWheelScrollPage, 18)
#if POINTER_BLANK
 def(pointerBlank,         19)
#else
 nodef(pointerBlank)
#endif
 def(cursorBlink,          20)
 def(secondaryScreen,      21)
 def(secondaryScroll,      22)
 def(pastableTabs,         23)
 def(cursorUnderline,      24)
#if ENABLE_FRILLS
 def(insecure,             25) // insecure esc sequences
 def(borderLess,           26) // mwm borderless hints
 def(hold,                 27) // hold window open after exit
 def(override_redirect,    28)
 def(urgentOnBell,         29)
#else
 nodef(insecure)
 nodef(borderLess)
 nodef(hold)
 nodef(override_redirect)
 nodef(urgentOnBell)
#endif
#ifdef BUILTIN_GLYPHS
 def(skipBuiltinGlyphs,    30) // do not use internal glyphs
#else
 nodef(skipBuiltinGlyphs)
#endif
#if ENABLE_STYLES
 def(intensityStyles,      31) // font styles imply intensity
#else
 nodef(intensityStyles)
#endif
#if ISO_14755
 def(iso14755_52,          32)
#else
 nodef(iso14755_52)
#endif
 def(console,              33)
#if XFT
 def(buffered,             34)
#else
 nodef(buffered)
#endif

