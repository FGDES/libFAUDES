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

-- trim a rabin automaton

ar:Read("data/omg_rnottrim.gen")
faudes.RabinTrim(ar)
ar:GraphWrite("tmp_omg_rtrim.png")

-- report result
print("=== wrote trimmed automaton to 'tmp_omg_rtrim.png'")

