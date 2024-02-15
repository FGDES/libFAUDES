-- Demo for implementing synthesis algorithms in luafaudes, tmoor, 20110315

-- This luafaudes script makes use of state- and transition-iterators
-- to implement a merge operation that combines a nominal and a degraded specification.
-- The resulting fault-accommodating specification is meant to insist in the nominal specification
-- only until a failure event occurs. After the ocurance of the failure event, the system
-- needs to satisfy an alternative specification (so called degraded specification). Using a 
-- fault-accommodating specification for a standard supervisor design procedure is meant 
-- to result in a fault-tolerant controller.

-- Note. This really is just a demo script and must not be confused with the
-- general discussion given in "Fault-Tolerant Control of Discrete Event Systems based 
-- on Fault-Accommodating Models" by Wittmann/Richter/Moor, accepted for presentation at
-- Safeprocess 2012.

-- Say hello
print([[

This demo script implements the function FtcSpecification to support a 
naive variant of fault-tolerant control.

As a test case, the script runs an extended version of the very-simple-machine 
scenario, where the second machine may fail to process a workpiece. Input
data is embedded with the script, incl some more comments. For graphical 
representations of input data and results see the files "tmp_ftc_*,png".

]])




-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------

-- Tolerant specification synthesis

function faudes.FtcSpecification(En,Ef,SigmaF,Et) 

  -- Parameters
  -- En: nominal specification
  -- Ef: degraded specification
  -- SigmaF: set of failure events 
  -- Et: fault-tolerant specification (return value)

  -- Declare variables local to this function
  local Enf, evf
  local pcmap, sit, tit
  local tr, qf, qn, sname, nname, fx, nx, qfnext


  -- Test input data consistency
  --
  -- The current implementation assumes SigmaF to hold exactly one failure event.

  if SigmaF:Size()~= 1 then
    faudes.Error("FtcSpecification(): Error: SigmaF must consist of one failure event")
  end
  evf=SigmaF:Begin():Index()

  -- Relate states between En and Ef
  --
  -- Technically, we apply a procduct composition Enf = En x Ef to realize the 
  -- intersection  L(En) ^ L(Ef) and use the composition map to relate states.
  
  Enf=faudes.Generator()
  pcmap=faudes.ProductCompositionMap()
  faudes.Product(En,Ef,pcmap,Enf)


  -- Union construct to merge Enf, En and Ef. 
  -- 
  -- At this stage, only Enf contributes to the initial states and neither
  -- En nor Ef are reachable. The construct uses consecutive states with offset nx 
  -- for En-states and fx for Ef-states. A future version of libFAUDES may provide 
  -- a UnionCompositionMap with similar semantics as the ProductCompositionMap.

  Et:Assign(Enf)  -- (tech. note: must use "Assign" for "return by reference")

  Et:InsEvents(SigmaF)
  nx=Et:MaxStateIndex()+1
  sit=En:StatesBegin()
  while sit~=En:StatesEnd() do
    Et:InsState(sit:Index() + nx)
    if En:ExistsMarkedState(sit:Index()) then
      Et:SetMarkedState(sit:Index() + nx)
    end
    sit:Inc()
  end
  tit=En:TransRelBegin()
  while tit~=En:TransRelEnd() do
    tr=tit:Transition();
    Et:SetTransition(tr.X1+nx,tr.Ev,tr.X2+nx)
    tit:Inc()
  end

  fx=Et:MaxStateIndex()+1
  sit=Ef:StatesBegin()
  while sit~=Ef:StatesEnd() do
    Et:InsState(sit:Index() + fx)
    if Ef:ExistsMarkedState(sit:Index()) then
      Et:SetMarkedState(sit:Index() + fx)
    end
    sit:Inc()
  end
  tit=Ef:TransRelBegin()
  while tit~=Ef:TransRelEnd() do
    tr=tit:Transition();
    Et:SetTransition(tr.X1+fx,tr.Ev,tr.X2+fx)
    tit:Inc()
  end


  -- Have nice state names
  --
  -- State names are purely cosmetic but essential for debugging/development

  if Et:StateNamesEnabled() and En:StateNamesEnabled() and Et:StateNamesEnabled() then
    Et:ClearStateNames()
    sit=Enf:StatesBegin()
    while sit~= Enf:StatesEnd() do
      qn=pcmap:Arg1State(sit:Index())
      nname = En:StateName(qn)
      if nname == "" then nname = qn end 
      sname = Et:UniqueStateName("NF_" .. nname)
      Et:StateName(qn,sname)
      sit:Inc()
    end   
    sit=En:StatesBegin()
    while sit~=En:StatesEnd() do
      nname = En:StateName(sit:Index())
      if nname == "" then nname = sit:Index() end 
      sname = Et:UniqueStateName("NF_" .. nname)
      Et:StateName(sit:Index() + nx, sname)
      sit:Inc()
    end
    sit=Ef:StatesBegin()
    while sit~=Ef:StatesEnd() do
      nname = Ef:StateName(sit:Index())
      if nname == "" then nname = sit:Index() end 
      sname = Et:UniqueStateName("F_" .. sname)
      Et:StateName(sit:Index() + fx, sname)
      sit:Inc()
    end
  end

  -- Link Enf with Ef by failure-event transitions
  --
  -- Iterate over all Enf-states, and test whether in the corresponding
  -- Ef-state a failure-event transition is enabled. If so, introduce a transion
  -- from the Enf-state to the corresponding Ef-state.

  sit=Enf:StatesBegin()
  while sit~= Enf:StatesEnd() do
    qf=pcmap:Arg2State(sit:Index())
    qfnext=Ef:SuccessorState(qf,evf)
    if qfnext~=0 then
      Et:SetTransition(sit:Index(),evf,qfnext+fx)
    end
    sit:Inc()
  end

  -- Link Enf with En whenever Enf restricts En
  --
  -- Iterate over all Enf-states, and test whether in the corresponding
  -- En-state an event is enabled which is disabled in Enf. If so, introduce a transtion
  -- from the Enf-state to the target En-state.

  sit=Enf:StatesBegin()
  while sit~= Enf:StatesEnd() do
    qn=pcmap:Arg1State(sit:Index())
    tit=En:TransRelBegin(qn)
    while tit~= En:TransRelEnd(qn) do
      if tit:Transition().Ev ~= evf then
        if Enf:SuccessorState(sit:Index(),tit:Transition().Ev) == 0 then
          Et:SetTransition(sit:Index(),tit:Transition().Ev,tit:Transition().X2+nx)
        end
      end
      tit:Inc()
    end
    sit:Inc()
  end

  -- Remove unreachable artifacts
  Et:Accessible()

  -- Name return
  Et:Name("FtcSpecification(" .. En:Name() .. ", " .. Ef:Name() .. ")")

