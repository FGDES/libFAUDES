-- Test/demonstrate core faudes generator functions

-- ------------------------------------------
-- Deterministic
-- ------------------------------------------

-- Announce
print("################# Deterministic");

-- Convert nondeterministic generator
deter_nondet = faudes.Generator("data/deterministic_nondet.gen");
deter_det = faudes.Generator();
faudes.Deterministic(deter_nondet,deter_det);

-- Report
deter_nondet:Write();
deter_det:Write();

-- Record test case
FAUDES_TEST_DUMP("deterministic",deter_det)

-- ------------------------------------------
-- State Minimisation
-- ------------------------------------------

-- Announce
print("################# State Minimisation");

-- Convert non minimal generator
minimal_nonmin = faudes.Generator("data/minimal_nonmin.gen");
minimal_min = faudes.Generator();
faudes.StateMin(minimal_nonmin,minimal_min);

-- Report
minimal_nonmin:Write();
minimal_min:Write();

-- Record test case
FAUDES_TEST_DUMP("statemin",minimal_min)

-- ------------------------------------------
-- Natural Projection
-- ------------------------------------------

-- Announce
print("################# Projection");

project_g = faudes.Generator("data/project_g.gen");
project_prog = faudes.Generator();
alph_proj = faudes.EventSet();
alph_proj:Insert("a");
alph_proj:Insert("c");
alph_proj:Insert("e");
alph_proj:Insert("g");
faudes.Project(project_g,alph_proj,project_prog);

-- Report
project_g:Write();
alph_proj:Write();
project_prog:Write();

-- Record test case
FAUDES_TEST_DUMP("project",project_prog)

-- ------------------------------------------
-- Boolean Operations
-- ------------------------------------------

-- Announce
print("################# Boolean Operations");

-- Union
boolean_g1 = faudes.Generator("data/boolean_g1.gen");
boolean_g2 = faudes.Generator("data/boolean_g2.gen");
boolean_union = faudes.Generator();
boolean_union:StateNamesEnabled(false);
faudes.LanguageUnion(boolean_g1, boolean_g2, boolean_union);
  
-- Intersection
boolean_intersection = faudes.Generator();
boolean_intersection:StateNamesEnabled(false);
faudes.LanguageIntersection(boolean_g1, boolean_g2, boolean_intersection);
  
-- Test: perform complement twice
boolean_complement_g1=faudes.Generator(boolean_g1); 
boolean_complement_g1:StateNamesEnabled(false);
faudes.LanguageComplement(boolean_complement_g1);
boolean_complement2_g1=faudes.Generator(boolean_complement_g1); 
faudes.LanguageComplement(boolean_complement2_g1);
  
--  Write results
boolean_g1:Write("tmp_boolean_g1.gen");
boolean_g2:Write("tmp_boolean_g2.gen");
boolean_union:Write("tmp_union_g1g2.gen");
boolean_intersection:Write("tmp_intersection_g1g2.gen");
boolean_complement_g1:Write("tmp_complement_g1.gen");
boolean_complement2_g1:Write("tmp_complement2_g1.gen");

-- Record test case
FAUDES_TEST_DUMP("lang. union",boolean_union)
FAUDES_TEST_DUMP("lang. intersection",boolean_intersection)
FAUDES_TEST_DUMP("lang. complement",boolean_complement_g1)
FAUDES_TEST_DUMP("lang. complement2",boolean_complement2_g1)

-- Inspect on console

print("################# Results:");
boolean_union:DWrite();
boolean_intersection:DWrite();
boolean_complement_g1:DWrite();

-- Compare languages

print("################# Compare:");
if faudes.LanguageInclusion(boolean_g1,boolean_union) then 
  print("Lm(g1) <= Lm(g1) v Lm(g2): OK");
else
  print("Lm(g1) > Lm(g1) v Lm(g2): ERR");
end

if faudes.LanguageDisjoint(boolean_complement_g1,boolean_g1) then
  print("Lm(g1) ^ ~Lm(g1) = empty: OK");
else
  print("(Lm(g1) v Lm(g2)) ^ ~(Lm(g1) v Lm(g2)) != empty: ERR");
end

if faudes.LanguageEquality(boolean_g1,boolean_complement2_g1) then
  print("Lm(g1) = ~~Lm(g1) : OK");
else
  print("Lm(g1) != ~~Lm(g1) : ERR");
end

-- Record test case

FAUDES_TEST_DUMP("lang. inclusion",faudes.LanguageInclusion(boolean_g1,boolean_union))
FAUDES_TEST_DUMP("lang. disjoint",faudes.LanguageDisjoint(boolean_complement_g1,boolean_g1))
FAUDES_TEST_DUMP("lang. equality",faudes.LanguageEquality(boolean_g1,boolean_complement2_g1))


-- ------------------------------------------
-- Concatenation/Kleene-Closure
-- ------------------------------------------

-- Announce
print("################# Concatenation/Kleene-Closure");

-- Read K and L
concat_gk = faudes.Generator("data/concat_gk.gen")
concat_gl = faudes.Generator("data/concat_gl.gen")

-- Version 1: std functions
concat_k_l_star_1 = faudes.Generator(concat_gl)
faudes.KleeneClosure(concat_k_l_star_1)
faudes.LanguageConcatenate(concat_gk,concat_k_l_star_1,concat_k_l_star_1)
faudes.StateMin(concat_k_l_star_1,concat_k_l_star_1)

-- Version 2: use non-deterministic intermediate results, disable state names
concat_k_l_star_2 = faudes.Generator(concat_gl);
concat_k_l_star_2:StateNamesEnabled(false);
faudes.KleeneClosureNonDet(concat_k_l_star_2);
faudes.LanguageConcatenateNonDet(concat_gk,concat_k_l_star_2,concat_k_l_star_2)
faudes.Deterministic(concat_k_l_star_2,concat_k_l_star_2)
faudes.StateMin(concat_k_l_star_2,concat_k_l_star_2)

-- Compare results
concat_ok = faudes.LanguageEquality(concat_k_l_star_1,concat_k_l_star_2)

-- Inspect on console

print("################# Results:");
concat_k_l_star_1:SWrite();
concat_k_l_star_2:SWrite();
if concat_ok then
  print("results match --- OK")
else  
  print("results dont match --- ERROR")
end

-- Graphical output
-- concat_gk:GraphWrite("tmp_concat_gk.png");
-- concat_gl:GraphWrite("tmp_concat_gl.png");
-- concat_k_l_star_1:StateNamesEnabled(false);
-- concat_k_l_star_1:GraphWrite("tmp_concat_k_l_star_1.png");
-- concat_k_l_star_2:GraphWrite("tmp_concat_k_l_star_2.png");


-- Record test case
FAUDES_TEST_DUMP("lang. concat",concat_k_l_star_1)
FAUDES_TEST_DUMP("lang. concat",concat_k_l_star_2)
FAUDES_TEST_DUMP("lang. concat",concat_ok)


