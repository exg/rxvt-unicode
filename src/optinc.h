// all resource indices, used by rxvt.h and rxvtperl.xs

 def(loginShell)
 def(multiClickTime)
 def(iconic)
 def(visualBell)
 def(mapAlert)
 def(reverseVideo)
 def(utmpInhibit)
 def(scrollBar)
 def(scrollBar_right)
 def(scrollBar_floating)
 def(meta8)
 def(scrollTtyOutput)
 def(scrollTtyKeypress)
 def(transparent)
 def(scrollWithBuffer)
 def(jumpScroll)
 def(skipScroll)
 def(mouseWheelScrollPage)
#if POINTER_BLANK
 def(pointerBlank)
#else
 nodef(pointerBlank)
#endif
 def(cursorBlink)
 def(secondaryScreen)
 def(secondaryScroll)
 def(pastableTabs)
 def(cursorUnderline)
#if ENABLE_FRILLS
 def(tripleclickwords)
 def(disablePasteBrackets)
 def(borderLess) // mwm borderless hints
#endif
#if ENABLE_FRILLS
 def(insecure)   // insecure esc sequences
 def(hold)       // hold window open after exit
 def(override_redirect)
 def(urgentOnBell)
#else
 nodef(insecure)
 nodef(hold)
 nodef(override_redirect)
 nodef(urgentOnBell)
#endif
#ifdef BUILTIN_GLYPHS
 def(skipBuiltinGlyphs) // do not use internal glyphs
#else
 nodef(skipBuiltinGlyphs)
#endif
#if ENABLE_STYLES
 def(intensityStyles)   // font styles imply intensity
#else
 nodef(intensityStyles)
#endif
#if ISO_14755
 def(iso14755)
 def(iso14755_52)
#else
 nodef(iso14755)
 nodef(iso14755_52)
#endif
 def(console)
#if XFT
 def(buffered)
#else
 nodef(buffered)
#endif
 def(dockapp)

