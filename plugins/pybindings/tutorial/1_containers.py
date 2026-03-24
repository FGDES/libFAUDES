## Test/demonstrate basic faudes containers 

## import our module (lazy variant, make global)
from faudes import *

## Set level of verbosity
## Verbosity(1)

## ##########################################
## Set of indicees (eg used as StateSet)
## ##########################################

## Announce
print("################# Set of indicees")

## Instantiate set of indices
## (relevant faudes typed objects all support default/copy/file constructors)
i1 = IndexSet()                     # default constructor (empty set)
i2 = IndexSet(i1)                   # copy constructor
i3 = IndexSet("data/1_states.fts")  # construct from file (faudes token stream)

## Edit programatically
i1.Name("Indicees")                 # give a name (cosmetic)
i1.Insert(2)                        # insert element
i1.Insert(4)                        # insert element
i1.Insert(10)                       # insert element
i1.Erase(2)                         # remove element

## Print the set for inspection 
print(f'A small IndexSet with two elements: {i1}')

## Read from string (faudes token streanm format)
i1.FromString('<Q> 2 4 8 10 </Q>')
print(f'An IndexSet from a FAUDES token stream: {i1}')

## Read from Python list
i2.FromList([1, 2, 3, 4])
print(f'An IndexSet from a Python list: {i2}')

## Iterate, Python style
print(f'Iterating over the IndexSet "{i1.Name()}" (Python style)')
for q in i1:
  print(q)

## Convert back to Python list
i1l = list(i1)
print(f'Obtaining a Python list from an IndexSet: {i1l}')

## Assignment
i1R = i1           # gives a reference, ***not*** a copy
i2 = IndexSet(i1)  # use the copy contructor for an actual copy
i3 = i1.Copy()     # use the copy method for an actual copy
i1R.Name("Q'")

## Manipulate the original set ...
print(f'Assignment vs. Copy:  erase one item affects the reference, not the copy')
i1.Erase(10)
print(f'Assignment vs. Copy:  orig {i1} ref {i1R} copy {i2}')

## All faudes sets support STL style iteration (legacy)
print(f'Iterating over the IndexSet "{i1.Name()}" (STL style)')
it=i1.Begin()
while it!=i1.End():
  print(it.Index())
  it.Inc()

## Record test case
TestDump("iset orig",i1)
TestDump("iset ref",i1R)
TestDump("iset copy 2",i2)
TestDump("iset copy 3",i3)


## ##########################################
## Set of indicees with symbolic names 
## ##########################################

## Announce
print("################# Set of symbolic names")

## Create a NameSet (alias EventSet)
n1 = NameSet()

## Edit programmatically
n1.Name("SetOfSymbols")   # give it a name
n1.Insert("alpha")        # insert a symbol
n1.Insert("beta")         # insert a symbol
n1.Insert("lambda")       # insert a symbol
n1.Insert("mue")          # insert a symbol
n1.Erase("lambda")        # remove a symbol

## Print a set
print(f'A NameSet: {n1}')

## Set from string (using static method as constructor, since "n2" does not yet exist))
n2=NameSet.NewFromString('<S> "alpha" "beta" "mue" "lambda" </S>')
print(f'A NameSet from a FAUDES token stream: {n2}')

## Set from Python list
n3=NameSet.NewFromList(["wellcome", "to", "libFAUDES"])
print(f'A NameSet from a Python list: {n3}')

## Symbol lookup
bidx=n1.Index("beta")
bname=n1.SymbolicName(bidx)
print(f'"{bname}" has index {bidx}')

## Iterate Python style (STL style is also available; see above IndexSet example))
print(f'Iterating over NameSet "{n1.Name()}" (Python style)')
for e in n1: 
  print(f'Element: idx {e} symbol "{n1.SymbolicName(e)}" info "{n1.Str(e)}"')

## Erase an element by symbolic name or index
n1.Erase("mue")
n1.Erase(bidx)

## Assignment
n1R = n1        # have a reference (not a copy)
n2  = n1.Copy() # have a copy

## Write to file
n2.Write("tmp_nameset.fts")


## ##########################################
## Boolean operations on sets
## ##########################################

print("################# Boolean operations")

aset=NameSet()
bset=NameSet()
aset.FromString('<A> "a" "b" "c" </A>')
bset.FromString('<B> "c" "d" "e" </B>')
print(f'Two sets A {aset} and B {bset}')

## Apply boolean operations
print(aset+bset)    # union
print(aset*bset)    # interscetion
print(aset-bset)    # difference

## Inclusion
if aset <= aset + bset:
  print("A <= A+B       ##  OK")
else:
  print("not (A <= A+B) ##  FAIL")

## Equality
if aset == bset:
  print("A == B         ##  FAIL")
else:
  print("A != B         ##  OK")


## ##########################################
## Set of events with control-system attributes
## ##########################################

## Announce
print("################# Set of events with control-system attributes")

## Create the Alphabet (eg from the an NameSet)
eset = Alphabet(n1)

## Edit
eset.Name("Sigma")   # have a name
eset.Insert("beta")     # insert more events
eset.Insert("lambda")   # insert more events

## Set existing event to be controllabel
eset.Attribute("alpha").SetControllable()

## Insert new event with attribute prepared
cattr=AttributeCFlags()
cattr.SetControllable()
eset.Insert("mue",cattr)

## Assign attribute to element
eset.Attribute("lambda",cattr)

## Report
print(f'Alphabet: {eset}')

