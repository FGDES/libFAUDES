----------------------------OA--------------------------------------------------------
-- Finite capacity buffer in HIO architecture
------------------------------------------------------------------------------------


--[[

This a variation of an example developped in the context of Christine Agethens PhD
project. The base idea is to control two FIFO buffers to act as a single buffer with
a capacity that amounts to the sum the component capacity. The physical layout is
the following

[source |LL] ==> [LL| left buffer L |LR] ==> [RL| right buffer R |RR] ==> [RR| sink]

The flow of workpieces is modelled by environment events. We have

X_eg  get a work piece
X_ep  put a work piece
X_ea  positive acknowledge
X_en  negative acknowledge

The prefix X_ represents the layout, i.e., who is whos environment. We use LL_e*
for the left side of the left buffer. So LL_eg represents a request from buffer L to
get a workpiece from the environment. The perfect source will after LL_eg have
LL_ea as only successor event.

Likewise, RL_eg represents a request from the right buffer R to get a workpiece from its
left environment. A corresponding environment model decodes the request. If buffer L
recently devilered a workpiece LR_ep, the only elidgible event will be RL_ea. Otherwise
it will be RL_en.

Either buffer reports to the supervisor its status and expects commands. We have

Z_pe  buffer is empty
Z_pn  buffer is neither empty nor full
Z_pf  buffer is full
Z_pw  do nothing
Z_pg  get a workpiece
Z_pp  pass on a workpiece.

Here Z_ stands either for L_ (the left buffer) or R_ the right buffer. These are the only
observable events, with the commands in addition being controllable.


Staus: we do not do partial observation by now --- so this is work in progress 


--]]



-- convenience shortcut
SF=string.format


------------------------------------------------------------------------------------
-- set up buffer with capacity n
------------------------------------------------------------------------------------
function buffer(pre,left,right,n)
  print(SF("buffer #%d %s %s %s",n,pre,left,right))
  local b=faudes.System()
  -- events
  local pe = b:InsEvent(SF("%s_pe",pre))
  local pn = b:InsEvent(SF("%s_pn",pre))
  local pf = b:InsEvent(SF("%s_pf",pre))
  local pw = b:InsControllableEvent(SF("%s_pw",pre))
  local pg = b:InsControllableEvent(SF("%s_pg",pre))
  local pp = b:InsControllableEvent(SF("%s_pp",pre))
  local leg = b:InsUnobservableEvent(SF("%s_eg",left))
  local lea = b:InsUnobservableEvent(SF("%s_ea",left))
  local len = b:InsUnobservableEvent(SF("%s_en",left))
  local rep = b:InsUnobservableEvent(SF("%s_ep",right))
  local rea = b:InsUnobservableEvent(SF("%s_ea",right))
  local ren = b:InsUnobservableEvent(SF("%s_en",right))
  -- state empty
  local I0 = b:InsInitState("I0")
  local C0 = b:InsState("C0")
  local G0 = b:InsState("G0")
  local E0 = b:InsState("E0")
  b:SetMarkedState(I0);
  b:SetTransition(I0,pe,C0)
  b:SetTransition(C0,pw,I0)
  b:SetTransition(C0,pg,G0)
  b:SetTransition(G0,leg,E0)
  b:SetTransition(E0,len,I0)
  -- state full
  local In = b:InsState(SF("I%d",n))
  local Cn = b:InsState(SF("C%d",n))
  local Pn = b:InsState(SF("P%d",n))
  local Fn = b:InsState(SF("F%d",n))
  b:SetMarkedState(In);
  b:SetTransition(In,pf,Cn)
  b:SetTransition(Cn,pw,In)
  b:SetTransition(Cn,pp,Pn)
  b:SetTransition(Pn,rep,Fn)
  b:SetTransition(Fn,ren,In)
  -- special case cap 1
  if n==1 then
    b:SetTransition(E0,lea,In)
    b:SetTransition(Fn,rea,I0)
  end
  -- state i: insert
  for i=1,n-1 do
    local Ii = b:InsState(SF("I%d",i))
    local Ci = b:InsState(SF("C%d",i))
    local Gi = b:InsState(SF("G%d",i))
    local Pi = b:InsState(SF("P%d",i))
    local Ei = b:InsState(SF("E%d",i))
    local Fi = b:InsState(SF("F%d",i))
    b:SetMarkedState(Ii);
  end  
  -- state i: transitions  
  for i=1,n-1 do
    local Ii = b:StateIndex(SF("I%d",i))
    local Iim1 = b:StateIndex(SF("I%d",i-1))
    local Iip1 = b:StateIndex(SF("I%d",i+1))
    local Ci = b:StateIndex(SF("C%d",i))
    local Gi = b:StateIndex(SF("G%d",i))
    local Pi = b:StateIndex(SF("P%d",i))
    local Ei = b:StateIndex(SF("E%d",i))
    local Eim1 = b:StateIndex(SF("E%d",i-1))
    local Fi = b:StateIndex(SF("F%d",i))
    local Fip1 = b:StateIndex(SF("F%d",i+1))
    b:SetTransition(Eim1,lea,Ii)
    b:SetTransition(Ii,pn,Ci)
    b:SetTransition(Ci,pw,Ii)
    b:SetTransition(Ci,pg,Gi)
    b:SetTransition(Ci,pp,Pi)
    b:SetTransition(Gi,leg,Ei)
    b:SetTransition(Ei,len,Ii)
    b:SetTransition(Ei,lea,Iip1)
    b:SetTransition(Pi,rep,Fi)
    b:SetTransition(Fi,ren,Ii)
    b:SetTransition(Fi,rea,Iim1)    
    b:SetTransition(Fip1,rea,Ii)    
  end
  -- alphabets
  oevs=faudes.Alphabet()
  oevs:Insert(pe)
  oevs:Insert(pn)
  oevs:Insert(pf)
  oevs:Insert(leg)
  oevs:Insert(rep)
  ievs=faudes.Alphabet()
  ievs:Insert(pw)
  ievs:Insert(pg)
  ievs:Insert(pp)
  ievs:Insert(lea)
  ievs:Insert(len)
  ievs:Insert(rea)
  ievs:Insert(ren)

  return b, ievs, oevs
