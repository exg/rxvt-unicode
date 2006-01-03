=encoding utf8

=head1 NAME

@@RXVT_NAME@@perl - rxvt-unicode's embedded perl interpreter

=head1 SYNOPSIS

   # create a file grab_test in $HOME:

   sub on_sel_grab {
      warn "you selected ", $_[0]->selection;
      ()
   }

   # start a @@RXVT_NAME@@ using it:

   @@RXVT_NAME@@ --perl-lib $HOME -pe grab_test

=head1 DESCRIPTION

Everytime a terminal object gets created, scripts specified via the
C<perl> resource are loaded and associated with it.

Scripts are compiled in a 'use strict' and 'use utf8' environment, and
thus must be encoded as UTF-8.

Each script will only ever be loaded once, even in @@RXVT_NAME@@d, where
scripts will be shared (but not enabled) for all terminals.

=head2 Prepackaged Extensions

This section describes the extensiosn delivered with this version. You can
find them in F<@@RXVT_LIBDIR@@/urxvt/perl/>.

You can activate them like this:

  @@RXVT_NAME@@ -pe <extensionname>

=over 4

=item selection

Miscellaneous selection modifications.

=over 4

=item rot13

Rot-13 the selection when activated. Used via keyboard trigger:

   URxvt.keysym.C-M-r: perl:selection:rot13

=back

=item digital-clock

Displays a very simple digital clock in the upper right corner of the
window. Illustrates overwriting the refresh callbacks to create your own
overlays or changes.

=item simple-overlay-clock

Displays a digital clock using the built-in overlay (colourful, useless).

=back

=head2 General API Considerations

All objects (such as terminals, time watchers etc.) are typical
reference-to-hash objects. The hash can be used to store anything you
like. All members starting with an underscore (such as C<_ptr> or
C<_hook>) are reserved for internal uses and must not be accessed or
modified).

When objects are destroyed on the C++ side, the perl object hashes are
emptied, so its best to store related objects such as time watchers and
the like inside the terminal object so they get destroyed as soon as the
terminal is destroyed.

=head2 Hooks

The following subroutines can be declared in loaded scripts, and will be called
whenever the relevant event happens.

All of them must return a boolean value. If it is true, then the event
counts as being I<consumed>, and the invocation of other hooks is skipped,
and the relevant action might not be carried out by the C++ code.

When in doubt, return a false value (preferably C<()>).

=over 4

=item on_init $term

Called after a new terminal object has been initialized, but before
windows are created or the command gets run.

=item on_reset $term

Called after the screen is "reset" for any reason, such as resizing or
control sequences. Here is where you can react on changes to size-related
variables.

=item on_start $term

Called at the very end of initialisation of a new terminal, just before
returning to the mainloop.

=item on_sel_make $term, $eventtime

Called whenever a selection has been made by the user, but before the
selection text is copied, so changes to the beginning, end or type of the
selection will be honored.

Returning a true value aborts selection making by urxvt, in which case you
have to make a selection yourself by calling C<< $term->selection_grab >>.

=item on_sel_grab $term, $eventtime

Called whenever a selection has been copied, but before the selection is
requested from the server.  The selection text can be queried and changed
by calling C<< $term->selection >>.

Returning a true value aborts selection grabbing. It will still be hilighted.

=item on_focus_in $term

Called whenever the window gets the keyboard focus, before urxvt does
focus in processing.

=item on_focus_out $term

Called wheneever the window loses keyboard focus, before urxvt does focus
out processing.

=item on_view_change $term, $offset

Called whenever the view offset changes, i..e the user or program
scrolls. Offset C<0> means display the normal terminal, positive values
show this many lines of scrollback.

=item on_scroll_back $term, $lines, $saved

Called whenever lines scroll out of the terminal area into the scrollback
buffer. C<$lines> is the number of lines scrolled out and may be larger
than the scroll back buffer or the terminal.

It is called before lines are scrolled out (so rows 0 .. min ($lines - 1,
$nrow - 1) represent the lines to be scrolled out). C<$saved> is the total
number of lines that will be in the scrollback buffer.

=item on_tty_activity $term *NYI*

Called whenever the program(s) running in the urxvt window send output.

=item on_refresh_begin $term

Called just before the screen gets redrawn. Can be used for overlay
or similar effects by modify terminal contents in refresh_begin, and
restoring them in refresh_end. The built-in overlay and selection display
code is run after this hook, and takes precedence.

=item on_refresh_end $term

Called just after the screen gets redrawn. See C<on_refresh_begin>.

=item on_keyboard_command $term, $string

Called whenever the user presses a key combination that has a
C<perl:string> action bound to it (see description of the B<keysym>
resource in the @@RXVT_NAME@@(1) manpage).

=back

=head2 Functions in the C<urxvt> Package

