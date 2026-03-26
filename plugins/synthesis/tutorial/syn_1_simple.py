# Synthesis demo -- two simple machines with a bfufer of size one

# import faudes module (lazy all global)
from faudes import *

# machine 1
gL1=Generator.NewFromLists(
  delta=[
    ['Idle','alpha1','Busy'],
    ['Busy','beta1','Idle']
  ],
  Q0=['Idle'],
  Qm=['Idle']
)
gL1.Name("M1")
      
# machine 2
gL2=Generator.NewFromLists(
  delta=[
    ['Idle','alpha2','Busy'],
    ['Busy','beta2','Idle']
  ],
  Q0=['Idle'],
  Qm=['Idle']
)
gL2.Name("M2")
      
# overall plant
gL=Parallel(gL1,gL2)

# controllable events
sCtrl=EventSet.NewFromList(
  ['alpha1', 'alpha2']
)

# specification aka buffer
gE=Generator.NewFromLists(
  delta=[
    ['Empty', 'beta1', 'Full'],
    ['Full', 'alpha2', 'Empty']
  ],
  Q0=['Empty'],
  Qm=['Empty']
)
gE.Name("Spec")

# lift specification to overall eventset
sAll=gL.Alphabet().Copy()
InvProject(gE,sAll,gE)

gL1.Write()
gL2.Write()
gE.Write()

# supremal closed loop
gK=SupCon(gL,sCtrl,gE)

# show result on console
gK.Write()
 
# export result as graphics
gK.GraphWrite("tmp_K.png")

# record test case
TestDump("gK",gK)

# validate test 
TestDiff()

