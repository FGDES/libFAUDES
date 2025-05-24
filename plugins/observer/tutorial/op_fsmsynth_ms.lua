-- Perform the hierarchical synthesis for the overall "Manufacturing System" (MS)

-- Make faudes name space global
faudes.MakeGlobal()

-- Disable graph output
faudes.DotExecPath("none")  

-- Run low level synthesis scripts
dofile("op_fsmsynth_ds.lua")
dofile("op_fsmsynth_pc1.lua")
dofile("op_fsmsynth_pc2.lua")
dofile("op_fsmsynth_exit1.lua")
dofile("op_fsmsynth_exit2.lua")
dofile("op_fsmsynth_ics.lua")
dofile("op_fsmsynth_pc1_exit1_ics.lua")


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
high1Gen = System()
high2Gen = System()
high3Gen = System()
high5Gen = System()
high6Gen = System()
highAlph = EventSet()

-- =================================
-- Highlevel-synthesis
-- =================================

-- compose plant
high1Gen = System("data/fsmsynth/pc1-exit1-ics/pc1exit1ics[4]_msalcc.gen");
high2Gen = System("data/fsmsynth/ds/ds[2]_msalcc.gen");
high3Gen = System("data/fsmsynth/pc2/pc2[3]_msalcc.gen");
high5Gen = System("data/fsmsynth/exit2/exit2[3]_msalcc.gen")
Parallel(high1Gen,high3Gen,supGen);
Parallel(supGen,high2Gen,high1Gen);
Parallel(high1Gen,high5Gen,plantGen)
plantGen:StateNamesEnabled(false);
plantGen:Write("data/fsmsynth/ms[4]_msalcc.gen");

-- perform synthesis
SupCon(plantGen,plantGen,supGen);
supGen:StateNamesEnabled(false);
supGen:Write("data/fsmsynth/ms[4]_sup_msalcc.gen");
print("State count of the MS supervisor: ", supGen:Size() )

-- record testcase
supGen:Name("mssup")
FAUDES_TEST_DUMP("MS",supGen)

