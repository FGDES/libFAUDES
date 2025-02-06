--
-- Model for N-conveyor belts for transport from the left to the right, incl supervisors
--

-- configure
N=8


-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------
--
--  Event naming conventions
--
-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------

-- sd_i:  synch event (utilised in spec only) for sending workpiece from i-th conveyor (or source)
-- ar_i:  workpiece arrives at sensor 
-- lv_i:  workpiece enters leaves sensor 
-- on_i:  turn motor on
-- off_i: turn motor off


-- original event names (concise for discussion/publication)
function EventSdI(par_i)
  return string.format('sd_%d',par_i)
end
function EventArI(par_i)
  return string.format('ar_%d',par_i)
end
function EventLvI(par_i)
  return string.format('lv_%d',par_i)
end
function EventOnI(par_i)
  return string.format('on_%d',par_i)
end
function EventOffI(par_i)
  return string.format('off_%d',par_i)
end

-- FlexFact event names (for hardware-in-the-loop simulation)
--[[
function EventInI(par_i)
  return string.format('cb%d_sd',par_i)
end
function EventArI(par_i)
  if par_i == N+1 then
    return 'xs_wpar'   -- "xs_wpar" is "ar_(N+1)" aka "arive at exit slide sensor"
  end
  return string.format('cb%d_wpar',par_i)
end
function EventLvI(par_i)
if par_i == N+1 then
    return 'xs_wplv'   -- "xs_wplv" is "lv_(N+1)" aka "leave from exit slide sensor"
  end
  if par_i == 0 then
    return 'sf_wplv'   -- "sf_wplv" is "lv_(0)" aka "leaving sensor left of CB1"
  end
  return string.format('cb%d_wplv',par_i)
end
function EventOnI(par_i)
  return string.format('cb%d_bm+',par_i)
end
function EventOffI(par_i)
  return string.format('cb%d_boff',par_i)
end
--]]

--
-- Alphabets
--

-- overall alphabet, incl source and sink
function SetSigmaAll(par_n)
  SigmaAll=faudes.Alphabet();
  SigmaAll:Name(string.format('sigma_all_%d',par_n));
  -- std components
  local i
  for i=1,par_n do
    SigmaAll:Insert(EventSdI(i))    
    SigmaAll:Insert(EventArI(i))
    SigmaAll:Insert(EventLvI(i))
    SigmaAll:Insert(EventOnI(i))
    SigmaAll:Insert(EventOffI(i))
  end
  -- left environment (feeder)
  SigmaAll:Insert(EventSdI(0))
  SigmaAll:Insert(EventArI(0))
  SigmaAll:Insert(EventLvI(0))
  -- right environment (slide)
  SigmaAll:Insert(EventLvI(par_n+1))  
  SigmaAll:Insert(EventArI(par_n+1))  
end


-- execute
SetSigmaAll(N)

--
-- Priorities
--

-- priorities for universum of all events
function SetPriorities(par_n)
   PrioritiesAll=faudes.EventPriorities() 
   eit=SigmaAll:Begin()
   eit_end=SigmaAll:End()
   while eit~=eit_end do 
     ev=eit:Name()
     if ev:sub(1, 1) == "s" then
       -- sd* gets prio 0
       PrioritiesAll:InsPriority(ev,0)
     else
       if ev:sub(1, 1) == "o" then
         -- on/off gets prio 1
         PrioritiesAll:InsPriority(ev,1)
       else
         -- anything else gets 2
         PrioritiesAll:InsPriority(ev,2)
       end
    end
    eit:Inc()
  end
end


-- execute
SetPriorities(N)

--
-- File names
--
-- The below functions provide names for our models, e.g., for file io
--
function NamePcbI(par_i)
  return string.format('g_plant_cb_%d',par_i)
end
function NamePconI(par_i)
  return string.format('g_plant_con_%d',par_i)
end
function NameCI(par_i)
  return string.format('g_controller_%d',par_i)
end
function NameCompI(par_i)
  return string.format('g_component_%d',par_i)
end
function NameSrc()
  return 'source'
end
function NameSnk()
  return 'sink'
end



-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------
--
-- Model components 
--
-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------

