faudes.MakeGlobal()
-- Read automat from files
g1 = System("gen1.gen")
g1:GraphWrite("gen1.png")
g2 = System("gen2.gen")
g2:GraphWrite("gen2.png")
g3 = System("gen3.gen")
g3:GraphWrite("gen3.png")

spec = System("spec.gen")
spec:GraphWrite("spec.png")

res = GeneratorVector()
coord = Generator()

-- genVect
gVector = GeneratorVector()
gVector:Append(g1)
gVector:Append(g2)
gVector:Append(g3)

-- controllable events
ec = Alphabet("ec.alph") 
ek = Alphabet("ek0.alph") 

SupConditionalControllable(spec,gVector,ec,ek,res,coord)

coord:GraphWrite("supCk.png")
for i=1,res:Size() do
  res[i-1]:GraphWrite("supC"..tostring(i).."+k.png")
end
