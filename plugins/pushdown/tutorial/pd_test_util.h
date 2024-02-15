/** @file pd_test_util.h  Creation of test classes*/


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013  Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_TEST_UTIL_H
#define FAUDES_PD_TEST_UTIL_H

#include "corefaudes.h"
#include "pd_include.h"

namespace faudes {

/**
 * build a test generator
 * 
 * @return
 *      the populated test generator
 */
PushdownGenerator TestGenerator1();

/**
 * build a test generator
 * 
<Generator>
Generator    

% 
%  Statistics for Generator
% 
%  States:        3
%  Init/Marked:   1/1
%  Events:        3
%  Transitions:   4
%  StateSymbols:  3
%  Attrib. E/S/T: 1/0/4
% 

<Alphabet>
lambda        a             +C+           b            d
</Alphabet>

<States>
s1            s2            s3           
</States>

<TransRel>
s1            a             s2           
++            +lambda+      +dot, dot+   
s1            b             s2           
++            +square+      +dot, square+ 
s2            lambda        s3           
++            +dot, dot+    +square+     
s3            b             s1           
++            +square+      +square+     
</TransRel>

<InitStates>
s1           
</InitStates>

<MarkedStates>
s2           
</MarkedStates>

<StackSymbols>
lambda        dot           square        bottom       
</StackSymbols>

</Generator>

 * @return
 *      the populated test generator
 */
PushdownGenerator TestGenerator2();

/**
<Generator>
Generator    

% 
%  Statistics for Generator
% 
%  States:        3
%  Init/Marked:   1/1
%  Events:        2
%  Transitions:   4
%  StateSymbols:  3
%  Attrib. E/S/T: 1/0/4
% 

<Alphabet>
a             +C+           b            
</Alphabet>

<States>
s1            s2            s3           
</States>

<TransRel>
s1            a             s2           
++            +dot+         +dot, dot+   
s1            b             s2           
++            +square+      +dot, square+ 
s2            a             s3           
++            +dot+         +dot+        
s3            b             s1           
++            +dot+         +dot+        
</TransRel>

<InitStates>
s1           
</InitStates>

<MarkedStates>
s2           
</MarkedStates>

<StackSymbols>
lambda        dot           square       
</StackSymbols>

</Generator>
*/
PushdownGenerator TestGenerator3();

/**
<Generator>
Generator    

% 
%  Statistics for Generator
% 
%  States:        3
%  Init/Marked:   1/1
%  Events:        2
%  Transitions:   4
%  StateSymbols:  3
%  Attrib. E/S/T: 1/0/4
% 

<Alphabet>
a             +C+           b            
</Alphabet>

<States>
s1            s2            s3           
</States>

<TransRel>
s1            a             s2           
++            +dot, dot, dot+  +dot, dot+   
s1            b             s2           
++            +dot, square+  +dot, square+ 
s2            a             s3           
++            +dot, dot+    +dot+        
s3            b             s1           
++            +dot+         +dot+        
</TransRel>

<InitStates>
s1           
</InitStates>

<MarkedStates>
s2           
</MarkedStates>

<StackSymbols>
lambda        dot           square       
</StackSymbols>

</Generator>
*/
PushdownGenerator TestGenerator4();

/**
<Generator>
Generator    

% 
%  Statistics for Generator
% 
%  States:        3
%  Init/Marked:   1/1
%  Events:        3
%  Transitions:   4
%  StateSymbols:  3
%  Attrib. E/S/T: 1/0/4
% 

<Alphabet>
lambda        a             +C+           b            
</Alphabet>

<States>
s1            s2            s3           
</States>

<TransRel>
s1            a             s2           
++            +square+      +square+     
s1            b             s2           
++            +dot+         +square+     
s2            lambda        s3           
++            +dot+        
+square, dot, dot+  +square+      +square+     
s3            lambda        s1           
++            +square+     
+lambda+      +square+      +square, dot+ 
</TransRel>

<InitStates>
s1           
</InitStates>

<MarkedStates>
s2           
</MarkedStates>

<StackSymbols>
lambda        dot           square       
</StackSymbols>

</Generator>

*/
PushdownGenerator TestGenerator5();

PushdownGenerator TestGenerator6();

PushdownGenerator TestGenerator7();

PushdownGenerator TestGenerator8();

PushdownGenerator TestGenerator9();

PushdownGenerator TestGenerator10();

PushdownGenerator TestGenerator11();

PushdownGenerator TestGenerator12();

PushdownGenerator TestGenerator13();

PushdownGenerator TestGenerator14();

PushdownGenerator TestGenerator15();

PushdownGenerator TestGenerator16();

PushdownGenerator TestGenerator17();

PushdownGenerator TestGenerator18();

PushdownGenerator TestGenerator19();

PushdownGenerator TestGenerator20();

PushdownGenerator TestGenerator21();

PushdownGenerator TestGenerator22();

PushdownGenerator TestGenerator23();

PushdownGenerator TestGenerator24();

PushdownGenerator TestGenerator25();

/**
 * 
<Generator>
Generator    

% 
%  Statistics for Generator
% 
%  States:        3
%  Init/Marked:   1/1
%  Events:        3
%  Transitions:   5
%  StateSymbols:  3
%  Attrib. E/S/T: 2/0/0
% 

<Alphabet>
a             +C+           b             c             +C+          
</Alphabet>

<States>
s1            s2            s3           
</States>

<TransRel>
s1            a             s2           
s1            b             s3           
s1            c             s3           
s2            b             s1           
s3            a             s2           
</TransRel>

<InitStates>
s1           
</InitStates>

<MarkedStates>
s3           
</MarkedStates>


</Generator>

 * 
 * @return
 *      the populated test system
 */
System TestSystem1();

System TestSystem2();

System TestSystem3();

System TestSystem4();

System TestSystem5();

/**
 * build a test grammer based on the generator from TestGenerator()
 * 
 * start symbol: (3, 2)
 * nonterminals: (1, 2, 2), (2, 3, 3), (3, 2), (4, 3), (5, 3, 1)
 * terminals: a, b
 * grammar productions: 
 *      (1, 2, 2) --> (2, 3, 3)
 *      (1, 2, 2) --> (5, 3, 1)
 *      (3, 2) --> (4, 3)b
 *      (4, 3) --> a(1, 2, 2)b
 *      (5, 3, 1) --> b
 * 
 * @return
 *      the populated test grammar
 */
Grammar TestGrammar1();

/**
 * build a test grammer based on the generator from TestGenerator()
 * 
 * start symbol: (4, 3)
 * nonterminals: (1, 2, 2), (2, 3, 3), (3, 2), (4, 3), (5, 3, 1)
 * terminals: a, b
 * grammar productions: 
 *      (1, 2, 2) --> (2, 3, 3)
 *      (1, 2, 2) --> (5, 3, 1)
 *      (4, 3) --> a(1, 2, 2)b
 *      (5, 3, 1) --> b
 * @return
 *      the populated test grammar
 */
Grammar TestGrammar2();
Grammar TestGrammar3();
Grammar TestGrammar4();
Grammar TestGrammar5();
Grammar TestGrammar6();
Grammar TestGrammar7();
Grammar TestGrammar8();
Grammar TestGrammar9();
Grammar TestGrammar10();
Grammar TestGrammar11();

/**
 * write to console to indicate start of test
 * 
 * @param name
 *      name of the test
 */
void TestStart(std::string name);

/**
 * write to console to indicate end of test
 * 
 * @param name
 *      name of the test
 */
void TestEnd(std::string name);

} // namespace faudes

#endif