end


------------------------------------------------------------------------------------
-- set up source
------------------------------------------------------------------------------------
function source(pre)
  print(SF("source %s",pre))
  local b=faudes.Generator()
  -- alphabet
  local eg = b:InsEvent(SF("%s_eg",pre))
  local ea = b:InsEvent(SF("%s_ea",pre))
  local en = b:InsEvent(SF("%s_en",pre))
  -- states 
  local I = b:InsInitState("I")
  local B = b:InsState("B")
  faudes.MarkAllStates(b)
  -- transitions
  b:SetTransition(I,eg,B)
  b:SetTransition(B,ea,I)
  
  
  return b
end  


------------------------------------------------------------------------------------
-- set up sink
------------------------------------------------------------------------------------
function sink(pre)
  print(SF("sink %s",pre))
  local b=faudes.Generator()
  -- alphabet
  local ep = b:InsEvent(SF("%s_ep",pre))
  local ea = b:InsEvent(SF("%s_ea",pre))
  local en = b:InsEvent(SF("%s_en",pre))
  -- states 
  local I = b:InsInitState("I")
  local B = b:InsState("B")
  faudes.MarkAllStates(b)
  -- sink
  b:SetTransition(I,ep,B)
  b:SetTransition(B,ea,I)

  return b
end  


------------------------------------------------------------------------------------
-- set up environment
------------------------------------------------------------------------------------
function environment(left,right)
  print(SF("environment %s %s",left,right))
  local b=faudes.Generator()
  -- alphabet
  local lep = b:InsEvent(SF("%s_ep",left))
  local lea = b:InsEvent(SF("%s_ea",left))
  local len = b:InsEvent(SF("%s_en",left))
  local reg = b:InsEvent(SF("%s_eg",right))
  local rea = b:InsEvent(SF("%s_ea",right))
  local ren = b:InsEvent(SF("%s_en",right))
  -- states 
  local I = b:InsInitState("I")
  local A = b:InsState("A")
  local B = b:InsState("B")
  local C = b:InsState("C")
  local E = b:InsState("E")
  faudes.MarkAllStates(b)
  -- lft buffer put: ok and pass on
  b:SetTransition(I,lep,A)
  b:SetTransition(A,lea,B)
  b:SetTransition(B,reg,C)
  b:SetTransition(C,rea,I)
  -- right buffer get: fail
  b:SetTransition(I,reg,E)
  b:SetTransition(E,ren,I)

  return b
