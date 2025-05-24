-- elevator_synthesis.lua
--
-- This lua-script is part of the HIL-project "elevator". It computes
-- a monolithic supervisor to operate the elevator.


-------------------------------------------------------------------------------
-- read files
-------------------------------------------------------------------------------

-- report
print("elevator: reading plant and specification data")

-- load plant components from file
plant_cabin    = faudes.System("data/elevator_plant_cabin.gen")
plant_door     = faudes.System("data/elevator_plant_door.gen")
plant_lbarrier = faudes.System("data/elevator_plant_lbarrier.gen")
plant_buttons  = faudes.System("data/elevator_plant_buttons.gen")
plant_leds     = faudes.System("data/elevator_plant_leds.gen")

-- load specification components from file
spec_opcmd   = faudes.Generator("data/elevator_spec_opcmd.gen")
spec_opcmds  = faudes.Generator("data/elevator_spec_opcmds.gen")
spec_opled   = faudes.Generator("data/elevator_spec_opled.gen")
spec_cabmot  = faudes.Generator("data/elevator_spec_cabmot.gen")
spec_cabmots = faudes.Generator("data/elevator_spec_cabmots.gen")
spec_drmot1  = faudes.Generator("data/elevator_spec_drmot1.gen")
spec_drsaf1  = faudes.Generator("data/elevator_spec_drsaf1.gen")
spec_drsaf2  = faudes.Generator("data/elevator_spec_drsaf2.gen")
spec_scheds  = faudes.Generator("data/elevator_spec_scheds.gen")

-- dont name states
faudes.StateNamesOff();


-------------------------------------------------------------------------------
-- version 1: simple case, ignore door and button lights
-------------------------------------------------------------------------------

-- report
print("elevator: build pant model")

-- prepare plant model
plant_core = faudes.System() 
faudes.FullLanguage(faudes.EventSet(),plant_core)

-- build core model consisting of cabin, door and operator
faudes.Parallel(plant_core,plant_cabin,plant_core)
faudes.Parallel(plant_core,plant_buttons,plant_core)

-- have a sensible name
plant_core:Name("elevator core plant (cabin plus buttons)")

-- write to file
plant_core:Write("tmp_elevator_plant_core.gen")

-- report
print("elevator: build specification")

-- compose overall specification
spec_core = faudes.Generator()
faudes.FullLanguage(faudes.EventSet(),spec_core)
faudes.Parallel(spec_core,spec_opcmd,spec_core)
faudes.Parallel(spec_core,spec_cabmot,spec_core)

-- write to file
spec_core:Write("tmp_elevator_spec_core.gen")

-- match plant and spec alphabets
alphabet_core = faudes.EventSet();
alphabet_plant_extra = faudes.EventSet();
faudes.AlphabetUnion(plant_core:Alphabet(),spec_core:Alphabet(),alphabet_core);
faudes.AlphabetDifference(alphabet_core,plant_core:Alphabet(),alphabet_plant_extra);
faudes.InvProject(spec_core,alphabet_core);
faudes.InvProject(plant_core,alphabet_core);

-- extra events in the specification are controllable
plant_core:SetControllable(alphabet_plant_extra);

-- report
print("elevator: synthesis")

-- compute supervisor for core elevator setup
super_core = faudes.System();
super_core_min = faudes.System();
faudes.SupCon(plant_core,spec_core,super_core)
super_core:Name("elevator supervisor (core)")
faudes.StateMin(super_core,super_core_min)

-- write to file
super_core_min:Write("tmp_elevator_super_core.gen")

-- report
print("elevator: core supervisor statistics")
super_core_min:SWrite()

-- record test case
FAUDES_TEST_DUMP("simple",super_core_min)
 
-------------------------------------------------------------------------------
-- version 2: full version, incl door and button lights
-------------------------------------------------------------------------------

-- report
print("elevator: build pant model")

-- prepare plant model
plant_full = faudes.System() 
faudes.FullLanguage(faudes.EventSet(),plant_full)
plant_full_min = faudes.System()

-- build full model consisting of cabin, door and operator
faudes.Parallel(plant_full,plant_cabin,plant_full)
faudes.Parallel(plant_full,plant_door,plant_full)
faudes.Parallel(plant_full,plant_lbarrier,plant_full)
faudes.Parallel(plant_full,plant_buttons,plant_full)
faudes.Parallel(plant_full,plant_leds,plant_full)

-- have a sensible name
plant_full:Name("elevator full plant (cabin plus buttons)")

-- get eventset of plant model
plant_full_alphabet = faudes.EventSet(plant_full:Alphabet())

-- reduce state space 
faudes.StateMin(plant_full,plant_full_min)

-- write to file
plant_full_min:Write("tmp_elevator_plant_full.gen")

-- report
print("elevator: build specification")

-- prepare overall specification
spec_full = faudes.Generator()
faudes.FullLanguage(faudes.EventSet(),spec_full)
spec_full_min = faudes.Generator()

-- compose overall specification
faudes.Parallel(spec_full,spec_opcmds,spec_full)
faudes.Parallel(spec_full,spec_scheds,spec_full)
faudes.Parallel(spec_full,spec_opled,spec_full)
faudes.Parallel(spec_full,spec_cabmots,spec_full)
faudes.Parallel(spec_full,spec_drmot1,spec_full)
faudes.Parallel(spec_full,spec_drsaf1,spec_full)
faudes.Parallel(spec_full,spec_drsaf2,spec_full)
faudes.StateMin(spec_full,spec_full_min)

-- have a sensible name
spec_full:Name("elevator full specification (cabin plus buttons)")

-- write to file
spec_full_min:Write("tmp_elevator_spec_full.gen")

-- report alphabets
print("plant alphabet")
plant_full:Alphabet():Write()
print("spec alphabet")
spec_full:Alphabet():Write()

-- match plant and spec alphabets
alphabet_full = faudes.EventSet();
alphabet_plant_extra = faudes.EventSet();
faudes.AlphabetUnion(plant_full:Alphabet(),spec_full:Alphabet(),alphabet_full);
faudes.AlphabetDifference(alphabet_full,plant_full:Alphabet(),alphabet_plant_extra);
faudes.InvProject(spec_full_min,alphabet_full);
faudes.InvProject(plant_full_min,alphabet_full);

-- extra events in the specification are controllable
plant_full_min:SetControllable(alphabet_plant_extra);

-- report
print("elevator: synthesis")

-- compute supervisor for full elevator setup
super_full = faudes.System();
super_full_min = faudes.System();
faudes.SupCon(plant_full_min,spec_full_min,super_full)
super_full:Name("elevator supervisor (full)")
faudes.StateMin(super_full,super_full_min)

-- write to file
super_full_min:Write("tmp_elevator_super_full.gen")

-- report
print("elevator: full supervisor statistics")
super_full_min:SWrite()

-- record test case
FAUDES_TEST_DUMP("simple",super_full_min)

 
