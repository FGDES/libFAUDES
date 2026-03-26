## Test/demonstrate core faudes generator functions

## import our module (lazy all global)
from faudes import *

## ##########################################
## Deterministic
## ##########################################

## Announce
print("################# Deterministic")

## Convert nondeterministic generator
g   = Generator("data/deterministic_nondet.gen")
g_d = Deterministic(g)
g.Write()
g_d.Write()

## Record test case
TestDump("deterministic",g_d)

## ##########################################
## State Minimisation
## ##########################################

## Announce
print("################# State Minimisation")

## Convert non minimal generator
g  = Generator("data/minimal_nonmin.gen")
g.Write()
StateMin(g)
g.Write()

## Record test case
TestDump("statemin",g)

## ##########################################
## Natural Projection
## ##########################################

## Announce
print("################# Projection")

g    = Generator("data/project_g.gen")
sig0 = EventSet.NewFromList(['a', 'c', 'e', 'g'])
g0   = Project(g,sig0)

## Report
g.Write()
sig0.Write()
g0.Write()

## Record test case
TestDump("project",g0)

## ##########################################
## Boolean Operations
## ##########################################

## Announce
print("################# Boolean Operations")

## Union
g1  = Generator("data/boolean_g1.gen")
g2  = Generator("data/boolean_g2.gen")
g_or = Generator()
g_or.StateNamesEnabled(False)
LanguageUnion(g1, g2, g_or)
  
## Intersection
g_and = Generator()
g_and.StateNamesEnabled(False)
LanguageIntersection(g1, g2, g_and)
  
## Test: perform complement twice
g1_not=g1.Copy() 
g1_not.StateNamesEnabled(False)
LanguageComplement(g1_not)
g1_notnot=g1_not.Copy()
LanguageComplement(g1_notnot)
  
##  Write results
g1.Write("tmp_boolean_g1.gen")
g2.Write("tmp_boolean_g2.gen")
g_or.Write("tmp_union_g1g2.gen")
g_and.Write("tmp_intersection_g1g2.gen")
g1_not.Write("tmp_complement_g1.gen")
g1_notnot.Write("tmp_complement2_g1.gen")

## Record test case
TestDump("lang. union",g_or)
TestDump("lang. intersection",g_and)
TestDump("lang. complement",g1_not)
TestDump("lang. complement2",g1_notnot)

## Inspect on console

print("################# Results:")
g_or.DWrite()
g_and.DWrite()
g1_not.DWrite()

## Compare languages

print("################# Compare:")
if LanguageInclusion(g1,g_or):
  print("Lm(g1) <= Lm(g1) v Lm(g2) : OK")
else:
  print("Lm(g1) > Lm(g1) v Lm(g2) : ERR")

if LanguageDisjoint(g1_not,g1):
  print("Lm(g1) ^ ~Lm(g1) = empty : OK")
else:
  print("(Lm(g1) v Lm(g2)) ^ ~(Lm(g1) v Lm(g2)) != empty : ERR")

if LanguageEquality(g1,g1_notnot):
  print("Lm(g1) = ~~Lm(g1) : OK")
else:
  print("Lm(g1) != ~~Lm(g1) : ERR")

## Record test case

TestDump("lang. inclusion",LanguageInclusion(g1,g_or))
TestDump("lang. disjoint",LanguageDisjoint(g1_not,g1))
TestDump("lang. equality",LanguageEquality(g1,g1_notnot))


## ##########################################
## Concatenation/Kleene-Closure
## ##########################################

## Announce
print("################# Concatenation/Kleene-Closure")

## Read K and L
gk = Generator("data/concat_gk.gen")
gl = Generator("data/concat_gl.gen")

## Version 1: std functions
k_l_star_1 = Generator(gl)
KleeneClosure(k_l_star_1)
LanguageConcatenate(gk,k_l_star_1,k_l_star_1)
StateMin(k_l_star_1,k_l_star_1)

## Version 2: use non-deterministic intermediate results, disable state names
k_l_star_2 = Generator(gl)
k_l_star_2.StateNamesEnabled(False)
KleeneClosureNonDet(k_l_star_2)
LanguageConcatenateNonDet(gk,k_l_star_2,k_l_star_2)
Deterministic(k_l_star_2,k_l_star_2)
StateMin(k_l_star_2,k_l_star_2)

## Compare results
ok = LanguageEquality(k_l_star_1,k_l_star_2)

## Inspect on console

print("################# Results:")
k_l_star_1.SWrite()
k_l_star_2.SWrite()
if ok:
  print("results match : OK")
else:  
  print("results dont match : ERROR")

## Graphical output
## gk.GraphWrite("tmp_concat_gk.png")
## gl.GraphWrite("tmp_concat_gl.png")
## k_l_star_1.StateNamesEnabled(False)
## k_l_star_1.GraphWrite("tmp_concat_k_l_star_1.png")
## k_l_star_2.GraphWrite("tmp_concat_k_l_star_2.png")


## Record test case
TestDump("lang. concat",k_l_star_1)
TestDump("lang. concat",k_l_star_2)
TestDump("lang. concat",ok)

## validate test case
TestDiff()


