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

Intelligent selection. This extension tries to be more intelligent when
the user extends selections (double-click). Right now, it tries to select
urls and complete shell-quoted arguments, which is very convenient, too,
if your F<ls> supports C<--quoting-style=shell>.

It also offers the following bindable event:

=over 4

=item rot13

Rot-13 the selection when activated. Used via keyboard trigger:

   URxvt.keysym.C-M-r: perl:selection:rot13

=back

=item digital-clock

Displays a digital clock using the built-in overlay.

=item mark-urls

Uses per-line filtering (C<on_line_update>) to underline urls.

=item example-refresh-hooks

Displays a very simple digital clock in the upper right corner of the
window. Illustrates overwriting the refresh callbacks to create your own
overlays or changes.

=item example-filter-input

A not very useful example of filtering all text output to the terminal, by
underlining all urls that matches a certain regex (i.e. some urls :). It
is not very useful because urls that are output in multiple steps (e.g.
when typing them) do not get marked.

=back

=head2 General API Considerations

All objects (such as terminals, time watchers etc.) are typical
reference-to-hash objects. The hash can be used to store anything you
like. All members starting with an underscore (such as C<_ptr> or
C<_hook>) are reserved for internal uses and B<MUST NOT> be accessed or
modified).

When objects are destroyed on the C++ side, the perl object hashes are
emptied, so its best to store related objects such as time watchers and
the like inside the terminal object so they get destroyed as soon as the
terminal is destroyed.

Argument names also often indicate the type of a parameter. Here are some
hints on what they mean:

=over 4

=item $text

Rxvt-unicodes special way of encoding text, where one "unicode" character
always represents one screen cell. See L<row_t> for a discussion of this format.

=item $string

A perl text string, with an emphasis on I<text>. It can store all unicode
characters and is to be distinguished with text encoded in a specific
encoding (often locale-specific) and binary data.

=item $octets

Either binary data or - more common - a text string encoded in a
locale-specific way.

=back

=head2 Hooks

The following subroutines can be declared in loaded scripts, and will be
called whenever the relevant event happens.

The first argument passed to them is an object private to each terminal
and extension package.  You can call all C<urxvt::term> methods on it, but
its not a real C<urxvt::term> object. Instead, the real C<urxvt::term>
object that is shared between all packages is stored in the C<term>
member.

All of them must return a boolean value. If it is true, then the event
counts as being I<consumed>, and the invocation of other hooks is skipped,
and the relevant action might not be carried out by the C++ code.

When in doubt, return a false value (preferably C<()>).

=over 4

=item on_init $term

Called after a new terminal object has been initialized, but before
windows are created or the command gets run. Most methods are unsafe to
call or deliver senseless data, as terminal size and other characteristics
have not yet been determined. You can safely query and change resources,
though.

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

=item on_sel_extend $term

Called whenever the user tries to extend the selection (e.g. with a double
click) and is either supposed to return false (normal operation), or
should extend the selection itelf and return true to suppress the built-in
processing.

See the F<selection> example extension.

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

=item on_osc_seq $term, $string

Called whenever the B<ESC ] 777 ; string ST> command sequence (OSC =
operating system command) is processed. Cursor position and other state
information is up-to-date when this happens. For interoperability, the
string should start with the extension name and a colon, to distinguish
it from commands for other extensions, and this might be enforced in the
future.

Be careful not ever to trust (in a security sense) the data you receive,
as its source can not easily be controleld (e-mail content, messages from
other users on the same system etc.).

=item on_add_lines $term, $string

Called whenever text is about to be output, with the text as argument. You
can filter/change and output the text yourself by returning a true value
and calling C<< $term->scr_add_lines >> yourself. Please note that this
might be very slow, however, as your hook is called for B<all> text being
output.

=item on_line_update $term, $row

Called whenever a line was updated or changed. Can be used to filter
screen output (e.g. underline urls or other useless stuff). Only lines
that are being shown will be filtered, and, due to performance reasons,
not always immediately.

The row number is always the topmost row of the line if the line spans
multiple rows.

