use Test::Base tests => 2;

sub perl514 {
    skip "perl-5.14 regexp stringification is different", shift || 1
        if $] > 5.013;
}

run_unlike('html', 're1');

SKIP: { perl514;
    run_is 're1' => 're2';
}

__END__

=== Unlike Test
--- html
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="he" dir="rtl">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

        <link rel="stylesheet" href="/htdocs/default.css" type="text/css" />
--- re1 regexp=i
software error
--- re2 chomp
(?i-xsm:software error)
