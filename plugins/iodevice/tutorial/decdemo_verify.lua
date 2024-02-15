-- decdemo_verify.lua
--
-- This Lua-script is part of the example "decdemo". It verifies
-- controllability conditions of the decentralized control demo.


-------------------------------------------------------------------------------
-- Read files
-------------------------------------------------------------------------------

planta = faudes.System("data/decdemo_planta.gen")
plantb = faudes.System("data/decdemo_plantb.gen")
contra = faudes.Generator("data/decdemo_contra.gen")
contrb = faudes.Generator("data/decdemo_contrb.gen")

-- Set up monolithic models
plant=faudes.System()
faudes.Parallel(planta,plantb,plant)
contr=faudes.Generator()
faudes.Parallel(contra,contrb,contr)
cloop=faudes.Generator()
faudes.Parallel(plant,contr,cloop)
cloop:Name("Closed loop (monolithic)")
cloop:ClearStateNames()

-- Set up alphabets
falph=faudes.Alphabet()
faudes.AlphabetUnion(plant:Alphabet(),contr:Alphabet(),falph);
halph = faudes.Alphabet()
halph:FromString("<A> alpha beta mue lambda</A>")
lalph = faudes.Alphabet()
faudes.AlphabetDifference(falph,halph,lalph);

-- Project to highlevel alphabet
hcloop=faudes.Generator()
faudes.Project(cloop,halph,hcloop)

-- Lift component models to full alphabet
fplant=faudes.System()
faudes.InvProject(plant,falph,fplant)
fcontra=faudes.Generator()
faudes.InvProject(contra,falph,fcontra)
fcontrb=faudes.Generator()
faudes.InvProject(contrb,falph,fcontrb)

-- Report
print("##########  closed-loop bahvaiour")
cloop:Write()
print("##########  closed-loop bahvaiour, high level")
hcloop:Write()


-- Test controllability ...
print("##########  controllability conditions")

-- Prepare variables
siguc=faudes.EventSet()
sigc=faudes.EventSet()

-- Test controllability w.r.t actual plant
siguc:FromString("<A> p_inprog p_complete m_request m_complete</A>")
faudes.AlphabetDifference(falph,siguc,sigc)
isc = faudes.IsControllable(fplant,sigc,cloop)
if isc then
  print("Controllability w.r.t. plant: passed (EXPECTED)")
else
  print("Controllability w.r.t. plant: failes (TEST CASE ERROT)")
end

-- Test controllability w.r.t contra
siguc:FromString("<A> p_start p_stop</A>")
faudes.AlphabetDifference(falph,siguc,sigc)
isc = faudes.IsControllable(fcontra,sigc,cloop)
if isc then
  print("Controllability w.r.t. contra: passed (EXPECTED)")
else
  print("Controllability w.r.t. contra: failes (TEST CASE ERROT)")
end


-- Test controllability w.r.t contrb
siguc:FromString("<A> m_start m_stop lambda</A>")
faudes.AlphabetDifference(falph,siguc,sigc)
isc = faudes.IsControllable(fcontrb,sigc,cloop)
if isc then
  print("Controllability w.r.t. contrb: passed (EXPECTED)")
else
  print("Controllability w.r.t. contrb: failes (TEST CASE ERROT)")
end


-- Images to inspect
planta:GraphWrite("tmp_decdemo_planta.png")
plantb:GraphWrite("tmp_decdemo_plantb.png")
cloop:GraphWrite("tmp_decdemo_cloop.png")
hcloop:GraphWrite("tmp_decdemo_hcloop.png")

-- Copy for html docu
cloop:ClearStateNames()
contra:ClearStateNames()
contrb:ClearStateNames()
planta:Write("tmp_decdemo_planta.gen")
plantb:Write("tmp_decdemo_plantb.gen")
contra:Write("tmp_decdemo_contra.gen")
contrb:Write("tmp_decdemo_contrb.gen")
cloop:Write("tmp_decdemo_cloop.gen")
hcloop:Write("tmp_decdemo_hcloop.gen")