end  


------------------------------------------------------------------------------------
-- IOshuffle 
------------------------------------------------------------------------------------
function faudes.IOShuffle(plant1,plant2,result)

 local sorter=faudes.Generator()
  -- sorter states
  sorter:InsState("I")
  sorter:InsState("P1")
  sorter:InsState("P2")
  sorter:SetInitState("I")
  sorter:SetMarkedState("I")
  sorter:SetMarkedState("P1")
  sorter:SetMarkedState("P2")
  -- sorter transitions
  local eit=plant1:AlphabetBegin()
  while eit~=plant1:AlphabetEnd() do
    sorter:InsEvent(plant1:EventName(eit:Index()))
    sorter:SetTransition("I",plant1:EventName(eit:Index()),"P1")
    sorter:SetTransition("P1",plant1:EventName(eit:Index()),"I")
    eit:Inc()
  end
  local eit=plant2:AlphabetBegin()
  while eit~=plant2:AlphabetEnd() do
    sorter:InsEvent(plant2:EventName(eit:Index()))
    sorter:SetTransition("I",plant2:EventName(eit:Index()),"P2")
    sorter:SetTransition("P2",plant2:EventName(eit:Index()),"I")
    eit:Inc()
  end
  
  local altenator=faudes.Generator()
  -- altenator states
  altenator:InsState("I")
  altenator:InsState("P1")
  altenator:InsState("P2")
  altenator:SetInitState("I")
  altenator:SetMarkedState("I")
  -- altenator transitions
  local eit=plant1:AlphabetBegin()
  while eit~=plant1:AlphabetEnd() do
    altenator:InsEvent(plant1:EventName(eit:Index()))
    altenator:SetTransition("I",plant1:EventName(eit:Index()),"P1")
    altenator:SetTransition("P1",plant1:EventName(eit:Index()),"P1")
    altenator:SetTransition("P2",plant1:EventName(eit:Index()),"I")
    eit:Inc()
  end
  local eit=plant2:AlphabetBegin()
  while eit~=plant2:AlphabetEnd() do
    altenator:InsEvent(plant2:EventName(eit:Index()))
    altenator:SetTransition("I",plant2:EventName(eit:Index()),"P2")
    altenator:SetTransition("P2",plant2:EventName(eit:Index()),"P2")
    altenator:SetTransition("P1",plant2:EventName(eit:Index()),"I")
    eit:Inc()
  end
  -- do compose
  faudes.BuechiParallel(plant1,plant2,result)
  faudes.Parallel(result,sorter,result)
  faudes.BuechiParallel(result,altenator,result)
end 


------------------------------------------------------------------------------------
-- two buffers with capacity #1
------------------------------------------------------------------------------------


nl=1
nr=1
print(SF("=== setting up plant with capacity left #%d right #%d",nl,nr))

src=source("LL")
bl=buffer("L","LL","LR",nl)
env=environment("LR","RL")
br=buffer("R","RL","RR",nr)
snk=sink("RR")

plant=faudes.Generator()
faudes.IOShuffle(bl,br,plant)
faudes.Parallel(plant,env,plant)
faudes.Parallel(plant,src,plant)
faudes.Parallel(plant,snk,plant)
faudes.BuechiTrim(plant)
plant:Name("Plant")

plant:SWrite()


print(SF("=== setting up specification with capacity %d",nl+nr))

src=source("LL")
b2=buffer("S","LL","RR",nl+nr)
snk=sink("RR")

spec=faudes.Generator();
faudes.Parallel(src,b2,spec)
faudes.Parallel(spec,snk,spec)
faudes.BuechiTrim(spec)
spec:Name("Specification")

spec:SWrite()


print("=== figure alphabets")

