#! perl

$file = join("",<>);
$file=~s/[^\000]+\/\* functions \*\///;
$file=~s/#endif\s+$//;
foreach my $def (split(/\s*;\s*/,$file))
{
	$def =~ s/\s+/ /g;
	my $paren = ($def=~s/\(([^\(\)]*)\)\s*//)?$1:'';
	my $type = ($def=~s/^([^\000]*\s+\*?)//)?$1:'';
	$type=~s/^\s+//; 
	$type=~s/\s+$//;  
	push(@list,{def=>$def,type=>$type,paren=>$paren}); 
}
foreach my $s (sort {$a->{def} cmp $b->{def}} @list) 
{
	my $def = $s->{def};
	my $type = $s->{type};
	my $paren=$s->{paren};
	my $params = $paren;
	$params=~s/[\[\]\*]//g;
	$params=~s/\s+,/,/g;
	$params=~s/[^,\s]+\s+//g;
	next if ($params =~ /\.\.\./);
	next if ($def eq 'XOpenDisplay');
	my $return = $type eq 'void' ? '':'return ';
	print "
typedef $type (proto_$def)($paren);
static proto_$def *func_$def = NULL;
$type $def($paren) {
	if (!func_$def) func_$def=(proto_$def *)_loadfunc(\"$def\");
	$return(func_$def)($params);
}
";

}
