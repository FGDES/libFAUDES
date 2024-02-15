-- hio_2_hiogenerators:lua

-- Tutorial, classes HioPlant, HioController, HioEnvironment, HioConstraint
-- This tutorial demonstrates basic maintenance of HioGenerator objects

print('hiosys: lua tutorial 2 - HioGenerators')

 --*************************************************
 -- * HIO PLANT
 -- ***************************************************/

  --*****************
  -- * constructor and file io
  --********************/

  -- At first we create an empty HioPlant object

  g1=faudes.HioPlant()

  -- We create a HioPlant by reading a HioPlant file

  g2=faudes.HioPlant('data/2_hiogenerators/hio_simplemachine_A.gen')
  FAUDES_TEST_DUMP("HioPlant from genfile",g2)

  -- We create a HioPlant by reading a plain Generator file (no event attributes)

  g1:Read('data/2_hiogenerators/simplemachine_A.gen')

  -- We copy a plain Generator to a HioPlant
  g3=faudes.HioPlant(g1)
  
  -- We copy a HioPlant to another HioPlant
  g4=faudes.HioPlant(g2)
  FAUDES_TEST_DUMP("HioPlant construct from HioPlant",g4)

  -- construct HioPlant from plain generator and event sets
  ypEvents=faudes.EventSet(g2:YpEvents())
  FAUDES_TEST_DUMP("get Yp events",ypEvents)
  upEvents=faudes.EventSet(g2:UpEvents())
  yeEvents=faudes.EventSet(g2:YeEvents())
  ueEvents=faudes.EventSet(g2:UeEvents())  
  g5=faudes.HioPlant(g3,ypEvents,upEvents,yeEvents,ueEvents)
  FAUDES_TEST_DUMP("HioPlant(vGen,EventSets)",g5)

  -- -- We write the HioPlant to files 

  g1:Write('tmp_hiosimplemachine_A1.gen')
  g2:Write('tmp_hiosimplemachine_A2.gen')
  g3:Write('tmp_hiosimplemachine_A3.gen')
  g5:Write('tmp_hiosimplemachine_A5.gen')
  
  g1:GraphWrite('tmp_hiosimplemachine_A.png')


  -- See result on console

  print ( '######################################')
  print ( '# hio simple machine:')
  g2:Write()
  print ( '######################################')

 -- -- ****************************
 -- -- * check if HioPlant meets IO-Plant form
 -- -- ******************************/

  print ( '######################################')
  print ( '# IsHioPlantForm(g1):')  
  -- fails for g1 due to missing event attributes
  -- run IsHioPlantForm with detailed report string  s
  b, s =faudes.IsHioPlantForm(g1)
  FAUDES_TEST_DUMP("IsHioPlant fail",b)
  if b then
	print("Fulfilled! - Report:")
	print(s)
  else
	print("Not fulfilled! - Report:")
	print(s)
  end
  print ( '######################################')
  
  print ( '######################################')
  print ( '# IsHioPlantForm(g2):')  
  -- fulfilled for g2:
  b,s =faudes.IsHioPlantForm(g2)
  FAUDES_TEST_DUMP("IsHioPlant pass",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  -- remove some transitions with input-event
  
  -- Transitions
  TransToClear=faudes.TransSet()
  tit=g2:TransRelBegin()
  while(tit~=g2:TransRelEnd()) do
       if(g2:EventName(tit:Transition().Ev)=='A_stp' or g2:EventName(tit:Transition().Ev)=='A_nack') then
  		TransToClear:Insert(tit:Transition())
  	 end
    tit:Inc()
  end

  tit=TransToClear:Begin()
  while(tit~=TransToClear:End()) do
  	g2:ClrTransition(tit:Transition())
  	tit:Inc()
  end
  
  g2:Write('tmp_hiosimplemachine_A_broken.gen')
  g2:GraphWrite('tmp_hiosimplemachine_A_broken.png')
  -- now, the input is no longer free in g2
  print ( '######################################')
  print ( '# IsHioPlantForm(g2) after removing input transitions:')  
  -- fulfilled for g2:
  if faudes.IsHioPlantForm(g2) then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  -- repair
  faudes.HioFreeInput(g2)
  FAUDES_TEST_DUMP("HioFreeInput",g2)
  
  -- HioPlantForm is retrieved for g2:
  print ( '######################################')
  print ( '# IsHioPlantForm(g2) after HioFreeInput():')  
  -- fulfilled for g2:
  if faudes.IsHioPlantForm(g2) then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  
  FAUDES_TEST_DUMP("HioPlant construction",g2)
  g2:Write()
  g2:Write('tmp_hiosimplemachine_A_repaired.gen')
  g2:GraphWrite('tmp_hiosimplemachine_A_repaired.png')

 --******************************************
  -- * access to hio-property of states
  -- *
  -- * note: hio state properties need not be up to date
  -- *  - always run faudes.IsHioPlantForm() to set state attributes
-- *******  **************************************/
  
  -- Get states of g2 sorted according to their active event sets
  QYpYe=g2:QYpYeStates()
  QUp=g2:QUpStates()
  FAUDES_TEST_DUMP("get Up states",QUp)
  QUe=g2:QUeStates()
  
   -- show on console
  print ( '######################################')
  print ( '# QYpYe,QUp and QUe of simple machine:')
  QYpYe:Write()
  QUp:Write()
  QUe:Write()
  print ( '######################################') 


  -- ************************
  -- * access to hio-property of events
  -- **************************/

  -- Retrieve faudes.EventSets containing all YP-, UP-, YE-, UE-events  from g2

  ypEvents=g2:YpEvents()
  upEvents=g2:UpEvents()
  yeEvents=g2:YeEvents()
  ueEvents=g2:UeEvents()

  -- Set YP-, UP-, YE-, UE-events  in g1
  g1:SetYp(ypEvents)
  g1:SetUp(upEvents)
  g1:SetYe(yeEvents)
  g1:SetUe(ueEvents)
  
  -- now, also g1 is in HioPlantForm
  print ( '######################################')
  print ( '# IsHioPlantForm of g1 after setting event attributes:')
  if faudes.IsHioPlantForm(g1) then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  print ( '######################################')

  -- file i/o and access to attributes are analogous for HioConstraint, HioController and HioEnvironment:

  -- *************************************************
  -- * HIO CONTROLLER
  -- ***************************************************/

  -- Construct simple HioController structure
  c1=faudes.HioController()
  c1:Name('HioController')
  yp1=c1:InsYpEvent('yp1')
  yp2=c1:InsYpEvent('yp2')
  yc=c1:InsYcEvent('yc')
  uc1=c1:InsUcEvent('uc1')
  uc2=c1:InsUcEvent('uc2')
  up=c1:InsUpEvent('up')
  
  st1=c1:InsInitState()
  c1:SetMarkedState(st1)
  st2=c1:InsMarkedState()
  st3=c1:InsMarkedState()
  st4=c1:InsMarkedState()
  
  c1:SetTransition(st1,yp1,st2)
  c1:SetTransition(st2,yc,st3)
  c1:SetTransition(st3,uc1,st4)
  c1:SetTransition(st4,up,st1)

  -- up to now, no I/O-controller form as inputs yp2 and uc2 are not accepted:
  print('')
  print('######################################')
  print('####### I/O CONTROLLER: #########')
  print ( '######################################')
  
  print ( '# IsHioControllerForm of hiocontroller before setting all input transitions:')
  b,s =faudes.IsHioControllerForm(c1)
  FAUDES_TEST_DUMP("IsHioControllerForm fail",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  print ( '######################################')
  
  -- test correct file I/O
  c1:Write('tmp_hiocontroller_incomplete.gen')
  c1:Read('tmp_hiocontroller_incomplete.gen')
  c1:Write()
  c1:GraphWrite('tmp_hiocontroller_incomplete.png')
  
  -- repair HioControllerForm using HioFreeInput
  print('++++ Before HioFreeInput')
  c1:Write()
  faudes.HioFreeInput(c1)
  print('++++ After HioFreeInput')
  c1:Write()
  print ( '######################################')
  print ( '# IsHioControllerForm of hiocontroller after HioFreeInput:')
  b,s =faudes.IsHioControllerForm(c1)
  FAUDES_TEST_DUMP("IsHioControllerForm pass",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  FAUDES_TEST_DUMP("HioController construction",c1)
  print ( '######################################')
  c1:Write()
  c1:Write('tmp_hiocontroller_repaired.gen')
  c1:GraphWrite('tmp_hiocontroller_repaired.png')
  print('######################################')
  print('')
  
 --*************************************************
 -- * HIO ENVIRONMENT
 -- ***************************************************/
  -- Construct simple HioEnvironment structure
  e1=faudes.HioEnvironment()
  e1:Name('HioEnvironment')
  ye1=e1:InsYeEvent('ye1')
  ye2=e1:InsYeEvent('ye2')
  yl=e1:InsYlEvent('yl')
  ul1=e1:InsUlEvent('ul1')
  ul2=e1:InsUlEvent('ul2')
  ue=e1:InsUeEvent('ue')
  
  st1=e1:InsInitState()
  e1:SetMarkedState(st1)
  st2=e1:InsMarkedState()
  st3=e1:InsMarkedState()
  st4=e1:InsMarkedState()
  
  e1:SetTransition(st1,ye1,st2)
  e1:SetTransition(st2,yl,st3)
  e1:SetTransition(st3,ul1,st4)
  e1:SetTransition(st4,ue,st1)

  -- up to now, no I/O-environment form as inputs ye2 and ul2 are not accepted:
  print('######################################')
  print('####### I/O ENVIRONMENT: #########')
  print ( '######################################')
  print ( '# IsHioEnvironmentForm of hioenvironment before setting all input transitions:')
  b,s =faudes.IsHioEnvironmentForm(e1)
  FAUDES_TEST_DUMP("IsHioEnvironmentForm fail",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  print ( '######################################')
  -- test correct file I/O  
  e1:Write('tmp_hioenvironment_incomplete.gen')
  e1:Read('tmp_hioenvironment_incomplete.gen')
  e1:Write()
  e1:GraphWrite('tmp_hioenvironment_incomplete.png')
  
  -- repair HioEnvironmentForm using HioFreeInput
  faudes.HioFreeInput(e1)
  print ( '######################################')
  print ( '# IsHioEnvironmentForm of hioenvironment after HioFreeInput:')
  b,s =faudes.IsHioEnvironmentForm(e1)
  FAUDES_TEST_DUMP("IsHioEnvironmentForm pass",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  FAUDES_TEST_DUMP("HioEnvironment construction",e1)
  print ( '######################################')
  e1:Write()
  e1:Write('tmp_hioenvironment_repaired.gen')
  e1:GraphWrite('tmp_hioenvironment_repaired.png')
  print('######################################')
  print('')
  
 --*************************************************
 -- * HIO CONSTRAINT
 -- ***************************************************/
  -- Construct simple HioConstraint structure
  cnstr1=faudes.HioConstraint()
  cnstr1:Name('HioConstraint')
  y1=cnstr1:InsYEvent('y1')
  y2=cnstr1:InsYEvent('y2')
  u=cnstr1:InsUEvent('u')
  
  st1=cnstr1:InsInitState()
  cnstr1:SetMarkedState(st1)
  st2=cnstr1:InsMarkedState()
  
  cnstr1:SetTransition(st1,y1,st2)
  cnstr1:SetTransition(st2,u,st1)

  -- up to now, no I/O-constraint form as input u2 is not accepted:
  print('')
  print('######################################')
  print('####### I/O CONSTRAINT: #########')
  print('#######')
  -- access to event properties:
  yEvents=cnstr1:YEvents()
  yEvents:Name('####### HioConstraint: Y-Events')
  yEvents:Write()
  print ( '######################################')
  print ( '# IsHioConstraintForm of hioconstraint before setting all input transitions:')
  b,s =faudes.IsHioConstraintForm(cnstr1)
  FAUDES_TEST_DUMP("IsHioConstraintForm fail",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  print ( '######################################')  
  -- test correct file I/O
  cnstr1:Write('tmp_hioconstraint_incomplete.gen')
  cnstr1:Read('tmp_hioconstraint_incomplete.gen')
  cnstr1:Write()
  cnstr1:GraphWrite('tmp_hioconstraint_incomplete.png')
  
  -- repair HioEnvironmentForm using HioFreeInput
  faudes.HioFreeInput(cnstr1)
  print ( '######################################')
  print ( '# IsHioConstraintForm of hioconstraint after HioFreeInput:')
  b,s =faudes.IsHioConstraintForm(cnstr1)
  FAUDES_TEST_DUMP("IsHioConstraintForm pass",b)
  if b then
	print("Fulfilled!")
  else
	print("Not fulfilled!")
  end
  FAUDES_TEST_DUMP("HioController construction",cnstr1)
  print ( '######################################')
  cnstr1:Write()
  cnstr1:Write('tmp_hioconstraint_repaired.gen')
  cnstr1:GraphWrite('tmp_hioconstraint_repaired.png')
  print('######################################')
