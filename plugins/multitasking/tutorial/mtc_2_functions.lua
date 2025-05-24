-- Test/demonstrate functions on MtcSystems

-- ------------------------------------------
-- MtcSystem: reachability
-- ------------------------------------------

-- Announce
print("################# reachability")

-- Read example
sys = faudes.MtcSystem("data/mtc_functions_1a_not_trim.gen")

-- Test reachability
acc=faudes.IsAccessible(sys);
scoacc=faudes.IsStronglyCoaccessible(sys);
strim=faudes.IsStronglyTrim(sys);

-- Report
if acc then 
  print("accessible:            PASS [test err?]")
else
  print("accessible:            FAIL [expected]")
end
if scoacc then 
  print("strongly coaccessible: PASS [test err?]")
else
  print("strongly coaccessible: FAIL [expected]")
end
if strim then 
  print("strongly trim:         PASS [test err?]")
else
  print("strongly trim:         FAIL [expected]")
end

-- Record test case
FAUDES_TEST_DUMP("accessible", acc);
FAUDES_TEST_DUMP("str. coaccessible", scoacc);
FAUDES_TEST_DUMP("str. trim", strim);

-- Convert to accessible MtcSystem (by removing unaccessible states)
accsys = sys:Copy();
faudes.Accessible(accsys); 
accsys:Write("tmp_mtc_functions_1b_acc.gen")

-- Test and report
acc=faudes.IsAccessible(accsys);
if acc then 
  print("accessible:            PASS [expected]")
else
  print("accessible:            FAIL [test err?]")
end

-- Convert to strongly coaccessible MtcSystem (here: implies str. trim)
trimsys = accsys:Copy();
faudes.StronglyCoaccessible(trimsys); 
trimsys:Write("tmp_mtc_functions_1c_str_trim.gen")

-- Test and report
strim=faudes.IsStronglyTrim(trimsys);
if acc then 
  print("strongly trim:         PASS [expected]")
else
  print("strongly trim:         FAIL [test err?]")
end

-- Record test case
FAUDES_TEST_DUMP("fix trim sys", trimsys);
FAUDES_TEST_DUMP("fix trim test", strim);


-- ------------------------------------------
-- MtcSystem: determinsim
-- ------------------------------------------

-- Announce
print("################# determinism")

-- Read example
sys = faudes.MtcSystem("data/mtc_functions_2a_nondet.gen")

-- Test determinism
det=faudes.IsDeterministic(sys);
if det then 
  print("determinism:           PASS [test err?]")
else
  print("determinism:           FAIL [expected]")
end

-- Convert to deterministic MtcSystem
detsys = faudes.MtcSystem()
faudes.MtcDeterministic(sys,detsys); 
detsys:Write("tmp_mtc_functions_2b_det.gen")

-- Test determinism
det=faudes.IsDeterministic(detsys);
if det then 
  print("determinism:           PASS [expected]")
else
  print("determinism:           FAIL [test err?]")
end

-- Record test case
FAUDES_TEST_DUMP("determinism",detsys)


-- ------------------------------------------
-- MtcSystem: projection
-- ------------------------------------------

-- Announce
print("################# projection")

-- Read example
sys = faudes.MtcSystem("data/mtc_functions_3a_system.gen")
alph = faudes.EventSet()
alph:FromString("<A> a b c </A>")

-- Project
prosys = faudes.MtcSystem()
faudes.MtcProject(sys,alph,prosys)

-- Project (non det)
pndsys = faudes.MtcSystem()
faudes.MtcProjectNonDet(sys,alph,pndsys)

-- Write result
prosys:Write("tmp_mtc_functions_3b_projected.gen")
pndsys:Write("tmp_mtc_functions_3c_projected_nondet.gen")

-- Report
print("states:", pndsys:Size(), "[expected 5]")
print("states:", prosys:Size(), "[expected 4]")

-- Record test case
FAUDES_TEST_DUMP("projection",prosys)

-- ------------------------------------------
-- MtcSystem: parallel composition
-- ------------------------------------------

-- Announce
print("################# parallel")

-- Read example
sysa = faudes.MtcSystem("data/mtc_functions_4a_system.gen")
sysb = faudes.MtcSystem("data/mtc_functions_4b_system.gen")
 
-- Compose
compsys = faudes.MtcSystem()
faudes.MtcParallel(sysa,sysb,compsys)

-- Write result
compsys:Write("tmp_mtc_functions_4c_parallel.gen")

-- Report
print("states:", compsys:Size(), "[expected 4]")

-- Record test case
FAUDES_TEST_DUMP("parallel",compsys)


-- ------------------------------------------
-- MtcSystem: synthesis
-- ------------------------------------------

-- Announce
print("################# synthesis")

-- Read example
plant = faudes.MtcSystem("data/mtc_functions_5_plant.gen")
spec = faudes.MtcSystem("data/mtc_functions_5_spec.gen")

-- Run synthesis
super=faudes.MtcSystem()
supernb=faudes.MtcSystem()
faudes.InvProject(spec,plant:Alphabet())
faudes.MtcSupConClosed(plant,spec,super)
faudes.MtcSupCon(plant,spec,supernb)

--  Write result
super:Write("tmp_mtc_functions_5_super.gen")
supernb:Write("tmp_mtc_functions_5_supernb.gen")

-- Report
print("states:", super:Size(), "[expected 5]")
print("states:", supernb:Size(), "[expected 4]")

-- Record test case
FAUDES_TEST_DUMP("supcon",super)
FAUDES_TEST_DUMP("supconnb",supernb)

-- Goodbye
print("################# done")
