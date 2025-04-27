--
-- Automata models for an N conveyor belts setup to transport workpieces from the left to
-- the right, incl. local supervisors, a source and a sink.
--
-- This example has been reported by Tang/Moor at WODES 2022, and was further developed for
-- the journal version published in DEDS 2024. Allthough the transition structures for both
-- variants are identical, the WODES 2022 version uses the common synchronous marking while
-- the DEDS 2024 variant uses a more elaborate fairness constraint. Both papers are
-- available e.g. via
--
-- https://fgdes.tf.fau.de/publications.html
--
-- There is no Lua-interface for fairness constraints, hence we only set up autoamta and then
-- run the corresponding C-tutorial.
--

-- configure number of conveyor belts via command line, default to 5
N=arg[1]
if N == nil then N=5 end  
print(string.format('pev_cbs_setup.lua: number of conveyor belts N=%d',N))



-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------
--
--  Event naming conventions
--
-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------

-- sd_i:  synch event for sending workpiece from i-th conveyor (or source)
-- ar_i:  workpiece arrives at sensor 
-- lv_i:  workpiece enters leaves sensor 
-- on_i:  turn motor on
-- off_i: turn motor off

-- symolic event names
function EventSdI(par_i)  return string.format('sd_%d',par_i) end
function EventArI(par_i)  return string.format('ar_%d',par_i) end
function EventLvI(par_i)  return string.format('lv_%d',par_i) end
function EventOnI(par_i)  return string.format('on_%d',par_i) end
function EventOffI(par_i) return string.format('off_%d',par_i) end

-- overall alphabet, incl source and sink
function SetSigmaAll(par_n)
  SigmaAll=faudes.EventPriorities();
  SigmaAll:Name(string.format('sigma_all_%d',par_n));
  -- std components
  local i
  for i=1,par_n do
    SigmaAll:InsPriority(EventSdI(i),0)    
    SigmaAll:InsPriority(EventArI(i),2)
    SigmaAll:InsPriority(EventLvI(i),2)
    SigmaAll:InsPriority(EventOnI(i),1)
    SigmaAll:InsPriority(EventOffI(i),1)
  end
  -- left environment (feeder)
  SigmaAll:InsPriority(EventSdI(0),0)
  SigmaAll:InsPriority(EventArI(0),2)
  SigmaAll:InsPriority(EventLvI(0),2)
  -- right environment (slide)
  SigmaAll:InsPriority(EventLvI(par_n+1),2)  
  SigmaAll:InsPriority(EventArI(par_n+1),2)  
end


-- execute
SetSigmaAll(N)

--
-- Generator names
--
-- The below functions provide names for our models
--
function NamePcbI(par_i)  return string.format('cb_%d',par_i) end
function NamePconI(par_i) return string.format('con_%d',par_i) end
function NameCI(par_i)    return string.format('sup_%d',par_i) end
function NameCompI(par_i) return string.format('cmp_%d',par_i) end
function NameSrc()        return 'src' end
function NameSnk()        return 'snk' end



-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------
--
-- Model components 
--
-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------

--
-- Plant, single conveyor belt
-- ** workpiece may arrive or leave only when motor is on
--
function ModelPcbI(par_i)

-- prepare result
  pcb=faudes.Generator()
  pcb:Clear()
  -- have a name
  pcb:Name(NamePcbI(par_i))
  -- insert events
  local ev_ar  =pcb:InsEvent(EventArI(par_i))
  local ev_lv  =pcb:InsEvent(EventLvI(par_i))
  local ev_on =pcb:InsEvent(EventOnI(par_i))
  local ev_off =pcb:InsEvent(EventOffI(par_i))
  -- insert states
  local st_offvac=pcb:InsState(string.format('p%d_offvac',par_i))   -- motor off, sensor vaccant
  local st_onvac=pcb:InsState(string.format('p%d_onvac',par_i))     -- motor on, sensor vaccant
  local st_onocc=pcb:InsState(string.format('p%d_onocc',par_i))     -- motor on, sensor occupied
  local st_offocc=pcb:InsState(string.format('p%d_offocc',par_i))   -- motor off, sensor occupied
  pcb:InsInitState(st_offvac)
  pcb:InsMarkedState(st_offvac)
  pcb:InsMarkedState(st_offocc) -- TM: want to use this for DEDS variant and WODES variant
  -- insert transitions (intended operation)
  pcb:SetTransition(st_offvac,ev_on,st_onvac)
  pcb:SetTransition(st_onvac,ev_off,st_offvac)
  pcb:SetTransition(st_offocc,ev_on,st_onocc)
  pcb:SetTransition(st_onocc,ev_off,st_offocc)
  pcb:SetTransition(st_onvac,ev_ar,st_onocc)
  pcb:SetTransition(st_onocc,ev_lv,st_onvac)
  -- insert transitions (actuator selfloops)
  pcb:SetTransition(st_offvac,ev_off,st_offvac)
  pcb:SetTransition(st_onvac,ev_on,st_onvac)
  pcb:SetTransition(st_offocc,ev_off,st_offocc)
  pcb:SetTransition(st_onocc,ev_on,st_onocc)
  -- done
  return pcb

