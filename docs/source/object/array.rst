class Array
============

Array class is an array class.

You can construct an Array instance from array literal like:
<pre>[1,2,3, "hoge"]</pre>
Array can contains any objects.
 

Array#size()

Get the number of elements in an array.
 

Array#sort()

Get a sorted array. This method is unstable sort(Perl5's sort function is stable sort).
 

Array#push($elem)

Push a object to the array. $elem put at end of array.
Perl5: push(@array, $elem);
 

$array.pop() : Any
Pop a object from array.
 

$array.unshift($v)
push $v to front.
 

$array.shift() : Any
pop value from front.
 

['hoge', 'fuga'].join(' ')
concatenete strings in array.
 

[1,2,3].map(-> $n { say($n) })
 

$array.capacity() : Int
returns the number of elements that can be held in currently allocated storage
 

$array.reserve($n)
reserves storage
 

