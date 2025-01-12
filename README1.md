### Arbitary number of variables for procedure +,-,*,/ (Unfinished due to time)

In chev-scheme, the following commands is allowed:

(+ 1 2 3 4 5)

(+ 1)

(+)

(- 1)

(- 1 2 3)

(*)

(* 1 3 2)

(/ 3)

(/ 1 2 4)

The return value will be 15, 1, 0, -1, -4, 1, 6, 1/3, 1/8 correspondingly.

And command (-) and (/) are not permitted.

So I tries to make my code allows procedure +,- to accept arbitary number (probably 0 for + and *) of variables.

###