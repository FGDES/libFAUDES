
S.Perk, Nov 2009

------------------------------------------------------------------------
8 Conveyor Belts, left to right transport, designed with hiosys-approach
------------------------------------------------------------------------

Controllers for the conveyor-belt chain cb11-cb4-cb12-cb5 of the
Fischertechnik model.
This folder contains:
- a script 'run.sh' to execute 
- simfaudes (the binary: 'simfaudes_toy_2009_2.13t') with the 
- device file 'comedi_lrtlab.dev' and with
- an according simulation file 'hio_cbl2r_3SENSORS.sim', and
- a hierarchy of automata for 8 conveyor belts, that, for e.g. the first
  4 conveyor belts, looks as follows:

							ABCD_controller
								  |
				-------------------------------------
				|									|
			AB_controller					CD_controller
				|									|
	  -------------------- 				  --------------------
	  |					 |				  |					 |
A_controller		B_controller	  C_controller		D_controller    
	  |					 |				  |					 |
	_____________________________________________________________________
	|		phys_interface(eg.'phys_A_l_implies_cb11-x-y.gen')			|
	---------------------------------------------------------------------
	  |					 |				  |					 |			|
  plant_cb11		plant_cb4		 plant_cb12			plant_cb5	plant_TICK

The set of files *_controller.gen is the controller hierarchy computed by the hiosys-tutorial
'hio_5_conveyor_belts'. It is supplemented by the manually created files 'A_controller_and_rt1.gen'
and G_controller_and_rt4.gen that implement appropriate rotary table movements.
The physical interface (all files 'phys_*.gen') translates physical sensor edges to abstract inputs for
the controllers 'A_' to 'D_', and translates abstract controller outputs to physical actuator edges.

The lowest level (all files 'plant_*.gen') implements a timed model of the
real-world conveyor-belt chain that is part of the lrtlab Fischertechnik model.
The model 'plant_TICK' implements a passing of time for the controllers 'A_'
to 'D_'(20 time units per wait-loop). This plant layer is realized only for the
above 4 conveyor belts and is ment to be used for SIL-Simulation.
For HIL-Simulation of the controllers for all 8 conveyor belts, the "plant_" layer is
not listed in 'hio_cbl2r_3SENSORS.sim' but replaced by the IO device using the device file
'comedi_lrtlab.dev'.

By the commands ABCDEFGH_FL, ABCDEFGH_TR and ABCDEFGH_STP, the controlled chain of 
conveyor belts takes (and expects) workpieces (wp's) from left (event ABCDEFGH_FL), 
provides workpieces to the right (ABCDEFGH_TR), or remains in standby (ABCDEFGH_STP, 
no wp exchange from/to environment). 
Without further measures, these events and the internal wp transfer are issued 
statistically by simfaudes.
Beyond that, to demonstrate that the controller hierarchy captures all possibilities 
to transport workpieces from left to right, some exemplary operator behaviours are 
provided as 
a) files operator*.gen, which are listed but commented in hio_cbl2r_3SENSORS.sim
   (see beginning of hio_cbl2r_3SENSORS.sim)
b) event priorities, which are listed but commented in hio_cbl2r_3SENSORS.sim
   (see end of hio_cbl2r_3SENSORS.sim)
   
All in all, to run the HIL-simulation do:
- if necessary, copy this folder (/hil_demo/) to the lrtlab-PC TOY
  (running from USB-stick is also possible),
- execute 'run.sh' as user 'toy' on the lrtlab-PC TOY.

SIL-Simulation is prepared for the above 4 conveyor by the timed models 'plant_*.gen':
- include 'plant_*.gen' in the ".sim" file
- comment everything in the '.sim' file that is not related to the first 4 conveyor belts
- run simfaudes, but WITHOUT the device file 'comedi_lrtlab.dev'.
