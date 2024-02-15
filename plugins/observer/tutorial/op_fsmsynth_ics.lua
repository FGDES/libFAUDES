-- Perform the hierarchical synthesis for the Module ICS

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
-- supervisor computation cb5
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[0]_spec.png")
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[0]_sup.png") 

-- msa-observer computation with lcc
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB5 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[1].png")


-- ================================
-- supervisor computation cb9
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB9 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[1].png")


-- ================================
-- supervisor computation cb12
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[0]_sup.gen")   
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB12 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[1].png")


-- ================================
-- supervisor computation rt2
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[0]_sup.gen")  
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT2 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[1].png")


-- ================================ 
-- supervisor computation rt2-cb12
-- ================================

-- plant
high1Gen = System("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/cb12/cb12[1].gen")
high2Gen = System("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2/rt2[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[1].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[1].png")

-- spec
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[1]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[1]_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[1]_sup.png")

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[2]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT2-CB12 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[2]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[2]_msalcc.png")


-- ================================
-- supervisor computation cb13
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[0]_sup.gen")  
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of CB13 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[1].png")


-- ================================
-- supervisor computation rt3
-- ================================

-- local supervisor computation
plantGen = System("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[0]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)   
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)  
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[0]_sup.gen")  
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[0]_sup.png") 

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[1]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT3 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[1].gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[1].png")


-- ================================ 
-- supervisor computation rt3-cb13
-- ================================

-- plant
high1Gen = System("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/cb13/cb13[1].gen")
high2Gen = System("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3/rt3[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[1].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[1].png")

-- spec
specGen = Generator("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[1]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[1]_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[1]_sup.png")

-- msa-observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[2]_msalcc.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of RT3-CB13 abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[2]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[2]_msalcc.png")


-- ================================
-- supervisor computation ics
-- ================================

-- plant
high1Gen = System("data/fsmsynth/pc1-exit1-ics/ics/rt2-cb12/rt2cb12[2]_msalcc.gen")
high2Gen = System("data/fsmsynth/pc1-exit1-ics/ics/rt3-cb13/rt3cb13[2]_msalcc.gen")
high3Gen = System("data/fsmsynth/pc1-exit1-ics/ics/cb5/cb5[1].gen")
high4Gen = System("data/fsmsynth/pc1-exit1-ics/ics/cb9/cb9[1].gen")
Parallel(high1Gen,high2Gen,plantGen)
Parallel(plantGen,high3Gen,supGen)
Parallel(supGen,high4Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_msalcc.gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_msalcc.png")

-- spec
spec1Gen = System("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_spec1.gen")
spec1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_spec1.png")
spec2Gen = System("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_spec2.gen")
spec2Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_spec2.png")
Parallel(spec1Gen,spec2Gen,specGen)
specGen:StateNamesEnabled(false)
specGen:Write("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_spec.png")
InvProject(specGen,plantGen:Alphabet() )
Parallel(plantGen,specGen,supGen)
SupConNB(plantGen,specGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_sup_msalcc.gen") 
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/ics[2]_sup_msalcc.png")

-- msa observer computation
highAlph:Read("data/fsmsynth/pc1-exit1-ics/ics/ics[3]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/pc1-exit1-ics/ics/ics[3]_obs.alph","Alphabet")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of ICS abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/ics/ics[3]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/ics/ics[3]_msalcc.png")
print("Accumulated state count of the ICS supervisors: ", supSize)


-- record testcase
FAUDES_TEST_DUMP("ICS",supSize)
