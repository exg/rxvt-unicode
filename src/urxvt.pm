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

Every time a terminal object gets created, extension scripts specified via
the C<perl> resource are loaded and associated with it.

Scripts are compiled in a 'use strict' and 'use utf8' environment, and
thus must be encoded as UTF-8.

Each script will only ever be loaded once, even in @@RXVT_NAME@@d, where
scripts will be shared (but not enabled) for all terminals.

You can disable the embedded perl interpreter by setting both "perl-ext"
and "perl-ext-common" resources to the empty string.

=head1 PREPACKAGED EXTENSIONS

This section describes the extensions delivered with this release. You can
find them in F<@@RXVT_LIBDIR@@/urxvt/perl/>.

You can activate them like this:

  @@RXVT_NAME@@ -pe <extensionname>

Or by adding them to the resource for extensions loaded by default:

  URxvt.perl-ext-common: default,selection-autotransform

=over 4

=item selection (enabled by default)

(More) intelligent selection. This extension tries to be more intelligent
when the user extends selections (double-click and further clicks). Right
now, it tries to select words, urls and complete shell-quoted
arguments, which is very convenient, too, if your F<ls> supports
C<--quoting-style=shell>.

A double-click usually selects the word under the cursor, further clicks
will enlarge the selection.

The selection works by trying to match a number of regexes and displaying
them in increasing order of length. You can add your own regexes by
specifying resources of the form:

   URxvt.selection.pattern-0: perl-regex
   URxvt.selection.pattern-1: perl-regex
   ...

The index number (0, 1...) must not have any holes, and each regex must
contain at least one pair of capturing parentheses, which will be used for
the match. For example, the following adds a regex that matches everything
between two vertical bars:

   URxvt.selection.pattern-0: \\|([^|]+)\\|

Another example: Programs I use often output "absolute path: " at the
beginning of a line when they process multiple files. The following
pattern matches the filename (note, there is a single space at the very
end):

   URxvt.selection.pattern-0: ^(/[^:]+):\ 

You can look at the source of the selection extension to see more
interesting uses, such as parsing a line from beginning to end.

This extension also offers following bindable keyboard commands:

=over 4

=item rot13

Rot-13 the selection when activated. Used via keyboard trigger:

   URxvt.keysym.C-M-r: perl:selection:rot13

=back

=item option-popup (enabled by default)

Binds a popup menu to Ctrl-Button2 that lets you toggle (some) options at
runtime.

Other extensions can extend this popup menu by pushing a code reference
onto C<@{ $term->{option_popup_hook} }>, which gets called whenever the
popup is being displayed.

Its sole argument is the popup menu, which can be modified.  It should
either return nothing or a string, the initial boolean value and a code
reference. The string will be used as button text and the code reference
will be called when the toggle changes, with the new boolean value as
first argument.

The following will add an entry C<myoption> that changes
C<< $self->{myoption} >>:

   push @{ $self->{term}{option_popup_hook} }, sub {
      ("my option" => $myoption, sub { $self->{myoption} = $_[0] })
   };

=item selection-popup (enabled by default)

Binds a popup menu to Ctrl-Button3 that lets you convert the selection
text into various other formats/action (such as uri unescaping, perl
evaluation, web-browser starting etc.), depending on content.

Other extensions can extend this popup menu by pushing a code reference
onto C<@{ $term->{selection_popup_hook} }>, which gets called whenever the
popup is being displayed.

Its sole argument is the popup menu, which can be modified. The selection
is in C<$_>, which can be used to decide whether to add something or not.
It should either return nothing or a string and a code reference. The
string will be used as button text and the code reference will be called
when the button gets activated and should transform C<$_>.

