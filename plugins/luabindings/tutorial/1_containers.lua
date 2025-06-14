-- Test/demonstrate basic faudes containers 

-- Turn on debugging messages
-- faudes.Verbosity(0)

-- ------------------------------------------
-- Plain index set (eg StateSet)
-- ------------------------------------------

-- Announce
print("################# Set of indicees")

-- Create plain set of indices
iset = faudes.IndexSet()

-- Have a name
iset:Name("PlainIndexContainer")

-- Insert some indicees
iset:Insert(2)
iset:Insert(4)
iset:Insert(8)
iset:Insert(10)

-- Print the set via lua string conversion
print(iset)

-- Access via iterator
print("Iterating over IndexSet:",iset:Name())
iit=iset:Begin()
while iit~=iset:End() do
  -- Dereference iterator 
  print(iit:Index())
  -- Increment iterator
  iit:Inc()
end

-- Simple assignment gives a reference, not a copy
isetR = iset

-- To create a copy, use copy constructor or copy method
i2set = faudes.IndexSet(iset)
i3set = iset:Copy()

-- Manipulate the original iset ...
iset:Erase(10)

-- ... which affects isetR, but not i2set and i3set
print(i2set)
print(i3set)
print(isetR)

-- Record test case
FAUDES_TEST_DUMP("iset orig",iset)
FAUDES_TEST_DUMP("iset ref",isetR)
FAUDES_TEST_DUMP("iset copy 2",i2set)
FAUDES_TEST_DUMP("iset copy 3",i3set)


-- ------------------------------------------
-- Set of indicees with symbolic names 
-- ------------------------------------------

-- Announce
print("################# Set of symbols")

-- Create the set
nset = faudes.EventSet()

-- Have a name
nset:Name("SymbolicIndiceesContainer")

-- Insert some elements
nset:Insert("alpha")
nset:Insert("beta")

-- Print the set
print(nset)

-- Symbol lookup
idx=nset:Index("beta")
name=nset:SymbolicName(idx)
print(name, " has index ", idx)

-- Iterate
print("Iterating over EventSet",nset:Name())
nit=nset:Begin()
while nit~=nset:End() do
  -- Variants of deref
  print("Element: ", nit:Index(), nit:Name(), nset:SymbolicName(nit), nset:Str(nit))
  -- Increment
  nit:Inc()
end 

-- Erase an element
nset:Erase("beta")

-- Create a copy of the set
n2set = nset:Copy()

-- Write to file
n2set:Write("tmp_nameset.txt")

-- ------------------------------------------
-- Boolean operations on sets
-- ------------------------------------------

print("################# Boolean operations")

aset=faudes.EventSet()
bset=faudes.EventSet()
aset:FromString('<A> "a" "b" "c" </A>')
bset:FromString('<B> "c" "d" "e" </B>')

print(aset)
print(bset)

-- Union
print(aset+bset)

-- Intersection
print(aset*bset)

-- Difference
print(aset-bset)

-- Inclusion
if aset <= aset + bset then
  print("a <= a+b     ---  OK")
end

-- Equality
if aset ~= bset  then
  print("a ~= b       ---  OK")
end

-- ------------------------------------------
-- Set of events with control-system attribute
-- ------------------------------------------

-- Announce
print("################# Set of events with control-system attribute")

-- Create the set eg from the above set
eset = faudes.Alphabet(nset)

-- Have a name
eset:Name("Alphabet")

-- Insert some more elements
eset:Insert("beta")
eset:Insert("lambda")

-- Set event to be controllabel
eset:Attribute("alpha"):SetControllable()

-- Insert with an attribute prepared
cattr=faudes.AttributeCFlags()
cattr:SetControllable()
eset:Insert("mue",cattr)

-- Set attribute of element
eset:Attribute("lambda",cattr)

-- Print the set
print(eset)

-- Iterate
print("Iterating over alphabet",eset:Name())
eit=eset:Begin()
while eit~=eset:End() do
  -- Variants of deref
  print("Element: ", eit:Index(), eit:Name(), eset:Str(eit), eset:Attribute(eit))
  -- Increment
  eit:Inc()
end


-- ------------------------------------------
-- Boolean operations on sets with attributes
-- ------------------------------------------

print("################# Boolean operations incl attributes")

aset=faudes.Alphabet()
bset=faudes.Alphabet()
aset:FromString('<A> "a" +C+ "b" +o+  "c" +o+  </A>')
bset:FromString('<B> "c" +o+ "d"      "e" +CF+ </B>')
cset=faudes.Alphabet(aset);
cset:Attribute("a"):ClrControllable()

print(aset)
print(bset)
print(cset)