Please note that, if you change the line, then the hook might get called
later with the already-modified line (e.g. if unrelated parts change), so
you cannot just toggle rendition bits, but only set them.

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

=item on_key_press $term, $event, $octets

=item on_key_release $term, $event

=item on_button_press $term, $event

=item on_button_release $term, $event

=item on_motion_notify $term, $event

Called whenever the corresponding X event is received for the terminal If
the hook returns true, then the even will be ignored by rxvt-unicode.

The event is a hash with most values as named by Xlib (see the XEvent
manpage), with the additional members C<row> and C<col>, which are the row
and column under the mouse cursor.

C<on_key_press> additionally receives the string rxvt-unicode would
output, if any, in locale-specific encoding.

subwindow.

=back

=head2 Variables in the C<urxvt> Package

=over 4

=item $urxvt::TERM

The current terminal. Whenever a callback/Hook is bein executed, this
variable stores the current C<urxvt::term> object.

=back

=head2 Functions in the C<urxvt> Package

=over 4

=item $term = new urxvt [arg...]

Creates a new terminal, very similar as if you had started it with
C<system $binfile, arg...>. Croaks (and probably outputs an error message)
if the new instance couldn't be created.  Returns C<undef> if the new
instance didn't initialise perl, and the terminal object otherwise. The
C<init> and C<start> hooks will be called during the call.

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

=back

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
use Scalar::Util ();

our $TERM;
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

      $TERM->{_hook}[$htype]{$pkg} = $ref;
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

our $retval; # return value for urxvt

# called by the rxvt core
sub invoke {
   local $TERM = shift;
   my $htype = shift;

   if ($htype == 0) { # INIT
      my @dirs = ((split /:/, $TERM->resource ("perl_lib")), "$LIBDIR/perl");

      for my $ext (map { split /:/, $TERM->resource ("perl_ext_$_") } 1, 2) {
         my @files = grep -f $_, map "$_/$ext", @dirs;

         if (@files) {
            register_package script_package $files[0];
         } else {
            warn "perl extension '$ext' not found in perl library search path\n";
         }
      }
   }

   $retval = undef;

   if (my $cb = $TERM->{_hook}[$htype]) {
      verbose 10, "$HOOKNAME[$htype] (" . (join ", ", $TERM, @_) . ")"
         if $verbosity >= 10;

      keys %$cb;

      while (my ($pkg, $cb) = each %$cb) {
         $retval = $cb->(
            $TERM->{_pkg}{$pkg} ||= do {
               my $proxy = bless { }, urxvt::term::proxy::;
               Scalar::Util::weaken ($proxy->{term} = $TERM);
               $proxy
            },
            @_,
         ) and last;
      }
   }

   if ($htype == 1) { # DESTROY
      # remove hooks if unused
      if (my $hook = $TERM->{_hook}) {
         for my $htype (0..$#$hook) {
            $hook_count[$htype] -= scalar keys %{ $hook->[$htype] || {} }
               or set_should_invoke $htype, 0;
         }
      }

      # clear package objects
      %$_ = () for values %{ $TERM->{_pkg} };

      # clear package
      %$TERM = ();
   }

   $retval
}

sub urxvt::term::proxy::AUTOLOAD {
   $urxvt::term::proxy::AUTOLOAD =~ /:([^:]+)$/
      or die "FATAL: \$AUTOLOAD '$urxvt::term::proxy::AUTOLOAD' unparsable";

   eval qq{
      sub $urxvt::term::proxy::AUTOLOAD {
         my \$proxy = shift;
         \$proxy->{term}->$1 (\@_)
      }
      1
   } or die "FATAL: unable to compile method forwarder: $@";

   goto &$urxvt::term::proxy::AUTOLOAD;
}

=head2 The C<urxvt::term> Class

=over 4

=item $term->destroy

Destroy the terminal object (close the window, free resources etc.).

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
  mouseWheelScrollPage name pastableTabs path perl_eval perl_ext_1 perl_ext_2
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

=item $rend = $term->rstyle ([$new_rstyle])

Return and optionally change the current rendition. Text that is output by
the terminal application will use this style.

=item ($row, $col) = $term->screen_cur ([$row, $col])

