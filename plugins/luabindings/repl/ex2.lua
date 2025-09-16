-- ==========================================
-- Simple Machine Example
-- ==========================================


-- declare generator and initialize from string
g=faudes.Generator()
g:FromString([[
<Generator>
<T>
idle  alpha  busy
busy  beta   idle
busy  mue    down
down  lambda idle
</T>
<I> idle </I>
<M> idle </M>
</Generator>
]])

-- have two copies with suffix "_1" and "_2", resp
g1=g:Version(1);
g2=g:Version(2);

-- buffer capacity one specfication
e=faudes.Generator()
e:FromString([[
<Generator>
<T>
empty beta_1  full
full  alpha_2 empty
</T>
<I> empty </I>
<M> empty </M>
</Generator>
]])

-- have overall eventset
sigma = g1:Alphabet() 
  + g2:Alphabet()
  + e:Alphabet()

-- have controllable events
sigma_c = faudes.EventSet()
sigma_c:FromString("<A> alpha_1 alpha_2 lambda_1 lambda_2 </A>")

-- compose overall plant
g12=faudes.Generator()
faudes.Parallel(g1,g2,g12)

-- re-interpret specification w.r.t. full alphabet
e12=faudes.Generator()
faudes.InvProject(e,sigma,e12)

-- synthesis
k=faudes.Generator()
faudes.SupConNB(g12,sigma_c,e12,k)

-- report
k:Write()

