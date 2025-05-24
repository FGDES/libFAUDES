-- Perform the hierarchical synthesis for the Module "Exit 2" 

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
highGen = System()
cGen1 = System()
cGen2 = System()
cGen3 = System()
high5Gen = System()
high6Gen = System()
highAlph = EventSet()
filePath = "data/fsmsynth/"
supSize = 0


-- ==================================
-- Converyor belt CB16
-- ==================================

plantGen = System("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[0].gen");
plantGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[0].png");
specGen = System("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[0]_spec.gen");
InvProject(specGen,plantGen:Alphabet() )
SupCon(plantGen,specGen,supGen);
supSize = supGen:Size()
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[0]_sup.gen");
supGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[0]_sup.png");

-- msa-observer computation with Lcc
highAlph:Read("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[1]_orig.alph","Alphabet");
supSize = supSize + MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
Project(supGen,highAlph,highGen);
StateMin(highGen)
print("State count of CB16 abstraction: ", highGen:Size())
highAlph:Write("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[1]_msalcc.alph");
highGen:Write("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[1]_msalcc.gen");
highGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[1]_msalcc.png");


-- ================================
-- supervisor computation rts2
-- ================================   

cGen1:Read("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_6-5.gen");
cGen1:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_6-5.png"); 
cGen2:Read("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_5-4.gen");
cGen2:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_5-4.png"); 
Parallel(cGen1,cGen2,plantGen);
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0].gen");
plantGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0].png");
specGen = System("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_spec.gen");
InvProject(specGen,plantGen:Alphabet() )
SupCon(plantGen,specGen,supGen);
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_sup.gen");
supGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[0]_sup.png");

-- msa-observer with Lcc
highAlph:Read("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[1]_orig.alph","Alphabet");
supSize = supSize + MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
Project(supGen,highAlph,highGen);
StateMin(highGen)
print("State count of RTS2 abstraction: ", highGen:Size())
highAlph:Write("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[1]_msalcc.alph");
highGen:Write("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[1]_msalcc.gen");
highGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[1]_msalcc.png");
 

-- ====================================================================
-- supervisor computation rts2-cb16 with nmsa-observer and with Lcc
-- ====================================================================

cGen1:Read("data/fsmsynth/exit2/rts2-cb16/cb16/cb16[1]_msalcc.gen");
cGen2:Read("data/fsmsynth/exit2/rts2-cb16/rts2/rts2[1]_msalcc.gen");
specGen = System("data/fsmsynth/exit2/rts2-cb16/rts2cb16[1]_spec.gen");
Parallel(cGen1,cGen2,plantGen);
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/exit2/rts2-cb16/rts2cb16[1]_msalcc.gen");
plantGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2cb16[1]_msalcc.png");
InvProject(specGen,plantGen:Alphabet() );
SupCon(plantGen,specGen,supGen);
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/exit2/rts2-cb16/rts2cb16[1]_msalcc_sup.gen");
supGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2cb16[1]_msalcc_sup.png");
highAlph:Read("data/fsmsynth/exit2/rts2-cb16/rts2cb16[2]_orig.alph","Alphabet");
supSize = supSize + MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
Project(supGen,highAlph,highGen);
StateMin(highGen)
print("State count of RTS2-CB16 abstraction: ", highGen:Size())
highAlph:Write("data/fsmsynth/exit2/rts2-cb16/rts2cb16[2]_msalcc.alph");
highGen:Write("data/fsmsynth/exit2/rts2-cb16/rts2cb16[2]_msalcc.gen");
highGen:GraphWrite("data/fsmsynth/exit2/rts2-cb16/rts2cb16[2]_msalcc.png");


-- ================================
-- Abstraction of rc2
-- ================================
supGen = System("data/fsmsynth/exit2/rc2/rc2[0].gen"); 
supSize = supSize + supGen:Size()
highAlph:Read("data/fsmsynth/exit2/rc2/rc2[1].alph","Alphabet");
Project(supGen,highAlph,highGen);
StateMin(highGen)
print("State count of RC2 abstraction: ", highGen:Size())
highGen:StateNamesEnabled(false);
highGen:Write("data/fsmsynth/exit2/rc2/rc2[1].gen");
highGen:GraphWrite("data/fsmsynth/exit2/rc2/rc2[1].png");


-- ==============================================================
-- supervisor computation exit2 with msa-observer and with Lcc
-- ==============================================================

cGen1:Read("data/fsmsynth/exit2/rts2-cb16/rts2cb16[2]_msalcc.gen");
cGen2:Read("data/fsmsynth/exit2/rc2/rc2[1].gen");
Parallel(cGen1,cGen2,plantGen);
SupCon(plantGen,plantGen,supGen);
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/exit2/exit2[2]_msalcc_sup.gen");
supGen:GraphWrite("data/fsmsynth/exit2/exit2[2]_msalcc_sup.png");
highAlph:Read("data/fsmsynth/exit2/exit2[3]_orig.alph","Alphabet");
supSize = supSize + MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
Project(supGen,highAlph,highGen);
StateMin(highGen)
print("State count of EXIT2 abstraction: ", highGen:Size())
highAlph:Write("data/fsmsynth/exit2/exit2[3]_msalcc.alph");
highGen:Write("data/fsmsynth/exit2/exit2[3]_msalcc.gen");
highGen:GraphWrite("data/fsmsynth/exit2/exit2[3]_msalcc.png");
print("Accumulated state count of the EXIT2 supervisors: ", supSize)



-- record testcase
FAUDES_TEST_DUMP("EX2",supSize)
