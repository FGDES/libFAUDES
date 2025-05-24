-- Test/demonstrate basic synthesis

-- ------------------------------------------
-- Prepare plant model: two simple machines
-- ------------------------------------------

-- Read original machine
machine  = faudes.System("data/verysimplemachine.gen")

-- Report
machine:Write()

-- Prepare two copies
machine1 = faudes.System()
machine2 = faudes.System()
faudes.Version(machine,"1",machine1)
faudes.Version(machine,"2",machine2)

-- Report
machine1:Write()

-- Compose overall plant
plant = faudes.System()
faudes.Parallel(machine1,machine2,plant)

-- Report
plant:Write()

-- ------------------------------------------
-- Prepare specifications
-- ------------------------------------------

-- Load specification
buffer  = faudes.Generator("data/buffer.gen")

-- Selfloop (alternative: use inv. projection)
spec  = faudes.Generator(buffer)
extra = faudes.EventSet()
faudes.AlphabetDifference(plant:Alphabet(),spec:Alphabet(),extra)
faudes.SelfLoop(spec,extra)
spec:Write()

-- ------------------------------------------
-- Test controllability/relatively marking of specification
-- ------------------------------------------

contr = faudes.IsControllable(plant,plant:ControllableEvents(),spec)
if contr then
  print("Controllability of specification: ok (test case error!)")
else
  print("Controllability of specification: failed (expected!)")
end

relmark = faudes.IsRelativelyMarked(plant,spec);
if relmark then
  print("Specification is relatively marked: ok (test case error!)")
else
  print("Specification is relatively marked: failed (expected!)")
end

clspec = faudes.Generator(spec);
faudes.PrefixClosure(clspec);
clrelmark = faudes.IsRelativelyMarked(plant,clspec);
if clrelmark then
  print("Closure of specification is relatively marked: ok (expected!)")
else
  print("Closure of specification is relatively marked: failed (test case error!)")
end

-- Record test case
FAUDES_TEST_DUMP("spec controllable",contr)
FAUDES_TEST_DUMP("spec relmarked",relmark)
FAUDES_TEST_DUMP("clspec relmarked",clrelmark)


-- ------------------------------------------
-- Do synthesis
-- ------------------------------------------

-- Run synthesis on closed languages
super_cl = faudes.System()
faudes.SupConClosed(plant,spec,super_cl)
super_cl:Name("supervisor")

-- Run synthesis incl marking
super = faudes.System()
faudes.SupCon(plant,spec,super)
super:Name("supervisor")

-- Record test case
FAUDES_TEST_DUMP("super cl",super_cl)
FAUDES_TEST_DUMP("super",super)

-- ------------------------------------------
-- Test controllability of supervisor
-- ------------------------------------------

contr = faudes.IsControllable(plant,plant:ControllableEvents(),super)
if contr then
  print("Controllability of supervisor: ok (expected!)")
else
  print("Controllability of supervisor: failed (test case error)")
end

relclosed = faudes.IsRelativelyPrefixClosed(plant,super);
if relclosed then
  print("Supervisor is relatively closed: ok (test case error!)")
else
  print("Supervisor is relatively closed: failed (expected!)")
end

-- Record test case
FAUDES_TEST_DUMP("super controllable",contr)
FAUDES_TEST_DUMP("super relclosed",relclosed)


-- ------------------------------------------
-- Report
-- ------------------------------------------

-- Report result
plant:Write()
spec:Write()
super:Write()

-- Report test case
print("Expected closed loop statistics: 6 states, 8 transitions");
super:SWrite()

-- Record test case
FAUDES_TEST_DUMP("supervisor",super)


-- ------------------------------------------
-- Normality
-- ------------------------------------------

-- Have closed behaviours
clplant = faudes.System(plant)
faudes.PrefixClosure(clplant)
clsuper = faudes.Generator(super)
faudes.PrefixClosure(clsuper)

-- Test, whether our supervisor is normal if beta_1 is not observable
clplant:ClrObservable("beta_1");
print("A"); ok = faudes.IsNormal(clplant,clsuper) ; print("B");

if ok then
  print("Normality when beta_1 is not observable: ok (test case error!)")
else
  print("Normality when beta_1 is not observable: failed (expected)")
end

-- compute supremal normal sublanguage of specification for prefix closed case
spec_o = faudes.Generator()
faudes.LanguageIntersection(spec,clplant,spec_o)
faudes.SupNorm(clplant,spec_o,spec_o)

