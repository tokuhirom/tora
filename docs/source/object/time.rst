class Time
===========

The Time object.
 

my $t = Time.new(Int $epoch);
my $t = Time.now(Int $epoch);
Create a time object from $epoch time.
Timezone set to local.
 

$time.epoch() : Int
Get a epoch time from Time object.
 

$time.strftime(Str $format) : Str
Format time object to string by $format. See strftime(3).
Limitation: Result string must be smaller than 256-1 bytes.(patches welcome)
 

$time.year() : Int
Get a year from $time.
 

$time.month() : Int
Get a month from $time.
 

$time.day() : Int
Get a month from $time.
 

$time.hour() : Int
Get a month from $time.
 

$time.minute() : Int
Get a month value from $time.
 

$time.minute() : Int
Get a month value from $time.
 

$time.day_of_week() : Int
This method returns day of week. It returns 1..7.
 

