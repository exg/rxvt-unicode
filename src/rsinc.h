// all resource indices, used by rxvt.h anf rxvtperl.xs

  Rs_def(display_name)
  Rs_def(term_name)
  Rs_def(iconName)
  Rs_def(geometry)
  Rs_def(reverseVideo)
  Rs_def(color)
  Rs_reserve(color, NRS_COLORS)
  Rs_def(font)
#if ENABLE_STYLES
  Rs_def(boldFont)
  Rs_def(italicFont)
  Rs_def(boldItalicFont)
  Rs_def(intensityStyles)
#endif
  Rs_def(name)
  Rs_def(title)
#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
  Rs_def(path)
#endif
#ifdef XPM_BACKGROUND
  Rs_def(backgroundPixmap)
#endif
#if (MENUBAR_MAX)
  Rs_def(menu)
#endif
  Rs_def(loginShell)
  Rs_def(jumpScroll)
#ifdef HAVE_SCROLLBARS
  Rs_def(scrollBar)
  Rs_def(scrollBar_right)
  Rs_def(scrollBar_floating)
  Rs_def(scrollBar_align)
  Rs_def(scrollstyle)
  Rs_def(scrollBar_thickness)
#endif
  Rs_def(scrollTtyOutput)
  Rs_def(scrollTtyKeypress)
  Rs_def(scrollWithBuffer)
  Rs_def(saveLines)
  Rs_def(utmpInhibit)
  Rs_def(visualBell)
#if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
  Rs_def(mapAlert)
#endif
#ifdef META8_OPTION
  Rs_def(meta8)
#endif
#ifdef MOUSE_WHEEL
  Rs_def(mouseWheelScrollPage)
#endif
#ifndef NO_BACKSPACE_KEY
  Rs_def(backspace_key)
#endif
#ifndef NO_DELETE_KEY
  Rs_def(delete_key)
#endif
  Rs_def(selectstyle)
#ifdef PRINTPIPE
  Rs_def(print_pipe)
#endif
#ifdef USE_XIM
  Rs_def(preeditType)
  Rs_def(inputMethod)
#endif
#ifdef TRANSPARENT
  Rs_def(transparent)
  Rs_def(transparent_all)
#endif
#if ENABLE_FRILLS
  Rs_def(pty_fd)
  Rs_def(hold)
  Rs_def(ext_bwidth)
  Rs_def(int_bwidth)
  Rs_def(borderLess)
  Rs_def(lineSpace)
  Rs_def(cursorUnderline)
#endif
#if CURSOR_BLINK
  Rs_def(cursorBlink)
#endif
#if ENABLE_XEMBED
  Rs_def(embed)
#endif
  Rs_def(cutchars)
  Rs_def(modifier)
  Rs_def(answerbackstring)
  Rs_def(tripleclickwords)
  Rs_def(insecure)
  Rs_def(pointerBlank)
  Rs_def(pointerBlankDelay)
  Rs_def(imLocale)
  Rs_def(imFont)
  Rs_def(pastableTabs)
#ifndef NO_SECONDARY_SCREEN
  Rs_def(secondaryScreen)
  Rs_def(secondaryScroll)
#endif
#ifdef OFF_FOCUS_FADING
  Rs_def(fade)
#endif
#ifdef TINTING
  Rs_def(shade)
#endif
#if ENABLE_PERL
  Rs_def(perl_lib)
  Rs_def(perl_eval)
  Rs_def(perl)
#endif