-- compute closed loop under restricted observation (for html docu)
super_o = faudes.Generator()
sigma_o = faudes.EventSet(clplant:ObservableEvents())
faudes.Project(clsuper, sigma_o, super_o)
clloop1 = faudes.Generator(super_o)
clloop1:StateNamesEnabled(false)
faudes.InvProject(clloop1, plant:Alphabet())
faudes.Parallel(clplant,clloop1,clloop1)

-- Report test case
print("Expected closed loop statistics: 8 states, 12 transitions");
clloop1:SWrite()

-- Record test case
FAUDES_TEST_DUMP("sup normal",spec_o)
FAUDES_TEST_DUMP("normal 1",ok)
FAUDES_TEST_DUMP("closed loop 1",clloop1)

-- Test, whether our supervisor is normal if beta_2 is not observable
clplant:SetObservable("beta_1");
clplant:ClrObservable("beta_2");
ok = faudes.IsNormal(clplant,clsuper)

-- Report
if ok then
  print("Normality when beta_2 is not observable: ok (expected)")
else
  print("Normality when beta_2 is not observable: failed (test case error!)")
end

-- Compute closed loop under restricted observation (for html docu)
super_o = faudes.Generator()
sigma_o = faudes.EventSet(clplant:ObservableEvents())
faudes.Project(clsuper, sigma_o, super_o)
clloop2 = faudes.Generator(super_o)
faudes.InvProject(clloop2, plant:Alphabet())
faudes.Parallel(clplant,clloop2,clloop2)
clloop2:StateNamesEnabled(false)

-- Doublecheck: does closed loop match original supervisor?
ok2=faudes.LanguageEquality(clloop2,clsuper)

-- Report
if ok2 then
  print("Closed loop matches original supervisor: ok (expected)");
else
  print("Closed loop matches original supervisor: failed (test case error)");
end

-- Record test case
FAUDES_TEST_DUMP("normal 2",ok)
FAUDES_TEST_DUMP("closed loop 2",clloop2)
FAUDES_TEST_DUMP("doublcheck 2",ok2)


-- ------------------------------------------
-- Supervision under partial observation
-- ------------------------------------------

-- Prepare plant and specification (std buffer, but closed)
plant_po = plant:Copy()
plant_po:ClrObservable("beta_1")
spec_po = buffer:Copy()
faudes.PrefixClosure(spec_po)
faudes.InvProject(spec_po,plant:Alphabet())

-- Synthesise controllable, normal and closed supervisor
super_poc = faudes.Generator()
faudes.SupConNormClosed(plant_po,spec_po,super_poc)

-- Synthesise controllable and normal supervisor 
super_po = faudes.Generator()
faudes.SupConNorm(plant_po,spec_po,super_po)

-- Load/prepare specification (2-place buffer)
buffer2  = faudes.Generator("data/buffer2.gen")
spec_po2 = buffer2:Copy()
faudes.InvProject(spec_po2,plant:Alphabet())

-- Synthesise controllable and normal supervisor
super_po2 = faudes.Generator()
faudes.SupConNorm(plant_po,spec_po2,super_po2)

-- Record test case
FAUDES_TEST_DUMP("supervisor partial obs, closed",super_poc)
FAUDES_TEST_DUMP("supervisor partial obs, nb",super_po)
FAUDES_TEST_DUMP("supervisor partial obs, nb buffer2",super_po2)

-- ------------------------------------------
-- Prepare output for HTML docu
-- ------------------------------------------

-- copy output used for html docu
print("Writing results to file")
machine1:Write("tmp_syn_m1.gen")
machine2:Write("tmp_syn_m2.gen")
buffer:Write("tmp_syn_buffer.gen")
buffer2:Write("tmp_syn_buffer2.gen")
plant:Write("tmp_syn_plant.gen")
spec:Write("tmp_syn_spec.gen")
--spec_o:Write("tmp_syn_speco.gen")
super:Write("tmp_syn_super.gen")
--super_cl:Write("tmp_syn_supercl.gen")
super_po:Write("tmp_syn_superpo.gen")
super_po2:Write("tmp_syn_superpo2.gen")
super_poc:Write("tmp_syn_superpoc.gen")
clsuper:Write("tmp_syn_clsuper.gen")
clplant:Write("tmp_syn_clplant.gen")
clloop1:Write("tmp_syn_clloop1.gen")
clloop2:Write("tmp_syn_clloop2.gen")

-- for docu only
product = faudes.Generator();
faudes.Product(plant,spec,product);
product:Write("tmp_syn_product.gen")

