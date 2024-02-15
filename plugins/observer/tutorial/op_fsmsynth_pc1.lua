-- Perform the hierarchical synthesis for the Module "Production Cell 1" (PC1)

faudes.MakeGlobal()

-- =================================
-- Variables
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
filePath = "data/fsmsynth/pc1-exit1-ics/"
supSize = 0


-- ================================
-- supervisor computation cb11
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[0]_sup.gen") 
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB11 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[1].png")


-- ================================
-- supervisor computation rt1
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[0]_sup.gen")   
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT1 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[1].png")

-- ================================ 
-- supervisor computation rt1-cb11
-- ================================

-- plant
high1Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/cb11/cb11[1].gen")
high2Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1/rt1[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[1].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[1].png")

-- spec
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[1]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[1]_sup.gen")
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[1]_sup.png")

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[2]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT1-CB11 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[2]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[2]_msalcc.png")


-- ================================
-- supervisor computation cb4
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[0]_sup.gen")   
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB4 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[1].png")


-- ================================
-- supervisor computation mh1d1
-- ================================

-- local supervisor computation
high1Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1[0].gen")
high2Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/d1[0].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[0]_sup.gen")   
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[0]_sup.png") 
-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of MH1D1 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[1]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[1]_msalcc.png")


-- ================================
-- supervisor computation cb4-mh1-d1
-- ================================

-- local supervisor computation
high1Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/mh1d1/mh1d1[1]_msalcc.gen")
high2Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4/cb4[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[1]_msalcc.gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[1]_msalcc.png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[1]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[1]_sup_msalcc.gen")   
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[1]_sup_msalcc.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[2]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB4-MH1D1 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[2]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[2]_msalcc.png")


-- ================================
-- supervisor computation cb7
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[0]_sup.gen")   
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB7 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[1].png")


-- ================================
-- supervisor computation pc1
-- ================================

-- plant
high1Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb7/cb7[1].gen")
high2Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/cb4-mh1-d1/cb4mh1d1[2]_msalcc.gen")
high3Gen:Read("data/fsmsynth/pc1-exit1-ics/pc1/rt1-cb11/rt1cb11[2]_msalcc.gen")
Parallel(high1Gen,high2Gen,supGen)
Parallel(supGen,high3Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/pc1[2]_msalcc.gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/pc1[2]_msalcc.png")

-- spec
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1/pc1[2]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/pc12]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1/pc1[2]_sup_msalcc.gen") 
supSize = supSize + supGen:Size()
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/pc1[2]_sup_msalcc.png")   

-- natural observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/pc1/pc1[3]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1/pc1[3]_msalcc.alph","Alphabet");
Project(supGen,highAlph,high1Gen);
StateMin(high1Gen)
print("State count of PC1 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false);
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1/pc1[3]_msalcc.gen");
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1/pc1[3]_msalcc.png");
print("Accumulated state count of the PC1 supervisors: ", supSize)

-- record testcase
FAUDES_TEST_DUMP("PC1",supSize)
