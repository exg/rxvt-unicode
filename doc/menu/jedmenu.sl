%--------------------------------*-SLang-*--------------------------------
% An example of using the rxvt menuBar for the JED editor
#if$TERM xterm*
%!% provide a hook to imitated the S-Lang> prompt
%!% use ESC[m to shadow the ESC[M used by mouse reporting
define menuFn ()
{
   variable ch, cmd;

   cmd = Null_String;
   forever
     {
	ch = getkey ();
	if (ch == '\r') break;
	cmd = strcat (cmd, char (ch));
     }
   eval (cmd);
}
local_setkey ("menuFn", "\e[m");	% menu

%!% allow the user to bind their owm commands
define menucmd (str) { tt_send (Sprintf ("\e]10;%s\a", str, 1)); }
%-------------------------------------------------------------------------
% integrate these with any existing suspend/resume/exit hooks
% the suspend hook works best if there was already a menu defined
% before invoking JED

define suspend_hook () { menucmd ("[prev]"); }
define resume_hook () { menucmd ("[next]"); }
define exit_hook () { menucmd ("[rm]"); exit_jed (); }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
menucmd ("[read:jedmenu.sl]");		% read this file

% format _jed_version xyyzz into x.yy-zz
menucmd (Sprintf ("[:[title:Jed%d.%d-%d (%%n-%%v)]:]",
		  (_jed_version/10000),
		  ((_jed_version mod 10000)/100),
		  (_jed_version mod 100),
		  3));

% get rid off pixmap stuff
if (strcmp (getenv ("COLORTERM"), "rxvt-xpm"))
{
   menucmd ("[menu][:-/Terminal/Pixmap:][show]");
}
#endif	% xterm*
%%%%%%%%%%%%%%%%%%%%%%%%%%% end-of-file (SLang) %%%%%%%%%%%%%%%%%%%%%%%%%%
% rxvt menu database
#iffalse
% An example of using the rxvt menuBar for the JED editor
% possibly useful things for the JED editor -- assuming Emacs bindings

[menu:jed]

[title:Jed menu (%n-%v)]
% some convenient arrows
<b>\E[m<l>bskip_word<u>backward_paragraph<d>forward_paragraph<r>skip_word<e>\r

/File/*
{Open}{^X^F}
{Save}{^X^W}
{Save Buffers}{^Xs}
{Insert File}{^Xi}
{-}
{Shell Cmd}{M-!}
{-}
{Exit}{^X^C}

/Edit/*
{Undo}{^_}
{-}
{Cut}{^W}
{Copy}{M-W}
{Paste}{^Y}

/Search/*
{Forward}{^S}
{Backward}{^R}
{Replace}{M-%}
{-}
./Regexp/*
{Forward}{M-^S}
{Backward}{M-^R}
{Replace}	\E[mquery_replace_match\r

/Buffers/*
{Kill}{^Xk}
{List}{^X^B}
{Switch}{^Xb}
{-}
./Modes/*
{C}		\E[mc_mode\r
{SLang}	\E[mslang_mode\r
{None}		\E[mno_mode\r
{LaTeX}	\E[mlatex_mode\r
{Text}		\E[mtext_mode\r
{Fortran}	\E[mfortran_mode\r

/Window/*
{Delete}{^X0}
{One}{^X1}
{Split}{^X2}
{Other}{^Xo}
{-}
{Recenter}{^L}
{-}
./Color Schemes/*
{White-on-Black}	\E[mset_color_scheme("15;0")\r
{Black-on-White}	\E[mset_color_scheme("0;15")\r
{White-on-default-Black}\E[mset_color_scheme("15;default;0")\r
{Black-on-default-White}\E[mset_color_scheme("0;default;15")\r

/Utils/*
{Bufed}	\E[mbufed\r
{Dired}	\E[mdired\r
{Mail}	\E[mmail\r
{Rmail}	\E[mrmail\r
{-}
{EvalBuffer}	\E[mevalbuffer\r
{Trim-Buffer}	\E[mtrim_buffer\r

[read:terminal]

/?/*
{Info}{^X?i}
{Man}{^X?m}
{-}
{Apropos}{^X?a}
{Show Key}{^X?k}
{Where Is}{^X?w}

[show]
[done]
#endif
%%%%%%%%%%%%%%%%%%%%%%%%%%% end-of-file (SLang) %%%%%%%%%%%%%%%%%%%%%%%%%%
