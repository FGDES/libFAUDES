-- Tutorial to illustrate the LuaExtension "ExtIntro"

-- Read input data
l1gen = faudes.Generator("data/gen_lg.gen")

-- Apply test and report
l1=faudes.IsLargeGenerator(l1gen);
if l1 then
  print("l1gen is a large generator [expected]")
else
  print("l1gen is not a large generator [test case error]")
end

-- Record test case
FAUDES_TEST_DUMP("lgen test 1",l1)

-- Constrcut a small generator
l2gen = faudes.Generator()
faudes.LargeGenerator(5,3,l2gen)

-- Apply test and report
l2=faudes.IsLargeGenerator(l2gen);
if l2 then
  print("l2gen is a large generator [test case error]")
else
  print("l2gen is not a large generator [expected]")
end

-- Constrcut a large generator
l3gen = faudes.Generator()
faudes.LargeGenerator(12,5,l3gen)

-- Produce graphical output for documentation
-- (plain .png image)
l3gen:GraphWrite("tmp_extintro_l3gen.png")

-- Produce graphical output for documentation
-- (trigger flsinstall to create extra reference page with *.svg)
l3gen:Write("tmp_extintro_l3gen.gen")
l3gen:GraphWrite("tmp_extintro_l3gen.svg")

-- Apply test and report
l3=faudes.IsLargeGenerator(l3gen);
if l3 then
  print("l3gen is a large generator [expected]")
else
  print("l3gen is not a large generator [test case error]")
end

-- Record test case
FAUDES_TEST_DUMP("lgen test 3",l3)

-- Trigger validation
FAUDES_TEST_DIFF()