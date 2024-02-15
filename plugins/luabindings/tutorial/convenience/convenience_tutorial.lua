-- Test/demonstrate conveninece function for std synthesis

-- ------------------------------------------
-- Prepare plant model: two simple machines
-- ------------------------------------------

-- Read original machine
machine  = faudes.System("data/verysimplemachine.gen")

-- Prepare two versions
machine1 = faudes.System()
machine2 = faudes.System()
faudes.Version(machine,"1",machine1)
faudes.Version(machine,"2",machine2)

-- Container of components
plantvect = faudes.GeneratorVector()
plantvect:Append(machine1)
plantvect:Append(machine2)

-- ------------------------------------------
-- Prepare specification
-- ------------------------------------------

-- Load specification
spec = faudes.Generator("data/buffer.gen")

-- Container of components
specvect = faudes.GeneratorVector()
specvect:Append(spec)

-- ------------------------------------------
-- Run synthesis procedure
-- ------------------------------------------

super=faudes.System()
faudes.StdSynthesis(plantvect,specvect,super)
super:Name("supervisor")

-- ------------------------------------------
-- Report
-- ------------------------------------------

super:Write()

-- Report test case
print("Expected closed loop statistics: 6 states, 8 transitions");
super:SWrite()

-- Record test case
FAUDES_TEST_DUMP("supervisor",super)

-- ------------------------------------------
-- Test
-- ------------------------------------------
FAUDES_TEST_DIFF()