=over 4

=item urxvt::fatal $errormessage

Fatally aborts execution with the given error message. Avoid at all
costs! The only time this is acceptable is when the terminal process
starts up.

=item urxvt::warn $string

Calls C<rxvt_warn> with the given string which should not include a
newline. The module also overwrites the C<warn> builtin with a function
that calls this function.

Using this function has the advantage that its output ends up in the
correct place, e.g. on stderr of the connecting urxvtc client.

=item $time = urxvt::NOW

Returns the "current time" (as per the event loop).

=head2 RENDITION

Rendition bitsets contain information about colour, font, font styles and
similar information for each screen cell.

The following "macros" deal with changes in rendition sets. You should
never just create a bitset, you should always modify an existing one,
as they contain important information required for correct operation of
rxvt-unicode.

=over 4

=item $rend = urxvt::DEFAULT_RSTYLE

Returns the default rendition, as used when the terminal is starting up or
being reset. Useful as a base to start when creating renditions.

=item $rend = urxvt::OVERLAY_RSTYLE

Return the rendition mask used for overlays by default.

=item $rendbit = urxvt::RS_Bold, RS_Italic, RS_Blink, RS_RVid, RS_Uline

Return the bit that enabled bold, italic, blink, reverse-video and
underline, respectively. To enable such a style, just logically OR it into
the bitset.

=item $foreground = urxvt::GET_BASEFG $rend

=item $background = urxvt::GET_BASEBG $rend

Return the foreground/background colour index, respectively.

=item $rend = urxvt::SET_FGCOLOR ($rend, $new_colour)

=item $rend = urxvt::SET_BGCOLOR ($rend, $new_colour)

Replace the foreground/background colour in the rendition mask with the
specified one.

=item $value = urxvt::GET_CUSTOM ($rend)

Return the "custom" value: Every rendition has 5 bits for use by
extensions. They can be set and changed as you like and are initially
zero.

=item $rend = urxvt::SET_CUSTOM ($rend, $new_value)

Change the custom value.

=back

=cut

package urxvt;

use strict;

our $term;
our @HOOKNAME;
our $LIBDIR;

BEGIN {
   urxvt->bootstrap;

   # overwrite perl's warn
   *CORE::GLOBAL::warn = sub {
      my $msg = join "", @_;
      $msg .= "\n"
         unless $msg =~ /\n$/;
      urxvt::warn ($msg);
   };
}

my @hook_count;
my $verbosity = $ENV{URXVT_PERL_VERBOSITY};

sub verbose {
   my ($level, $msg) = @_;
   warn "$msg\n" if $level <= $verbosity;
}

# find on_xxx subs in the package and register them
# as hooks
sub register_package($) {
   my ($pkg) = @_;

   for my $htype (0.. $#HOOKNAME) {
      my $name = $HOOKNAME[$htype];

      my $ref = $pkg->can ("on_" . lc $name)
         or next;

      $term->{_hook}[$htype]{$ref*1} = $ref;
      $hook_count[$htype]++
         or set_should_invoke $htype, 1;
   }
}

my $script_pkg = "script0000";
my %script_pkg;

# load a single script into its own package, once only
sub script_package($) {
   my ($path) = @_;

   $script_pkg{$path} ||= do {
      my $pkg = "urxvt::" . ($script_pkg++);

      verbose 3, "loading script '$path' into package '$pkg'";

      open my $fh, "<:raw", $path
         or die "$path: $!";

      my $source = "package $pkg; use strict; use utf8;\n"
                   . "#line 1 \"$path\"\n{\n"
                   . (do { local $/; <$fh> })
                   . "\n};\n1";

      eval $source or die "$path: $@";

      $pkg
   }
}

# called by the rxvt core
sub invoke {
   local $term = shift;
   my $htype = shift;

   if ($htype == 0) { # INIT
      my @dirs = ((split /:/, $term->resource ("perl_lib")), "$LIBDIR/perl");

      for my $ext (split /:/, $term->resource ("perl_ext")) {
         my @files = grep -f $_, map "$_/$ext", @dirs;

         if (@files) {
            register_package script_package $files[0];
         } else {
            warn "perl extension '$ext' not found in perl library search path\n";
         }
      }

   } elsif ($htype == 1) { # DESTROY
      if (my $hook = $term->{_hook}) {
         for my $htype (0..$#$hook) {
            $hook_count[$htype] -= scalar keys %{ $hook->[$htype] || {} }
               or set_should_invoke $htype, 0;
         }
      }
   }

   my $cb = $term->{_hook}[$htype]
      or return;

   verbose 10, "$HOOKNAME[$htype] (" . (join ", ", $term, @_) . ")"
      if $verbosity >= 10;

   while (my ($k, $v) = each %$cb) {
      return 1 if $v->($term, @_);
   }

   0
}

=back

=head2 The C<urxvt::term> Class

=over 4

=item $value = $term->resource ($name[, $newval])

Returns the current resource value associated with a given name and
optionally sets a new value. Setting values is most useful in the C<init>
hook. Unset resources are returned and accepted as C<undef>.

The new value must be properly encoded to a suitable character encoding
before passing it to this method. Similarly, the returned value may need
to be converted from the used encoding to text.

Resource names are as defined in F<src/rsinc.h>. Colours can be specified
as resource names of the form C<< color+<index> >>, e.g. C<color+5>. (will
likely change).

Please note that resource strings will currently only be freed when the
terminal is destroyed, so changing options frequently will eat memory.

Here is a a likely non-exhaustive list of resource names, not all of which
are supported in every build, please see the source to see the actual
list:

  answerbackstring backgroundPixmap backspace_key boldFont boldItalicFont
  borderLess color cursorBlink cursorUnderline cutchars delete_key
  display_name embed ext_bwidth fade font geometry hold iconName
  imFont imLocale inputMethod insecure int_bwidth intensityStyles
  italicFont jumpScroll lineSpace loginShell mapAlert menu meta8 modifier
  mouseWheelScrollPage name pastableTabs path perl_eval perl_ext
  perl_lib pointerBlank pointerBlankDelay preeditType print_pipe pty_fd
  reverseVideo saveLines scrollBar scrollBar_align scrollBar_floating
  scrollBar_right scrollBar_thickness scrollTtyKeypress scrollTtyOutput
  scrollWithBuffer scrollstyle secondaryScreen secondaryScroll selectstyle
  shade term_name title transparent transparent_all tripleclickwords
  utmpInhibit visualBell

=cut

sub urxvt::term::resource($$;$) {
   my ($self, $name) = (shift, shift);
   unshift @_, $self, $name, ($name =~ s/\s*\+\s*(\d+)$// ? $1 : 0);
   goto &urxvt::term::_resource;
}

=item ($row, $col) = $term->selection_mark ([$row, $col])

=item ($row, $col) = $term->selection_beg ([$row, $col])

=item ($row, $col) = $term->selection_end ([$row, $col])

Return the current values of the selection mark, begin or end positions,
and optionally set them to new values.

=item $success = $term->selection_grab ($eventtime)

Try to request the primary selection from the server (for example, as set
by the next method).

=item $oldtext = $term->selection ([$newtext])

Return the current selection text and optionally replace it by C<$newtext>.

=item $term->scr_overlay ($x, $y, $text)

Create a simple multi-line overlay box. See the next method for details.

=cut

sub urxvt::term::scr_overlay {
   my ($self, $x, $y, $text) = @_;

   my @lines = split /\n/, $text;

   my $w = 0;
   for (map $self->strwidth ($_), @lines) {
      $w = $_ if $w < $_;
   }

   $self->scr_overlay_new ($x, $y, $w, scalar @lines);
   $self->scr_overlay_set (0, $_, $lines[$_]) for 0.. $#lines;
}

=item $term->scr_overlay_new ($x, $y, $width, $height)

Create a new (empty) overlay at the given position with the given
width/height. A border will be put around the box. If either C<$x> or
C<$y> is negative, then this is counted from the right/bottom side,
respectively.

=item $term->scr_overlay_off

Switch the overlay off again.

=item $term->scr_overlay_set_char ($x, $y, $char, $rend = OVERLAY_RSTYLE)

Put a single character (specified numerically) at the given overlay
position.

=item $term->scr_overlay_set ($x, $y, $text)

Write a string at the given position into the overlay.

=item $cellwidth = $term->strwidth $string

Returns the number of screen-cells this string would need. Correctly
accounts for wide and combining characters.

=item $octets = $term->locale_encode $string

Convert the given text string into the corresponding locale encoding.

=item $string = $term->locale_decode $octets

Convert the given locale-encoded octets into a perl string.

=item $term->tt_write ($octets)

Write the octets given in C<$data> to the tty (i.e. as program input). To
pass characters instead of octets, you should convert your strings first
to the locale-specific encoding using C<< $term->locale_encode >>.

=item $nrow = $term->nrow

=item $ncol = $term->ncol

Return the number of rows/columns of the terminal window (i.e. as
specified by C<-geometry>, excluding any scrollback).

=item $nsaved = $term->nsaved

Returns the number of lines in the scrollback buffer.

=item $view_start = $term->view_start ([$newvalue])

Returns the negative row number of the topmost line. Minimum value is
C<0>, which displays the normal terminal contents. Larger values scroll
this many lines into the scrollback buffer.

=item $term->want_refresh

Requests a screen refresh. At the next opportunity, rxvt-unicode will
compare the on-screen display with its stored representation. If they
differ, it redraws the differences.

Used after changing terminal contents to display them.

=item $text = $term->ROW_t ($row_number[, $new_text[, $start_col]])

Returns the text of the entire row with number C<$row_number>. Row C<0>
is the topmost terminal line, row C<< $term->$ncol-1 >> is the bottommost
terminal line. The scrollback buffer starts at line C<-1> and extends to
line C<< -$term->nsaved >>.

If C<$new_text> is specified, it will replace characters in the current
line, starting at column C<$start_col> (default C<0>), which is useful
to replace only parts of a line. The font index in the rendition will
automatically be updated.

C<$text> is in a special encoding: tabs and wide characters that use more
than one cell when displayed are padded with urxvt::NOCHAR characters
(C<chr 65535>). Characters with combining characters and other characters
that do not fit into the normal tetx encoding will be replaced with
characters in the private use area.

You have to obey this encoding when changing text. The advantage is
that C<substr> and similar functions work on screen cells and not on
characters.

The methods C<< $term->special_encode >> and C<< $term->special_decode >>
can be used to convert normal strings into this encoding and vice versa.

=item $rend = $term->ROW_r ($row_number[, $new_rend[, $start_col]])

Like C<< $term->ROW_t >>, but returns an arrayref with rendition
bitsets. Rendition bitsets contain information about colour, font, font
styles and similar information. See also C<< $term->ROW_t >>.

When setting rendition, the font mask will be ignored.

See the section on RENDITION, above.

=item $length = $term->ROW_l ($row_number[, $new_length])

Returns the number of screen cells that are in use ("the line length"). If
it is C<-1>, then the line is part of a multiple-row logical "line", which
means all characters are in use and it is continued on the next row.

=item $text = $term->special_encode $string

Converts a perl string into the special encoding used by rxvt-unicode,
where one character corresponds to one screen cell. See
C<< $term->ROW_t >> for details.

=item $string = $term->special_decode $text

Converts rxvt-unicodes text reprsentation into a perl string. See
C<< $term->ROW_t >> for details.

=back

=head2 The C<urxvt::timer> Class

This class implements timer watchers/events. Time is represented as a
fractional number of seconds since the epoch. Example:

   # create a digital clock display in upper right corner
   $term->{timer} = urxvt::timer
                    ->new
                    ->start (urxvt::NOW)
                    ->cb (sub {
                       my ($timer) = @_;
                       my $time = $timer->at;
                       $timer->start ($time + 1);
                       $self->scr_overlay (-1, 0, 
                          POSIX::strftime "%H:%M:%S", localtime $time);
                    });

=over 4

=item $timer = new urxvt::timer

Create a new timer object in stopped state.

=item $timer = $timer->cb (sub { my ($timer) = @_; ... })

Set the callback to be called when the timer triggers.

=item $tstamp = $timer->at

Return the time this watcher will fire next.

=item $timer = $timer->set ($tstamp)

Set the time the event is generated to $tstamp.

=item $timer = $timer->start

Start the timer.

=item $timer = $timer->start ($tstamp)

Set the event trigger time to C<$tstamp> and start the timer.

=item $timer = $timer->stop

Stop the timer.

=back

=head2 The C<urxvt::iow> Class

This class implements io watchers/events. Example:

  $term->{socket} = ...
  $term->{iow} = urxvt::iow
                 ->new
                 ->fd (fileno $term->{socket})
                 ->events (1) # wait for read data
                 ->start
                 ->cb (sub {
                   my ($iow, $revents) = @_;
                   # $revents must be 1 here, no need to check
                   sysread $term->{socket}, my $buf, 8192
                      or end-of-file;
                 });


=over 4

=item $iow = new urxvt::iow

Create a new io watcher object in stopped state.

=item $iow = $iow->cb (sub { my ($iow, $reventmask) = @_; ... })

Set the callback to be called when io events are triggered. C<$reventmask>
is a bitset as described in the C<events> method.

=item $iow = $iow->fd ($fd)

Set the filedescriptor (not handle) to watch.

=item $iow = $iow->events ($eventmask)

Set the event mask to watch. Bit #0 (value C<1>) enables watching for read
data, Bit #1 (value C<2>) enables watching for write data.

=item $iow = $iow->start

Start watching for requested events on the given handle.

=item $iow = $iow->stop

Stop watching for events on the given filehandle.

=back

=head1 ENVIRONMENT

=head2 URXVT_PERL_VERBOSITY

This variable controls the verbosity level of the perl extension. Higher
numbers indicate more verbose output.

=over 4

=item 0 - only fatal messages

=item 3 - script loading and management

=item 10 - all events received

=back

=head1 AUTHOR

 Marc Lehmann <pcg@goof.com>
 http://software.schmorp.de/pkg/rxvt-unicode

=cut

1
