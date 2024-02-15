--  Language diagnoser examples

-- ----------------------------------------------------------------
-- Example that fails to be lang.-diagnosable
-- ----------------------------------------------------------------

-- Report
print('diagnosis: lua tutorial 2')
print('diagnosis: language diagnosis example "system 2"')

-- Read input data
gen=faudes.System('data/diag_system_2_language.gen')
spec=faudes.Generator('data/diag_specification_2_language.gen')

-- Write input data (for HTML docu)
gen:Write("tmp_diag_system_2_language.gen") 
spec:Write("tmp_diag_specification_2_language.gen")

-- Run test
print('diagnosis: run language-diagnosability test (expect FAIL)')
ok=faudes.IsLanguageDiagnosable(gen,spec)

-- Report
if ok then
  print('diagnosis: System is diagnosable.')
else
  print('diagnosis: System is not diagnosable.')
end

-- Synthesise diagnoser
print('diagnosis: run language-diagnoser synthesis (unsuccessful diagnoser)')
diag=faudes.Diagnoser()
faudes.LanguageDiagnoser(gen,spec,diag)

-- Write result
diag:Write("tmp_diag_diagnoser_2.gen");

-- Record test case
FAUDES_TEST_DUMP("language sys 2",ok)
FAUDES_TEST_DUMP("language sys 2",diag)


-- ----------------------------------------------------------------
-- Example for abstraction based language diagnosability
-- ----------------------------------------------------------------

-- Report
print('diagnosis: abstraction based diagnosis')

-- Read input data
gen=faudes.System('data/diag_system_2_abstraction_fails.gen')
halph=faudes.EventSet('data/diag_abstrAlph_2_abstraction_fails.alph')

-- Write input data (for HTML docu)
gen:Write("tmp_diag_system_2_abstraction_fails.gen") 

-- Run test
print('diagnosis: run loop preserving test (expect FAIL)')
ok=faudes.IsLoopPreservingObserver(gen,halph)

-- Report
if ok then
  print('diagnosis: Alphabet is loop preserving.')
else
  print('diagnosis: Alphabet is not loop preserving.')
end

-- Construct high level alphabet
print('diagnosis: construct high level alphabet')
hhalph=faudes.EventSet()
faudes.LoopPreservingObserver(gen,halph,hhalph)

-- Report
print('diagnosis: new high level alphabet:')
hhalph:Write()

-- Record test case
FAUDES_TEST_DUMP("high alph",hhalph)
