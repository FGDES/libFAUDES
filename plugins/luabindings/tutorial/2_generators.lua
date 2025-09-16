-- Test/demonstrate core faudes generators

-- ------------------------------------------
-- Generator: construct
-- ------------------------------------------

-- Announce
print("################# Construct a generator")

-- Create plain generator
gen = faudes.Generator()

-- Have a name
gen:Name("simple machine")

-- Insert some states by name
gen:InsState("idle")
gen:InsState("busy")

-- Insert anonymous states 
didx = gen:InsState()

-- Set name of anonymous state
gen:StateName(didx,"down")

-- Insert some events
gen:InsEvent("alpha")
gen:InsEvent("beta")
gen:InsEvent("mue")
gen:InsEvent("lambda")

-- Insert some transitions by names
gen:SetTransition("idle","alpha","busy")
gen:SetTransition("busy","beta","idle")
gen:SetTransition("busy","mue","down")

-- Construct transition and insert
trans=faudes.Transition()
trans.X1=gen:StateIndex("down")
trans.Ev=gen:EventIndex("lambda")
trans.X2=gen:StateIndex("idle")
print("Insert another transition: ",gen:TStr(trans))
gen:SetTransition(trans)

-- Indicate initial and marked states
gen:SetInitState("idle")
gen:SetMarkedState("idle")

-- Print 
gen:Write()

-- Record test case
FAUDES_TEST_DUMP("generator",gen)


-- ------------------------------------------
-- Generator: graphical output
-- ------------------------------------------

-- Announce graph output
print("################# Running Graphviz/dot")

-- Set dot path (or specify in PATH environment variable)
-- eg MacOS -- faudes.DotExecPath("/Applications/Graphviz.app/Contents/MacOS/dot")
-- eg linux -- faudes.DotExecPath("dot")
-- eg MsWin -- faudes.DotExecPath("c:\\Programs\Graphviz\dot")

-- Run dot
gen:GraphWrite("tmp_simplemachine.jpg")

-- Comments: alternative output formats include <tt>*.png</tt>, <tt>*.svg</tt> 
-- and <tt>*.dot</tt>. The latter is particular useful to apply advanced dot-processing 
-- options to affect e.g. the font or resolution.

-- ------------------------------------------
-- System: generator with contr. attributes
-- ------------------------------------------

-- Announce
print("################# Construct a system")

-- Initialize system from generator
sys=faudes.System(gen)
sys:Name("simple machine plant model")

-- Have some controllable events
sys:SetControllable("alpha")
sys:SetControllable("lambda")

-- Report
sys:Write()

-- Record test case
FAUDES_TEST_DUMP("system",sys)


-- ------------------------------------------
-- Generator: inspect
-- ------------------------------------------

-- Announce 
print("################# Container access")

-- Entire alphabet (retrieve reference)
alph = gen:Alphabet()
alph:Write()
-- alph:Clear() -- this will break the generator (!)

-- Entire alphabet (retrieve copy)
alph = gen:Alphabet():Copy()
alph:Write()
alph:Clear()    -- this is ok since alph is a copy

-- Inspect alphabet (iterate)
print("Iterate over events")
eit=gen:AlphabetBegin()
while(eit~=gen:AlphabetEnd()) do
  print("Event:", gen:EStr(eit))
  eit:Inc()
end

-- Entire stateset (retrieve reference)
states=gen:States()
states:Write()

-- Inspect states
print("Iterate over states")
sit=gen:StatesBegin()
while(sit~=gen:StatesEnd()) do
  print("State:", gen:SStr(sit))
  sit:Inc()
end

-- Inspect transitions
print("Iterate over transitions")
tit=gen:TransRelBegin()
while(tit~=gen:TransRelEnd()) do
  print("Transition:", gen:TStr(tit))
  tit:Inc()
end

-- Entire alphabet with attributes (retrieve reference)
print("Alphabet of system incl. attributes")
alph=sys:Alphabet()
alph:Write();

-- Entire alphabet with attributes (retrieve a copy)
print("Alphabet of system incl. attributes (copy)")
alph=sys:Alphabet():Copy()
alph:Write()
alph:Clear()  -- this is ok, since alph is a copy
print("Expect 4 events with 2 attributes")
sys:SWrite()

-- Record test case
FAUDES_TEST_DUMP("system",sys)


-- ------------------------------------------
-- Generator: example algorithm reachable states
-- ------------------------------------------

-- say hello
print("Computing reachable states for the simple machine")

-- initialize loop variable to accumulate the result
reach_acc  = gen:InitStates():Copy()   -- copy required to keep generator

-- traverse newly identified states
reach_prev = reach_acc:Copy()          -- copy by design pattern, not required here
while not reach_prev:Empty() do        -- iterate while new reachable states identified
  reach_next = faudes.IndexSet()
  sit=reach_prev:Begin()               -- test each newly identified state ...
  while(sit~=reach_prev:End()) do
    q1=sit:Index()
    tit=gen:TransRelBegin(q1) 
    while tit~=gen:TransRelEnd(q1) do  -- ... for successor states ...
      q2=tit:X2()
      if not reach_acc:Exists(q2) then -- ... that have not be identifies yet ...
        reach_next:Insert(q2)          -- ... and accumulate newly identified successors
      end
      tit:Inc()
    end
    sit:Inc()
  end
  reach_acc:InsertSet(reach_next)      -- accumulate result
  reach_prev=reach_next                -- pass on recent states by reference 
end


-- ------------------------------------------
-- Generator: relabel events
-- ------------------------------------------

rlmap=faudes.RelabelMap()
rlmap:Insert("alpha","alpha_2")
rlmap:Insert("alpha","alpha_x")
rlmap:Insert("beta","beta_2")
faudes.ApplyRelabelMap(rlmap,sys,sys)
print("relabeled system")
sys:DWrite()




-- log test case
FAUDES_TEST_DUMP("reach expect true", reach_acc == gen:AccessibleSet() )

-- show result (indices still refer to the generator)
print("Reachable states")
reach_acc:Name("States")
gen:WriteStateSet(reach_acc)