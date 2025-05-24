-- Mtc observer application example

-- ------------------------------------------
-- MtcSystem: reachability
-- ------------------------------------------

-- Read plant data
print("################# read plant data")
cb4_0=faudes.MtcSystem("data/cb4[0].gen")
mh1d1_0_sup=faudes.MtcSystem("data/mh1d1[0]_sup.gen")

-- Read specification
print("################# read specifications")
cb4_0_spec1=faudes.MtcSystem("data/cb4[0]_spec1.gen")
cb4_0_spec2=faudes.MtcSystem("data/cb4[0]_spec2.gen")
cb4_1_spec=faudes.MtcSystem("data/cb4[1]_spec.gen")
mh1d1_1_spec=faudes.MtcSystem("data/mh1d1[1]_spec.gen")

-- Fix alphabet by selfloop
faudes.MtcInvProject(cb4_0_spec1, cb4_0:Alphabet())


-- Supervisor Synthesis, prepare
print("################# cb4_0: compose specification")
cb4_0_spec=faudes.MtcSystem()
faudes.MtcParallel(cb4_0_spec1, cb4_0_spec2, cb4_0_spec)

-- Supervisor Synthesis
print("################# cb4_0: synthesis")
cb4_0_sup=faudes.MtcSystem()
faudes.MtcSupCon(cb4_0, cb4_0_spec, cb4_0_sup)

-- Report
print("################# write graphs")
cb4_0_spec:Write("tmp_cb4[0]_spec.gen")
cb4_0_spec:GraphWrite("tmp_cb4[0]_spec.png")
cb4_0_sup:Write("tmp_cb4[0]_sup.gen")
cb4_0_sup:GraphWrite("tmp_cb4[0]_sup.png")

-- Record test case
FAUDES_TEST_DUMP("cb4[0]_spec",cb4_0_spec)
FAUDES_TEST_DUMP("cb4[0]_sup",cb4_0_sup)

-- ----------------------------------------------
-- Level 1
-- -----------------------------------------------

print("################# hierarhical, level 1")

-- Prepare
cb4_1_alph=faudes.EventSet();
cb4_1_alph:FromString('<A> "cb11-cb4" "cb12-cb4" "cb4-cb11" "cb4-cb12" "cb4stp" </A>')

-- Abstract
if faudes.IsMtcObserver(cb4_0_sup, cb4_1_alph) then
  print("################# cb4_0_sup: observer condition fulfilled, compute projection")
  cb4_1=faudes.MtcSystem()
  faudes.MtcProject(cb4_0_sup, cb4_1_alph, cb4_1)
else
  print("################# cb4_0_sup: observer condition not fulfilled [test err?]")
end

-- Report result
cb4_1:StateNamesEnabled(false);
cb4_1:Write("tmp_cb4[1].gen");
cb4_1:GraphWrite("tmp_cb4[1].png");

-- Prepare
mh1d1_1_alph=faudes.EventSet()
mh1d1_1_alph:FromString('<A> "mh1start" "mh1end" </A>')

-- Abstract
if faudes.IsMtcObserver(mh1d1_0_sup, mh1d1_1_alph) then
  print("################# mh1d1_0_sup: observer condition fulfilled, compute projection")
  mh1d1_1=faudes.MtcSystem()
  faudes.MtcProject(mh1d1_0_sup, mh1d1_1_alph, mh1d1_1)
else
  print("################# mh1d1_0_sup: observer condition not fulfilled [test err?]")
end

-- Report result
mh1d1_1:StateNamesEnabled(false);
mh1d1_1:Write("tmp_mh1d1[1].gen");
mh1d1_1:GraphWrite("tmp_mh1d1[1].png");

-- Compose/synthesis
print("################# mh1d1_1/cb4_1: compose/synthesis")
cb4mh1d1_1=faudes.MtcSystem()
cb4mh1d1_1_sup=faudes.MtcSystem()
faudes.MtcParallel(cb4_1, mh1d1_1, cb4mh1d1_1);
faudes.MtcInvProject(mh1d1_1_spec, cb4mh1d1_1:Alphabet());
faudes.MtcSupCon(cb4mh1d1_1, mh1d1_1_spec, cb4mh1d1_1_sup);

-- Report
print("################# write graphs")
cb4mh1d1_1:Write("tmp_cb4mh1d1_1.gen");
cb4mh1d1_1:GraphWrite("tmp_cb4mh1d1_1.png");
cb4mh1d1_1_sup:Write("tmp_cb4mh1d1_1_sup.gen");
cb4mh1d1_1_sup:GraphWrite("tmp_cb4mh1d1_1_sup.png");

-- Record test case
FAUDES_TEST_DUMP("cb4mh1d1_1",cb4mh1d1_1)
FAUDES_TEST_DUMP("cb4mh1d1_1_sup",cb4mh1d1_1_sup)

-- ----------------------------------------------
-- Level 2
-- -----------------------------------------------

print("################# hierarhical, level 2")

-- Prepare
cb4mh1d1_2_alph=faudes.EventSet();
cb4mh1d1_2_alph:FromString('<A> "cb11-cb4" "cb12-cb4" "cb4-cb11" "cb4-cb12" "mh1end" </A>')

-- Abstract
cb4mh1d1_2=faudes.MtcSystem()
faudes.MtcProject(cb4mh1d1_1_sup, cb4mh1d1_2_alph, cb4mh1d1_2)

-- Report
print("################# write graphs")
cb4mh1d1_2:StateNamesEnabled(false)
cb4mh1d1_2:Write("tmp_cb4mh1d1_2_orig.gen")
cb4mh1d1_2:GraphWrite("tmp_cb4mh1d1_2_orig.png")
  

-- Record test case
FAUDES_TEST_DUMP("cb4mh1d1_2",cb4mh1d1_1)



-- ----------------------------------------------
-- Optimal colors
-- -----------------------------------------------

print("################# optimal colors")


-- Prepare
cb4mh1d1_2_alph=faudes.EventSet();
cb4mh1d1_2_alph:FromString('<A> "cb11-cb4" "cb12-cb4" "cb4-cb11" "cb4-cb12" </A>')

-- Get optimal set
optcol=faudes.ColorSet()
faudes.OptimalColorSet(cb4mh1d1_1_sup, optcol, cb4mh1d1_2_alph);

-- Report
print("optimal colors:", optcol)

-- Remove others    
allcol=cb4mh1d1_1_sup:Colors()
cit=allcol:Begin()
while cit~=allcol:End() do
  if not optcol:Exists(cit:Index()) then
    print("removing color:",allcol:SymbolicName(cit))
    cb4mh1d1_1_sup:DelColor(cit:Index());
  end
  -- Increment
  cit:Inc()
end 


-- Abstract
if faudes.IsMtcObserver(cb4mh1d1_1_sup, cb4mh1d1_2_alph) then
  print("################# cb4mh1d1_1_sup: observer condition fulfilled, compute projection")
  cb4mh1d1_2=faudes.MtcSystem()
  faudes.MtcProject(cb4mh1d1_1_sup, cb4mh1d1_2_alph, cb4mh1d1_2)
else
  print("################# cb4mh1d1_1_sup: observer condition not fulfilled [test err?]")
end

-- Report
print("################# write graphs")
cb4mh1d1_2:StateNamesEnabled(false)
cb4mh1d1_2:Write("tmp_cb4mh1d1_2_optcol.gen")
cb4mh1d1_2:GraphWrite("tmp_cb4mh1d1_2_optcol.png")
  

-- Record test case
FAUDES_TEST_DUMP("cb4mh1d1_2",cb4mh1d1_1)

-- Goodbye
print("################# done")
