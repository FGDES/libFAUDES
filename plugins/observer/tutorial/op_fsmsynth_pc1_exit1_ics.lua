-- Perform the hierarchical synthesis for the Module "Distribution System" (DS)

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
highGen = System()
high1Gen = System()
high2Gen = System()
high3Gen = System()
high5Gen = System()
high6Gen = System()
highAlph = EventSet()
filePath = "data/fsmsynth/"
supSize = 0


-- =============================================================================
-- ics, pc1 and exit1 (same supervisor computation for all types of abstractions
-- =============================================================================
high1Gen= System("data/fsmsynth/pc1-exit1-ics/ics/ics[3]_msalcc.gen");
high2Gen= System("data/fsmsynth/pc1-exit1-ics/pc1/pc1[3]_msalcc.gen");
high3Gen= System("data/fsmsynth/pc1-exit1-ics/exit1/exit1[3]_msalcc.gen");
Parallel(high1Gen,high2Gen,supGen);
Parallel(supGen,high3Gen,plantGen);
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[3].gen");

-- specification
specGen = Generator("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[3]_spec.gen");
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[3]_spec.png");
InvProject(specGen,plantGen:Alphabet() );

-- supervisor computation
SupConNB(plantGen,specGen,supGen);
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[3]_sup.gen");

-- ============
-- abstractions
-- ============

-- msa-observer with LCC
highAlph= EventSet("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[4]_orig.alph","Alphabet");
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlph);
highAlph:Write("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[4]_msalcc.alph","Alphabet");
Project(supGen,highAlph,high1Gen);
StateMin(high1Gen)
print("State count of PC1-EXIT1-ICS abstraction: ", high1Gen:Size())
high1Gen:StateNamesEnabled(false);
high1Gen:Write("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[4]_msalcc.gen");
print("Accumulated state count of the PC1-EXIT1-ICS supervisors: ", supSize)


-- record testcase
FAUDES_TEST_DUMP("PC1EX1",supSize)

