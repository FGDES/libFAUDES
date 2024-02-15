        --  hio_3_hiofunctions.lua

        -- report
        print('hiosys: lua tutorial 3 - functions')


	-- 
	-- Simple machine example for hiosys-tutorial:
	-- Machine sends YP-event 'A_wait' to operator:
	-- Does nothing after UP-event 'A_stp', or executes
	-- the process 'A_do', which requires a shared
	-- resource from the environment, modelled by
	-- 'A_req': The environment may provide or
	-- deny access to the resource ('A_pack' or
	-- 'A_nack', respectively):
	--
	 
	-- read plant A model
	plantA = faudes.HioPlant('data/3_hiofunctions/hio_simplemachine_A.gen')
	plantA:GraphWrite('tmp_hio_simplemachine_A.png')
	-- must be in I/O plant form
	b=faudes.IsHioPlantForm(plantA)
	if not b then
		print("Error: plantA not in HioPlantForm")
	end

	--
	-- Environment constraint:
	-- Resources are always provided as requested
	--
	-- read plant A environment constraint
	SeA=faudes.HioConstraint('data/3_hiofunctions/hio_simpleEnvConstr_A.gen')
	--should be in I/O constraint form:
	b=faudes.IsHioConstraintForm(SeA)
	if not b then
		print('Warning: environment constraint A not in I/O constraint form:')
	end
	SeA:GraphWrite('tmp_hio_simpleEnvConstr_A.png')
	
	--
	-- Operator constraint:
	-- simple machine is complete and YP-life wrt: to the
	-- environment constraint and a minimal operator
	-- constraint, which - for convenience - can be
	-- modelled by an epsilon language HioConstraint:
	--
	-- construct epsilon language operator constraint
	SpA=faudes.HioConstraint()
	init=SpA:InsInitState()
	SpA:SetMarkedState(init)
	

--============================================================================
--================ CONTROLLER SYNTHESIS FOR MONOLITHIC PLANT A ===============
--============================================================================

        -- report
        print('hiosys: controller synthesis for plant a')

	--
	-- Specification:
	-- Send YC-event 'A_READY' as feedback to operator:
	-- Accept UC-events 'A_STANDBY' or 'A_OPERATE' as 
	-- operator inputs:
	-- Command 'A_STANDBY': do nothing:
	-- Command 'A_OPERATE': Process two resources:
	--
	specA=faudes.HioPlant('data/3_hiofunctions/hio_simplespec_A.gen')
	specA:GraphWrite('tmp_hio_simplespec_A.png')
    
	--
	-- external operator constraint:
	-- also specification A is complete and YP-life wrt: to the
	-- environment constraint and a minimal operator
	-- constraint:
	--
	-- construct epsilon language external operator constraint
	ScA=faudes.HioConstraint()
	init=ScA:InsInitState()
	ScA:SetMarkedState(init)
	
	-- run controller synthesis algorithm
	controllerA=faudes.HioController()
	faudes.HioSynthMonolithic(plantA,specA,ScA,SpA,SeA,controllerA)
        FAUDES_TEST_DUMP("HioSynthMonolithic",controllerA)
	controllerA:Write('tmp_hio_simpleController_A.gen')
	controllerA:GraphWrite('tmp_hio_simpleController_A.png')
	
