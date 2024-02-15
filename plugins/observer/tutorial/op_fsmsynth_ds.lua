-- Perform the hierarchical synthesis for the Module "Distribution System" (DS)

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
filePath = "data/fsmsynth/"
supSize = 0
abstractionSize = 0

-- ==================================
-- Stack Feeder (SF)
-- ==================================

-- local supervisor synthesis
plantGen = System("data/fsmsynth/ds/sfcb1pu/sf/sf[0].gen")
plantGen:GraphWrite("data/fsmsynth/ds/sfcb1pu/sf/sf[0].png")
specGen = Generator("data/fsmsynth/ds/sfcb1pu/sf/sf[0]_spec.gen")
specGen:GraphWrite("data/fsmsynth/ds/sfcb1pu/sf/sf[0]_spec.png")
SupConNB(plantGen, specGen, supGen)
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/ds/sfcb1pu/sf/sf[0]_sup.gen")
supGen:GraphWrite("data/fsmsynth/ds/sfcb1pu/sf/sf[0]_sup.png")
supSize = supSize + supGen:Size()

--  msa observer computation with lcc
highAlph:Read("data/fsmsynth/ds/sfcb1pu/sf/sf[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph)
highAlph:Write("data/fsmsynth/ds/sfcb1pu/sf/sf[1]_msalcc.alph")
Project(supGen,highAlph,high1Gen)
StateMin(high1Gen)
print("State count of the SF abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false)
high1Gen:Write("data/fsmsynth/ds/sfcb1pu/sf/sf[1]_msalcc.gen")
high1Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/sf/sf[1]_msalcc.png")


-- ===============================================================
-- Computations for cb2
-- ===============================================================

plantGen = System("data/fsmsynth/ds/cb2/cb2[0].gen");
plantGen:GraphWrite("data/fsmsynth/ds/cb2/cb2[0].png");
specGen = Generator("data/fsmsynth/ds/cb2/cb2[0]_spec.gen");
specGen:GraphWrite("data/fsmsynth/ds/cb2/cb2[0]_spec.png");
SupConNB(plantGen, specGen, supGen);
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/ds/cb2/cb2[0]_sup.gen");
supGen:GraphWrite("data/fsmsynth/ds/cb2/cb2[0]_sup.png");
supSize = supSize + supGen:Size();
highAlph:Read("data/fsmsynth/ds/cb2/cb2[1]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
highAlph:Write("data/fsmsynth/ds/cb2/cb2[1].alph");
Project(supGen,highAlph,high2Gen);
StateMin(high2Gen)
print("State count of the CB2 abstraction: ", high2Gen:Size())
high1Gen:StateNamesEnabled(false)
high2Gen:StateNamesEnabled(false);
high2Gen:Write("data/fsmsynth/ds/cb2/cb2[1].gen");
high2Gen:GraphWrite("data/fsmsynth/ds/cb2/cb2[1].png");


-- ===============================================================
-- Computations for cb3
-- ===============================================================

plantGen = System("data/fsmsynth/ds/cb3/cb3[0].gen");
plantGen:GraphWrite("data/fsmsynth/ds/cb3/cb3[0].png");
specGen = Generator("data/fsmsynth/ds/cb3/cb3[0]_spec.gen");
specGen:GraphWrite("data/fsmsynth/ds/cb3/cb3[0]_spec.png");
SupConNB(plantGen, specGen, supGen);
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/ds/cb3/cb3[0]_sup.gen");
supGen:GraphWrite("data/fsmsynth/ds/cb3/cb3[0]_sup.png");
supSize = supSize + supGen:Size();
highAlph:Read("data/fsmsynth/ds/cb3/cb3[1]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
highAlph:Write("data/fsmsynth/ds/cb3/cb3[1].alph");
Project(supGen,highAlph,high2Gen);
StateMin(high2Gen)
print("State count of the CB3 abstraction: ", high2Gen:Size())
high1Gen:StateNamesEnabled(false)
high2Gen:StateNamesEnabled(false);
high2Gen:Write("data/fsmsynth/ds/cb3/cb3[1].gen");
high2Gen:GraphWrite("data/fsmsynth/ds/cb3/cb3[1].png");  


-- =============================================================
-- computations for the complete conveyor belt cb1 (maximally permissive control)
-- =============================================================

high1Gen = System("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1bpu1/cb1bpu1[0].gen");
high2Gen = System("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1apu2/cb1apu2[0].gen");

-- conveyor belt cb1_connect
high3Gen = System("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_connect.gen");
high3Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_connect.png");

-- additional specifications
spec1Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1bpu1/cb1bpu1[0]_spec.gen");
spec2Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1apu2/cb1apu2[0]_spec.gen");
spec3Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec1.gen");
spec3Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec1.png");
spec4Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec2.gen");
spec4Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec2.png");
spec5Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec3.gen");
spec5Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec3.png");
spec6Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec4.gen");
spec6Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec4.png");  

-- cb1pu plant
Parallel(high1Gen,high2Gen,supGen);
Parallel(supGen,high3Gen,plantGen);
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[0].gen");

-- cb1pu spec
Parallel(spec1Gen,spec2Gen,specGen);
Parallel(specGen,spec3Gen,spec7Gen);
Parallel(spec7Gen,spec4Gen,specGen);
Parallel(specGen,spec5Gen,spec7Gen);
Parallel(spec7Gen,spec6Gen,spec3Gen);
spec7Gen = Generator("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec5.gen");  
spec7Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1_connect/cb1[0]_spec5.png");
Parallel(spec3Gen,spec7Gen,specGen);
specGen:StateNamesEnabled(false);
specGen:Write("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[0]_spec.gen");
InvProject(specGen,plantGen:Alphabet() );
SupConNB(plantGen,specGen,supGen);
supGen:Write("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[0]_sup.gen");
supGen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[0]_sup.png");  
supSize = supSize + supGen:Size();

-- MSA observer with LCC
highAlph:Read("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[1]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
highAlph:Write("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[1]_msalcc.alph");
Project(supGen,highAlph,high5Gen);
StateMin(high5Gen)
print("State count of the CB1PU abstraction: ", high5Gen:Size())
high1Gen:StateNamesEnabled(false)
high5Gen:StateNamesEnabled(false);
high5Gen:Write("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[1]_msalcc.gen");
high5Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[1]_msalcc.png");


-- ========================================================
-- computations for the cb1pu and sf
-- ========================================================

high1Gen = System("data/fsmsynth/ds/sfcb1pu/cb1pu/cb1pu[1]_msalcc.gen");
high2Gen = System("data/fsmsynth/ds/sfcb1pu/sf/sf[1]_msalcc.gen");
Parallel(high1Gen,high2Gen,plantGen);
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/ds/sfcb1pu/sfcb1pu[1]_msalcc.gen");
SupConNB(plantGen,plantGen,supGen); -- nonblocking supervisor
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/ds/sfcb1pu/sfcb1pu[1]_sup_msalcc.gen");
supGen:GraphWrite("data/fsmsynth/ds/sfcb1pu/sfcb1pu[1]_sup_msalcc.png");
supSize = supSize + supGen:Size();
highAlph:Read("data/fsmsynth/ds/sfcb1pu/sfcb1pu[2]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
highAlph:Write("data/fsmsynth/ds/sfcb1pu/sfcb1pu[2]_msalcc.alph");
Project(supGen,highAlph,high5Gen);
StateMin(high5Gen)
print("State count of the combined CB!Pu and SF abstraction: ", high5Gen:Size())
high1Gen:StateNamesEnabled(false)
high5Gen:StateNamesEnabled(false);
high5Gen:Write("data/fsmsynth/ds/sfcb1pu/sfcb1pu[2]_msalcc.gen");
high5Gen:GraphWrite("data/fsmsynth/ds/sfcb1pu/sfcb1pu[2]_msalcc.png"); 
 

-- ========================================================
-- computations for the overall distribution system with 
-- cb1, sf, cb2 and cb3
-- ========================================================

-- plant for cb2 and cb3
high3Gen = System("data/fsmsynth/ds/cb2/cb2[1].gen");
high4Gen = System("data/fsmsynth/ds/cb3/cb3[1].gen");

-- plant
high2Gen = System("data/fsmsynth/ds/sfcb1pu/sfcb1pu[2]_msalcc.gen");
Parallel(high2Gen,high2Gen,plantGen);
Parallel(plantGen,high3Gen,high5Gen);
Parallel(high5Gen,high4Gen,plantGen);
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/ds/ds[1]_msalcc.gen");  

-- spec
spec1Gen = Generator("data/fsmsynth/ds/ds[1]_spec1.gen");
spec2Gen = Generator("data/fsmsynth/ds/ds[1]_spec2.gen");
Parallel(spec1Gen,spec2Gen,specGen);
InvProject(specGen,plantGen:Alphabet() );
SupConNB(plantGen,specGen,supGen);
supGen:Write("data/fsmsynth/ds/ds[1]_sup_msalcc.gen");
supGen:GraphWrite("data/fsmsynth/ds/ds[1]_sup_msalcc.png");
supSize = supSize + supGen:Size();
highAlph:Read("data/fsmsynth/ds/ds[2]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
highAlph:Write("data/fsmsynth/ds/ds[2]_msalcc.alph");
Project(supGen,highAlph,high5Gen);
StateMin(high5Gen)
print("State count of the DS abstraction: ", high5Gen:Size())
high1Gen:StateNamesEnabled(false)
high5Gen:StateNamesEnabled(false);
high5Gen:Write("data/fsmsynth/ds/ds[2]_msalcc.gen");
high5Gen:GraphWrite("data/fsmsynth/ds/ds[2]_msalcc.png");
print("Accumulated state count of the DS supervisors ", supSize)
 

-- record testcase
FAUDES_TEST_DUMP("DS",supSize)
