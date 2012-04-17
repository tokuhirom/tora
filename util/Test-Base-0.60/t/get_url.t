use Test::Base;

plan skip_all => "Need to figure out network testing";
# plan tests => 1;

run_like html => 'match';

__DATA__
=== Test kwiki.org
--- (html) get_url: http://www.kwiki.org
--- (match) regexp
The Official Kwiki Web Site