end




-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------

-- Test scenario input data

-- Report
print("###################################################")
print("### prepare input data")

-- -------------------------------------------------------------------------------
-- Input data: plant component 1, nominal

G1n=faudes.System()
G1n:FromString([[

<Generator>
"M1nominal"

% Std "very simple machine" with events
% alpha1: start process
% beta1:  process complete

<Alphabet> alpha1 +C+ beta1 </Alphabet>

<States> I B </States>

<TransRel>
I alpha1 B
B beta1  I
</TransRel>

<InitStates> I </InitStates>
<MarkedStates> I </MarkedStates>

</Generator>
]])

-- -------------------------------------------------------------------------------
-- Input data: plant component 2, nominal

G2n=faudes.System()
G2n:FromString([[

<Generator>
"M2nominal"

% Extended "very simple machine", can run in "fast mode"
% or "thorough mode". Mode selection is considered
% controllable, while the "process start" event
% alpha2 is uncontrollable. Hence, a supervisor must only enable
% mode selection when a work piece is available.

% gamma2: prepare to run process in "fast mode"
% rho2:   prepare to run process in "thorough mode"
% alpha2: process start
% beta2:  process complete

<Alphabet> alpha2 beta2 gamma2 +C+ rho2 +C+ </Alphabet>

<States> I Sf St B </States>

<TransRel>
I gamma2 Sf
I rho2 St
Sf alpha2 B
St alpha2 B
B beta2  I
</TransRel>

<InitStates> I </InitStates>
<MarkedStates> I </MarkedStates>

</Generator>
]])

