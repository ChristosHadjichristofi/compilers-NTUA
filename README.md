1. multipleSemErrors/fibonacci.lla   -> not all errors & segfault
2. multipleSemErrors/bubblesort.lla  -> not all errors & segfault
3. semCorrect/whichday.lla           -> segfault & prints errors which are not
4. semCorrect/maze.lla               -> segfault & prints errors which are not
5. semCorrect/listinsert.lla         -> segfault {at program line 7 types are given (let rec listInsert (l : list) (d : int) : list) -> they cause the problem/when removed works fine}
6. semCorrect/testfun.lla            -> segfault {error @ line 167, id->sem() function call (for f which is entry_param and at S.E does not have params)}
7. semCorrect/various.lla            -> segfault
8. semCorrect/various_sem.lla        -> segfault
9. semCorrect/various_sem_2.lla      -> segfault
10. semCorrect/mult_call.lla          -> segfault {the error appears when print_int arg is (f 1). (f 1) is like calling g with 1 (which is an inner function in f). I think is called closure, basically f function returns what g does}
11. semCorrect/closure.lla           -> segfault {same as above}
12. semCorrect/deepunit.lla          -> prints error which is not
13. semCorrect/eqUdts2.lla           -> prints error which is not (this program should fail in runtime)   
14. semCorrect/evenodd.lla           -> segfault & prints error which is not {we should do the same thing we did with user defined types}
15. semCorrect/high_order.lla        -> segfault
16. semCorrect/high_order_1.lla      -> segfault
17. semCorrect/ho.lla                -> segfault
18. semCorrect/local_ho.lla          -> segfault
19. semCorrect/lstsecond.lla         -> segfault && prints dupl error (which is true but nickie does not print :D )
20. semCorrect/strtest.lla           -> segfault
21. semCorrect/unitEq.lla            -> segfault
