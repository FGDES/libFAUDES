# determinsing a Buechi automaton with ltl2dstar

# set my tools
GEN2HOA=./gen2hoa
HOA2GEN=./hoa2gen
GEN2DOT=./waut2dot
LTL2DSTAR=~/opt/ltl2dstar 
DOT=dot

# read Buechi automata and convert to HOA format
echo "== gen->hoa"
${GEN2HOA} -s tmp_omg_nba.sym data/omg_nba.gen tmp_omg_nba.hoa 

# Safra determinisation by ltl2dstar, all using HOA format
echo "== determinisation"
${LTL2DSTAR} --input=nba --output-format=hoa --detailed-states=no tmp_omg_nba.hoa tmp_omg_dra.hoa

# read Rabin automaton and convert to libFAUDES generator format
echo "== hoa->gen"
${HOA2GEN} -s tmp_omg_nba.sym tmp_omg_dra.hoa tmp_omg_dra.gen

# docs
echo "== dot processing"
${LTL2DSTAR} --input=nba --output-format=dot --detailed-states=yes tmp_omg_nba.hoa tmp_omg_dra.dot
${DOT} -Tpng tmp_omg_dra.dot > tmp_omg_dra.png
${GEN2DOT} tmp_omg_dra.gen tmp_omg_dra_gen.dot
${DOT} -Tpng tmp_omg_dra_gen.dot > tmp_omg_dra_gen.png


# reference, i.e., without gen->HAO->gen
echo "== reference"
${LTL2DSTAR} --input=nba --output-format=hoa data/omg_nba.hoa tmp_omg_dra_ref.hoa
${LTL2DSTAR} --input=nba --output-format=dot --detailed-states=yes data/omg_nba.hoa tmp_omg_dra_ref.dot
${DOT} -Tpng tmp_omg_dra_ref.dot > tmp_omg_dra_ref.png




