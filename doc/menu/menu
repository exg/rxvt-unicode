#!/bin/sh --
# shell wrapper to avoid typing Menu escape sequences
if test $# -eq 0; then
echo "\
usage: `basename $0` cmd
where the most common commands are
	[menu] [menu:name]
	[read:file] [read:file;name]
	[title:string]
	+/path/menu
	+/path/menu/*
	+/menu/path/{-}
	+/menu/path/{item}{rtext} action

	-/*
	-/path/menu
	-/path/menu/*
	-/path/{-}
	-/path/{item}

	<b>Begin<r>Right<l>Left<u>Up<d>Down<e>End
	[done]

	[rm] [rm:] [rm*] [rm:*] [rm:name]
	[swap] [prev] [next]
	[clear] [show] [hide]
	[pixmap:file]
	[dump]
NB: commands may need to be quoted to avoid shell expansion
"
exit
fi
Echo="echo -n"
# some systems/shells don't like `echo -n'
case `/bin/uname` in
    SunOS) Echo="echo";;
esac
while [ $# -gt 0 ]
do
    case $1 in
	+* | -* | '<'* | '['*)		# send raw commands
	$Echo "]10;$1"
	;;

	*)					# read in menu files
	if test $1 = "default";
	then
	    $Echo "]10;[read:$0]"
	else
	    $Echo "]10;[read:$1]"
	fi
	if test "$COLORTERM" != "rxvt-xpm";	# remove pixmap stuff
	then
	    $Echo "]10;[menu][:-/Terminal/Pixmap:][show]"
	fi
	;;
    esac
    shift
done
exit	# stop shell here!
#-------------------------------------------------------------------------
# since everything before a [menu] tag is ignored, we can put a default
# menu here
#-------------------------------------------------------------------------
[menu:default]

/Programs/*
{Edit}		${EDITOR:-vi}\r
{Top}		top\r
{Dir}		ls -la|${PAGER:-more}\r
{Dir-Time}	ls -lat|${PAGER:-more}\r
{Space Left}	df\r
{-}
{Exit}		exit\r

/Shell/*
{check mail}	checkmail\r
{Background}	^Z bg\r
{Kill}		^C\r

[show]
[done]
#--------------------------------------------------------------------- eof