-- -------------------------------------------------------------------------------
-- Input data: nominal specification

En=faudes.Generator()
En:FromString([[

<Generator>
"nominal specification"

% The nominal specification models the constraints imposed 
% by a capacity-1 buffer placed between the two machines.
% Since "thorough mode" is considered more expensive,
% the nominal specification requires machine2 to run 
% in "fast mode" exclusively
%
% Technial note: we only enable mode selection of
% M2 when the buffer is full; this reduces the state
% count of the supervisors and makes them more easy
% to inspect.

<Alphabet> beta1 alpha2 gamma2 rho2</Alphabet>

<States> E F </States>

<TransRel>
E beta1 F
F alpha2 E
F gamma2 F
</TransRel>

<InitStates> E </InitStates>

<MarkedStates> E </MarkedStates>

</Generator>
]])

-- -------------------------------------------------------------------------------
-- Input data: plant component 2, degraded
G2f=faudes.System()
G2f:FromString([[

<Generator>
"M2degraded"

% This varaint of M2 introduces the unobservable failure event "fault"
% which can occur when choosing "fast mode". Thus, the degraded M2 
% may fail to process a work piece in "fast mode", but it allways
% succeeds in "thorough mode". Events

% fault: machine break down in "fast mode"
% *:     as in M2nominal

<Alphabet> alpha2 +C+ beta2 gamma2 +C+ rho2 +C+ fault +o+ </Alphabet>

<States> I Sf St B </States>

<TransRel>
I gamma2 Sf
I rho2 St
Sf alpha2 B
Sf fault I
St alpha2 B
B beta2  I
</TransRel>

<InitStates> I </InitStates>
<MarkedStates> I </MarkedStates>

</Generator>
]])

-- -------------------------------------------------------------------------------
-- Input data: degraded specification
Ef=faudes.Generator()
Ef:FromString([[

<Generator>
"degraded specification"

<Alphabet> beta1 alpha2 gamma2 rho2</Alphabet>

% In contrast to the nominal specification, the degraded
% case allows both "fast mode" and "thorough mode". Thus,
% the generated fault tolerant specification will insist in
% "fast mode" until a fault occurs, and then allow both modes.

<States> E F </States>

<TransRel>
E beta1  F
F alpha2 E
F gamma2 F
F rho2   F
</TransRel>

<InitStates> E </InitStates>
<MarkedStates> E </MarkedStates>

</Generator>
]])

-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------

-- Test scenario synthesis

-- Report
print("###################################################")
print("### compose components Gn and Gf")

-- Nominal plant
Gn = G1n:Copy();
faudes.Parallel(Gn,G2n,Gn);

-- Faulty plant
Gf = G1n:Copy();
faudes.Parallel(Gf,G2f,Gf);

-- SigmaF
SigmaF = faudes.EventSet()
SigmaF:Insert("fault")

-- SigmaN
SigmaN = Gn:Alphabet():Copy();

-- Sigma
Sigma = SigmaN + SigmaF

-- Normalize specification alphabets
faudes.InvProject(En,SigmaN,En)
faudes.InvProject(Ef,Sigma,Ef)

-- Prepare prefix closed plant models
clGf=Gf:Copy()
faudes.PrefixClosure(clGf)
clGn=Gn:Copy()
faudes.PrefixClosure(clGn)

-- Report
print("###################################################")
print("### nominal design of Kn to satisfy En")

-- Nominal design
Kn=faudes.Generator()
faudes.SupConNB(Gn,En,Kn)
--Kn:SWrite()

-- Report
print("###################################################")
print("### degraded design of Kf to satisfy Ef")

-- Enforce ontrollability
Kf=faudes.Generator()
faudes.SupConNB(Gf,Ef,Kf)
--Kf:SWrite()

-- Test normality
clKf=Kf:Copy()
faudes.PrefixClosure(clKf)
isnorm=faudes.IsNormal(clGf,clKf)
if isnorm then
  print("normality test: passed")
else
  print("normality test: failed (need to implement synthesis iteration!)")
end

-- Report
print("###################################################")
print("### synthesize tolerant specification Et")

