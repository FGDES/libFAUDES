-- Test/demonstrate core faudes pushdown generators

-- ------------------------------------------
-- PushdownGenerator: construct controller
-- Example 1
-- ------------------------------------------

-- Announce
print("################# Construct a system")

-- Create plain generator
sys = faudes.System()

-- Have a name
sys:Name("example plant")

-- Insert some states by name
sys:InsState("s0")
sys:InsState("s1")

-- Indicate initial and marked states
sys:SetInitState("s0")
sys:SetMarkedState("s1")

-- Insert events
sys:InsControllableEvent("a")
sys:InsControllableEvent("b")


-- Insert transitions by names
sys:SetTransition("s0","a","s0")
sys:SetTransition("s0","b","s1")
sys:SetTransition("s1","b","s1")

-- Print dot
sys:DotWrite("tmp__pd_luatutorial_sys.dot")


print()

-- Announce
print("################# Construct a spezification")


-- Create plain pushdowngenerator
pd = faudes.PushdownGenerator()

-- Have a name
pd:Name("example spezification")

-- Insert some states by name
pd:InsState("p0")
pd:InsState("p1")

-- Indicate initial and marked states
pd:SetInitState("p0")
pd:SetMarkedState("p0")

-- Insert events
pd:InsControllableEvent("a")
pd:InsControllableEvent("b")
pd:InsEvent("lambda")

-- Insert stack symbold
pd:InsStackSymbol("A")
pd:InsStackSymbol("B")
pd:InsStackSymbol("$")

-- Indicate stack bottom
pd:SetStackBottom("$")

-- Insert lambda stack symbol for empty pop or push
pd:InsLambdaStackSymbol()

-- Insert transitions by names
pd:SetTransition("p0","a","[$]","[A,$]","p1")
pd:SetTransition("p0","b","[$]","[B,$]","p1")

pd:SetTransition("p1","a","[A]","[A,A]","p1")
pd:SetTransition("p1","a","[B]","[]","p1")
pd:SetTransition("p1","b","[A]","[]","p1")
pd:SetTransition("p1","b","[B]","[B,B]","p1")

pd:SetTransition("p1","lambda","[$]","[$]","p0")

-- Print 
pd:DotWrite("tmp__pd_luatutorial_spec.dot")


-- ---------------------------------------
-- construct controller
-- ---------------------------------------
print("################# Construct Controller")
-- Create plain pushdowngenerator for result
res = faudes.PushdownGenerator()

-- construct controller
faudes.PushdownConstructController(pd,sys,res)

res:Write()
res:DotWrite("tmp__pd_luatutorial_controller.dot")


-- ------------------------------------------
-- PushdownGenerator: construct controller
-- Example 2
-- ------------------------------------------

print("################# Construct Controller from dot files")

-- parse Dot files
sys2 = faudes.SystemFromDot("data/graph/pd_tut_plant.dot")
pd2 = faudes.PushdownGenerator("data/graph/pd_tut_spec.dot")

-- construct controller
faudes.PushdownConstructController(pd,sys,res)

res:Write()
res:DotWrite("tmp__pd_luatutorial_controllerFromDot.dot")


