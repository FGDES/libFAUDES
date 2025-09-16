-- variant of the shape operator



-- -----------------------------------------------------------
-- -----------------------------------------------------------
-- -----------------------------------------------------------
--
-- shape a generator to adhere to event priorities,
-- as in the faudes function ShapePriorities ...
-- ... except that we maintaine selfloops for
-- a specified set of evets
-- 
--
-- arguments
--
--   gen:   the generator to shape (faudes.Generator)
--   prios: event priorities (faudes.EventPriorities)
--   msl:   set of events for which self-loops will be
--          retained (faudes.EventSet)
--
function ShapeMSL(gen,prios,msl)
  -- iterate over all states
  local sit_end=gen:StatesEnd()
  local sit=gen:StatesBegin()
  while sit ~= sit_end do
    local x1=sit:Index()
    -- iterate figure highest priority
    local hp=-1
    local tit=gen:TransRelBegin(x1)
    local tit_end=gen:TransRelEnd(x1)
    while tit~= tit_end do
      local p=prios:Priority(tit:Ev())
      if p > hp then
        hp=p
      end
      tit:Inc()            
    end
    -- figure transitions to remove
    local tit=gen:TransRelBegin(x1)
    local tit_end=gen:TransRelEnd(x1)
    while tit~= tit_end do
      -- will to remove ...
      local rm=true
      -- ... except if we are not preempted priority ...
      local p=prios:Priority(tit:Ev())
      if p >= hp then
        rm=false
      end
      -- ... except if we a to-be-maintained self loop
      if msl:Exists(tit:Ev()) and (tit:X1()==tit:X2()) then
        rm=false
      end
      -- either remove incl increment or plain inrcement
      if rm then
        tit=gen:ClrTransition(tit)
      else
        tit:Inc()
      end	
    end    
    sit:Inc()
  end
end


-- -----------------------------------------------------------
-- -----------------------------------------------------------
-- -----------------------------------------------------------
-- selfcontained demo

-- data

g=faudes.Generator()
g:FromString([[
 <Generator>
 <T>
   1 a 2
   1 t 1    % selfloop that shall be maintained
   1 t 2    % transition that will be preempted
   2 a 1
   2 b 2    % selfloop that will be preempted
 </T>
 </Generator>
]])

p=faudes.EventPriorities()
p:InsPriority("a",3)
p:InsPriority("b",2)
p:InsPriority("t",1)

m=faudes.EventSet()
m:Insert("t")

-- report/run

print("=== genertor")
g:Write()
print("=== priorities")
p:Write()
print("=== ticks")
m:Write()

ShapeMSL(g,p,m)


print("=== result")
g:Write()