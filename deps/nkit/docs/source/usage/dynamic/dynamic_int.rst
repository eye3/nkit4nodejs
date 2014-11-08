Integers
--------

Dynamic variables of INTEGER type could be constructed by providing any integer
value - (u)int8_t, (u)int16_t, (u)int32_t, (u)int64_t, size_t - to the
Dynamic constructor.

All arithmetic and comparison operators are supported:

===================   =======================
Arithmetic operator   Description
===================   =======================
\+                    addition
\-                    subtraction
\*                    multiplication
/                     division
+=                    in-place addition
-=                    in-place subtraction
\*=                   in-place multiplication
/=                    in-place division
===================   =======================


====================  =======================
Comparison operator   Description
====================  =======================
>                     greater
>=                    greater or equal
<                     less
<=                    less or equal
==                    equal
!=                    not equal
====================  =======================


When converted to boolean, Dynamic integers with value 0 (zero)
are cast to **false** and overs are cast to **true**.

.. literalinclude:: ../../../../test/examples/example_dynamic_int.cpp
  :language: cpp