The following will add an entry C<a to b> that transforms all C<a>s in
the selection to C<b>s, but only if the selection currently contains any
C<a>s:

   push @{ $self->{term}{selection_popup_hook} }, sub {
      /a/ ? ("a to b" => sub { s/a/b/g }
          : ()
   };

=item searchable-scrollback<hotkey> (enabled by default)

Adds regex search functionality to the scrollback buffer, triggered
by a hotkey (default: C<M-s>). While in search mode, normal terminal
input/output is suspended and a regex is displayed at the bottom of the
screen.

Inputting characters appends them to the regex and continues incremental
search. C<BackSpace> removes a character from the regex, C<Up> and C<Down>
search upwards/downwards in the scrollback buffer, C<End> jumps to the
bottom. C<Escape> leaves search mode and returns to the point where search
was started, while C<Enter> or C<Return> stay at the current position and
additionally stores the first match in the current line into the primary
selection if the C<Shift> modifier is active.

The regex defaults to "(?i)", resulting in a case-insensitive search. To
get a case-sensitive search you can delete this prefix using C<BackSpace>
or simply use an uppercase character which removes the "(?i)" prefix.

See L<perlre> for more info about perl regular expression syntax.

=item readline (enabled by default)

A support package that tries to make editing with readline easier. At
the moment, it reacts to clicking shift-left mouse button by trying to
move the text cursor to this position. It does so by generating as many
cursor-left or cursor-right keypresses as required (this only works
for programs that correctly support wide characters).

To avoid too many false positives, this is only done when:

=over 4

=item - the tty is in ICANON state.

=item - the text cursor is visible.

=item - the primary screen is currently being displayed.

=item - the mouse is on the same (multi-row-) line as the text cursor.

=back

The normal selection mechanism isn't disabled, so quick successive clicks
might interfere with selection creation in harmless ways.

=item selection-autotransform

This selection allows you to do automatic transforms on a selection
whenever a selection is made.

It works by specifying perl snippets (most useful is a single C<s///>
operator) that modify C<$_> as resources:

   URxvt.selection-autotransform.0: transform
   URxvt.selection-autotransform.1: transform
   ...

For example, the following will transform selections of the form
C<filename:number>, often seen in compiler messages, into C<vi +$filename
$word>:

   URxvt.selection-autotransform.0: s/^([^:[:space:]]+):(\\d+):?$/vi +$2 \\Q$1\\E\\x0d/

And this example matches the same,but replaces it with vi-commands you can
paste directly into your (vi :) editor:

   URxvt.selection-autotransform.0: s/^([^:[:space:]]+(\\d+):?$/:e \\Q$1\\E\\x0d:$2\\x0d/

Of course, this can be modified to suit your needs and your editor :)

To expand the example above to typical perl error messages ("XXX at
FILENAME line YYY."), you need a slightly more elaborate solution:

   URxvt.selection.pattern-0: ( at .*? line \\d+[,.])
   URxvt.selection-autotransform.0: s/^ at (.*?) line (\\d+)[,.]$/:e \\Q$1\E\\x0d:$2\\x0d/

The first line tells the selection code to treat the unchanging part of
every error message as a selection pattern, and the second line transforms
the message into vi commands to load the file.

=item tabbed

This transforms the terminal into a tabbar with additional terminals, that
is, it implements what is commonly referred to as "tabbed terminal". The topmost line
displays a "[NEW]" button, which, when clicked, will add a new tab, followed by one
button per tab.

Clicking a button will activate that tab. Pressing B<Shift-Left> and
B<Shift-Right> will switch to the tab left or right of the current one,
while B<Shift-Down> creates a new tab.

The tabbar itself can be configured similarly to a normal terminal, but
with a resource class of C<URxvt.tabbed>. In addition, it supports the
following four resources (shown with defaults):

   URxvt.tabbed.tabbar-fg: <colour-index, default 3>
   URxvt.tabbed.tabbar-bg: <colour-index, default 0>
   URxvt.tabbed.tab-fg:    <colour-index, default 0>
   URxvt.tabbed.tab-bg:    <colour-index, default 1>

See I<COLOR AND GRAPHICS> in the @@RXVT_NAME@@(1) manpage for valid
indices.

=item matcher

Uses per-line display filtering (C<on_line_update>) to underline text
matching a certain pattern and make it clickable. When clicked with the
mouse button specified in the C<matcher.button> resource (default 2, or
middle), the program specified in the C<matcher.launcher> resource
(default, the C<urlLauncher> resource, C<sensible-browser>) will be started
with the matched text as first argument.  The default configuration is
suitable for matching URLs and launching a web browser, like the
former "mark-urls" extension.

The default pattern to match URLs can be overridden with the
C<matcher.pattern.0> resource, and additional patterns can be specified
with numbered patterns, in a manner similar to the "selection" extension.
The launcher can also be overridden on a per-pattern basis.

It is possible to activate the most recently seen match or a list of matches
from the keyboard.  Simply bind a keysym to "perl:matcher:last" or
"perl:matcher:list" as seen in the example below.

Example configuration:

    URxvt.perl-ext:           default,matcher
    URxvt.urlLauncher:        sensible-browser
    URxvt.keysym.C-Delete:    perl:matcher:last
    URxvt.keysym.M-Delete:    perl:matcher:list
    URxvt.matcher.button:     1
    URxvt.matcher.pattern.1:  \\bwww\\.[\\w-]+\\.[\\w./?&@#-]*[\\w/-]
    URxvt.matcher.pattern.2:  \\B(/\\S+?):(\\d+)(?=:|$)
    URxvt.matcher.launcher.2: gvim +$2 $1

=item xim-onthespot

This (experimental) perl extension implements OnTheSpot editing. It does
not work perfectly, and some input methods don't seem to work well with
OnTheSpot editing in general, but it seems to work at least for SCIM and
kinput2.

You enable it by specifying this extension and a preedit style of
C<OnTheSpot>, i.e.:

   @@RXVT_NAME@@ -pt OnTheSpot -pe xim-onthespot

=item kuake<hotkey>

A very primitive quake-console-like extension. It was inspired by a
description of how the programs C<kuake> and C<yakuake> work: Whenever the
user presses a global accelerator key (by default C<F10>), the terminal
will show or hide itself. Another press of the accelerator key will hide
or show it again.

Initially, the window will not be shown when using this extension.

This is useful if you need a single terminal that is not using any desktop
space most of the time but is quickly available at the press of a key.

The accelerator key is grabbed regardless of any modifiers, so this
extension will actually grab a physical key just for this function.

If you want a quake-like animation, tell your window manager to do so
(fvwm can do it).

=item overlay-osc

This extension implements some OSC commands to display timed popups on the
screen - useful for status displays from within scripts. You have to read
the sources for more info.

=item block-graphics-to-ascii

A not very useful example of filtering all text output to the terminal
by replacing all line-drawing characters (U+2500 .. U+259F) by a
similar-looking ascii character.

=item digital-clock

Displays a digital clock using the built-in overlay.

=item remote-clipboard

Somewhat of a misnomer, this extension adds two menu entries to the
selection popup that allows one to run external commands to store the
selection somewhere and fetch it again.

We use it to implement a "distributed selection mechanism", which just
means that one command uploads the file to a remote server, and another
reads it.

The commands can be set using the C<URxvt.remote-selection.store> and
C<URxvt.remote-selection.fetch> resources. The first should read the
selection to store from STDIN (always in UTF-8), the second should provide
the selection data on STDOUT (also in UTF-8).

The defaults (which are likely useless to you) use rsh and cat:

   URxvt.remote-selection.store: rsh ruth 'cat >/tmp/distributed-selection'
   URxvt.remote-selection.fetch: rsh ruth 'cat /tmp/distributed-selection'

=item selection-pastebin

This is a little rarely useful extension that uploads the selection as
textfile to a remote site (or does other things). (The implementation is
not currently secure for use in a multiuser environment as it writes to
F</tmp> directly.).

It listens to the C<selection-pastebin:remote-pastebin> keyboard command,
i.e.

   URxvt.keysym.C-M-e: perl:selection-pastebin:remote-pastebin

Pressing this combination runs a command with C<%> replaced by the name of
the textfile. This command can be set via a resource:

   URxvt.selection-pastebin.cmd: rsync -apP % ruth:/var/www/www.ta-sa.org/files/txt/.

And the default is likely not useful to anybody but the few people around
here :)

The name of the textfile is the hex encoded md5 sum of the selection, so
the same content should lead to the same filename.

After a successful upload the selection will be replaced by the text given
in the C<selection-pastebin-url> resource (again, the % is the placeholder
for the filename):

   URxvt.selection-pastebin.url: http://www.ta-sa.org/files/txt/%

I<Note to xrdb users:> xrdb uses the C preprocessor, which might interpret
the double C</> characters as comment start. Use C<\057\057> instead,
which works regardless of whether xrdb is used to parse the resource file
or not.

=item macosx-clipboard and macosx-clipboard-native

These two modules implement an extended clipboard for Mac OS X. They are
used like this:

   URxvt.perl-ext-common: default,macosx-clipboard
   URxvt.keysym.M-c: perl:macosx-clipboard:copy
   URxvt.keysym.M-v: perl:macosx-clipboard:paste

The difference between them is that the native variant requires a
perl from apple's devkit or so, and C<macosx-clipboard> requires the
C<Mac::Pasteboard> module, works with other perls, has fewer bugs, is
simpler etc. etc.

=item example-refresh-hooks

Displays a very simple digital clock in the upper right corner of the
window. Illustrates overwriting the refresh callbacks to create your own
overlays or changes.

=item confirm-paste

Displays a confirmation dialog when a paste containing at least a full
line is detected.

=item bell-command

Runs the command specified by the C<URxvt.bell-command> resource when
a bell event occurs. For example, the following pops up a notification
bubble with the text "Beep, Beep" using notify-send:

   URxvt.bell-command: notify-send "Beep, Beep"

=back

=head1 API DOCUMENTATION

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

Rxvt-unicode's special way of encoding text, where one "unicode" character
always represents one screen cell. See L<ROW_t> for a discussion of this format.

=item $string

A perl text string, with an emphasis on I<text>. It can store all unicode
characters and is to be distinguished with text encoded in a specific
encoding (often locale-specific) and binary data.

=item $octets

Either binary data or - more common - a text string encoded in a
locale-specific way.

=item $keysym

an integer that is a valid X11 keysym code. You can convert a string
into a keysym and viceversa by using C<XStringToKeysym> and
C<XKeysymToString>.

=back

=head2 Extension Objects

Every perl extension is a perl class. A separate perl object is created
for each terminal, and each terminal has its own set of extension objects,
which are passed as the first parameter to hooks. So extensions can use
their C<$self> object without having to think about clashes with other
extensions or other terminals, with the exception of methods and members
that begin with an underscore character C<_>: these are reserved for
internal use.

Although it isn't a C<urxvt::term> object, you can call all methods of the
C<urxvt::term> class on this object.

It has the following methods and data members:

=over 4

=item $urxvt_term = $self->{term}

Returns the C<urxvt::term> object associated with this instance of the
extension. This member I<must not> be changed in any way.

=item $self->enable ($hook_name => $cb, [$hook_name => $cb..])

Dynamically enable the given hooks (named without the C<on_> prefix) for
this extension, replacing any previous hook. This is useful when you want
to overwrite time-critical hooks only temporarily.

=item $self->disable ($hook_name[, $hook_name..])

Dynamically disable the given hooks.

=back

=head2 Hooks

The following subroutines can be declared in extension files, and will be
called whenever the relevant event happens.

The first argument passed to them is an extension object as described in
the in the C<Extension Objects> section.

B<All> of these hooks must return a boolean value. If any of the called
hooks returns true, then the event counts as being I<consumed>, and the
relevant action might not be carried out by the C++ code.

I<< When in doubt, return a false value (preferably C<()>). >>

=over 4

=item on_init $term

Called after a new terminal object has been initialized, but before
windows are created or the command gets run. Most methods are unsafe to
call or deliver senseless data, as terminal size and other characteristics
have not yet been determined. You can safely query and change resources
and options, though. For many purposes the C<on_start> hook is a better
place.

=item on_start $term

Called at the very end of initialisation of a new terminal, just before
trying to map (display) the toplevel and returning to the main loop.

=item on_destroy $term

Called whenever something tries to destroy terminal, when the terminal is
still fully functional (not for long, though).

=item on_reset $term

Called after the screen is "reset" for any reason, such as resizing or
control sequences. Here is where you can react on changes to size-related
variables.

=item on_child_start $term, $pid

Called just after the child process has been C<fork>ed.

=item on_child_exit $term, $status

Called just after the child process has exited. C<$status> is the status
from C<waitpid>.

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

Returning a true value aborts selection grabbing. It will still be highlighted.

=item on_sel_extend $term

Called whenever the user tries to extend the selection (e.g. with a double
click) and is either supposed to return false (normal operation), or
should extend the selection itself and return true to suppress the built-in
processing. This can happen multiple times, as long as the callback
returns true, it will be called on every further click by the user and is
supposed to enlarge the selection more and more, if possible.

See the F<selection> example extension.

=item on_view_change $term, $offset

Called whenever the view offset changes, i.e. the user or program
scrolls. Offset C<0> means display the normal terminal, positive values
show this many lines of scrollback.

=item on_scroll_back $term, $lines, $saved

Called whenever lines scroll out of the terminal area into the scrollback
buffer. C<$lines> is the number of lines scrolled out and may be larger
than the scroll back buffer or the terminal.

It is called before lines are scrolled out (so rows 0 .. min ($lines - 1,
$nrow - 1) represent the lines to be scrolled out). C<$saved> is the total
number of lines that will be in the scrollback buffer.

=item on_osc_seq $term, $op, $args, $resp

Called on every OSC sequence and can be used to suppress it or modify its
behaviour. The default should be to return an empty list. A true value
suppresses execution of the request completely. Make sure you don't get
confused by recursive invocations when you output an OSC sequence within
this callback.

C<on_osc_seq_perl> should be used for new behaviour.

=item on_osc_seq_perl $term, $args, $resp

Called whenever the B<ESC ] 777 ; string ST> command sequence (OSC =
operating system command) is processed. Cursor position and other state
information is up-to-date when this happens. For interoperability, the
string should start with the extension name (sans -osc) and a semicolon,
to distinguish it from commands for other extensions, and this might be
enforced in the future.

For example, C<overlay-osc> uses this:

   sub on_osc_seq_perl {
      my ($self, $osc, $resp) = @_;

      return unless $osc =~ s/^overlay;//;

      ... process remaining $osc string
   }

Be careful not ever to trust (in a security sense) the data you receive,
as its source can not easily be controlled (e-mail content, messages from
other users on the same system etc.).

For responses, C<$resp> contains the end-of-args separator used by the
sender.

=item on_add_lines $term, $string

Called whenever text is about to be output, with the text as argument. You
can filter/change and output the text yourself by returning a true value
and calling C<< $term->scr_add_lines >> yourself. Please note that this
might be very slow, however, as your hook is called for B<all> text being
output.

=item on_tt_write $term, $octets

Called whenever some data is written to the tty/pty and can be used to
suppress or filter tty input.

=item on_tt_paste $term, $octets

Called whenever text is about to be pasted, with the text as argument. You
can filter/change and paste the text yourself by returning a true value
and calling C<< $term->tt_paste >> yourself. C<$octets> is
locale-encoded.

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

Called just before the screen gets redrawn. Can be used for overlay or
similar effects by modifying the terminal contents in refresh_begin, and
restoring them in refresh_end. The built-in overlay and selection display
code is run after this hook, and takes precedence.

=item on_refresh_end $term

Called just after the screen gets redrawn. See C<on_refresh_begin>.

=item on_user_command $term, $string

Called whenever a user-configured event is being activated (e.g. via
a C<perl:string> action bound to a key, see description of the B<keysym>
resource in the @@RXVT_NAME@@(1) manpage).

The event is simply the action string. This interface is assumed to change
slightly in the future.

=item on_register_command $term, $keysym, $modifiermask, $string

Called after parsing a keysym resource but before registering the
associated binding. If this hook returns TRUE the binding is not
registered. It can be used to modify a binding by calling
C<register_command>.

=item on_resize_all_windows $term, $new_width, $new_height

Called just after the new window size has been calculated, but before
windows are actually being resized or hints are being set. If this hook
returns TRUE, setting of the window hints is being skipped.

=item on_x_event $term, $event

Called on every X event received on the vt window (and possibly other
windows). Should only be used as a last resort. Most event structure
members are not passed.

=item on_root_event $term, $event

Like C<on_x_event>, but is called for events on the root window.

=item on_focus_in $term

Called whenever the window gets the keyboard focus, before rxvt-unicode
does focus in processing.

=item on_focus_out $term

Called whenever the window loses keyboard focus, before rxvt-unicode does
focus out processing.

=item on_configure_notify $term, $event

=item on_property_notify $term, $event

=item on_key_press $term, $event, $keysym, $octets

=item on_key_release $term, $event, $keysym

=item on_button_press $term, $event

=item on_button_release $term, $event

=item on_motion_notify $term, $event

=item on_map_notify $term, $event

=item on_unmap_notify $term, $event

Called whenever the corresponding X event is received for the terminal. If
the hook returns true, then the event will be ignored by rxvt-unicode.

The event is a hash with most values as named by Xlib (see the XEvent
manpage), with the additional members C<row> and C<col>, which are the
(real, not screen-based) row and column under the mouse cursor.

C<on_key_press> additionally receives the string rxvt-unicode would
output, if any, in locale-specific encoding.

subwindow.

=item on_client_message $term, $event

=item on_wm_protocols $term, $event

=item on_wm_delete_window $term, $event

Called when various types of ClientMessage events are received (all with
format=32, WM_PROTOCOLS or WM_PROTOCOLS:WM_DELETE_WINDOW).

=item on_bell $term

Called on receipt of a bell character.

=back

=cut

package urxvt;

use utf8;
use strict;
use Carp ();
use Scalar::Util ();
use List::Util ();

our $VERSION = 1;
our $TERM;
our @TERM_INIT;
our @TERM_EXT;
our @HOOKNAME;
our %HOOKTYPE = map +($HOOKNAME[$_] => $_), 0..$#HOOKNAME;
our %OPTION;

our $LIBDIR;
our $RESNAME;
our $RESCLASS;
our $RXVTNAME;

our $NOCHAR = chr 0xffff;

=head2 Variables in the C<urxvt> Package

=over 4

=item $urxvt::LIBDIR

The rxvt-unicode library directory, where, among other things, the perl
modules and scripts are stored.

=item $urxvt::RESCLASS, $urxvt::RESCLASS

The resource class and name rxvt-unicode uses to look up X resources.

=item $urxvt::RXVTNAME

The basename of the installed binaries, usually C<urxvt>.

=item $urxvt::TERM

The current terminal. This variable stores the current C<urxvt::term>
object, whenever a callback/hook is executing.

=item @urxvt::TERM_INIT

All code references in this array will be called as methods of the next newly
created C<urxvt::term> object (during the C<on_init> phase). The array
gets cleared before the code references that were in it are being executed,
so references can push themselves onto it again if they so desire.

This complements to the perl-eval command line option, but gets executed
first.

=item @urxvt::TERM_EXT

Works similar to C<@TERM_INIT>, but contains perl package/class names, which
get registered as normal extensions after calling the hooks in C<@TERM_INIT>
but before other extensions. Gets cleared just like C<@TERM_INIT>.

=back

=head2 Functions in the C<urxvt> Package

=over 4

=item urxvt::fatal $errormessage

Fatally aborts execution with the given error message (which should
include a trailing newline). Avoid at all costs! The only time this
is acceptable (and useful) is in the init hook, where it prevents the
terminal from starting up.

=item urxvt::warn $string

Calls C<rxvt_warn> with the given string which should include a trailing
newline. The module also overwrites the C<warn> builtin with a function
that calls this function.

Using this function has the advantage that its output ends up in the
correct place, e.g. on stderr of the connecting urxvtc client.

Messages have a size limit of 1023 bytes currently.

=item @terms = urxvt::termlist

Returns all urxvt::term objects that exist in this process, regardless of
whether they are started, being destroyed etc., so be careful. Only term
objects that have perl extensions attached will be returned (because there
is no urxvt::term object associated with others).

=item $time = urxvt::NOW

Returns the "current time" (as per the event loop).

=item urxvt::CurrentTime

=item urxvt::ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask,
Mod3Mask, Mod4Mask, Mod5Mask, Button1Mask, Button2Mask, Button3Mask,
Button4Mask, Button5Mask, AnyModifier

=item urxvt::NoEventMask, KeyPressMask, KeyReleaseMask,
ButtonPressMask, ButtonReleaseMask, EnterWindowMask, LeaveWindowMask,
PointerMotionMask, PointerMotionHintMask, Button1MotionMask, Button2MotionMask,
Button3MotionMask, Button4MotionMask, Button5MotionMask, ButtonMotionMask,
KeymapStateMask, ExposureMask, VisibilityChangeMask, StructureNotifyMask,
ResizeRedirectMask, SubstructureNotifyMask, SubstructureRedirectMask,
FocusChangeMask, PropertyChangeMask, ColormapChangeMask, OwnerGrabButtonMask

=item urxvt::KeyPress, KeyRelease, ButtonPress, ButtonRelease, MotionNotify,
EnterNotify, LeaveNotify, FocusIn, FocusOut, KeymapNotify, Expose,
GraphicsExpose, NoExpose, VisibilityNotify, CreateNotify, DestroyNotify,
UnmapNotify, MapNotify, MapRequest, ReparentNotify, ConfigureNotify,
ConfigureRequest, GravityNotify, ResizeRequest, CirculateNotify,
CirculateRequest, PropertyNotify, SelectionClear, SelectionRequest,
SelectionNotify, ColormapNotify, ClientMessage, MappingNotify

Various constants for use in X calls and event processing.

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

=item $rendbit = urxvt::RS_Bold, urxvt::RS_Italic, urxvt::RS_Blink,
urxvt::RS_RVid, urxvt::RS_Uline

Return the bit that enabled bold, italic, blink, reverse-video and
underline, respectively. To enable such a style, just logically OR it into
the bitset.

=item $foreground = urxvt::GET_BASEFG $rend

=item $background = urxvt::GET_BASEBG $rend

Return the foreground/background colour index, respectively.

=item $rend = urxvt::SET_FGCOLOR $rend, $new_colour

=item $rend = urxvt::SET_BGCOLOR $rend, $new_colour

=item $rend = urxvt::SET_COLOR $rend, $new_fg, $new_bg

Replace the foreground/background colour in the rendition mask with the
specified one.

=item $value = urxvt::GET_CUSTOM $rend

Return the "custom" value: Every rendition has 5 bits for use by
extensions. They can be set and changed as you like and are initially
zero.

=item $rend = urxvt::SET_CUSTOM $rend, $new_value

Change the custom value.

=back

=cut

BEGIN {
   # overwrite perl's warn
   *CORE::GLOBAL::warn = sub {
      my $msg = join "", @_;
      $msg .= "\n"
         unless $msg =~ /\n$/;
      urxvt::warn ($msg);
   };
}

no warnings 'utf8';

my $verbosity = $ENV{URXVT_PERL_VERBOSITY};

sub verbose {
   my ($level, $msg) = @_;
   warn "$msg\n" if $level <= $verbosity;
}

my %extension_pkg;

# load a single script into its own package, once only
sub extension_package($) {
   my ($path) = @_;

   $extension_pkg{$path} ||= do {
      $path =~ /([^\/\\]+)$/;
      my $pkg = $1;
      $pkg =~ s/[^[:word:]]/_/g;
      $pkg = "urxvt::ext::$pkg";

      verbose 3, "loading extension '$path' into package '$pkg'";

      open my $fh, "<:raw", $path
         or die "$path: $!";

      my $source =
         "package $pkg; use strict; use utf8; no warnings 'utf8';\n"
         . "#line 1 \"$path\"\n{\n"
         . (do { local $/; <$fh> })
         . "\n};\n1";

      eval $source
         or die "$path: $@";

      $pkg
   }
}

our $retval; # return value for urxvt

# called by the rxvt core
sub invoke {
   local $TERM = shift;
   my $htype = shift;

   if ($htype == 0) { # INIT
      my @dirs = ((split /:/, $TERM->resource ("perl_lib")), "$ENV{HOME}/.urxvt/ext", "$LIBDIR/perl");

      my %ext_arg;

      {
         my @init = @TERM_INIT;
         @TERM_INIT = ();
         $_->($TERM) for @init;
         my @pkg = @TERM_EXT;
         @TERM_EXT = ();
         $TERM->register_package ($_) for @pkg;
      }

      for (grep $_, map { split /,/, $TERM->resource ("perl_ext_$_") } 1, 2) {
         if ($_ eq "default") {
            $ext_arg{$_} ||= [] for qw(selection option-popup selection-popup searchable-scrollback readline);
         } elsif (/^-(.*)$/) {
            delete $ext_arg{$1};
         } elsif (/^([^<]+)<(.*)>$/) {
            push @{ $ext_arg{$1} }, $2;
         } else {
            $ext_arg{$_} ||= [];
         }
      }

      for my $ext (sort keys %ext_arg) {
         my @files = grep -f $_, map "$_/$ext", @dirs;

         if (@files) {
            $TERM->register_package (extension_package $files[0], $ext_arg{$ext});
         } else {
            warn "perl extension '$ext' not found in perl library search path\n";
         }
      }

      eval "#line 1 \"--perl-eval resource/argument\"\n" . $TERM->resource ("perl_eval");
      warn $@ if $@;
   }

   $retval = undef;

   if (my $cb = $TERM->{_hook}[$htype]) {
      verbose 10, "$HOOKNAME[$htype] (" . (join ", ", $TERM, @_) . ")"
         if $verbosity >= 10;

      for my $pkg (keys %$cb) {
         my $retval_ = eval { $cb->{$pkg}->($TERM->{_pkg}{$pkg}, @_) };
         $retval ||= $retval_;

         if ($@) {
            $TERM->ungrab; # better to lose the grab than the session
            warn $@;
         }
      }

      verbose 11, "$HOOKNAME[$htype] returning <$retval>"
         if $verbosity >= 11;
   }

   if ($htype == 1) { # DESTROY
      # clear package objects
      %$_ = () for values %{ $TERM->{_pkg} };

      # clear package
      %$TERM = ();
   }

   $retval
}

sub SET_COLOR($$$) {
   SET_BGCOLOR (SET_FGCOLOR ($_[0], $_[1]), $_[2])
}

sub rend2mask {
   no strict 'refs';
   my ($str, $mask) = (@_, 0);
   my %color = ( fg => undef, bg => undef );
   my @failed;
   for my $spec ( split /\s+/, $str ) {
      if ( $spec =~ /^([fb]g)[_:-]?(\d+)/i ) {
         $color{lc($1)} = $2;
      } else {
         my $neg = $spec =~ s/^[-^]//;
         unless ( exists &{"RS_$spec"} ) {
            push @failed, $spec;
            next;
         }
         my $cur = &{"RS_$spec"};
         if ( $neg ) {
            $mask &= ~$cur;
         } else {
            $mask |= $cur;
         }
      }
   }
   ($mask, @color{qw(fg bg)}, \@failed)
}

# urxvt::term::extension

package urxvt::term::extension;

sub enable {
   my ($self, %hook) = @_;
   my $pkg = $self->{_pkg};

   while (my ($name, $cb) = each %hook) {
      my $htype = $HOOKTYPE{uc $name};
      defined $htype
         or Carp::croak "unsupported hook type '$name'";

      $self->set_should_invoke ($htype, +1)
         unless exists $self->{term}{_hook}[$htype]{$pkg};

      $self->{term}{_hook}[$htype]{$pkg} = $cb;
   }
}

sub disable {
   my ($self, @hook) = @_;
   my $pkg = $self->{_pkg};

   for my $name (@hook) {
      my $htype = $HOOKTYPE{uc $name};
      defined $htype
         or Carp::croak "unsupported hook type '$name'";

      $self->set_should_invoke ($htype, -1)
         if delete $self->{term}{_hook}[$htype]{$pkg};
   }
}

our $AUTOLOAD;

sub AUTOLOAD {
   $AUTOLOAD =~ /:([^:]+)$/
      or die "FATAL: \$AUTOLOAD '$AUTOLOAD' unparsable";

   eval qq{
      sub $AUTOLOAD {
         my \$proxy = shift;
         \$proxy->{term}->$1 (\@_)
      }
      1
   } or die "FATAL: unable to compile method forwarder: $@";

   goto &$AUTOLOAD;
}

sub DESTROY {
   # nop
}

# urxvt::destroy_hook

sub urxvt::destroy_hook::DESTROY {
   ${$_[0]}->();
}

sub urxvt::destroy_hook(&) {
   bless \shift, urxvt::destroy_hook::
}

package urxvt::anyevent;

=head2 The C<urxvt::anyevent> Class

The sole purpose of this class is to deliver an interface to the
C<AnyEvent> module - any module using it will work inside urxvt without
further programming. The only exception is that you cannot wait on
condition variables, but non-blocking condvar use is ok. What this means
is that you cannot use blocking APIs, but the non-blocking variant should
work.

=cut

our $VERSION = '5.23';

$INC{"urxvt/anyevent.pm"} = 1; # mark us as there
push @AnyEvent::REGISTRY, [urxvt => urxvt::anyevent::];

sub timer {
   my ($class, %arg) = @_;

   my $cb = $arg{cb};

   urxvt::timer
      ->new
      ->after ($arg{after}, $arg{interval})
      ->cb ($arg{interval} ? $cb : sub {
        $_[0]->stop; # need to cancel manually
        $cb->();
      })
}

sub io {
   my ($class, %arg) = @_;

   my $cb = $arg{cb};
   my $fd = fileno $arg{fh};
   defined $fd or $fd = $arg{fh};

   bless [$arg{fh}, urxvt::iow
             ->new
             ->fd ($fd)
             ->events (($arg{poll} =~ /r/ ? 1 : 0)
                     | ($arg{poll} =~ /w/ ? 2 : 0))
             ->start
             ->cb ($cb)
         ], urxvt::anyevent::
}

sub idle {
   my ($class, %arg) = @_;

   my $cb = $arg{cb};

   urxvt::iw
      ->new
      ->start
      ->cb ($cb)
}

sub child {
   my ($class, %arg) = @_;

   my $cb = $arg{cb};

   urxvt::pw
      ->new
      ->start ($arg{pid})
      ->cb (sub {
        $_[0]->stop; # need to cancel manually
        $cb->($_[0]->rpid, $_[0]->rstatus);
      })
}

sub DESTROY {
   $_[0][1]->stop;
}

# only needed for AnyEvent < 6 compatibility
sub one_event {
   Carp::croak "AnyEvent->one_event blocking wait unsupported in urxvt, use a non-blocking API";
}

package urxvt::term;

=head2 The C<urxvt::term> Class

=over 4

=cut

# find on_xxx subs in the package and register them
# as hooks
sub register_package {
   my ($self, $pkg, $argv) = @_;

   no strict 'refs';

   urxvt::verbose 6, "register package $pkg to $self";

   @{"$pkg\::ISA"} = urxvt::term::extension::;

   my $proxy = bless {
      _pkg => $pkg,
      argv => $argv,
   }, $pkg;
   Scalar::Util::weaken ($proxy->{term} = $self);

   $self->{_pkg}{$pkg} = $proxy;

   for my $name (@HOOKNAME) {
      if (my $ref = $pkg->can ("on_" . lc $name)) {
         $proxy->enable ($name => $ref);
      }
   }
}

=item $term = new urxvt::term $envhashref, $rxvtname, [arg...]

Creates a new terminal, very similar as if you had started it with system
C<$rxvtname, arg...>. C<$envhashref> must be a reference to a C<%ENV>-like
hash which defines the environment of the new terminal.

Croaks (and probably outputs an error message) if the new instance
couldn't be created.  Returns C<undef> if the new instance didn't
initialise perl, and the terminal object otherwise. The C<init> and
C<start> hooks will be called before this call returns, and are free to
refer to global data (which is race free).

=cut

sub new {
   my ($class, $env, @args) = @_;

   $env  or Carp::croak "environment hash missing in call to urxvt::term->new";
   @args or Carp::croak "name argument missing in call to urxvt::term->new";

   _new ([ map "$_=$env->{$_}", keys %$env ], \@args);
}

=item $term->destroy

Destroy the terminal object (close the window, free resources
etc.). Please note that @@RXVT_NAME@@ will not exit as long as any event
watchers (timers, io watchers) are still active.

=item $term->exec_async ($cmd[, @args])

Works like the combination of the C<fork>/C<exec> builtins, which executes
("starts") programs in the background. This function takes care of setting
the user environment before exec'ing the command (e.g. C<PATH>) and should
be preferred over explicit calls to C<exec> or C<system>.

Returns the pid of the subprocess or C<undef> on error.

=cut

sub exec_async {
   my $self = shift;

   my $pid = fork;

   return $pid
      if !defined $pid or $pid;

   %ENV = %{ $self->env };

   exec @_;
   urxvt::_exit 255;
}

=item $isset = $term->option ($optval[, $set])

Returns true if the option specified by C<$optval> is enabled, and
optionally change it. All option values are stored by name in the hash
C<%urxvt::OPTION>. Options not enabled in this binary are not in the hash.

Here is a likely non-exhaustive list of option names, please see the
source file F</src/optinc.h> to see the actual list:

 borderLess buffered console cursorBlink cursorUnderline hold iconic
 insecure intensityStyles iso14755 iso14755_52 jumpScroll loginShell
 mapAlert meta8 mouseWheelScrollPage override_redirect pastableTabs
 pointerBlank reverseVideo scrollBar scrollBar_floating scrollBar_right
 scrollTtyKeypress scrollTtyOutput scrollWithBuffer secondaryScreen
 secondaryScroll skipBuiltinGlyphs skipScroll transparent tripleclickwords
 urgentOnBell utmpInhibit visualBell

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

Here is a likely non-exhaustive list of resource names, not all of which
are supported in every build, please see the source file F</src/rsinc.h>
to see the actual list:

  answerbackstring backgroundPixmap backspace_key blendtype blurradius
  boldFont boldItalicFont borderLess buffered chdir color cursorBlink
  cursorUnderline cutchars delete_key depth display_name embed ext_bwidth
  fade font geometry hold iconName iconfile imFont imLocale inputMethod
  insecure int_bwidth intensityStyles iso14755 iso14755_52 italicFont
  jumpScroll letterSpace lineSpace loginShell mapAlert meta8 modifier
  mouseWheelScrollPage name override_redirect pastableTabs path perl_eval
  perl_ext_1 perl_ext_2 perl_lib pointerBlank pointerBlankDelay
  preeditType print_pipe pty_fd reverseVideo saveLines scrollBar
  scrollBar_align scrollBar_floating scrollBar_right scrollBar_thickness
  scrollTtyKeypress scrollTtyOutput scrollWithBuffer scrollstyle
  secondaryScreen secondaryScroll shade skipBuiltinGlyphs skipScroll
  term_name title transient_for transparent tripleclickwords urgentOnBell
  utmpInhibit visualBell

=cut

sub resource($$;$) {
   my ($self, $name) = (shift, shift);
   unshift @_, $self, $name, ($name =~ s/\s*\+\s*(\d+)$// ? $1 : 0);
   goto &urxvt::term::_resource
}

=item $value = $term->x_resource ($pattern)

Returns the X-Resource for the given pattern, excluding the program or
class name, i.e.  C<< $term->x_resource ("boldFont") >> should return the
same value as used by this instance of rxvt-unicode. Returns C<undef> if no
resource with that pattern exists.

This method should only be called during the C<on_start> hook, as there is
only one resource database per display, and later invocations might return
the wrong resources.

=item $success = $term->parse_keysym ($key, $octets)

Adds a key binding exactly as specified via a resource. See the
C<keysym> resource in the @@RXVT_NAME@@(1) manpage.

=item $term->register_command ($keysym, $modifiermask, $string)

Adds a key binding. This is a lower level api compared to
C<parse_keysym>, as it expects a parsed key description, and can be
used only inside either the C<on_init> hook, to add a binding, or the
C<on_register_command> hook, to modify a parsed binding.

=item $rend = $term->rstyle ([$new_rstyle])

Return and optionally change the current rendition. Text that is output by
the terminal application will use this style.

=item ($row, $col) = $term->screen_cur ([$row, $col])

Return the current coordinates of the text cursor position and optionally
set it (which is usually bad as applications don't expect that).

=item ($row, $col) = $term->selection_mark ([$row, $col])

=item ($row, $col) = $term->selection_beg ([$row, $col])

=item ($row, $col) = $term->selection_end ([$row, $col])

Return the current values of the selection mark, begin or end positions.

When arguments are given, then the selection coordinates are set to
C<$row> and C<$col>, and the selection screen is set to the current
screen.

=item $screen = $term->selection_screen ([$screen])

Returns the current selection screen, and then optionally sets it.

=item $term->selection_make ($eventtime[, $rectangular])

Tries to make a selection as set by C<selection_beg> and
C<selection_end>. If C<$rectangular> is true (default: false), a
rectangular selection will be made. This is the preferred function to make
a selection.

=item $success = $term->selection_grab ($eventtime[, $clipboard])

Try to acquire ownership of the primary (clipboard if C<$clipboard> is
true) selection from the server. The corresponding text can be set
with the next method. No visual feedback will be given. This function
is mostly useful from within C<on_sel_grab> hooks.

=item $oldtext = $term->selection ([$newtext, $clipboard])

Return the current selection (clipboard if C<$clipboard> is true) text
and optionally replace it by C<$newtext>.

=item $term->selection_clear ([$clipboard])

Revoke ownership of the primary (clipboard if C<$clipboard> is true) selection.

=item $term->overlay_simple ($x, $y, $text)

Create a simple multi-line overlay box. See the next method for details.

=cut

sub overlay_simple {
   my ($self, $x, $y, $text) = @_;

   my @lines = split /\n/, $text;

   my $w = List::Util::max map $self->strwidth ($_), @lines;

   my $overlay = $self->overlay ($x, $y, $w, scalar @lines);
   $overlay->set (0, $_, $lines[$_]) for 0.. $#lines;

   $overlay
}

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

=item $overlay->set ($x, $y, $text[, $rend])

Similar to C<< $term->ROW_t >> and C<< $term->ROW_r >> in that it puts
text in rxvt-unicode's special encoding and an array of rendition values
at a specific position inside the overlay.

If C<$rend> is missing, then the rendition will not be changed.

=item $overlay->hide

If visible, hide the overlay, but do not destroy it.

=item $overlay->show

If hidden, display the overlay again.

=back

=item $popup = $term->popup ($event)

Creates a new C<urxvt::popup> object that implements a popup menu. The
C<$event> I<must> be the event causing the menu to pop up (a button event,
currently).

=cut

sub popup {
   my ($self, $event) = @_;

   $self->grab ($event->{time}, 1)
      or return;

   my $popup = bless {
      term  => $self,
      event => $event,
   }, urxvt::popup::;

   Scalar::Util::weaken $popup->{term};

   $self->{_destroy}{$popup} = urxvt::destroy_hook { $popup->{popup}->destroy };
   Scalar::Util::weaken $self->{_destroy}{$popup};

   $popup
}

=item $cellwidth = $term->strwidth ($string)

Returns the number of screen-cells this string would need. Correctly
accounts for wide and combining characters.

=item $octets = $term->locale_encode ($string)

Convert the given text string into the corresponding locale encoding.

=item $string = $term->locale_decode ($octets)

Convert the given locale-encoded octets into a perl string.

=item $term->scr_xor_span ($beg_row, $beg_col, $end_row, $end_col[, $rstyle])

XORs the rendition values in the given span with the provided value
(default: C<RS_RVid>), which I<MUST NOT> contain font styles. Useful in
refresh hooks to provide effects similar to the selection.

=item $term->scr_xor_rect ($beg_row, $beg_col, $end_row, $end_col[, $rstyle1[, $rstyle2]])

Similar to C<scr_xor_span>, but xors a rectangle instead. Trailing
whitespace will additionally be xored with the C<$rstyle2>, which defaults
to C<RS_RVid | RS_Uline>, which removes reverse video again and underlines
it instead. Both styles I<MUST NOT> contain font styles.

=item $term->scr_bell

Ring the bell!

=item $term->scr_add_lines ($string)

Write the given text string to the screen, as if output by the application
running inside the terminal. It may not contain command sequences (escape
codes), but is free to use line feeds, carriage returns and tabs. The
string is a normal text string, not in locale-dependent encoding.

Normally its not a good idea to use this function, as programs might be
confused by changes in cursor position or scrolling. Its useful inside a
C<on_add_lines> hook, though.

=item $term->scr_change_screen ($screen)

Switch to given screen - 0 primary, 1 secondary.

=item $term->cmd_parse ($octets)

Similar to C<scr_add_lines>, but the argument must be in the
locale-specific encoding of the terminal and can contain command sequences
(escape codes) that will be interpreted.

=item $term->tt_write ($octets)

Write the octets given in C<$octets> to the tty (i.e. as program input). To
pass characters instead of octets, you should convert your strings first
to the locale-specific encoding using C<< $term->locale_encode >>.

=item $term->tt_paste ($octets)

Write the octets given in C<$octets> to the tty as a paste, converting NL to
CR and bracketing the data with control sequences if bracketed paste mode
is set.

=item $old_events = $term->pty_ev_events ([$new_events])

Replaces the event mask of the pty watcher by the given event mask. Can
be used to suppress input and output handling to the pty/tty. See the
description of C<< urxvt::timer->events >>. Make sure to always restore
the previous value.

=item $fd = $term->pty_fd

Returns the master file descriptor for the pty in use, or C<-1> if no pty
is used.

=item $windowid = $term->parent

Return the window id of the toplevel window.

=item $windowid = $term->vt

Return the window id of the terminal window.

=item $term->vt_emask_add ($x_event_mask)

Adds the specified events to the vt event mask. Useful e.g. when you want
to receive pointer events all the times:

   $term->vt_emask_add (urxvt::PointerMotionMask);

=item $term->set_urgency ($set)

Enable/disable the urgency hint on the toplevel window.

=item $term->focus_in

=item $term->focus_out

=item $term->key_press ($state, $keycode[, $time])

=item $term->key_release ($state, $keycode[, $time])

Deliver various fake events to to terminal.

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

=item $topmost_scrollback_row = $term->top_row

Return various integers describing terminal characteristics.

=item $x_display = $term->display_id

Return the DISPLAY used by rxvt-unicode.

=item $lc_ctype = $term->locale

Returns the LC_CTYPE category string used by this rxvt-unicode.

=item $env = $term->env

Returns a copy of the environment in effect for the terminal as a hashref
similar to C<\%ENV>.

=item @envv = $term->envv

Returns the environment as array of strings of the form C<VAR=VALUE>.

=item @argv = $term->argv

Return the argument vector as this terminal, similar to @ARGV, but
includes the program name as first element.

=cut

sub env {
   +{ map /^([^=]+)(?:=(.*))?$/s && ($1 => $2), $_[0]->envv }
}

=item $modifiermask = $term->ModLevel3Mask

=item $modifiermask = $term->ModMetaMask

=item $modifiermask = $term->ModNumLockMask

Return the modifier masks corresponding to the "ISO Level 3 Shift" (often
AltGr), the meta key (often Alt) and the num lock key, if applicable.

=item $screen = $term->current_screen

Returns the currently displayed screen (0 primary, 1 secondary).

=item $cursor_is_hidden = $term->hidden_cursor

Returns whether the cursor is currently hidden or not.

=item $view_start = $term->view_start ([$newvalue])

Returns the row number of the topmost displayed line. Maximum value is
C<0>, which displays the normal terminal contents. Lower values scroll
this many lines into the scrollback buffer.

=item $term->want_refresh

Requests a screen refresh. At the next opportunity, rxvt-unicode will
compare the on-screen display with its stored representation. If they
differ, it redraws the differences.

Used after changing terminal contents to display them.

=item $text = $term->ROW_t ($row_number[, $new_text[, $start_col]])

Returns the text of the entire row with number C<$row_number>. Row C<< $term->top_row >>
is the topmost terminal line, row C<< $term->nrow-1 >> is the bottommost
terminal line. Nothing will be returned if a nonexistent line
is requested.

If C<$new_text> is specified, it will replace characters in the current
line, starting at column C<$start_col> (default C<0>), which is useful
to replace only parts of a line. The font index in the rendition will
automatically be updated.

C<$text> is in a special encoding: tabs and wide characters that use more
than one cell when displayed are padded with C<$urxvt::NOCHAR> (chr 65535)
characters. Characters with combining characters and other characters that
do not fit into the normal text encoding will be replaced with characters
in the private use area.

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
line. Works for rows outside the line, too, and returns corresponding
offsets outside the string.

=item ($row, $col) = $line->coord_of ($offset)

Translates a string offset into terminal coordinates again.

=back

=cut

sub line {
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

=item $text = $term->special_encode $string

Converts a perl string into the special encoding used by rxvt-unicode,
where one character corresponds to one screen cell. See
C<< $term->ROW_t >> for details.

=item $string = $term->special_decode $text

Converts rxvt-unicodes text representation into a perl string. See
C<< $term->ROW_t >> for details.

=item $success = $term->grab_button ($button, $modifiermask[, $window = $term->vt])

=item $term->ungrab_button ($button, $modifiermask[, $window = $term->vt])

Register/unregister a synchronous button grab. See the XGrabButton
manpage.

=item $success = $term->grab ($eventtime[, $sync])

Calls XGrabPointer and XGrabKeyboard in asynchronous (default) or
synchronous (C<$sync> is true). Also remembers the grab timestamp.

=item $term->allow_events_async

Calls XAllowEvents with AsyncBoth for the most recent grab.

=item $term->allow_events_sync

Calls XAllowEvents with SyncBoth for the most recent grab.

=item $term->allow_events_replay

Calls XAllowEvents with both ReplayPointer and ReplayKeyboard for the most
recent grab.

=item $term->ungrab

Calls XUngrabPointer and XUngrabKeyboard for the most recent grab. Is called automatically on
evaluation errors, as it is better to lose the grab in the error case as
the session.

=item $atom = $term->XInternAtom ($atom_name[, $only_if_exists])

=item $atom_name = $term->XGetAtomName ($atom)

=item @atoms = $term->XListProperties ($window)

=item ($type,$format,$octets) = $term->XGetWindowProperty ($window, $property)

=item $term->XChangeProperty ($window, $property, $type, $format, $octets)

=item $term->XDeleteProperty ($window, $property)

=item $window = $term->DefaultRootWindow

=item $term->XReparentWindow ($window, $parent, [$x, $y])

=item $term->XMapWindow ($window)

=item $term->XUnmapWindow ($window)

=item $term->XMoveResizeWindow ($window, $x, $y, $width, $height)

=item ($x, $y, $child_window) = $term->XTranslateCoordinates ($src, $dst, $x, $y)

=item $term->XChangeInput ($window, $add_events[, $del_events])

=item $keysym = $term->XStringToKeysym ($string)

=item $string = $term->XKeysymToString ($keysym)

Various X or X-related functions. The C<$term> object only serves as
the source of the display, otherwise those functions map more-or-less
directly onto the X functions of the same name.

=back

=cut

package urxvt::popup;

=head2 The C<urxvt::popup> Class

=over 4

=cut

sub add_item {
   my ($self, $item) = @_;

   $item->{rend}{normal} = "\x1b[0;30;47m" unless exists $item->{rend}{normal};
   $item->{rend}{hover}  = "\x1b[0;30;46m" unless exists $item->{rend}{hover};
   $item->{rend}{active} = "\x1b[m"        unless exists $item->{rend}{active};

   $item->{render} ||= sub { $_[0]{text} };

   push @{ $self->{item} }, $item;
}

=item $popup->add_title ($title)

Adds a non-clickable title to the popup.

=cut

sub add_title {
   my ($self, $title) = @_;

   $self->add_item ({
      rend => { normal => "\x1b[38;5;11;44m", hover => "\x1b[38;5;11;44m", active => "\x1b[38;5;11;44m" },
      text => $title,
      activate => sub { },
   });
}

=item $popup->add_separator ([$sepchr])

Creates a separator, optionally using the character given as C<$sepchr>.

=cut

sub add_separator {
   my ($self, $sep) = @_;

   $sep ||= "=";

   $self->add_item ({
      rend => { normal => "\x1b[0;30;47m", hover => "\x1b[0;30;47m", active => "\x1b[0;30;47m" },
      text => "",
      render => sub { $sep x $self->{term}->ncol },
      activate => sub { },
   });
}

=item $popup->add_button ($text, $cb)

Adds a clickable button to the popup. C<$cb> is called whenever it is
selected.

=cut

sub add_button {
   my ($self, $text, $cb) = @_;

   $self->add_item ({ type => "button", text => $text, activate => $cb});
}

=item $popup->add_toggle ($text, $initial_value, $cb)

Adds a toggle/checkbox item to the popup. The callback gets called
whenever it gets toggled, with a boolean indicating its new value as its
first argument.

=cut

sub add_toggle {
   my ($self, $text, $value, $cb) = @_;

   my $item; $item = {
      type => "button",
      text => "  $text",
      value => $value,
      render => sub { ($_[0]{value} ? "* " : "  ") . $text },
      activate => sub { $cb->($_[1]{value} = !$_[1]{value}); },
   };

   $self->add_item ($item);
}

=item $popup->show

Displays the popup (which is initially hidden).

=cut

sub show {
   my ($self) = @_;

   local $urxvt::popup::self = $self;

   my $env = $self->{term}->env;
   # we can't hope to reproduce the locale algorithm, so nuke LC_ALL and set LC_CTYPE.
   delete $env->{LC_ALL};
   $env->{LC_CTYPE} = $self->{term}->locale;

   my $term = urxvt::term->new (
      $env, "popup",
      "--perl-lib" => "", "--perl-ext-common" => "",
      "-pty-fd" => -1, "-sl" => 0,
      "-b" => 1, "-bd" => "grey80", "-bl", "-override-redirect",
      "--transient-for" => $self->{term}->parent,
      "-display" => $self->{term}->display_id,
      "-pe" => "urxvt-popup",
   ) or die "unable to create popup window\n";

   unless (delete $term->{urxvt_popup_init_done}) {
      $term->ungrab;
      $term->destroy;
      die "unable to initialise popup window\n";
   }
}

sub DESTROY {
   my ($self) = @_;

   delete $self->{term}{_destroy}{$self};
   $self->{term}->ungrab;
}

=back

=cut

package urxvt::watcher;

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

=item $timer = $timer->set ($tstamp[, $interval])

Set the time the event is generated to $tstamp (and optionally specifies a
new $interval).

=item $timer = $timer->interval ($interval)

By default (and when C<$interval> is C<0>), the timer will automatically
stop after it has fired once. If C<$interval> is non-zero, then the timer
is automatically rescheduled at the given intervals.

=item $timer = $timer->start

Start the timer.

=item $timer = $timer->start ($tstamp[, $interval])

Set the event trigger time to C<$tstamp> and start the timer. Optionally
also replaces the interval.

=item $timer = $timer->after ($delay[, $interval])

Like C<start>, but sets the expiry timer to c<urxvt::NOW + $delay>.

=item $timer = $timer->stop

Stop the timer.

=back

=head2 The C<urxvt::iow> Class

This class implements io watchers/events. Example:

  $term->{socket} = ...
  $term->{iow} = urxvt::iow
                 ->new
                 ->fd (fileno $term->{socket})
                 ->events (urxvt::EV_READ)
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

Set the file descriptor (not handle) to watch.

=item $iow = $iow->events ($eventmask)

Set the event mask to watch. The only allowed values are
C<urxvt::EV_READ> and C<urxvt::EV_WRITE>, which might be ORed
together, or C<urxvt::EV_NONE>.

=item $iow = $iow->start

Start watching for requested events on the given handle.

=item $iow = $iow->stop

Stop watching for events on the given file handle.

=back

=head2 The C<urxvt::iw> Class

This class implements idle watchers, that get called automatically when
the process is idle. They should return as fast as possible, after doing
some useful work.

=over 4

=item $iw = new urxvt::iw

Create a new idle watcher object in stopped state.

=item $iw = $iw->cb (sub { my ($iw) = @_; ... })

Set the callback to be called when the watcher triggers.

=item $timer = $timer->start

Start the watcher.

=item $timer = $timer->stop

Stop the watcher.

=back

=head2 The C<urxvt::pw> Class

This class implements process watchers. They create an event whenever a
process exits, after which they stop automatically.

   my $pid = fork;
   ...
   $term->{pw} = urxvt::pw
                    ->new
                    ->start ($pid)
                    ->cb (sub {
                       my ($pw, $exit_status) = @_;
                       ...
                    });

=over 4

=item $pw = new urxvt::pw

Create a new process watcher in stopped state.

=item $pw = $pw->cb (sub { my ($pw, $exit_status) = @_; ... })

Set the callback to be called when the timer triggers.

=item $pw = $timer->start ($pid)

Tells the watcher to start watching for process C<$pid>.

=item $pw = $pw->stop

Stop the watcher.

=back

=head1 ENVIRONMENT

=head2 URXVT_PERL_VERBOSITY

This variable controls the verbosity level of the perl extension. Higher
numbers indicate more verbose output.

=over 4

=item == 0 - fatal messages

=item >= 3 - script loading and management

=item >=10 - all called hooks

=item >=11 - hook return values

=back

=head1 AUTHOR

 Marc Lehmann <schmorp@schmorp.de>
 http://software.schmorp.de/pkg/rxvt-unicode

=cut

1

# vim: sw=3:
