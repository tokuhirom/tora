use Test::Base;

plan tests => 41;

ok(defined &plan);
ok(defined &ok);
ok(defined &is);
ok(defined &isnt);
ok(defined &like);
ok(defined &unlike);
ok(defined &is_deeply);
ok(defined &cmp_ok);
ok(defined &skip);
ok(defined &todo_skip);
ok(defined &pass);
ok(defined &fail);
ok(defined &eq_array);
ok(defined &eq_hash);
ok(defined &eq_set);
ok(defined &can_ok);
ok(defined &isa_ok);
ok(defined &diag);
ok(defined &use_ok);

ok(defined &blocks);
ok(defined &next_block);
ok(defined &delimiters);
ok(defined &spec_file);
ok(defined &spec_string);
ok(defined &filters);
ok(not defined &filters_map);
ok(defined &filters_delay);
ok(defined &run);
ok(defined &run_is);
ok(defined &run_like);
ok(defined &run_unlike);
ok(defined &run_compare);
ok(not defined &diff_is);
ok(defined &default_object);

ok(defined &WWW);
ok(defined &XXX);
ok(defined &YYY);
ok(defined &ZZZ);

ok(defined &croak);
ok(defined &carp);
# ok(defined &cluck);
ok(defined &confess);
