-- Test/demonstrate generators with colored states

-- ------------------------------------------
-- MtcSystem: construct
-- ------------------------------------------

-- Announce
print("################# Construct an mtc system")

-- Default constructor
mtcsys = faudes.MtcSystem()

-- Have a name
mtcsys:Name("mtc tutorial")

-- Insert some states by name
s1=mtcsys:InsState("down")
s2=mtcsys:InsState("middle")

-- Insert a states with color
s3=mtcsys:InsColoredState("up","low")

-- Retrieve color index 
c1=mtcsys:Colors():Index("low")

-- Insert a color to a state, record color index
c2=mtcsys:InsColor(s1,"high")

-- Insert a color to a state ...
mtcsys:InsColor(s2,c1)

-- ... and get rid of it again
mtcsys:DelColor(s2,c1)

-- Insert some events and transitions
mtcsys:InsEvent("go_up")
mtcsys:InsEvent("go_down")
mtcsys:SetTransition("up","go_down","middle")
mtcsys:SetTransition("middle","go_down","down")
mtcsys:SetTransition("down","go_up","middle")
mtcsys:SetTransition("middle","go_up","up")

-- We still have access to controllability attributes
mtcsys:SetControllable("go_down")
mtcsys:SetControllable("go_up")

-- Report to console
mtcsys:Write()

-- Graph output
mtcsys:GraphWrite('tmp_mtc_demo_gen_1.png')
-- Graph output
mtcsys:DotWrite('tmp_mtc_demo_gen_1.dot')

-- Record test case
FAUDES_TEST_DUMP("mtc system",mtcsys)

-- Test XML IO
mtcsys:XWrite('tmp_mtc_demo_gen_1.ftx')
mtcsys:Read('tmp_mtc_demo_gen_1.ftx')

-- Record test case
FAUDES_TEST_DUMP("mtc system xml io",mtcsys)

-- ------------------------------------------
-- MtcSystem: inspect
-- ------------------------------------------

-- Inspect states
print("Iterate over states")
sit=mtcsys:StatesBegin()
while(sit~=mtcsys:StatesEnd()) do
  print("State:", mtcsys:SStr(sit))
  print("Colors as set:", mtcsys:Colors(sit:Index()))
  print("Colors as attribute:", mtcsys:StateAttribute(sit:Index()))
  sit:Inc()
end