-- Synthesize tolerant specification
Et=faudes.Generator()
faudes.FtcSpecification(En,Ef,SigmaF,Et)
--Et:SWrite();

-- Report
print("###################################################")
print("### tolerant design of Kt to satisfy Et")

-- Enforce ontrollability
Kt=faudes.Generator()
faudes.SupConNB(Gf,Et,Kt)
--Kt:SWrite()

-- Test normality
clKt=Kt:Copy()
faudes.PrefixClosure(clKt)
isnorm=faudes.IsNormal(clGf,clKt)
if isnorm then
  print("normality test: passed")
else
  print("normality test: failed (need to use SupConNormNB fro synthesis)")
end

-- Project for supervisor implementation
SigmaO=Gf:ObservableEvents()
Kto=faudes.Generator()
faudes.Project(Kt,SigmaO,Kto)

-- Report
print("###################################################")
print("### generating graph output")

-- For inspection
G1n:GraphWrite("tmp_ftc_g1n.png")
G2n:GraphWrite("tmp_ftc_g2n.png")
G2f:GraphWrite("tmp_ftc_g2f.png")
En:GraphWrite("tmp_ftc_en.png")
Ef:GraphWrite("tmp_ftc_ef.png")
Et:GraphWrite("tmp_ftc_et.png")
Kt:GraphWrite("tmp_ftc_kt.png")
Kn:GraphWrite("tmp_ftc_kn.png")
Kf:GraphWrite("tmp_ftc_kf.png")
Kto:GraphWrite("tmp_ftc_kto.png")


-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------
-- -------------------------------------------------------------------------------

-- Compare with expected result

-- Report
print("###################################################")
print("### validate")

-- -------------------------------------------------------------------------------
-- Input data: expected result
RefKt=faudes.Generator()
RefKt:FromString([[

<Generator>
"fault-tolerant supervisor, as of 20110210"

%  Statistics
% 
%  States:        15
%  Init/Marked:   1/2
%  Events:        7
%  Transitions:   22
%  StateSymbols:  15
%  Attrib. E/S/T: 0/0/0
% 

<Alphabet> alpha1 beta1 alpha2 beta2 gamma2 rho2 fault </Alphabet>

<States>
I|I|N_E       B|I|N_E       I|I|N_F       I|Sf|N_F      I|B|N_E       I|I|F_F      
I|Sf|F_F      I|St|F_F      I|B|F_E       B|B|F_E       I|I|F_E       B|I|F_E      
I|B|F_F       B|B|N_E       I|B|N_F      
</States>

<TransRel>
I|I|N_E       alpha1        B|I|N_E      
B|I|N_E       beta1         I|I|N_F      
I|I|N_F       gamma2        I|Sf|N_F     
I|Sf|N_F      alpha2        I|B|N_E      
I|Sf|N_F      fault         I|I|F_F      
I|B|N_E       alpha1        B|B|N_E      
I|B|N_E       beta2         I|I|N_E      
I|I|F_F       gamma2        I|Sf|F_F     
I|I|F_F       rho2          I|St|F_F     
I|Sf|F_F      alpha2        I|B|F_E      
I|Sf|F_F      fault         I|I|F_F      
I|St|F_F      alpha2        I|B|F_E      
I|B|F_E       alpha1        B|B|F_E      
I|B|F_E       beta2         I|I|F_E      
B|B|F_E       beta1         I|B|F_F      
B|B|F_E       beta2         B|I|F_E      
I|I|F_E       alpha1        B|I|F_E      
B|I|F_E       beta1         I|I|F_F      
I|B|F_F       beta2         I|I|F_F      
B|B|N_E       beta1         I|B|N_F      
B|B|N_E       beta2         B|I|N_E      
I|B|N_F       beta2         I|I|N_F      
</TransRel>

<InitStates>
I|I|N_E      
</InitStates>

<MarkedStates>
I|I|N_E       I|I|F_E      
</MarkedStates>

</Generator>

]])

-- -------------------------------------------------------------------------------
lmatch=faudes.LanguageEquality(RefKt,Kt)
if lmatch then
  print("supervisor Kt matches the expected result")
else
  print("validation failed")
end

-- Done
print("###################################################")
