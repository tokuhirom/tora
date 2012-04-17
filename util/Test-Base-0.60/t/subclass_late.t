use lib 't';
use Test::Base tests => 1;

# I can't remember why I added this but it was preventing multiple
# levels of inheritance which I needed for the YAML and YAML-Syck
# projects. And is also just damn useful in general.

SKIP: {
    skip("yagni For now...", 1);
    eval "use TestBass";

    like "$@", qr{Can't use TestBass after using Test::Base};
}