## Iterate (Python style; STL style also available, see above IndexSet)
print("Iterating over alphabet",eset.Name())
for e in eset:
  print(f'idx {e} symbol "{eset.SymbolicName(e)}" attrib {eset.Attribute(e)}')


## ##########################################
## Boolean operations on sets with attributes
## ##########################################

print("################# Boolean operations incl attributes")

aset=Alphabet()
bset=Alphabet()
aset.FromString('<A> "a" +C+ "b" +o+  "c" +o+  </A>')
bset.FromString('<B> "c" +o+ "d"      "e" +C+ </B>')
cset=aset.Copy()
cset.Name('C')
cset.Attribute("a").ClrControllable()

print(f'Set A: {aset}')
print(f'Set B: {bset}')
print(f'Set C: {cset}')

## The operators *, + and - return plain event sets
print("Operators swallow attributes:")
print(f'A + B: {aset + bset}')
print(f'A - B: {aset - bset}')
print(f'A * B: {aset * bset}')

## Set comparison operators ignore attributes
sle1 = (aset <= cset)
if sle1:
  print("A <= C ## OK")
seq1 = (aset == cset)
if seq1:
  print("A == C ## OK")

## Set comparison methods (when types don't match)
sle2 = AlphabetInclusion(aset, aset + bset)               
if sle2:
  print("A <= A + B ## OK (RHS is a plain NameSet)")
seq2 = AlphabetEquality(aset - bset + aset,aset)
if seq2:
  print("A - B + A == A ## OK (LHS is a plain NameSet)")


## Union, maintain attributes where possible
union=Alphabet()
AlphabetUnion(aset,bset,union)
print("AlphabetUnion(...) maintains attributes");
print(union)

## Intersection, maintain attributes
intersection=Alphabet()
AlphabetIntersection(aset,bset,intersection)
print("AlphabetIntersection(...) maintains attributes");
print(intersection)

## Difference, maintain attributes
difference=Alphabet()
AlphabetDifference(aset,bset,difference)
print("AlphabetDifference(...) maintains attributes");
print(difference)

## Record test case
TestDump("union with attrib.",union)
TestDump("intersection with attrib.",intersection)
TestDump("difference with attrib.",difference)
TestDump("inclusion with attrib.",sle1)
TestDump("equality with attrib.",seq1)
TestDump("inclusion with attrib.",sle2)
TestDump("equality with attrib.",seq2)

## ##########################################
## Transitions relations
## ##########################################

## Announce
print("################# Set of transitions")

## Create the set
t1 = TransSet()

## Have a name
t1.Name("Delta")

## Insert some transitions: by component
t1.Insert(1,"alpha",2)
t1.Insert(2,"beta",1)
t1.Insert(2,"mue",3)

## Insert some transitions: by transition object
trans = Transition(3,"lambda",1)
t1.Insert(trans)

## Print the set
print(f'TransSet {t1}')

## Read transitions from string (no symbolic names)
t2=TransSet.NewFromString("""<T2>
  1 2 3
  4 5 6
</T2>""")
print(f'TransSet from string: {t2}')

## Read transitions from Python list (no symbolic names)
t3=TransSet.NewFromList([
  ( 7, 8, 9),
  (10,11,12)
])
t3.Name('T3')
print(f'TransSet from Python list: {t3}')

## Obtain a Python list
t2l=list(t2)
print(f'TransSet T2 as Python list: {t2l}')

## Iterate (Python Style)
print(f'Iterating over transition set "{t1.Name()}"')
for t in t1:
  print(f'Transition: x1={t.X1} ev={t.Ev} x1={t.X2}')

## Retrieve set of all states and inspect on per state basis
print(f'Inspecting transition set "{t1.Name()}" on a per state basis')
xset=t1.States()
for x1 in xset:
  print(f'State X1 {x1}: active events {t1.ActiveEvents(x1)} successors {t1.SuccessorStates(x1)}')

## Sort in X2-Ev-X1 order
t1rev = TransSetX2EvX1()
t1rev.Name("Delta_Reverse")
t1.ReSort(t1rev)

## Inspect reverse transition relation on per state bais
print(f'Inspecting transition set "{t1rev.Name()}" on a per state basis')
for x2 in xset:
  print(f'State X2 {x2}: incomming events {t1rev.IncommingEvents(x2)} predecessors {t1rev.PredecessorStates(x2)}')


## ##########################################
## Vectors
## ##########################################

## Announce
print("################# Vector of alphabets")

## Create a vector of alphabets 
## (libFAUDES provides vectors of event sets, generators and systems with similarsyntax/semantics)
avec = AlphabetVector()

## Append alphabets to the vector (takes a copy, the vector owns its elements)
avec.PushBack(aset)
avec.PushBack(bset)

## Access entries (the operator [] provides an editable reference)
avec[0].Insert("alpha")
avec[0].Attribute("alpha").SetControllable()

## Access entries (yes, the [] operator does provide a reference)
aset=avec[0]
aset.Insert("beta")

## Access entries (fine for function arguments, too)
AlphabetUnion(avec[0], avec[1], avec[2])
AlphabetIntersection(avec[0], avec[1], avec[3])

## Set entries assignment (takes another copy, the vector owns its data)
avec[4]=aset
avec[5]=avec[2]

## Explicit resize (here: extend)
avec.Size(8)

## Report
avec.Write()

## Record test case
TestDump("vect test",avec)

## Validate
TestDiff()
