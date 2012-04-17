use Test::Base tests => 3;

run_like('html', 're1');
run_like 'html', 're2';
run_like html => qr{stylesheet};

__END__

=== Like Test
--- html
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="he" dir="rtl">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

        <link rel="stylesheet" href="/htdocs/default.css" type="text/css" />
--- re1 regexp=xis
<!doctype
.*
<html
.*
--- re2 regexp
1\.0 Strict
