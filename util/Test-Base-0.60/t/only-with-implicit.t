use Test::Base tests => 1;
no_diag_on_only;
run_is;

__END__

===
--- ONLY
--- foo: xxx
--- bar: xxx

===
--- foo: xxx
--- bar: yyy
