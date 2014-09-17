package Usage;

use strict;
use warnings;

our $VERSION = 'v1.0.0';

sub version {
    ## no critic (PunctuationVars)
    print("$0 version $main::VERSION\n(perl version $^V)\n");
    exit;
}

sub usage {
    eval { require Pod::Usage } and goto \&Pod::Usage::pod2usage;
    eval { require Pod::Text } and Pod::Text->filter( \*DATA ), exit;
    print( readline(DATA), "\n" );
    exit 1;
}

1;
