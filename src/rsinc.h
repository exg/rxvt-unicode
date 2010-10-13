// all resource indices, used by rxvt.h and rxvtperl.xs

  def (display_name)
  def (term_name)
  def (iconName)
  def (geometry)
  def (reverseVideo)
  def (chdir)
  def (color)
  reserve (color, TOTAL_COLORS)
  def (font)
#if ENABLE_STYLES
  def (boldFont)
  def (italicFont)
  def (boldItalicFont)
  def (intensityStyles)
#endif
  def (name)
  def (title)
#ifdef BG_IMAGE_FROM_FILE
  def (path)
  def (backgroundPixmap)
#endif
  def (loginShell)
  def (jumpScroll)
  def (skipScroll)
  def (scrollBar)
  def (scrollBar_right)
  def (scrollBar_floating)
  def (scrollBar_align)
  def (scrollstyle)
  def (scrollBar_thickness)
  def (scrollTtyOutput)
  def (scrollTtyKeypress)
  def (scrollWithBuffer)
  def (saveLines)
  def (utmpInhibit)
  def (visualBell)
#if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
  def (mapAlert)
#endif
#ifdef META8_OPTION
  def (meta8)
#endif
#ifdef MOUSE_WHEEL
  def (mouseWheelScrollPage)
#endif
#ifndef NO_BACKSPACE_KEY
  def (backspace_key)
#endif
#ifndef NO_DELETE_KEY
  def (delete_key)
#endif
#ifdef PRINTPIPE
  def (print_pipe)
#endif
#ifdef USE_XIM
  def (preeditType)
  def (inputMethod)
#endif
#ifdef ENABLE_TRANSPARENCY
  def (transparent)
  def (shade)
  def (blurradius)
#endif
#if XFT
  def (depth)
  def (buffered)
#endif
#if ENABLE_FRILLS
  def (transient_for)
  def (override_redirect)
  def (pty_fd)
  def (hold)
  def (ext_bwidth)
  def (int_bwidth)
  def (borderLess)
  def (lineSpace)
  def (letterSpace)
  def (cursorUnderline)
  def (urgentOnBell)
#endif
#ifdef BUILTIN_GLYPHS
  def (skipBuiltinGlyphs)
#endif
#if CURSOR_BLINK
  def (cursorBlink)
#endif
#if ENABLE_XEMBED
  def (embed)
#endif
  def (cutchars)
  def (modifier)
  def (answerbackstring)
  def (tripleclickwords)
  def (insecure)
  def (pointerBlank)
  def (pointerBlankDelay)
  def (imLocale)
  def (imFont)
  def (pastableTabs)
#ifndef NO_SECONDARY_SCREEN
  def (secondaryScreen)
  def (secondaryScroll)
#endif
#ifdef OFF_FOCUS_FADING
  def (fade)
#endif
#if ENABLE_PERL
  def (perl_eval)
  def (perl_ext_1)
  def (perl_ext_2)
  def (perl_lib)
#endif
#if ISO_14755
  def (iso14755)
  def (iso14755_52)
#endif
#ifdef HAVE_AFTERIMAGE
  def (blendtype)
#endif
#ifdef ENABLE_EWMH
  def (iconfile)
#endif
