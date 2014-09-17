package Throw;

use strict;
use warnings;

our $VERSION = 'v1.0.0';

# die() wrapper without annoying trailing-newline semantics
# message is blessed into a class that stringifies
sub throw {
    no strict 'refs';    ## no critic (strict)
    my $cls = __PACKAGE__ . '::Exception';
    *{ $cls . "::((" } = sub { };
    *{ $cls . "::(" . '""' } = sub { ${ $_[0] } };
    die bless \( my $err = "@_" ), $cls;
}

1;
