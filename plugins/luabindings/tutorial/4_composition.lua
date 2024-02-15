-- Test/demonstrate composition functions

-- ------------------------------------------
-- Prepare two simple machines
-- ------------------------------------------

-- Announce
print("################# Prepare two simple machines");

-- Read original machine
machine  = faudes.System("data/verysimplemachine.gen")

-- Prepare two copies
machine1 = faudes.System()
machine2 = faudes.System()
faudes.Version(machine,"1",machine1)
faudes.Version(machine,"2",machine2)

-- Report
machine1:Write()
machine2:Write()

-- ------------------------------------------
-- Std parallel
-- ------------------------------------------

-- Announce
print("################# Std parallel composition (here: shuffle product)");


-- Compose overall plant
machines = faudes.Generator()
faudes.Parallel(machine1,machine2,machines)

-- Report
machines:Write()

-- Record test case
FAUDES_TEST_DUMP("parallel",machines)

-- ------------------------------------------
-- Std parallel with attributes
-- ------------------------------------------

-- Announce
print("################# Std parallel composition (here: shuffle product, incl attributes)");


-- Compose overall plant
cmachines = faudes.System()
faudes.Parallel(machine1,machine2,cmachines)

-- Report
cmachines:Write()

-- Record test case
FAUDES_TEST_DUMP("parallel incl. attr.",cmachines)

-- ------------------------------------------
-- Std parallel with composition map 
-- ------------------------------------------

-- Announce
print("################# Std parallel composition (here: incl. comp. map)");


-- Compose overall plant
compmap = faudes.ProductCompositionMap();
faudes.Parallel(machine1,machine2,compmap,cmachines)

-- Report
print("Comp. map:", compmap)

-- Query map
sc_21 = compmap:CompState(2,1)
s1_3  = compmap:Arg1State(3)
s2_3  = compmap:Arg2State(3)
print("Composed state (2,1) is indexed ",sc_21)
print("Composed state 3 corresponds to machine1 state",s1_3)
print("Composed state 3 corresponds to machine2 state",s2_3)

-- Record test case
FAUDES_TEST_DUMP("parallel incl. comp. map a", sc_21)
FAUDES_TEST_DUMP("parallel incl. comp. map b", s1_3)
FAUDES_TEST_DUMP("parallel incl. comp. map c", s2_3)

-- ------------------------------------------
-- Omega parallel
-- ------------------------------------------

-- Announce
print("################# Omega parallel composition (incl attributes)");


-- Compose overall plant
omachines = faudes.System()
faudes.OmegaParallel(machine1,machine2,omachines)

-- Report
omachines:Write()
omachines:GraphWrite("tmp_omachines.svg")

-- Record test case
FAUDES_TEST_DUMP("omegaparallel incl. attr.",omachines)


-- ------------------------------------------
-- Std parallel, funny cases
-- ------------------------------------------

-- Announce
print("################# Std parallel composition (empty alphabets)");

-- Intentionally break machine1
sigma1=machine1:Alphabet();
machine1:DelEvents(sigma1);

-- Compose overall plant
machines = faudes.Generator()
faudes.Parallel(machine1,machine2,machines)

-- Report
machines:Write()

-- Record test case
FAUDES_TEST_DUMP("parallel empty alphabet a",machines)

-- Intentionally break machine1 even worse
machine1:ClearMarkedStates();

-- Compose overall plant
machines = faudes.Generator()
faudes.Parallel(machine1,machine2,machines)

-- Report
machines:Write()

-- Record test case
FAUDES_TEST_DUMP("parallel empty alphabet b",machines)

