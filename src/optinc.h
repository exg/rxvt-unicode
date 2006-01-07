 def(console,               0)
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
 def(mouseWheelScrollPage, 17)
#if POINTER_BLANK
 def(pointerBlank,         18)
#else
 nodef(pointerBlank)
#endif
 def(cursorBlink,          19)
 def(secondaryScreen,      20)
 def(secondaryScroll,      21)
 def(pastableTabs,         22)
 def(cursorUnderline,      23)
#if ENABLE_FRILLS
 def(insecure,             24) // insecure esc sequences
 def(borderLess,           25) // mwm borderless hints
 def(hold,                 26) // hold window open after exit
 def(skipBuiltinGlyphs,    27) // do not use internal glyphs
#else
 nodef(insecure)
 nodef(borderLess)
 nodef(hold)
 nodef(skipBuiltinGlyphs)
#endif
#if ENABLE_STYLES
 def(intensityStyles,      28) // font styles imply intensity
#else
 nodef(intensityStyles)
#endif

