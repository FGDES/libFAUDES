-- Perform the hierarchical synthesis for the Module "Production Cell 2" (PC2)
faudes.MakeGlobal()

-- =================================
-- Basic Variables
-- =================================

plantGen = System()
specGen = Generator()
spec1Gen = Generator()
spec2Gen = Generator()
spec3Gen = Generator()
spec4Gen = Generator()
spec5Gen = Generator()
spec6Gen = Generator()
spec7Gen = Generator()
supGen = System()
high1Gen = System()
high2Gen = System()
high3Gen = System()
high4Gen = System()
high5Gen = System()
high6Gen = System()
highAlph = EventSet()
filePath = "data/fsmsynth/"
supSize = 0


-- ================================
-- supervisor computation cb14
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[0].png")
specGen = Generator("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[0]_sup.gen") 
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB14 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[1].png")


-- ================================
-- supervisor computation rt4
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[0].png")
specGen = Generator("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)  
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT4 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[1].png")
 

-- ================================ 
-- supervisor computation rt4-cb14
-- ================================

-- plant
high1Gen:Read("data/fsmsynth/pc2/rt4-cb14/cb14/cb14[1].gen")
high2Gen:Read("data/fsmsynth/pc2/rt4-cb14/rt4/rt4[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc2/rt4-cb14/rt4cb14[1].gen")
plantGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4cb14[1].png")

-- spec
specGen = Generator("data/fsmsynth/pc2/rt4-cb14/rt4cb14[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4cb14[1]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
supSize = supSize + supGen:Size()
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc2/rt4-cb14/rt4cb14[1]_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4cb14[1]_sup.png")

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/rt4-cb14/rt4cb14[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/rt4-cb14/rt4cb14[2]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT4-CB14 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/rt4-cb14/rt4cb14[2]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/rt4-cb14/rt4cb14[2]_msalcc.png")


-- ================================
-- supervisor computation cb6
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[0].png")
specGen = Generator("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB6 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[1].png")


-- ================================
-- supervisor computation mh2d2
-- ================================

-- local supervisor computation
high1Gen:Read("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2[0].gen")
high2Gen:Read("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/d2[0].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[0].png")
specGen = Generator("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of MH2D2 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[1]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[1]_msalcc.png")

 
-- ================================
-- supervisor computation cb6-mh2-d2
-- ================================

-- local supervisor computation
high1Gen:Read("data/fsmsynth/pc2/cb6-mh2-d2/mh2d2/mh2d2[1]_msalcc.gen")
high2Gen:Read("data/fsmsynth/pc2/cb6-mh2-d2/cb6/cb6[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[1]_msalcc.gen")
plantGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[1]_msalcc.png")
specGen = Generator("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[1]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[1]_sup_msalcc.gen")   
supGen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[1]_sup_msalcc.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[2]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB6-MH2D2 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[2]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[2]_msalcc.png")


-- ================================
-- supervisor computation cb10
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc2/cb10/cb10[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc2/cb10/cb10[0].png")
specGen = Generator("data/fsmsynth/pc2/cb10/cb10[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/cb10/cb10[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc2/cb10/cb10[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc2/cb10/cb10[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc2/cb10/cb10[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/cb10/cb10[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB10 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/cb10/cb10[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/cb10/cb10[1].png")


-- ================================
-- supervisor computation pc2
-- ================================

-- plant
high1Gen:Read("data/fsmsynth/pc2/cb10/cb10[1].gen")
high2Gen:Read("data/fsmsynth/pc2/cb6-mh2-d2/cb6mh2d2[2]_msalcc.gen")
high3Gen:Read("data/fsmsynth/pc2/rt4-cb14/rt4cb14[2]_msalcc.gen")
Parallel(high1Gen,high2Gen,supGen)
Parallel(supGen,high3Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc2/pc2[2]_msalcc.gen")
plantGen:GraphWrite("data/fsmsynth/pc2/pc2[2]_msalcc.png")

-- spec
specGen = Generator("data/fsmsynth/pc2/pc2[2]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc2/pc22]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc2/pc2[2]_sup_msalcc.gen") 
supGen:GraphWrite("data/fsmsynth/pc2/pc2[2]_sup_msalcc.png")

-- natural observer computation
highAlph:Read("data/fsmsynth/pc2/pc2[3]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc2/pc2[3]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of PC2 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc2/c2[3]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc2/pc2[3]_msalcc.png")
print("Accumulated state count of the PC2 supervisors: ", supSize)


-- record testcase
FAUDES_TEST_DUMP("PC2",supSize)