sigall= bl:Alphabet() + br:Alphabet() + spec:Alphabet()
sigall:Name("SigmaAll")
sigvirt = sigall - (bl:Alphabet() + br:Alphabet())
sigvirt:Name("SigmaVirt")
sigctrl=bl:ControllableEvents() + br:ControllableEvents() + sigvirt
sigctrl:Name("SigmaCtrl")
sigobs=sigall - (bl:UnobservableEvents() + br:UnobservableEvents())
sigobs:Name("SigmaObs")

sigall:Write()
sigctrl:Write()
sigobs:Write()


print("=== running graphviz")

src:GraphWrite("tmp_omg_7_src.png")
bl:GraphWrite("tmp_omg_7_bl.png")
env:GraphWrite("tmp_omg_7_env.png")
br:GraphWrite("tmp_omg_7_br.png")
snk:GraphWrite("tmp_omg_7_snk.png")
--plant:GraphWrite("tmp_omg_7_plant.png")
spec:GraphWrite("tmp_omg_7_spec.png")


print("=== prepare synthesis")

faudes.InvProject(plant,sigall,plant)
faudes.InvProject(spec,sigall,spec)
cand=faudes.Generator()
faudes.BuechiParallel(plant,spec,cand)
faudes.BuechiTrim(cand)
relcl=faudes.IsBuechiRelativelyClosed(plant,cand)
if relcl then
  print("=== spec is rel topologically closed")
else
  print("=== spec fails to be rel topologically closed")
end
plant:Write("tmp_omg_7_plant.gen")
spec:Write("tmp_omg_7_spec.gen")
cand:Write("tmp_omg_7_cand.gen")


print("=== running synthesis: supremal controllable sublanguage")

sup=faudes.Generator();
faudes.SupBuechiCon(plant,sigctrl,spec,sup)             
faudes.BuechiTrim(sup)
faudes.StateMin(sup)
sup:Name("Supremal Controllable Sublanguage")
srelcl=faudes.IsBuechiRelativelyClosed(plant,sup)
if srelcl then
  print("=== supremal controllable sublanguage is rel topologically closed")
else
  print("=== supremal controllable sublanguage fails to be rel topologically closed")
end
sup:SWrite()
sup:Write("tmp_omg_7_sup.gen")


print("=== running synthesis: closed loop under greedy control")

ctrl=faudes.Generator()
faudes.BuechiCon(plant,sigctrl,spec,ctrl)    
faudes.BuechiClosure(ctrl)
faudes.StateMin(ctrl)
loop=faudes.Generator()
faudes.BuechiProduct(plant,ctrl,loop)
loop:Name("Loop under Greedy Control")
loop:SWrite()
loop:Write("tmp_omg_7_loop.gen")



print("=== verifying expected/desired properties")

cdet=faudes.IsDeterministic(loop)
if cdet then
  print("=== loop under greedy control is deterministic")
else
  print("=== loop under greedy control fails to be deterministic")
end
ctrim=faudes.IsBuechiTrim(loop)
if ctrim then
  print("=== loop under greedy control is buechi trim")
else
  print("=== loop under greedy control fails to be buechi trim")
end
crelcl=faudes.IsBuechiRelativelyClosed(plant,loop)
if crelcl then
  print("=== loop under greedy control is rel topologically closed")
else
  print("=== loop under greedy control fails to be rel topologically closed")
end
cstarctrl=faudes.IsControllable(plant,sigctrl,loop)
if cstarctrl then
  print("=== loop under greedy control is *-controllable")
else
  print("=== loop under greedy control fails to be *-controllable")
end
cenfspec=faudes.BuechiLanguageInclusion(loop,spec)
if cenfspec then
  print("=== loop under greedy control satisfied upper bound  spec")
else
  print("=== loop under greedy control fails to satisfy upper bound spec")
end



--[[
testing ...

looppro=faudes.Generator(loop)
faudes.ProjectNonDet(looppro,b2:Alphabet())                  -- not applicable for Buechi automata (!)
faudes.StateMin(looppro)
looppro:SWrite()

print("=== running graphviz")

looppro:GraphWrite("tmp_omg_7_looppro.png")
--]]


