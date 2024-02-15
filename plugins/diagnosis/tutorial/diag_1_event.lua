--  Diognoser synthesis example

-- Report
print('diagnosis: lua tutorial 1')
print('diagnosis: event diagnosis example')

-- Read nominal plant behaviour
nomdyn = faudes.System('data/diag_system_3.gen')
print('diagnosis: nominal plant dynamics from file')
nomdyn:Write()

-- Read failure partition and report
fmap = faudes.FailureTypeMap('data/diag_failure_typemap_3.txt')
print('diagnosis: failure type map from file')
fmap:Write()

-- Compute diagnoser
print('diagnosis: diagnoser synthesis')
diag=faudes.Diagnoser()
faudes.EventDiagnoser(nomdyn,fmap,diag)

-- Report result
print('diagnosis: result') 
diag:Write() 

diag:GraphWrite('tmp_diagnoser_1_lua.svg')
print('diagnosis: statistics (expect 5 states)') 
diag:SWrite()

-- Record test case
FAUDES_TEST_DUMP("diagnoser",diag)

-- Test XML IO
diag:XWrite("tmp_diag_1_xml.ftx") 
diag:Read("tmp_diag_1_xml.ftx") 
diag:XWrite()

-- Include XML IO as testcase
FAUDES_TEST_DUMP("diagnoser",diag)