-- The operators *, + and - return plain event sets
print("Operators swallow attributes:")
print(aset + bset)
print(aset - bset)
print(aset * bset)

-- Set comaprison operators (attributes ignored)
sle1 = (aset <= cset)
if sle1 then
  print("a <= c --- OK")
end
seq1 = (aset == cset)
if seq1 then
  print("a == c --- OK")
end

-- Set comparison methods (when types don't match)
sle2 = faudes.AlphabetInclusion(aset, aset + bset)               
if sle2 then
  print("a <= a + b --- OK")
end
seq2 = faudes.AlphabetEquality(aset - bset + aset,aset)
if seq2 then
  print("a - b + a == a --- OK")
end


-- Technical detail. By std. Lua semantics, the order operators "==", "<=" 
-- and "<" do not allow for overloading with mixed types. E.g., no matter 
-- what comparison function we implement, "==" is meant to insist in a 
-- matching type. It is only due to a rather crude hack that the below
-- code fragment evaluates as the one may expect. We recommend that you
-- avoid the issue alltogether and use the above functions.

sle3 = (aset <= aset + bset)               -- avoid this: types dont match
if sle3 then
  print("a <= a + b --- OK")
end
seq3 = (aset == aset - bset + aset )       -- avoid this: types dont match
if seq3 then
  print("a == a - b + a --- OK")
end


-- Union, maintain attributes
union=faudes.Alphabet()
faudes.AlphabetUnion(aset,bset,union)
print("AlphabetUnion(...) maintains attributes");
print(union)

-- Intersection, maintain attributes
intersection=faudes.Alphabet()
faudes.AlphabetIntersection(aset,bset,intersection)
print("AlphabetIntersection(...) maintains attributes");
print(intersection)

-- Difference, maintain attributes
difference=faudes.Alphabet()
faudes.AlphabetDifference(aset,bset,difference)
print("AlphabetDifference(...) maintains attributes");
print(difference)

-- Record test case
FAUDES_TEST_DUMP("union with attrib.",union)
FAUDES_TEST_DUMP("intersection with attrib.",intersection)
FAUDES_TEST_DUMP("difference with attrib.",difference)
FAUDES_TEST_DUMP("inclusion with attrib.",sle1)
FAUDES_TEST_DUMP("equality with attrib.",seq1)
FAUDES_TEST_DUMP("inclusion with attrib.",sle2)
FAUDES_TEST_DUMP("equality with attrib.",seq2)
FAUDES_TEST_DUMP("inclusion with attrib.",sle3)
FAUDES_TEST_DUMP("equality with attrib.",seq3)

-- ------------------------------------------
-- Transitions relations
-- ------------------------------------------

-- Announce
print("################# Set of transitions")

-- Create the set
tset = faudes.TransSet()

-- Have a name
tset:Name("TransitionRelation")

-- Insert some transitions: by component
tset:Insert(1,"alpha",2)
tset:Insert(2,"beta",1)
tset:Insert(2,"mue",3)

-- Insert some transitions: prepared
trans = faudes.Transition(3,"lambda",1)
tset:Insert(trans)

-- Print the set
tset:Write()

-- Iterate
print("Iterating over transition set",tset:Name())
tit=tset:Begin()
while tit~=tset:End() do
  -- Variants of deref
  print("Element: ", tit:Transition(), tset:Str(tit))
  -- Increment
  tit:Inc()
end


-- Sort in X2-Ev-X1 order
trset = faudes.TransSetX2EvX1()
trset:Name("ReverseTransitionSet")
tset:ReSort(trset)

-- Print the set
print("Transitionset in reverse order")
trset:Write()


-- ------------------------------------------
-- Vectors
-- ------------------------------------------

-- Announce
print("################# Vector of alphabets")

-- Create a vector of alphabets 
-- Note: corefaudes also provides vectors of event sets, generators and systems
avec = faudes.AlphabetVector()

-- Append alphabet to the vector (takes a copy)
avec:PushBack(aset)
avec:PushBack(bset)

-- Access entries (provides reference)
avec[0]:Insert("alpha")
avec[0]:Attribute("alpha"):SetControllable()

-- Access entries (yes, the [] operator does provide a reference)
aset=avec[0]
aset:Insert("beta")

-- Access entries (fine for function arguments, too)
faudes.AlphabetUnion(avec[0], avec[1], avec[2])
faudes.AlphabetIntersection(avec[0], avec[1], avec[3])

-- Set entries assignment (takes a copy)
avec[4]=aset
avec[5]=avec[2]

-- Explicit resize (here: extend)
avec:Size(8)

-- Report
avec:Write()

-- Record test case
FAUDES_TEST_DUMP("vect test",avec)

-- Validate
FAUDES_TEST_DIFF()