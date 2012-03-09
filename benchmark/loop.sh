#!/bin/sh
scons --quiet

./tora -V

time ./tora benchmark/loop/for-range.tra
time perl benchmark/loop/for-range.pl