--
-- Plant, single conveyor belt
-- ** workpiace arrive/leave only when motor is on
--
function ModelPcbI(par_i)

  -- Prepare result
  pcb=faudes.Generator()
  pcb:Clear()

  -- Insert events
  local ev_ar  =pcb:InsEvent(EventArI(par_i))
  local ev_lv  =pcb:InsEvent(EventLvI(par_i))
  local ev_on =pcb:InsEvent(EventOnI(par_i))
  local ev_off =pcb:InsEvent(EventOffI(par_i))
 
  -- Insert states
  local st_offvac=pcb:InsState(string.format('p%d_offvac',par_i))   -- motor off, sensor vaccant
  pcb:InsInitState(st_offvac)
  pcb:InsMarkedState(st_offvac)
  local st_onvac=pcb:InsState(string.format('p%d_onvac',par_i))
  local st_onocc=pcb:InsState(string.format('p%d_onocc',par_i))  -- motor on, sensor occupied
  local st_offocc=pcb:InsState(string.format('p%d_offocc',par_i))
  

  -- Insert transitions (nominal)
  pcb:SetTransition(st_offvac,ev_on,st_onvac)
  pcb:SetTransition(st_onvac,ev_off,st_offvac)
  pcb:SetTransition(st_offocc,ev_on,st_onocc)
  pcb:SetTransition(st_onocc,ev_off,st_offocc)
  pcb:SetTransition(st_onvac,ev_ar,st_onocc)
  pcb:SetTransition(st_onocc,ev_lv,st_onvac)

  -- Insert transitions (selfloops)
  pcb:SetTransition(st_offvac,ev_off,st_offvac)
  pcb:SetTransition(st_onvac,ev_on,st_onvac)
  pcb:SetTransition(st_offocc,ev_off,st_offocc)
  pcb:SetTransition(st_onocc,ev_on,st_onocc)

  -- have a name
  pcb:Name(NamePcbI(par_i))

  -- done
  return pcb

end

-- Plant, connecition between conveyor belts
-- ** only when a workpiece leaves cbi, cbi+1 can get a workpiece
-- ** if two workpieces leaves cbi without cbi+1 gets a workpiece in between, an error is issued
-- ** this is considered as a part of plant model of cbi
function ModelPconI(par_i)

  -- Prepare result
  pcon=faudes.Generator()
  pcon:Clear()

  -- Insert events
  local ev_lv  =pcon:InsEvent(EventLvI(par_i))
  local ev_ar  =pcon:InsEvent(EventArI(par_i + 1))
 
  -- Insert states
  local st_idle=pcon:InsState(string.format('p%d_idle',par_i))   -- idle state
  pcon:InsInitState(st_idle)
  pcon:InsMarkedState(st_idle)
  local st_busy=pcon:InsState(string.format('p%d_busy',par_i)) -- a workpiece is under transfer
  local st_err=pcon:InsState(string.format('p%d_err',par_i))  -- error state
  

  -- Insert transitions 
  pcon:SetTransition(st_idle,ev_lv,st_busy)
  pcon:SetTransition(st_busy,ev_ar,st_idle)
  pcon:SetTransition(st_busy,ev_lv,st_err)  

  -- have a name
  pcon:Name(NamePconI(par_i))

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
  src:InsInitState(st_idle)
  src:InsMarkedState(st_idle)
  local st_busy=src:InsState('src_busy')
  local st_done=src:InsState('src_done')

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
  snk:InsInitState(st_idle)
  snk:InsMarkedState(st_idle)
  local st_busy=snk:InsState('snk_busy')
  local st_done=snk:InsState('snk_done')
  local st_err=snk:InsState('snk_err')

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
--    cbi+1 becomes ready. The intension here is, if wee let sd-events have higher priority
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
 

  print(string.format('exporting models for N=%d',N))

  -- export model source
  ModelSrc():Write(string.format('pev_cbs_%s.gen',NameSrc()))

  -- export closed-loop conveyor belts
  local i
  local cmp
  for i=1,N do
    -- YT's verbose export
    --ModelPcbI(i):Write(string.format('%d_Pcb.gen',i))
    --ModelPconI(i):Write(string.format('%d_Pcon.gen',i))
    --ModelCI(i):Write(string.format('%d_C.gen',i))
    
    -- this is what I want
    ModelCLI(i):Write(string.format('pev_cbs_%d_cl.gen',i))
  end

  -- export model destination
  ModelSnk():Write(string.format('pev_cbs_%s.gen',NameSnk(N)))

  -- export priorities
  PrioritiesAll:Write('pev_cbs_prios.alph')


end


-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------
--
-- Testing
--
-- ----------------------------------------------------------------------------
-- ----------------------------------------------------------------------------

--[[
-- generate jpgs with N=1 for inspection
print('======================================')
print('======================================')
print('generating *.jpg for N=1')
SetN(1)
g_src=ModelSrc()
g_src:GraphWrite(string.format('%s.jpg',NameSrc()))
g_pa1=ModelPaI(1)
g_pa1:GraphWrite(string.format('%s.jpg',NamePaI(1)))
g_pb1=ModelPbI(1)
g_pb1:GraphWrite(string.format('%s.jpg',NamePbI(1)))
g_snk=ModelSnk(1)
g_snk:GraphWrite(string.format('%s.jpg',NameSnk(1)))
g_ca1=ModelCaI(1)
g_ca1:GraphWrite(string.format('%s.jpg',NameCaI(1)))
g_cb1=ModelCbI(1)
g_cb1:GraphWrite(string.format('%s.jpg',NameCbI(1)))
g_c1=ModelCI(1)
g_c1:GraphWrite(string.format('%s.jpg',NameCI(1)))
g_all=ModelAll()
MergeBlockingStates(g_all)
Shape(g_all)
g_all:GraphWrite("g_all.jpg")
--]]



-- export automata models with N=1 for external processing
print('======================================')
print('======================================')
ExportModels()

