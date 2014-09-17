# lazy load modules with fallbacks for small speed-sensitive scripts

sub version {
    ## no critic (PunctuationVars)
    print("$0 version $main::VERSION\n(perl version $^V)\n");
    exit;
}

# lazy load Pod::Usage::pod2usage
sub usage {
    if ( eval { require Pod::Usage } ) {
        return Pod::Usage::pod2usage(@_);
    }
    else {
        print( do { local $/ = undef; <DATA> }, "\n" );
        exit(1);
    }
}