--============================================================================
--===================== HIERARCHICAL CONTROLLER SYNTHESIS FOR PLANTS AND B ===
--============================================================================
	
        -- report
        print('hiosys: setup plant b data')

	--
	-- Machine B Sends YP-event 'B_wait' to operator:
	-- Does nothing after UP-event 'B_stp', or executes
	-- the process 'B_do', which produces a shared
	-- resource provided to the environment, modelled
	-- by 'B_prov': The environment may or may not
	-- accept the resource ('B_pack' or 'B_nack',
	-- respectively):
	--
	 
	-- read plant B model
        plantB=faudes.HioPlant('data/3_hiofunctions/hio_simplemachine_B.gen')
	plantB:GraphWrite('tmp_hio_simplemachine_B.png')
	-- must be in I/O plant form
	b=faudes.IsHioPlantForm(plantB)
	if not b then
		print("Error: plantB not in HioPlantForm")
	end
	
	--
	-- Environment constraint:
	-- Resources are always provided as requested:
	--
	-- read plant A environment constraint
	SeB=faudes.HioConstraint('data/3_hiofunctions/hio_simpleEnvConstr_B.gen')
	b=faudes.IsHioConstraintForm(SeB)
	if not b then
		print('Warning: environment constraint B not in I/O constraint form.')
	end
	SeB:GraphWrite('tmp_hio_simpleEnvConstr_B.png')
	
	--
	-- Operator constraint:
	-- simple machine B is complete and YP-life wrt: to the
	-- environment constraint and a minimal operator
	-- constraint, which - for convenience - can be
	-- modelled by an epsilon language HioConstraint:
	--
	-- construct epsilon language operator constraint
	SpB=faudes.HioConstraint()
	init=SpB:InsInitState()
	SpB:SetMarkedState(init)
	
	--===============================================
	--I/O SHUFFLE OF PLANT A AND PLANT B
	--===============================================

        -- report
        print('hiosys: shuffle plant a and b')

	ioShuffleAB=faudes.HioPlant()
	faudes.HioShuffle(plantA,plantB,ioShuffleAB)
        FAUDES_TEST_DUMP("HioShuffle",ioShuffleAB)  
	ioShuffleAB:Write('tmp_hio_simpleHioShuffle_AB.gen')
	ioShuffleAB:GraphWrite('tmp_hio_simpleHioShuffle_AB.png')
	
	-- combined constraints of plant A and B:
	-- Internal constraint for plant A and B:
	-- Combination of constraints of plant A and B
	-- results in the exclusion of A_nack and B_nack
	intConstrAB=faudes.Generator('data/3_hiofunctions/hio_simpleIntConstr_AB.gen')
	
	--===============================================
	--ENVIRONMENT MODEL FOR PLANT AB
	--=================================================--
	--
	-- Simple machine example for hiosys-tutorial:
	-- Environment:
	-- A request of machine A (A_req) for a resource is
	-- denied (A_nack) until machine B provides it
	-- (B_prov, B_pack): Then, machine A has to request
	-- the resource until machine B can provide the next
	-- one: A resource provided by machine B and
	-- requested by machine A is readily processed
	-- and can be provided to the external
	-- environment (AB_prov), which may or may not
	-- accept the processed resource (AB_pack,
	-- AB_nack):
	--
	envAB=faudes.HioEnvironment('data/3_hiofunctions/hio_simpleenvironment_AB.gen')
	envAB:GraphWrite('tmp_hio_simpleenvironment_AB.png')
	
	--
	-- Environment constraint:
	-- Processed resources are always accepted as provided:
	--
	-- read plant A environment constraint
	SlAB=faudes.HioConstraint('data/3_hiofunctions/hio_simpleEnvConstr_AB.gen')
	b=faudes.IsHioConstraintForm(SlAB)
	if not b then
		print('Warning: environment constraint AB not in I/O constraint form.')
	end
	SlAB:GraphWrite('tmp_hio_simpleEnvConstr_AB.png')
	
	--====================================================
	--SPECIFICATION AND EXTERNAL OPERATOR CONSTRAINT
	--======================================================--	
	--
	-- Simple machine example for hiosys-tutorial:
	-- Specification:
	-- Send YC-event 'AB_READY' as feedback to operator:
	-- Accept UC-events 'AB_STANDBY' or 'AB_OPERATE' as 
	-- operator inputs:
	-- Command 'AB_STANDBY': do nothing:
	-- Command 'AB_OPERATE': Provide processed resource (AB_prov):
	 --
	specAB=faudes.HioPlant('data/3_hiofunctions/hio_simplespec_AB.gen')
	specAB:GraphWrite('tmp_hio_simplespec_AB.png')
    
	--
	-- external operator constraint:
	-- also specification AB is complete and YP-life wrt: to the
	-- environment constraint and a minimal operator
	-- constraint:
	--
	-- construct epsilon language external operator constraint
	ScAB=faudes.HioConstraint()
	init=ScAB:InsInitState()
	ScAB:SetMarkedState(init)
	
        -- report
        print('hiosys: controller synthesis for plants a and b')

	-- run controller synthesis algorithm
	controllerAB=faudes.HioController()
	faudes.HioSynthHierarchical(ioShuffleAB,envAB,specAB,intConstrAB,ScAB,SlAB,controllerAB)
        FAUDES_TEST_DUMP("HioSynthHierarchical",controllerAB)
	controllerAB:Write('tmp_hio_simpleController_AB.gen')
	controllerAB:GraphWrite('tmp_hio_simpleController_AB.png')

        -- report
        print('statistics:')
        controllerAB:SWrite()

