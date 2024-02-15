-- Test: Demonstrate Supervisor Reduction
faudes.MakeGlobal()

-- First example for supervisor reduction
plant1 = System("data/plant1.gen")
plant1:Write("tmp_syn_3_plant1.gen")
sup1 = System("data/sup1.gen")
sup1:Write("tmp_syn_3_sup1.gen")
reduced1 = System()
success = SupReduce(plant1,sup1,reduced1)
reduced1:Write("tmp_syn_3_reduced1.gen")

-- Second example for supervisor reduction
plant2 = System("data/plant2.gen")
plant2:Write("tmp_syn_3_plant2.gen")
sup2 = System("data/sup2.gen")
sup2:Write("tmp_syn_3_sup2.gen")
reduced2 = System()
success = SupReduce(plant2,sup2,reduced2)
reduced2:Write("tmp_syn_3_reduced2.gen")

-- Record test case
FAUDES_TEST_DUMP("red1",reduced1)
FAUDES_TEST_DUMP("red2",reduced2)
FAUDES_TEST_DIFF()
