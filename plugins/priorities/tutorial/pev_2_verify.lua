-- Conveyor Belts Demo for Compositional Verification
--
-- Uses "pev_cbs_setup.lua" to generate the models


-- configure number of conveyor belts via command line, default to 5
N=arg[1]
if N == nil then N=5 end  
print(string.format('pev_2_verify.lua: number of conveyor belts N=%d',N))

-- generate models
dofile("pev_cbs_setup.lua")

-- read models
print('pev_2_verify.lua: reading models')
gvec=faudes.GeneratorVector()
gvec:Append('tmp_pev_cbs_src.gen')
for i=1,N do
  gvec:Append(string.format('tmp_pev_cbs_%d_cl.gen',i))
end
gvec:Append("tmp_pev_cbs_snk.gen")

-- read priorities
prios = faudes.EventPriorities("tmp_pev_cbs_prios.alph");

-- do verify (compositional)
print('pev_2_verify.lua: compositional verification')
isnc = faudes.IsPNonblocking(gvec,prios)
if isnc then
  print('pev_2_verify: nonconflictingness confirmed (PASS)')
else
  print('pev_2_verify: nonconflictingness failed (FAIL)')
end
FAUDES_TEST_DUMP("prio comp ver",isnc)

-- do verify (monolitic, for limited number of conveyor belts)
if tonumber(N) <6 then
  print('pev_2_verify.lua: monolitic verification')
  gall=faudes.Generator()
  faudes.ParallelLive(gvec,gall)
  print('pev_2_verify: overall statecount:',gall:Size())
  faudes.ShapePriorities(gall,prios)
  isnc = faudes.IsNonblocking(gall)
  if isnc then
    print('pev_2_verify: nonconflictingness confirmed (PASS)')
  else
    print('pev_2_verify: nonconflictingness failed (FAIL)')
  end
end

