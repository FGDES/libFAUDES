-- Test cases for IsStdSynthesisConsistent

-- ------------------------------------------
-- test case A
-- ------------------------------------------

-- read plant incl attributes
plant  = faudes.System("data/syn_5_planta.gen")

-- prepare abstraction
sig_o  = plant:ObservableEvents()
plant_o = faudes.Generator()
faudes.Project(plant,sig_o,plant_o)

-- perform test
ssc = faudes.IsStdSynthesisConsistent(plant,plant_o)

-- report
if ssc then
  print("consistency case A: passed [expected]")
else
  print("consistency case A: failed [ERROR]")
end

-- graphical output for documentation
plant:Write("tmp_syn_5_planta.gen")
plant:GraphWrite("tmp_syn_5_planta.svg")
plant_o:Write("tmp_syn_5_planta0.gen")
plant_o:GraphWrite("tmp_syn_5_planta0.svg")

-- record testcase
FAUDES_TEST_DUMP("std consistency A",ssc)



-- ------------------------------------------
-- test case B
-- ------------------------------------------

-- read plant incl attributes
plant  = faudes.System("data/syn_5_plantb.gen")

-- prepare abstraction
sig_o  = plant:ObservableEvents()
plant_o = faudes.Generator()
faudes.Project(plant,sig_o,plant_o)

-- perform test
ssc = faudes.IsStdSynthesisConsistent(plant,plant_o)

-- report
if ssc then
  print("consistency case B: passed [ERROR]")
else
  print("consistency case B: failed [expected]")
end

-- graphical output for documentation
plant:Write("tmp_syn_5_plantb.gen")
plant:GraphWrite("tmp_syn_5_plantb.svg")
plant_o:Write("tmp_syn_5_plantb0.gen")
plant_o:GraphWrite("tmp_syn_5_plantb0.svg")

-- record
FAUDES_TEST_DUMP("std consistency B",ssc)
