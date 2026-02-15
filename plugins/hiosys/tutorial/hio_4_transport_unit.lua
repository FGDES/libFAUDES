--  hio_4_transport_unit

-- Tutorial, transport chain example for  hiosys plugin:
-- Demonstrate hierarchical I/O controller synthesis
print('hiosys: lua tutorial 4 - transport unit chain example')
--
-- This tutorial treats controller synthesis for a chain of an exemplary number of 8 transport units
-- (TU's) that shall transport a workpiece from the left to the right.
-- The hierarchy can be extended to an arbitrary number of TU's, with
-- linear growth of complexity, measured in the sum of states of all involved generators.
-- By allowing for only one workpiece at a time, the example is kept at a simple tutorial level
-- such that the involved generators remain readable in a graphical view.
-- However, linear complexity is still achieved in the case of parallel transport of as
-- many workpieces as physically possible, see the real-world conveyor belt chain example.

-- The TU's are numbered
-- alphabetically from left to right. For each TU, an I/O plant model has to be provided as HioPlant.
-- Note that these plant model components, e.g. plantA and plantB, must not share events the
-- membership of each event to the respective component is indicated by the suffixes _A and
-- _B in the event labels, e.g. idle_A and idle_B. First, each TU is provided with a local controller
-- using the synthesis function HioSynthMonolithic. For that, a specification has to be given as
-- HioPlant describing the desired external behaviour of the locally controlled TU.

-- To design a control hierarchy,
-- we compound groups of two TU's, e.g. TU A and TU B. As the I/O based
-- approach allows for abstraction-based control, each locally controlled TU is abstracted by its
-- specification, so next the I/O shuffle of the specifications of two transport units is
-- computed. An environment model has to be provided as HioEnvironment that describes the
-- internal interaction of the two TU's within each other and the external interaction with the
-- remaining environment. With a given specification of the group's desired behaviour, the
-- function HioSynthHierarchical computes an I/O controller for the group. By specifying that
-- each group of TU's shall behave exactly as a single TU, we can derive all remaining controllers
-- of the hierarchy for 8 TU's by exploiting symmetry.
--

--======================================================================
  --====
  --==== controller synthesis for plantA:=====
  --====
--======================================================================

-- 
-- We consider a simple transport unit (TU), described as HioPlant
-- (data/4_transport_unit/4_transport_unit/plantA.gen).

-- The TU consists of a conveyor belt carrying a box that can hold the workpiece to be transported.
-- A spring sensor inside the box detects the absence or presence of a workpiece (empty, full).
-- The initial state (state 1) is defined such that the sensor reports empty. The operator
-- can choose between three different commands (state 2). After the no_op (no operation)
-- command, the TU does not move, and the system remains in the initial state. The command
-- del_tr (deliver to right) leads to an error state as there is currently no workpiece present to deliver.
-- Choosing the command take_fl (take from left) prompts the TU to move the box to its
-- left border (state 3). Now it depends on the environment if a workpiece is provided from the
-- left, which is modeled by the event req_fl unobservable to the operator. For a plant description
-- that is independent from the environment, we introduce the environment-events pack and nack
-- (positive/negative acknowledge) respecting that the environment may or may not comply with the
-- requests of the plant. If the environment is not in the condition to provide a workpiece (nack),
-- the request is repeated. When a workpiece is provided from the environment, the sensor reports
-- full. Now (state 6), the command take_fl leads to an error behaviour (the box can carry only
-- one workpiece), and after no_op the plant still reports full. By the command del_tr, the belt
-- moves the box to the right border. The event req_tr models the need for the workpiece to be withdrawn
-- to the right by the environment. In case of pack, the system returns to its initial state.

-- By  (UP,YP) := ({no_op, take_fl, del_tr}, {empty, full}), we identify the interaction with the operator,
-- (UE,YE) := ({pack,nack}, {req_fl, req_tr}) describes interaction with the environment.
-- Note that (UP,YP,UE,YE,LPE) features all I/O-plant properties posed in the formal Definition.
-- 

  print( '==================== reading files...') 

--read plant, spec and constraints:

  plantA=faudes.HioPlant('data/4_transport_unit/plantA.gen')
  plantA:Write('tmp_hio_tu_plantA.gen')

  specA=faudes.HioPlant('data/4_transport_unit/specA.gen')
  specA:Write('tmp_hio_tu_spec_A.gen')

  constrE_A=faudes.HioConstraint('data/4_transport_unit/constrE_A.gen')
  constrE_A:Write('tmp_hio_tu_constrE_A.gen')

  constrP_A=faudes.HioConstraint('data/4_transport_unit/constrP_A.gen')
  constrP_A:Write('tmp_hio_tu_constrP_A.gen')

  -- The operator constraint constrC of specification A is minimal.
  -- So, informally, it can be passed as an epsilon-language.
  constrC_A=faudes.HioConstraint()
  constrC_A:InsInitState('1')
  constrC_A:SetMarkedState('1')

  -- compute I/O controller:

  controllerA=faudes.HioController()
  faudes.HioSynthMonolithic(plantA, specA, constrC_A, constrP_A, constrE_A, controllerA)
  controllerA:Write('tmp_hio_tu_IOcontroller_A.gen')
  
  -- inspect full and external closed loop
  full_loop=faudes.Generator()
  ext_loop=faudes.Generator()
  
  faudes.Parallel(controllerA,plantA,full_loop)
  full_loop:Write('tmp_hio_tu_full_loop_A.gen')
  
  faudes.Project(full_loop,specA:Alphabet(),ext_loop)
  ext_loop:Write('tmp_hio_tu_ext_loop_A.gen')  

  --======================================================================  
  --====
  --==== plantAB: abstraction, I/O shuffle and environment.=====
  --====
  --======================================================================

  -- specA serves as abstraction of the closed loop of controllerA and plantA, and 
  -- so does specB for controllerB and plantB
  -- read specB and corresp. environment constraint (op. constraint is minimal):
  specB=faudes.HioPlant('data/4_transport_unit/specB.gen')
  constrE_B=faudes.HioConstraint('data/4_transport_unit/constrE_B.gen')

  print('==================== IOshuffle: tmp_hio_tu_shuffAB.gen/.png')

  shuffAB=faudes.HioPlant()
  faudes.HioShuffle(specA, specB, shuffAB)
  faudes.StateMin(shuffAB,shuffAB)
  shuffAB:StateNamesEnabled(false)
  shuffAB:Name('IO shuffle AB')
  shuffAB:Write('tmp_hio_tu_shuffAB.gen')

  --read environment:

  envAB=faudes.HioEnvironment('data/4_transport_unit/envAB.gen')
  envAB:Write('tmp_hio_tu_envAB.gen')


  --======================================================================
  --====
  --==== controller synthesis for plantAB:=====
  --====
  --======================================================================

  print('====================')
  print('==================== ready for Controller synthesis for plantAB')
  print('====================')

  --read specAB
  specAB=faudes.HioPlant('data/4_transport_unit/specAB.gen')
  specAB:Write('tmp_hio_tu_specAB.gen')  
  -- alternatively, the specification 'specA_ARB_FEEDBACK.gen'
  -- can be used. It does not specify an 'idle'-feedback
  -- after EACH wp transport, but after an ARBITRARY amount
  -- instead. The same result is achieved for controller A 
  -- because of the Yc-Acyclic property. However, this spec
  -- cannot be used as plant abstraction, as there are no
  -- constraints w.r.t. which this plant model is YP-live.

  -- The operator constraint constrC of specification AB is minimal.
  -- So, informally, it can be passed as an epsilon-language.
  constrC_AB=faudes.HioConstraint()
  constrC_AB:InsInitState('1')
  constrC_AB:SetMarkedState('1')


  --read environment constraint of specification AB (op. constraint is minimal)
  constrL_AB=faudes.HioConstraint('data/4_transport_unit/constrL_AB.gen')
  constrL_AB:Write('tmp_hio_tu_constrL_AB.gen')

  -- local constraints: composition of env. constraints for spec. A and spec B 
  -- (op. constraints for both are minimal)
  locConstrAB=faudes.Generator()
  faudes.Parallel(constrE_A,constrE_B,locConstrAB)

  --calculate IO controller for plantAB
  controllerAB=faudes.HioController()
  faudes.HioSynthHierarchical(shuffAB, envAB, specAB, locConstrAB, constrC_AB, constrL_AB, controllerAB)

  -- marking does not count in controller
  faudes.PrefixClosure(controllerAB)
  faudes.StateMin(controllerAB,controllerAB)
  controllerAB:StateNamesEnabled(false)
  controllerAB:Name('Controller AB')
  controllerAB:Write('tmp_hio_tu_IOcontrollerAB.gen')
  print('==================== Synthesis done for Controller AB.')
  controllerAB:SWrite()

  -- Again, we specify the same behaviour for plants C and D to receive a structurally 
  -- identical controller CD.
  -- Moreover, specification (ie abstract plant model)  AB is structurally identical to  specA and specB.
  -- Hence, the controller ABCD for spec (abstract plant model) AB and CD is structurally identical to 
  -- controller AB or CD ! The same holds for environment ABCD, which is derived directly from
  -- environment AB.

  --======================================================================
  --====
  --==== remaining plant- and controller hierarchy =====
  --==== UNCOMMENT AS SOON AS faudes::Version(string,string) IS AVAILABLE
  --======================================================================

  -- we derive all remaining components of the hierarchy by creating versions of the previous components.

  plantX=faudes.HioPlant()
  faudes.Version(plantA,'_A','_B',plantX)
  plantX:Write('tmp_hio_tu_plantB.gen')
  faudes.Version(plantA,'_A','_C',plantX)
  plantX:Write('tmp_hio_tu_plantC.gen')
  faudes.Version(plantA,'_A','_D',plantX)
  plantX:Write('tmp_hio_tu_plantD.gen')
  faudes.Version(plantA,'_A','_E',plantX)
  plantX:Write('tmp_hio_tu_plantE.gen')
  faudes.Version(plantA,'_A','_F',plantX)
  plantX:Write('tmp_hio_tu_plantF.gen')
  faudes.Version(plantA,'_A','_G',plantX)
  plantX:Write('tmp_hio_tu_plantG.gen')
  faudes.Version(plantA,'_A','_H',plantX)
  plantX:Write('tmp_hio_tu_plantH.gen')
  
  
  -- specX and consequently controllerX are identical
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_B',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_B.gen')
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_C',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_C.gen')
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_D',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_D.gen')
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_E',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_E.gen')
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_F',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_F.gen')
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_G',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_G.gen')
  controllerX=faudes.HioController()
  faudes.Version(controllerA,'_A','_H',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontroller_H.gen')


  controllerXtmp=faudes.HioController()

  faudes.Version(controllerAB,'_AB','_CD',controllerX)
  faudes.Version(controllerX,'_A','_C',controllerXtmp)
  faudes.Version(controllerXtmp,'_B','_D',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontrollerCD.gen')
  
  faudes.Version(controllerAB,'_AB','_EF',controllerX)
  faudes.Version(controllerX,'_A','_E',controllerXtmp)
  faudes.Version(controllerXtmp,'_B','_F',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontrollerEF.gen')
  
  faudes.Version(controllerAB,'_AB','_aBCD',controllerX)
  faudes.Version(controllerX,'_A','_AB',controllerXtmp)
  faudes.Version(controllerXtmp,'_B','_CD',controllerX)
  faudes.Version(controllerX,'_aBCD','_ABCD',controllerXtmp)
  controllerXtmp:Write('tmp_hio_tu_IOcontrollerABCD.gen')
  
  faudes.Version(controllerAB,'_AB','_EFGH',controllerX)
  faudes.Version(controllerX,'_A','_EF',controllerXtmp)
  faudes.Version(controllerXtmp,'_B','_GH',controllerX)
  controllerX:Write('tmp_hio_tu_IOcontrollerEFGH.gen')

  faudes.Version(controllerAB,'_AB','_a_H',controllerX)
  faudes.Version(controllerX,'_A','_ABCD',controllerXtmp)
  faudes.Version(controllerXtmp,'_B','_EFGH',controllerX)
  faudes.Version(controllerX,'_a_H','_A_H',controllerXtmp)
  controllerXtmp:Write('tmp_hio_tu_IOcontrollerA_H.gen')

  
  envXtmp=faudes.HioEnvironment()
  envX=faudes.HioEnvironment()

  faudes.Version(envAB,'_AB','_CD',envX)
  faudes.Version(envX,'_A','_C',envXtmp)
  faudes.Version(envXtmp,'_B','_D',envX)
  envX:Write('tmp_hio_tu_IOenvironmentCD.gen')
  
  faudes.Version(envAB,'_AB','_EF',envX)
  faudes.Version(envX,'_A','_E',envXtmp)
  faudes.Version(envXtmp,'_B','_F',envX)
  envX:Write('tmp_hio_tu_IOenvironmentEF.gen')
  
  faudes.Version(envAB,'_AB','_aBCD',envX)
  faudes.Version(envX,'_A','_AB',envXtmp)
  faudes.Version(envXtmp,'_B','_CD',envX)
  faudes.Version(envX,'_aBCD','_ABCD',envXtmp)
  envXtmp:Write('tmp_hio_tu_IOenvironmentABCD.gen')
  
  faudes.Version(envAB,'_AB','_EFGH',envX)
  faudes.Version(envX,'_A','_EF',envXtmp)
  faudes.Version(envXtmp,'_B','_GH',envX)
  envX:Write('tmp_hio_tu_IOenvironmentEFGH.gen')

  faudes.Version(envAB,'_AB','_a_H',envX)
  faudes.Version(envX,'_A','_ABCD',envXtmp)
  faudes.Version(envXtmp,'_B','_EFGH',envX)
  faudes.Version(envX,'_a_H','_A_H',envXtmp)
  envXtmp:Write('tmp_hio_tu_IOenvironmentA_H.gen')


  -- Now, the hierarchy is completed for a chain of 8 TU's and can be extended to an
  -- arbitrary chain length.


  -- record test case
  FAUDES_TEST_DUMP('plant x', plantX)
  FAUDES_TEST_DUMP('env x', envXtmp)
  FAUDES_TEST_DUMP('contr x', controllerXtmp)
