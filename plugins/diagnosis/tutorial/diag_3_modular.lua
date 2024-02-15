--  Modular diognoser synthesis example (stackfeeder/conveyor belt)

-- Report
print('diagnosis: lua tutorial 3')
print('diagnosis: modular diagnosis example "system sf/c1"')


-- Report
print('diagnosis: a) reading input data')

-- Read nominal plant behaviours to a vector
plant = faudes.SystemVector()
plant:Name('Plant components')
plant:Append('data/diag_system_3_modular_sf.gen')
plant:Append('data/diag_system_3_modular_c1.gen')

-- Read language specification to a vector
spec = faudes.GeneratorVector();
spec:Name('Specifications per component')
spec:Append('data/diag_specification_3_modular_sf.gen')
spec:Append('data/diag_specification_3_modular_c1.gen')

-- Test modular diagnosability
print('diagnosis: b) testing modular diagnosability (expect PASS)')
ok=faudes.IsModularDiagnosable(plant,spec)

-- Report
if ok then
  print("The overall system G is modular diagnosable with respect to overall specification")
else
  print("The overall system G is *NOT* modular diagnosable with respect to overall specification")
end

-- Synthesize modular diagnosers
print('diagnosis: c) synthesising diagnoser moduls')
diag=faudes.GeneratorVector()
faudes.ModularDiagnoser(plant,spec,diag)

-- Report for inspection
print('diagnosis: writing graph output')
diag[0]:GraphWrite('tmp_demo_diagnoser_3_modul_sf.png')
diag[1]:GraphWrite('tmp_demo_diagnoser_3_modul_c1.png')


-- Record test case
FAUDES_TEST_DUMP("modular sys 3",ok)
FAUDES_TEST_DUMP("modular sys 3",diag)
