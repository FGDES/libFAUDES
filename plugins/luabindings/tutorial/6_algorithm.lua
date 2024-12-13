-- Demo for implementing synthesis algorithms in luaFAUDES, tmoor, 2024

-- This script implements a variation of the common synthesis algorithm
-- to address event priorities. For each event, we associate a priority
-- by which it is executed in closed-loop configuration. Specifically,
-- uncontrollable ecents will be preempted by events with higher priority.


function faudes.SupPconNB(GPlant,ACtrl,TPrios,GSpec,GLoop)

  -- Parameters
  -- GPlant       plant model
  -- ACtrl        controllable events
  -- TPrios       table of event priorities
  -- GSpec        specification
  -- GLoop        closed-loop behaviour (result)

  -- Parameter Conditions
  -- GPlant and GSpec must be deterministic, and so will be the result
  -- TPrios is a Lua table to map events to a non-negative integer priority,
  -- (events not listed in the priorities table default to priority 0)

  -- prepare result
  local plant = GPlant:Copy()
  local spec = GSpec:Copy()
  local sigma_f = plant:Alphabet() + spec:Alphabet()
  faudes.InvProject(plant, sigma_f)
  faudes.InvProject(spec, sigma_f)

  -- closed-loop candidate
  local pcmap=faudes.ProductCompositionMap()
  faudes.Product(GPlant,GSpec,pcmap,GLoop)

  -- iteratively delete undesired states
  local delstates=faudes.IndexSet()
  repeat
    -- loop all candidate states
    delstates:Clear()
    local sit = GLoop:StatesBegin()
    local sit_end = GLoop:StatesEnd()
    while sit ~= sit_end do
      -- get both plant and loop indicees
      local lx = sit:Index()
      local px = pcmap:Arg1State(lx)
      print("testing loop state", GLoop:StateName(lx))
      sit:Inc()
      -- figure highest priority of event enabled in loop
      local lenabled=faudes.EventSet()
      local lmaxp=-1
      local lmaxe=nil
      local tit = GLoop:TransRelBegin(lx)
      local tit_end = GLoop:TransRelEnd(lx)
      while tit ~= tit_end do
        local ev= GLoop:EventName(tit:Ev())
	lenabled:Insert(ev)
        local p = TPrios[ev]
	if p == nil then p=0 end
	if p > lmaxp then lmaxp=p lmaxe=ev end
	tit:Inc()
      end
      print("highest priority loop event:",lmaxe,lmaxp)
      -- figure highest priority uncontrollable event disabled in plant 
      local pmaxp= -1
      local pmaxe=nil
      local tit = GPlant:TransRelBegin(px)
      local tit_end = GPlant:TransRelEnd(px)
      while tit ~= tit_end do
        local ev= GPlant:EventName(tit:Ev())
 	if not lenabled:Exists(ev) then
 	  if not ACtrl:Exists(ev) then
            local p = TPrios[ev]
  	    if p == nil then p=0 end
 	    if p > pmaxp then pmaxp=p pmaxe=ev end
	  end
	end  
	tit:Inc()
      end
      print("highest priority disabled uncontrollabe plant event:",pmaxe,pmaxp)
      -- remove the loop state if disabled unctrollable events are not preempted
      if pmaxp >= 0 and lmaxp < pmaxp then
        print("schedule delete",GLoop:StateName(lx))
        delstates:Insert(lx)
      end	     
    end
    -- delete states and unblock
    GLoop:DelStates(delstates)
    GLoop:Trim()
    -- terminate on fixpoint
  until delstates:Empty()



end

-- --------------------------------------------------------------------------------
-- Example:
-- Elevate a cabin (m_on) until the keyswitch in 1st floor becomes active (c_ar),
-- and stop (m_off) before the keyswitch becomes passive (c_lv); the actuator events
-- m_on and m_moff will be executed with higher priority and thus preempt the sensor
-- events c_ar and c_lv. Specifically, after c_ar we can preempt c_lv by m_off.
-- --------------------------------------------------------------------------------

-- plant model: motor can be on or off, cabin can be in basement, first or second floor
ElePlant=faudes.Generator()
ElePlant:FromString([[
<Generator>
"Elevator"
<Alphabet> mon moff car clv </Alphabet>
<States> OFF_0 ON_0 OFF_1 ON_1 OFF_2 ON_2 </States>
<TransRel>
OFF_0 mon  ON_0
ON_0  moff OFF_0
OFF_1 mon  ON_1
ON_1  moff OFF_1
OFF_2 mon  ON_2
ON_2  moff OFF_2
ON_0 car ON_1
ON_1 clv ON_2
</TransRel>
<InitStates> OFF_0 </InitStates>
<MarkedStates> OFF_0 OFF_1 OFF_2 </MarkedStates>
</Generator>
]])

-- specification: do whatever it needs to arrive in first floor
EleSpec=faudes.Generator()
EleSpec:FromString([[
<Generator>
"MoveToFloor1"
<Alphabet> mon moff car clv </Alphabet>
<States> ANY DONE </States>
<TransRel>
GO  mon   GO
GO  moff  GO
GO  clv   GO
GO  car   OK
OK  moff  OK
</TransRel>
<InitStates>   GO  </InitStates>
<MarkedStates> OK  </MarkedStates>
</Generator>
]])

-- controllable events aka actuators
EleCntrl=faudes.EventSet()
EleCntrl:FromString([[
<Alphabet> mon moff </Alphabet>
]])

-- priorities such that actuators preempt sensors
ElePrio={['mon']=1, ['moff']=1}

-- run example
EleLoop=faudes.Generator()
faudes.SupPconNB(ElePlant, EleCntrl, ElePrio, EleSpec, EleLoop)
EleLoop:Write()