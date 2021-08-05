1. multipleSemErrors/fibonacci.lla   -> not all errors & segfault
2. multipleSemErrors/bubblesort.lla  -> not all errors & segfault
3. semCorrect/whichday.lla           -> segfault & prints errors which are not
4. semCorrect/maze.lla               -> segfault & prints errors which are not
6. semCorrect/strtest.lla            -> segfault {high order}
7. semCorrect/unitEq.lla             -> segfault {high order}
8. semCorrect/mult_call.lla          -> segfault {the error appears when print_int arg is (f 1). (f 1) is like calling g with 1 (which is an inner function in f). I think is called closure, basically f function returns what g does}
9. semCorrect/closure.lla            -> segfault {same as above}
10. semCorrect/local_ho.lla          -> segfault
11. semCorrect/deepunit.lla          -> prints error which is not
12. semCorrect/eqUdts2.lla           -> prints error which is not (this program should fail in runtime)   
13. semCorrect/high_order.lla        -> segfault
14. semCorrect/high_order_1.lla      -> segfault
15. semCorrect/ho.lla                -> segfault
16. semCorrect/local_ho.lla          -> segfault
