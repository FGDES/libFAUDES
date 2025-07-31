-- Tutorial on  Rabin automata

-- ----------------------------
-- Rabin pair demo
-- ----------------------------

-- have some state sets to play with
sA=faudes.StateSet();
sB=faudes.StateSet();
sC=faudes.StateSet();
sA:FromString('<S> 1 2 3 </S>');
sB:FromString('<S> 4 5 6 </S>');
sC:FromString('<S> 7 8 9 </S>');

-- set up a Rabin pair
rpair=faudes.RabinPair()
rpair:RSet():InsertSet(sA)
rpair:ISet():InsertSet(sA + sB)

-- manipulate Rabin pair
rpair:ISet():Erase(5)
  
-- report/record result
print("=== a Rabin pair")
rpair:Write()
FAUDES_TEST_DUMP("a Rabin pair",rpair)


-- ----------------------------
-- Rabin acceptance condition demo
-- ----------------------------

-- instantiate
raccept=faudes.RabinAcceptance()

-- edit
raccept:Append(rpair)
rpair:Clear()
rpair:RSet():InsertSet(sC)
rpair:ISet():InsertSet(sC + sB)
raccept:Append(rpair)
raccept:At(0):ISet():Insert(77)

-- report/record result
print("=== a Rabin acceptance condition")
raccept:Write()
FAUDES_TEST_DUMP("a Rabin accept.cond.",raccept)


-- ----------------------------
-- Rabin automaton demo
-- ----------------------------

-- instantiate from file and show
ar= faudes.RabinAutomaton("data/omg_rabinaut.gen")
br=ar:Copy()
br:InsState(77)
br:RabinAcceptance():At(0):RSet():Insert(77)

-- report/record result
print("=== two Rabin automata")
br:Write()
ar:Write()
FAUDES_TEST_DUMP("first Rabin aut.",ar)
FAUDES_TEST_DUMP("second Rabin aut.",br)


-- ----------------------------
-- trim Rabin automaton
-- ----------------------------

-- trim rabin automaton
print("=== trimming/liveness")
ar:Read("data/omg_rnottrim.gen")
arl=faudes.IsRabinTrim(ar)
if arl then
  print("automata is trim (test case ERROR)")
else
  print("automata is not trim (expected)")
end
faudes.RabinTrim(ar)
arlt=faudes.IsRabinTrim(ar)
if arlt then
  print("automata is trim (expected)")
else
  print("automata is not trim (test case Error)")
end
ar:GraphWrite("tmp_omg_rtrim.png")

-- report result
print("=== wrote trimmed automaton to 'tmp_omg_rtrim.png'")

FAUDES_TEST_DUMP("trimmed aut.",ar)
FAUDES_TEST_DUMP("trim test 1",arl)
FAUDES_TEST_DUMP("trim test 2",arlt)


-- ----------------------------
-- CY reported error as test case 
-- ----------------------------

-- trim rabin automaton
print("=== CY test case")
ar:Read("data/omg_cytrim.gen")
ar:GraphWrite("tmp_omg_cytrim.png")
arl=faudes.IsRabinTrim(ar)
if arl then
  print("automata is trim (expected)")
else
  print("automata is not trim (test case ERROR)")
end
faudes.RabinTrim(ar)
ar:GraphWrite("tmp_omg_cytrimt.png")

FAUDES_TEST_DUMP("trim test 3",arl)

