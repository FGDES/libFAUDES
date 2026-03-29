## Test/demonstrate composition functions

## import our module (lazy aka all global)
from faudes import *

## ##########################################
## Prepare two simple machines
## ##########################################

## Announce
print("################# Prepare two simple machines");

## Read original machine
m  = System("data/vsmachine.gen")

## Prepare two copies
m1 = System()
m2 = System()
Version(m,"1",m1)
Version(m,"2",m2)

## Report
m1.Write()
m2.Write()

## ##########################################
## Std parallel
## ##########################################

## Announce
print("################# Std parallel composition (here: shuffle product)");


## Compose overall plant
m12 = Parallel(m1,m2)

## Report
m12.Write()

## Record test case
TestDump("parallel",m12)

## ##########################################
## Std parallel with attributes
## ##########################################

## Announce
print("################# Std parallel composition (here: shuffle product, incl attributes)");


## Compose overall plant
cm12 = System()
Parallel(m1,m2,cm12)

## Report
cm12.Write()

## Record test case
TestDump("parallel incl. attr.",cm12)

## ##########################################
## Std parallel with composition map 
## ##########################################

## Announce
print("################# Std parallel composition (here: incl. comp. map)");


## Compose overall plant
compmap = ProductCompositionMap();
Parallel(m1,m2,compmap,cm12)

## Report
print("Comp. map:", compmap)

## Query map
sc_21 = compmap.CompState(2,1)
s1_3  = compmap.Arg1State(3)
s2_3  = compmap.Arg2State(3)
print("Composed state (2,1) is indexed ",sc_21)
print("Composed state 3 corresponds to m1 state",s1_3)
print("Composed state 3 corresponds to m2 state",s2_3)

## Record test case
TestDump("parallel incl. comp. map a", sc_21)
TestDump("parallel incl. comp. map b", s1_3)
TestDump("parallel incl. comp. map c", s2_3)

## ##########################################
## Std parallel, funny cases
## ##########################################

## Announce
print("################# Std parallel composition (empty alphabets)");

## Intentionally break m1
sigma1=m1.Alphabet();
m1.DelEvents(sigma1);

## Compose overall plant
m12 = Generator()
Parallel(m1,m2,m12)

## Report
m12.Write()

## Record test case
TestDump("parallel empty alphabet a",m12)

## Intentionally break m1 even worse
m1.ClearMarkedStates();

## Compose overall plant
m12 = Generator()
Parallel(m1,m2,m12)

## Report
m12.Write()

## Record test case
TestDump("parallel empty alphabet b",m12)

## validate test case
TestDiff()


