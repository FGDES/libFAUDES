-- Test/demonstrate I/O systems

-- Optionally, turn on faudes debugging messages
faudes.Verbosity(0)

-- ------------------------------------------
-- IoSystem basic maintenance
-- ------------------------------------------

-- Announce
print("################# IoSystem basic maintenance")

-- Construct from file
sys = faudes.IoSystem("data/ios_system1.gen")

-- Report inputs and outputs
print("Input events: ", sys:InputEvents());
print("Output events: ", sys:OutputEvents());
sys:SWrite()

-- Graphical output
sys:GraphWrite("tmp_ios_system1.png")

-- Have graphical of tank system for html docu
tank = faudes.IoSystem("data/ios_tank.gen")
tank:GraphWrite("tmp_ios_tank.png")


-- Record test case
FAUDES_TEST_DUMP("system1",sys)



-- ------------------------------------------
-- Access individual attributes
-- ------------------------------------------

print("################# IoSystm access attributes")

-- Test
if sys:InputEvent("u1") then
  print("'u1' is an input event of ", sys:Name(),  "(expected)")
end
if sys:OutputEvent("u1") then
  print("'u1' is an output event of ", sys:Name(), "(test case error!)")
end

-- Manipulate 
sys:SetOutputEvent("u1")
sys:SetErrorState("s1")
sys:ClrOutputEvent("y1")

-- Report
print("Input events: ", sys:InputEvents());
print("Output events: ", sys:OutputEvents());
print("Error states: ", sys:StateSetToString(sys:ErrorStates()));

-- fix system1
sys = faudes.IoSystem("data/ios_system1.gen")

-- ------------------------------------------
-- Testing I/O properties
-- ------------------------------------------

print("################# Test I/O properties: IOSystem1")

-- basic I/O
if faudes.IsIoSystem(sys) then
  print("Basic I/O properties: ok")
else
  print("Basic I/O properties: failed");
  print("Error states: ", sys:StateSetToString(sys:ErrorStates()));
end

-- input locally free
if faudes.IsInputLocallyFree(sys) then
  print("Input locally free: ok");
else
  print("Input locally free: failed");
  print("Error states: ", sys:StateSetToString(sys:ErrorStates()));
end

-- input omega freey
if faudes.IsInputOmegaFree(sys) then
  print("Input omega-free: ok");
else
  print("Input omega-free: failed");
  print("Error states: ", sys:StateSetToString(sys:ErrorStates()));
end

-- test XML token IO
sys:XWrite('tmp_ios_sys.ftx')
sys:Read('tmp_ios_sys.ftx')

-- record test case
FAUDES_TEST_DUMP("xml io",sys)

print("################# Test I/O properties: IOTank")

-- have a state partition
faudes.IoStatePartition(tank)
print("Input states: ", tank:StateSetToString(tank:InputStates()));
print("Output states: ", tank:StateSetToString(tank:OutputStates()));

-- basic I/0
if faudes.IsIoSystem(tank) then
  print("Basic I/O properties: ok (expected)")
else
  print("Basic I/O properties: failed (test case error!)");
  print("Error states: ", tank:StateSetToString(tank:ErrorStates()));
end

-- input locally free
if faudes.IsInputLocallyFree(tank) then
  print("Input locally free: ok (test case error!)");
else
  print("Input locally free: failed (expected)");
  print("Error states: ", tank:StateSetToString(tank:ErrorStates()));
end

-- fix input locally
faudes.IoFreeInput(tank)
tank:GraphWrite("tmp_ios_tank_ifree.png")
if faudes.IsInputLocallyFree(tank) then
  print("Input locally free: ok (expected)");
else
  print("Input locally free: failed (test case error!)");
end

-- input omega free
if faudes.IsInputOmegaFree(tank) then
  print("Input omega free: ok (expected)");
else
  print("Input omega free: failed (test case error)");
  print("Error states: ", tank:StateSetToString(tank:ErrorStates()));
end

-- remove dummy states
tank2=faudes.IoSystem(tank);
faudes.RemoveIoDummyStates(tank2);
tank2:GraphWrite("tmp_ios_tank_reverted.png")





