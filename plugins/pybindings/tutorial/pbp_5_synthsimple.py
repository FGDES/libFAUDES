# load libFAUDES bindings
import faudes

# machine 1
gL1=faudes.Generator()
gL1.InsInitState("Idle")
gL1.SetMarkedState("Idle")
gL1.InsState("Busy")
gL1.InsEvent("alpha1")
gL1.InsEvent("beta1")
gL1.SetTransition("Idle","alpha1","Busy")
gL1.SetTransition("Busy","beta1","Idle")

# machine 2
gL2=faudes.Generator()
gL2.InsInitState("Idle")
gL2.SetMarkedState("Idle")
gL2.InsState("Busy")
gL2.InsEvent("alpha2")
gL2.InsEvent("beta2")
gL2.SetTransition("Idle","alpha2","Busy")
gL2.SetTransition("Busy","beta2","Idle")

# overall plant
gL=faudes.Generator()
faudes.Parallel(gL1,gL2,gL)

# controllable events
sCtrl=faudes.EventSet()
sCtrl.Insert("alpha1")
sCtrl.Insert("alpha2")

# specification aka buffer
gE=faudes.Generator()
gE.InsInitState("Empty")
gE.SetMarkedState("Empty")
gE.InsState("Full")
gE.InsEvent("beta1")
gE.InsEvent("alpha2")
gE.SetTransition("Empty","beta1","Full")
gE.SetTransition("Full","alpha2","Empty")

# lift specification to overall eventset
sAll=faudes.EventSet()
sAll=gL.Alphabet()
faudes.InvProject(gE,sAll)

# supremal closed loop
gK=faudes.Generator()
faudes.SupConNB(gL,sCtrl,gE,gK)

#show result on console
gK.Write()
 
#export result as graphics
gK.GraphWrite("tmp_K.png")

# record test case
FAUDES_TEST_DUMP("gK",gK)

# validate test 
FAUDES_TEST_DIFF()

