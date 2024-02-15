-- Perform the hierarchical synthesis for the Module "Exit 1" 

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
high2Gen = System()
high3Gen = System()
high4Gen = System()
high5Gen = System()
high6Gen = System()
highAlphabet = EventSet()
filePath = "data/fsmsynth/pc1-exit1-ics/"
supSize = 0


-- ================================
-- supervisor computation cb15
-- ================================

plantGen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[0]_spec.gen")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)
supSize = supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[0]_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[0]_sup.png")

-- msa-observer 
highAlphabet:Read("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlphabet)
Project(supGen,highAlphabet,highGen)
StateMin(highGen)
print("State count of the CB15 abstraction", highGen:Size() )
highAlphabet:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[1]_msalcc.alph")
highGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[1]_msalcc.gen")
highGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[1]_msalcc.png")


-- =================================
-- supervisor computation rts1
-- ================================

high2Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_1-2.gen")
high2Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_1-2.png") 
high3Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_2-3.gen")
high3Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_2-3.png") 
high4Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_3-4.gen")
high4Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_3-4.png") 
Parallel(high2Gen,high3Gen,high5Gen)
high5Gen:StateNamesEnabled(false)
high5Gen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_dum.gen")
high5Gen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_dum.png")
Parallel(high5Gen,high4Gen,high6Gen)
StateMin(high6Gen,plantGen)
plantGen:SetControllable(high6Gen:ControllableEvents() )
plantGen:StateNamesEnabled(false)
plantGen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0].gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0].png")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_spec.gen")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Read("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[0]_sup.png")

-- msa-observer and LCC
highAlphabet:Read("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[1]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlphabet)
Project(supGen,highAlphabet,highGen)
StateMin(highGen)
print("State count of the RTS1 abstraction", highGen:Size() )
highAlphabet:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[1]_msalcc.alph")
highGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[1]_msalcc.gen")
highGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[1]_msalcc.png")


-- ============================================
-- supervisor computation rts1-cb15 with msa-observer 
-- ============================================

high2Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/cb15/cb15[1]_msalcc.gen")
high3Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1/rts1[1]_msalcc.gen")
specGen = Generator("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[1]_spec.gen")
specGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[1]_spec.png")
Parallel(high2Gen,high3Gen,plantGen)
plantGen:StateNamesEnabled(false)
plantGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[1]_msalcc.gen")
plantGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[1]_msalcc.png")
InvProject(specGen,plantGen:Alphabet() )
SupConNB(plantGen,specGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[1]_msalcc_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[1]__msalcc_sup.png")
highAlphabet:Read("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[2]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlphabet)
Project(supGen,highAlphabet,highGen)
StateMin(highGen)
print("State count of the RTS1-CB15 abstraction", highGen:Size() )
highAlphabet:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[2]_msalcc.alph")
highGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[2]_msalcc.gen")
highGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[2]_msalcc.png")


-- ================================
-- Abstraction of rc1
-- ================================

supGen:Read("data/fsmsynth/pc1-exit1-ics/exit1/rc1/rc1[0].gen") 
supSize = supSize + supGen:Size()
highAlphabet:Read("data/fsmsynth/pc1-exit1-ics/exit1/rc1/rc1[1]_orig.alph","Alphabet")
Project(supGen,highAlphabet,highGen)
StateMin(highGen)
print("State count of the RC1 abstraction", highGen:Size() )
highGen:StateNamesEnabled(false)
highGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/rc1/rc1[1].gen")
highGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/rc1/rc1[1].png")


-- ================================
-- supervisor computation exit1 with msa-observer and lcc
-- ================================   

high2Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rts1-cb15/rts1cb15[2]_msalcc.gen")
high3Gen = System("data/fsmsynth/pc1-exit1-ics/exit1/rc1/rc1[1].gen")
Parallel(high2Gen,high2Gen,plantGen)
SupConNB(plantGen,plantGen,supGen)
supSize = supSize + supGen:Size()
supGen:StateNamesEnabled(false)
supGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/exit1[2]_msalcc_sup.gen")
supGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/exit1[2]_msalcc_sup.png")
highAlphabet:Read("data/fsmsynth/pc1-exit1-ics/exit1/exit1[3]_orig.alph","Alphabet")
MsaObserverLcc(supGen,supGen:ControllableEvents(),highAlphabet)
Project(supGen,highAlphabet,highGen)
StateMin(highGen)
print("State count of the EXIT1 abstraction", highGen:Size() )
highAlphabet:Write("data/fsmsynth/pc1-exit1-ics/exit1/exit1[3]_msalcc.alph")
highGen:Write("data/fsmsynth/pc1-exit1-ics/exit1/exit1[3]_msalcc.gen")
highGen:GraphWrite("data/fsmsynth/pc1-exit1-ics/exit1/exit1[3]_msalcc.png")
print("Accumulated state count of the EXIT1 supervisors: ", supSize)


-- record testcase
FAUDES_TEST_DUMP("EX1",supSize)
