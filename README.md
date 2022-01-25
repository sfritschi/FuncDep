# FuncDep
Functional Dependency Analysis for Database Design Theory.

Given file containing generic functional dependencies using letters A-Z as attribute names, find super-keys, candidate keys, closure etc.

Currently prints all candidate keys for set of functional dependencies using algorithms from https://www.sciencedirect.com/science/article/pii/0022000078900090.

## Sample input (dep_in/large.txt):
9
A -> B, C
B -> D, E
C -> F, G
D, G -> H
E, F -> I
H, I -> A


## Sample output:
Number of attributes: 9
Candidate keys for FDs in 'dep_in/large.txt':
H I 
D G I 
E F H 
B G I 
C D I 
D E F G 
B F H 
C E H 
A 
B C 
B F G 
C D E 
Number of candidate keys: 12
Took: 6.900e-05 s
