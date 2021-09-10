Semantic [some of the following 'problems' might be fixed - probably outdated readme]:
1. examples/semCorrect/types/maze.lla
2. examples/typeMismatch/high_order_2.lla - is ok? (diff between 'int' and 'unit')
3. examples/typeMismatch/bin_tree_2.lla -> recheck (not shows all errors it should)
4. examples/multipleSemErrors/fibbonacci.lla -> recheck (not shows all errors it should)
5. examples/typeMismatch/lessparams_singlecallparam.lla -> recheck tomorrow (int -> int -> int -> unit)

Overall, printing is fine.
We should:
1. Set some more custom messages in certain errors (i.e typeMismatch/missing_else_e2.lla)
2. Print binop symbol if Error occurs on its arguments
3. Maybe have a global variable counterErrors in order to say at the end how many Errors occured. Maybe if they exceed 8 say too many errors. Abort blah blah

CodeGen:
1. ArrayItem bug works only at 2 dims
2. StringLiteral should be created as an array
3. Functions
4. User defined types
5. check for exceptions (out of bounds etc)
6. New/Delete