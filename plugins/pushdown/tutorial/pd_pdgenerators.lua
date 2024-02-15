-- Test/demonstrate core faudes pushdown generators

-- ------------------------------------------
-- PushdownGenerator: construct
-- ------------------------------------------

-- Announce
print("################# Construct a pushdown generator")


-- Create plain pushdowngenerator
pd = faudes.PushdownGenerator()

-- Have a name
pd:Name("example pushdown")

-- Insert some states by name
pd:InsState("idle")
pd:InsState("busy")

-- Insert anonymous states 
didx = pd:InsState()

-- Indicate initial and marked states
pd:SetInitState("idle")
pd:SetMarkedState("idle")

-- Set name of anonymous state
pd:StateName(didx,"down")

-- Insert some events
pd:InsEvent("alpha")
pd:InsEvent("beta")
mue = pd:InsEvent("mue")
pd:InsEvent("lambda")

-- Insert some stack symbold
pd:InsStackSymbol("A")
pd:InsStackSymbol("B")
pd:InsStackSymbol("$")

-- Insert lambda stack symbol for empty pop or push
pd:InsLambdaStackSymbol()

-- Insert some transitions by names
pop = "[$]"
push = "[A, $]"

pd:SetTransition("busy","beta",pop,push,"idle")
pd:SetTransition("busy","mue","[A]","[]","idle")

exist = pd:ExistsTransition("busy","beta",pop,push,"idle")
clr = pd:ClrTransition("busy","beta",pop,push,"idle")
exist2 = pd:ExistsTransition("busy","beta",pop,push,"idle")

-- Indicate stack bottom
pd:SetStackBottom("$")

-- Print 
pd:Write()


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
pd:GraphWrite("tmp_examplepushdown.png")

-- Comments: alternative output formats include <tt>*.png</tt>, <tt>*.svg</tt> 
-- and <tt>*.dot</tt>. The latter is partivular useful to apply advanced dot-processing 
-- options to affect e.g. the font or resolution.
