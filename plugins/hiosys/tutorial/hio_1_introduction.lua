-- hio_1_introduction:lua

-- This tutorial uses a very simple example to exhibit
-- monolithic input-/output based controller synthesis.
-- The purpose is to generate graphs for the
-- introduction of hiosys in the libFAUDES user
-- reference, see 
-- <a href="../reference/hiosys_index.html">Hiosys</a>.
-- For other purposes such as learning how to use hiosys,
-- we refer to the tutorials hio_2_hiogenerators,
-- hio_3_hiofunctions and hio_4_transport_unit.

print('hiosys: lua tutorial 1 - introduction, simple machine')

 --*************************************************
 -- * PLANT MODEL WITH INPUTS AND OUTPUTS
 -- ***************************************************/

  --*****************
  --* plant dynamics of machines 1 and 2
  --* note: only the plant-I/O port (Up,Yp)  of HioPlant is used in this
  --*         simple non-hierarchical example. The remaining port (Ue,Ye)
  --*         is introduced as dummy.
  --********************/
  
  machine1=faudes.HioPlant()
  machine2=faudes.HioPlant()
  tempgen=faudes.HioPlant("data/1_introduction/hio_simplemachine_intro.gen")
  faudes.Version(tempgen,"1",machine1)
  faudes.Version(tempgen,"2",machine2)
  machine1:Write("tmp_hio_machine1_intro.gen")
  machine1:GraphWrite("tmp_hio_machine1_intro.png")
  machine2:Write("tmp_hio_machine2_intro.gen")
  machine2:GraphWrite("tmp_hio_machine2_intro.png")

  -- plant inputs up and outputs yp
  up1=machine1:UpEvents()
  up2=machine2:UpEvents()
  yp1=machine1:YpEvents()
  yp2=machine2:YpEvents()

  --------------------
  -- IOSHUFFLE
  --------------------  
  
  -- note: only the plant-I/O port (Up,Yp)  of HioPlant is used in this
  --         simple non-hierarchical example: The remaining port (Ue,Ye)
  --         is introduced as dummy:
  plant=faudes.HioPlant()
  faudes.HioShuffle(machine1,machine2,plant)
  
  -- ignore marking
  faudes.PrefixClosure(plant)
  -- cosmetics
  faudes.StateMin(plant,plant)
  plant:StateNamesEnabled(false)
  --remove dummy events for file output
 -- plant:Copy(tempgen) ???
  faudes.Project(plant,yp1+yp2+up1+up2,tempgen)
  tempgen:SetYp(yp1+yp2)
  tempgen:SetUp(up1+up2)
  tempgen:Name("HioShuffle(Simple Machine_1,Simple Machine_2)")
  tempgen:Write("tmp_hio_ioshuffle_intro.gen")
  tempgen:GraphWrite("tmp_hio_ioshuffle_intro.png")
  -- constraint to avoid err-states of ioshuffle
  ioconstr=faudes.HioConstraint("data/1_introduction/hio_ioconstraint_intro.gen")

  --------------------
  -- SPECIFICATION
  -- (poses a constraint, in the non-hierarchical case)
  --------------------  
  
  spec=faudes.HioConstraint("data/1_introduction/hio_specification_intro.gen")
  spec:Write("tmp_hio_specification_intro.gen")
  spec:GraphWrite("tmp_hio_specification_intro.png")
  -- compose constraint and specification to compound requirement to be met by controller
  faudes.Parallel(spec,ioconstr,spec)  
  -- in the hierarchical case, the actual specification is formulated over additional alphabets:
  -- here, this spec and according alphabets and constraint are introduced as dummies

  dummy_constraint=faudes.HioConstraint()
  ds1=dummy_constraint:InsInitState()
  dummy_constraint:SetMarkedState(ds1)
  dummy_yc=faudes.EventSet()
  dummy_uc=faudes.EventSet()
  dummy_yc:Insert("yc")
  dummy_uc:Insert("uc")

  dummy_spec=faudes.HioPlant()
  ds1=dummy_spec:InsInitState()
  dummy_spec:SetMarkedState(ds1)
  dummy_spec:InsEvents(plant:EEvents())
  dummy_spec:SetYe(plant:YeEvents())
  dummy_spec:SetUe(plant:UeEvents())
  dummy_spec:InsEvents(dummy_yc)
  dummy_spec:SetYp(dummy_yc)
  dummy_spec:InsEvents(dummy_uc)
  dummy_spec:SetUp(dummy_uc)
  dsp=dummy_spec:InsMarkedState()
  dse=dummy_spec:InsMarkedState()
  eit=dummy_spec:AlphabetBegin()
  while eit~=dummy_spec:AlphabetEnd() do
    if dummy_spec:IsYp(eit:Index()) then
       dummy_spec:SetTransition(ds1,eit:Index(),dsp)
    end
    if dummy_spec:IsUp(eit:Index()) then
       dummy_spec:SetTransition(dsp,eit:Index(),ds1)
    end
    if dummy_spec:IsYe(eit:Index()) then
       dummy_spec:SetTransition(ds1,eit:Index(),dse)
    end
    if dummy_spec:IsUe(eit:Index()) then
       dummy_spec:SetTransition(dse,eit:Index(),ds1)
    end
    eit:Inc()
  end 
  
  --------------------
  -- SYNTHESIS
  --------------------    
  
  -- note: only the controller-I/O port (Up,Yp)  of HioController is used in this
  --         simple non-hierarchical example: The remaining port (Uc,Yc)
  --         is introduced as dummy:
  controller=faudes.HioController()
  -- faudes.HioSynthUnchecked( .... ) ???

  faudes.HioSynth(plant, dummy_spec, dummy_constraint, spec, dummy_yc, dummy_uc, 
    yp1+yp2, up1+up2,plant:YeEvents(),plant:UeEvents(),controller)
  
  -- remove dummies yc,uc,ye,ue
  faudes.Project(controller,yp1+up1+yp2+up2,controller)
  -- restore event attributes
  controller:SetYp(yp1+yp2)
  controller:SetUp(up1+up2)
  controller:Write("tmp_hio_controller_intro.gen")

  -- Report to console
  print("io controller, statistics") 
  controller:SWrite()
  
    --------------------
    -- CLOSED LOOP
    --------------------      
  closedloop=faudes.Generator()
  faudes.Parallel(plant,controller,closedloop)
  -- remove dummy events
  faudes.Project(closedloop,yp1+up1+yp2+up2,closedloop)
  closedloop:Write("tmp_hio_closedloop_intro.gen")
  
    --------------------
    -- COMPARE WITH SCT
    --------------------   
  
  -- remove nonphysical io events
  supervisor=faudes.Generator()
  alphabeta=faudes.EventSet()
  alphabeta:Insert("alpha_1")
  alphabeta:Insert("alpha_2")
  alphabeta:Insert("beta_1")
  alphabeta:Insert("beta_2")
  faudes.Project(closedloop,alphabeta,supervisor)
  supervisor:Write("tmp_hio_physical_intro.gen")

  -- Report to console
  --   ->  should lead to same supervisor as in Synthesis PlugIn example !
  print("projected supervisor, statistic") 
  supervisor:SWrite()
