=head1 NAME

libptytty - OS independent and secure pty/tty and utmp/wtmp/lastlog handling

=head1 SYNOPSIS

   -lptytty

=head1 DESCRIPTION

TODO

=head1 SECURITY CONSIDERATIONS

I<< B<It is of paramount importance that you at least read the following
paragraph!> >>

If you are a typical terminal-like program that just wants one or more
ptys, you should call the C<ptytty::init ()> method (C: C<ptytty_init ()
function) as the very first thing in your program:

   int main (int argc, char *argv[])
   {
      // do nothing here
      ptytty::init ();
      // in C: ptytty_init ();

      // initialise, parse arguments, etc.
   }

This checks wether the program runs setuid or setgid. If yes then it will
fork a helper process and drop privileges.

Some programs need finer control over if and when this helper process
is started, and if and how to drop privileges. For those programs, the
methods C<ptytty::use_helper> and C<ptytty::drop_privileges> are more
useful.

=head1 C++ INTERFACE: THE ptytty CLASS

=head2 STATIC METHODS

=over 4

=item ptytty::init ()

The default way to initialise libptytty. Must be called imemdiately as
the first thing in the C<main> function, or earlier e.g. during static
construction time. The earlier, the better.

This method checks wether the program runs with setuid/setgid permissions
and, if yes, spawns a helper process for pty/tty management. IT then
drops the privileges completely, so the actual program runs without
setuid/setgid privileges.

=item ptytty::use_helper ()

Tries to start a helper process that retains privileges even when the
calling process does not. This is usually called from C<ptytty::init> when
it detects that the program is running setuid or setgid, but can be called
manually if it is inconvinient to drop privileges at startup, or when
you are not running setuid/setgid but want to drop privileges (e.g. when
running as a root-started daemon).

This method will try not to start more than one helper process. The same
helper process can usually be used from the process starting it an all its
fork'ed (not exec'ed) children.

Please note that starting a helper process after dropping privileges makes
no sense.

=item ptytty::drop_privileges ()

Drops privileges completely, i.e. sets real, effective and saved user id
to the real user id. Also aborts if this cnanot be achieved. Useful to
make sure that the process doesn't run with special privileges.

=item bool success = ptytty::send_fd (int socket, int fd)

Utility method to send a file descriptor over a unix domain
socket. Returns true if successful, false otherwise. This method is only
exposed for your convinience and is not required for normal operation.

=item int fd = ptytty::recv_fd (int socket)

Utility method to receive a file descriptor over a unix domain
socket. Returns the fd if sucecssful and C<-1> otherwise. This method
is only exposed for your convinience and is not required for normal
operation.

=item ptytty *pty = ptytty::create ()

Creates new ptytty object. Creation does not yet do anything besides
allocating the structure.

A static method is used because the actual ptytty implementation can
differ at runtime, so you need a dynamic object creation facility.

=back


=head2 DYNAMIC/SESSION-RELATED DATA MEMBERS AND METHODS

=over 4

=item int pty_fd = pty->pty

=item int tty_fd = pty->tty

These members contain the pty and tty file descriptors, respectively. They
initially contain C<-1> until a successful to C<ptytty::get>.

=item bool success = pty->get ()

Tries to find, allocate and initialise a new pty/tty pair. Returns C<true>
when successful.

=item pty->login (int cmd_pid, bool login_shell, const char *hostname)

Creates an entry in the systems session database(s) (utmp, wtmp, lastlog).
C<cmd_pid> must be the pid of the process representing the session
(such as the login shell), C<login_shell> defines wether the session is
associated with a login, which influences wether wtmp and lastlog entries
are created, and C<hostname> should identify the "hostname" the user logs
in from, which often is the value of the C<DISPLAY> variable or tty line
in case of local logins.

Calling this method is optional. A session starts at the time of the login
call and extends until the ptytty object is destroyed.

=item pty->close_tty ()

Closes the tty. Useful after forking in the parent/pty process.

=item bool success = pty->make_controlling_tty ()

Tries to make the pty/tty pair the controlling terminal of the current
process. Useful after forking in the child/tty process.

=item pty->set_utf8_mode (bool on)

On systems supporting special UTF-8 line disciplines (e.g. Linux), tries
to enable it for the given pty. Can be called at any time to change the
mode.

=back


=head1 C INTERFACE: THE ptytty FAMILY OF FUNCTIONS

=over 4

=item ptytty_init ()

See C<ptytty::init ()>.
   
=item PTYTTY ptytty_create ()

Creates a new opaque PTYTTY object and returns it. Do not try to access it
in any way excecp by testing it for truthness (e.g. C<if (pty) ....>). See
C<ptytty::create ()>.

=item int ptytty_pty (PTYTTY ptytty)

Return the pty file descriptor. See C<< pty->pty >>.
   
=item int ptytty_tty (PTYTTY ptytty)

Return the tty file descriptor. See C<< pty->tty >>.
   
=item void ptytty_delete (PTYTTY ptytty)

Destroys the PTYTTY object, freeing the pty/tty pair and cleaning up the
utmp/wtmp/lastlog databases, if initialised/used. Same as C<delete pty> in
C++.

=item int ptytty_get (PTYTTY ptytty)

See C<< pty->get >>, returns 0 in case of an error, non-zero otherwise.

=item void ptytty_login (PTYTTY ptytty, int cmd_pid, bool login_shell, const char *hostname)

See C<< pty->login >>.

=item void ptytty_close_tty (PTYTTY ptytty)

See C<< pty->close_tty >>.
   
=item int ptytty_make_controlling_tty (PTYTTY ptytty)

See C<< pty->make_controlling_tty >>.
   
=item void ptytty_set_utf8_mode (PTYTTY ptytty, int on)

See C<< pty->set_utf8_mode >>.

=item void ptytty_drop_privileges ()

See C<< ptytty::drop_privileges >>.
   
=item void ptytty_use_helper ()

See C<< ptytty::use_helper >>.

=back


=head1 BUGS

You kiddin'?

=head1 AUTHORS

Emanuele Giaquinta L<< <e.giaquinta@glauco.it> >>, Marc Alexander Lehmann
L<< <rxvt-unicode@schmorp.de> >>.