Return the current coordinates of the text cursor position and optionally
set it (which is usually bad as applications don't expect that).

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

#=item $term->overlay ($x, $y, $text)
#
#Create a simple multi-line overlay box. See the next method for details.
#
#=cut
#
#sub urxvt::term::scr_overlay {
#   my ($self, $x, $y, $text) = @_;
#
#   my @lines = split /\n/, $text;
#
#   my $w = 0;
#   for (map $self->strwidth ($_), @lines) {
#      $w = $_ if $w < $_;
#   }
#
#   $self->scr_overlay_new ($x, $y, $w, scalar @lines);
#   $self->scr_overlay_set (0, $_, $lines[$_]) for 0.. $#lines;
#}

=item $term->overlay ($x, $y, $width, $height[, $rstyle[, $border]])

Create a new (empty) overlay at the given position with the given
width/height. C<$rstyle> defines the initial rendition style
(default: C<OVERLAY_RSTYLE>).

If C<$border> is C<2> (default), then a decorative border will be put
around the box.

If either C<$x> or C<$y> is negative, then this is counted from the
right/bottom side, respectively.

This method returns an urxvt::overlay object. The overlay will be visible
as long as the perl object is referenced.

The methods currently supported on C<urxvt::overlay> objects are:

=over 4

=item $overlay->set ($x, $y, $text, $rend)

Similar to C<< $term->ROW_t >> and C<< $term->ROW_r >> in that it puts
text in rxvt-unicode's special encoding and an array of rendition values
at a specific position inside the overlay.

=item $overlay->hide

If visible, hide the overlay, but do not destroy it.

=item $overlay->show

If hidden, display the overlay again.

=back

=item $cellwidth = $term->strwidth ($string)

Returns the number of screen-cells this string would need. Correctly
accounts for wide and combining characters.

=item $octets = $term->locale_encode ($string)

Convert the given text string into the corresponding locale encoding.

=item $string = $term->locale_decode ($octets)

Convert the given locale-encoded octets into a perl string.

=item $term->scr_add_lines ($string)

Write the given text string to the screen, as if output by the application
running inside the terminal. It may not contain command sequences (escape
codes), but is free to use line feeds, carriage returns and tabs. The
string is a normal text string, not in locale-dependent encoding.

Normally its not a good idea to use this function, as programs might be
confused by changes in cursor position or scrolling. Its useful inside a
C<on_add_lines> hook, though.

=item $term->cmd_parse ($octets)

Similar to C<scr_add_lines>, but the argument must be in the
locale-specific encoding of the terminal and can contain command sequences
(escape codes) that will be interpreted.

=item $term->tt_write ($octets)

Write the octets given in C<$data> to the tty (i.e. as program input). To
pass characters instead of octets, you should convert your strings first
to the locale-specific encoding using C<< $term->locale_encode >>.

=item $windowid = $term->parent

Return the window id of the toplevel window.

=item $windowid = $term->vt

Return the window id of the terminal window.

=item $window_width = $term->width

=item $window_height = $term->height

=item $font_width = $term->fwidth

=item $font_height = $term->fheight

=item $font_ascent = $term->fbase

=item $terminal_rows = $term->nrow

=item $terminal_columns = $term->ncol

=item $has_focus = $term->focus

=item $is_mapped = $term->mapped

=item $max_scrollback = $term->saveLines

=item $nrow_plus_saveLines = $term->total_rows

=item $lines_in_scrollback = $term->nsaved

Return various integers describing terminal characteristics.

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
line C<< -$term->nsaved >>. Nothing will be returned if a nonexistent line
is requested.

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

Returns the number of screen cells that are in use ("the line
length"). Unlike the urxvt core, this returns C<< $term->ncol >> if the
line is joined with the following one.

=item $bool = $term->is_longer ($row_number)

Returns true if the row is part of a multiple-row logical "line" (i.e.
joined with the following row), which means all characters are in use
and it is continued on the next row (and possibly a continuation of the
previous row(s)).

=item $line = $term->line ($row_number)

Create and return a new C<urxvt::line> object that stores information
about the logical line that row C<$row_number> is part of. It supports the
following methods:

=over 4

=item $text = $line->t ([$new_text])

Returns or replaces the full text of the line, similar to C<ROW_t>

=item $rend = $line->r ([$new_rend])

Returns or replaces the full rendition array of the line, similar to C<ROW_r>

=item $length = $line->l

Returns the length of the line in cells, similar to C<ROW_l>.

=item $rownum = $line->beg

=item $rownum = $line->end

Return the row number of the first/last row of the line, respectively.

=item $offset = $line->offset_of ($row, $col)

Returns the character offset of the given row|col pair within the logical
line.

=item ($row, $col) = $line->coord_of ($offset)

Translates a string offset into terminal coordinates again.

=back

=cut

sub urxvt::term::line {
   my ($self, $row) = @_;

   my $maxrow = $self->nrow - 1;

   my ($beg, $end) = ($row, $row);

   --$beg while $self->ROW_is_longer ($beg - 1);
   ++$end while $self->ROW_is_longer ($end) && $end < $maxrow;

   bless {
      term => $self,
      beg  => $beg,
      end  => $end,
      ncol => $self->ncol,
      len  => ($end - $beg) * $self->ncol + $self->ROW_l ($end),
   }, urxvt::line::
}

sub urxvt::line::t {
   my ($self) = @_;

   if (@_ > 1)
     {
       $self->{term}->ROW_t ($_, $_[1], 0, ($_ - $self->{beg}) * $self->{ncol}, $self->{ncol})
          for $self->{beg} .. $self->{end};
     }

   defined wantarray &&
      substr +(join "", map $self->{term}->ROW_t ($_), $self->{beg} .. $self->{end}),
             0, $self->{len}
}

sub urxvt::line::r {
   my ($self) = @_;

   if (@_ > 1)
     {
       $self->{term}->ROW_r ($_, $_[1], 0, ($_ - $self->{beg}) * $self->{ncol}, $self->{ncol})
          for $self->{beg} .. $self->{end};
     }

   if (defined wantarray) {
      my $rend = [
         map @{ $self->{term}->ROW_r ($_) }, $self->{beg} .. $self->{end}
      ];
      $#$rend = $self->{len} - 1;
      return $rend;
   }

   ()
}

sub urxvt::line::beg { $_[0]{beg} }
sub urxvt::line::end { $_[0]{end} }
sub urxvt::line::l   { $_[0]{len} }

sub urxvt::line::offset_of {
   my ($self, $row, $col) = @_;

   ($row - $self->{beg}) * $self->{ncol} + $col
}

sub urxvt::line::coord_of {
   my ($self, $offset) = @_;

   use integer;

   (
      $offset / $self->{ncol} + $self->{beg},
      $offset % $self->{ncol}
   )
}

=item ($row, $col) = $line->coord_of ($offset)
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

   $term->{overlay} = $term->overlay (-1, 0, 8, 1, urxvt::OVERLAY_RSTYLE, 0);
   $term->{timer} = urxvt::timer
                    ->new
                    ->interval (1)
                    ->cb (sub {
                       $term->{overlay}->set (0, 0,
                          sprintf "%2d:%02d:%02d", (localtime urxvt::NOW)[2,1,0]);
                    });                                                                                                                                      

=over 4

=item $timer = new urxvt::timer

Create a new timer object in started state. It is scheduled to fire
immediately.

=item $timer = $timer->cb (sub { my ($timer) = @_; ... })

Set the callback to be called when the timer triggers.

=item $tstamp = $timer->at

Return the time this watcher will fire next.

=item $timer = $timer->set ($tstamp)

Set the time the event is generated to $tstamp.

=item $timer = $timer->interval ($interval)

Normally (and when C<$interval> is C<0>), the timer will automatically
stop after it has fired once. If C<$interval> is non-zero, then the timer
is automatically rescheduled at the given intervals.

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

=item =0 - only fatal messages

=item =3 - script loading and management

=item =10 - all events received

=back

=head1 AUTHOR

 Marc Lehmann <pcg@goof.com>
 http://software.schmorp.de/pkg/rxvt-unicode

=cut

1