end

-- Plant, relation between two consecutive conveyor belts
-- ** only when a workpiece leaves cbi, cbi+1 can get a workpiece
-- ** if two workpieces leave cbi without cbi+1 to get a workpiece in between,
--    this is considered an  error
-- ** this automaton is considered as a part of plant model of cbi
function ModelPconI(par_i)

  -- prepare result
  pcon=faudes.Generator()
  pcon:Clear()
  -- have a name
  pcon:Name(NamePconI(par_i))
  -- insert events
  local ev_lv  =pcon:InsEvent(EventLvI(par_i))
  local ev_ar  =pcon:InsEvent(EventArI(par_i + 1))
  -- insert states
  local st_idle=pcon:InsState(string.format('p%d_idle',par_i))   -- idle state
  local st_busy=pcon:InsState(string.format('p%d_busy',par_i)) -- a workpiece is under transfer
  local st_err=pcon:InsState(string.format('p%d_err',par_i))  -- error state
  pcon:InsInitState(st_idle)
  pcon:InsMarkedState(st_idle)
  -- insert transitions 
  pcon:SetTransition(st_idle,ev_lv,st_busy)
  pcon:SetTransition(st_busy,ev_ar,st_idle)
  pcon:SetTransition(st_busy,ev_lv,st_err)  
  -- done
  return pcon

end

--
-- Controlled source (feeder)
--
-- ** compose a simple sd-lv loop with its connection part to cb1
--
function ModelSrc()

  -- Prepare result
  src=faudes.Generator()
  src:Clear()
  -- Insert events
  local ev_ar    =src:InsEvent(EventArI(0))
  local ev_sd   =src:InsEvent(EventSdI(0))
  local ev_lv    =src:InsEvent(EventLvI(0))
  -- Insert states
  local st_idle=src:InsState('src_idle')
  local st_busy=src:InsState('src_busy')
  local st_done=src:InsState('src_done')
  src:InsInitState(st_idle)
  src:InsMarkedState(st_idle)
  -- Insert transitions (nominal)
  src:SetTransition(st_idle,ev_ar,st_busy)
  src:SetTransition(st_busy,ev_sd,st_done)
  src:SetTransition(st_done,ev_lv,st_idle)
  -- compose with its connecition part to cb1  
  faudes.Parallel(src,ModelPconI(0),src)
  -- have a name
  src:Name(NameSrc())
  -- done
  return src

end

--
-- "Controlled" sink
--
-- ** considered as the n+1-th component
-- ** only equipped with a sensor to detect workpiece 
-- ** "controlled" behaviour allows workpiece arrival (ar-n+1) only when sd-n is executed, which
--    is reset after lv-n+1 is executed. If not, an error is issued
--
function ModelSnk()

  -- Prepare result
  snk=faudes.Generator()
  snk:Clear()
  -- Insert events
  local ev_sd   =snk:InsEvent(EventSdI(N))
  local ev_ar   =snk:InsEvent(EventArI(N+1))
  local ev_lv   =snk:InsEvent(EventLvI(N+1))
  -- Insert states
  local st_idle=snk:InsState('snk_idle')
  local st_busy=snk:InsState('snk_busy')
  local st_done=snk:InsState('snk_done')
  local st_err=snk:InsState('snk_err')
  snk:InsInitState(st_idle)
  snk:InsMarkedState(st_idle)
  -- Insert transitions
  snk:SetTransition(st_idle,ev_sd,st_busy)
  snk:SetTransition(st_busy,ev_ar,st_done)
  snk:SetTransition(st_done,ev_lv,st_idle)
  snk:SetTransition(st_idle,ev_ar,st_err)
  -- have a name
  snk:Name(NameSnk(N+1))
  -- done
  return snk

end


