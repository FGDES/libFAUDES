faudes.MakeGlobal()
-- Read automat from files
g1 = System("gen1.gen")
g1:GraphWrite("gen1.png")
g2 = System("gen2.gen")
g2:GraphWrite("gen2.png")
g3 = System("gen3.gen")
g3:GraphWrite("gen3.png")

spec = System("spec.gen")
spec:GraphWrite("spec.png")

e1 = Alphabet("e1.alph")
e2 = Alphabet("e2.alph")
e3 = Alphabet("e3.alph")
ek = Alphabet("ek.alph")
ec = Alphabet("ec.alph") -- controllable events

e = EventSetVector()
e:Append(e1)
e:Append(e2)
e:Append(e3)

proof = System()

res = IsConditionalDecomposable(spec,e,ek,proof)
print("Is conditionally decomposable: ", res)

-- genVect
gVector = GeneratorVector()
gVector:Append(g1)
gVector:Append(g2)
gVector:Append(g3)

-- Global procedure
g = System()
Parallel(gVector,g)

-- Coordinator
gk = System()
Project(g,ek,gk)
StateMin(gk)
gk:GraphWrite("coord.png")

-- PkK
PkK = System()
Project(spec,ek,PkK)
PkK:GraphWrite("PkK.png")

-- P1kK
P1kK = System()
Project(spec,e1+ek,P1kK)
P1kK:GraphWrite("P1kK.png")

-- P2kK
P2kK = System()
Project(spec,e2+ek,P2kK)
P2kK:GraphWrite("P2kK.png")

-- P3kK
P3kK = System()
Project(spec,e3+ek,P3kK)
P3kK:GraphWrite("P3kK.png")

-- specVector
specV = GeneratorVector()
specV:Append(P1kK)
specV:Append(P2kK)
specV:Append(P3kK)

res = IsConditionalControllable(specV,PkK,gVector,gk,ec)
print("Is contidionally controllable: ", res)

res = IsConditionalClosed(specV,PkK,gVector,gk)
print("Is contidionally closed: ", res)

