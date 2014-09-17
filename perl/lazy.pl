# lazy load modules with fallbacks for small speed-sensitive scripts

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