--
-- Controller of each conveyor belt nominal flow of workpiece
--
-- ** generates the language of the regular expression (-- denotes concatenation):
--    (sd_{i-1} -- on_i -- ar_i -- (sd_i + (off_i -- sd_i)) -- on_i -- ar_{i+1} -- off_i)^*
--    the choice of (sd_i + (off_i -- sd_i)) can be interpreted as such that either we go on
--    for send to cbi+1 directly (if cbi+1 is ready to synch) or we first stop, and wait until
--    cbi+1 becomes ready. The intension here is, if we let sd-events have higher priority
--    over off-events, the (off_i -- sd_i) choice will not be entered if sd_i is directly executable
-- ** also, issue an error when sensor events occurs unexpectedly
function ModelCI(par_i)

  -- Prepare result
  ctrl=faudes.Generator()
  ctrl:Clear()
  -- Insert events
  local ev_on  =ctrl:InsEvent(EventOnI(par_i))
  local ev_off =ctrl:InsEvent(EventOffI(par_i))
  local ev_ar =ctrl:InsEvent(EventArI(par_i))
  local ev_arx =ctrl:InsEvent(EventArI(par_i+1))
  local ev_sd =ctrl:InsEvent(EventSdI(par_i))
  local ev_sdp =ctrl:InsEvent(EventSdI(par_i-1))
  -- Insert states
  local st_a=ctrl:InsState(string.format('%d_a',par_i))
  ctrl:InsInitState(st_a)
  local st_b=ctrl:InsState(string.format('%d_b',par_i))
  local st_c=ctrl:InsState(string.format('%d_c',par_i))
  local st_d=ctrl:InsState(string.format('%d_d',par_i)) -- choice occurs here
  local st_e=ctrl:InsState(string.format('%d_e',par_i)) -- branch of waiting
  local st_f=ctrl:InsState(string.format('%d_f',par_i)) -- proceed
  local st_g=ctrl:InsState(string.format('%d_g',par_i))
  local st_h=ctrl:InsState(string.format('%d_h',par_i))
  local st_err=ctrl:InsState(string.format('%d_err',par_i))
  -- Insert transitions (nominal)
  ctrl:SetTransition(st_a,ev_sdp,st_b)
  ctrl:SetTransition(st_b,ev_on,st_c)
  ctrl:SetTransition(st_c,ev_ar,st_d)
  ctrl:SetTransition(st_d,ev_off,st_e) -- branch of waiting
  ctrl:SetTransition(st_d,ev_sd,st_f) -- branch of direct proceed
  ctrl:SetTransition(st_e,ev_sd,st_f)
  ctrl:SetTransition(st_f,ev_on,st_g)
  ctrl:SetTransition(st_g,ev_arx,st_h)
  ctrl:SetTransition(st_h,ev_off,st_a)
  -- Insert transitions (error)
  ctrl:SetTransition(st_a,ev_ar,st_err)
  ctrl:SetTransition(st_a,ev_arx,st_err)
  ctrl:SetTransition(st_b,ev_ar,st_err)
  ctrl:SetTransition(st_b,ev_arx,st_err)
  -- ctrl:SetTransition(st_c,ev_ar,st_err) -- st_c has norminal ev_ar
  ctrl:SetTransition(st_c,ev_arx,st_err)
  ctrl:SetTransition(st_d,ev_ar,st_err) 
  ctrl:SetTransition(st_d,ev_arx,st_err)
  ctrl:SetTransition(st_e,ev_ar,st_err)
  ctrl:SetTransition(st_e,ev_arx,st_err)
  ctrl:SetTransition(st_f,ev_ar,st_err)
  ctrl:SetTransition(st_f,ev_arx,st_err)
  ctrl:SetTransition(st_g,ev_ar,st_err)
  -- ctrl:SetTransition(st_g,ev_arx,st_err) -- st_g has norminal ev_arx
  ctrl:SetTransition(st_h,ev_ar,st_err)
  ctrl:SetTransition(st_h,ev_arx,st_err)
  -- mark all states excep error state
  local marked=ctrl:States():Copy()
  marked:Erase(st_err)
  ctrl:InsMarkedStates(marked)
  -- have a name
  ctrl:Name(NameCI(par_i))
  -- done
  return ctrl

end



--
-- Closed-loop of a single conveyor belt
--
function ModelCLI(par_i)

  -- Prepare result
  cl=ModelCI(par_i)
  faudes.Parallel(cl,ModelPcbI(par_i),cl)
  faudes.Parallel(cl,ModelPconI(par_i),cl)
  -- done
  cl:StateNamesEnabled(false)
  return cl
  
end  


--
-- export generator files for external processing
--
function ExportModels()

  -- say hello
  print('pev_cbs_setup.lua: exporting models')
  -- export model source
  ModelSrc():Write(string.format('tmp_pev_cbs_%s.gen',NameSrc()))
  -- export closed-loop conveyor belts
  local i
  local cmp
  for i=1,N do
    -- this is what I want
    ModelCLI(i):Write(string.format('tmp_pev_cbs_%d_cl.gen',i))
  end
  -- export model sink
  ModelSnk():Write(string.format('tmp_pev_cbs_%s.gen',NameSnk(N)))
  -- export priorities
  SigmaAll:Write('tmp_pev_cbs_prios.alph')

end

-- execute
ExportModels()


-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------
--
-- Debugging
--
-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------

--
-- generate jpgs for inspection
--[[
print('======================================')
print('======================================')
g_src=ModelSrc()
g_src:GraphWrite(string.format('tmp_%s.jpg',NameSrc()))
g_pa1=ModelPcbI(1)
g_pa1:GraphWrite(string.format('tmp_%s.jpg',NamePcbI(1)))
g_pb1=ModelPconI(1)
g_pb1:GraphWrite(string.format('tmp_%s.jpg',NamePconI(1)))
g_pb1=ModelCI(1)
g_pb1:GraphWrite(string.format('tmp_%s.jpg',NameCI(1)))
g_pb1=ModelCI(N)
g_pb1:GraphWrite(string.format('tmp_%s.jpg',NameCI(N)))
g_snk=ModelSnk()
g_snk:GraphWrite(string.format('tmp_%s.jpg',NameSnk(1)))
--]]


