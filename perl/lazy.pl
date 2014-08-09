# lazy load modules with fallbacks for small speed-sensitive scripts

# lazy load Getopt::Long::VersionMessage
sub version {
    if ( eval { require Getopt::Long } ) {
        return Getopt::Long::VersionMessage(@_);
    }
    else {
        ## no critic (PunctuationVars)
        print("$0 version $VERSION\n(perl version $^V)\n");
        exit(0);
    }
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
