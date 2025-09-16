# testing language inclusion with spot

# set my tools
LUAFAUDES=../../../bin/luafaudes
GEN2HOA=./gen2hoa
AUTFILT=~/opt/bin/autfilt

# say hello
echo "==== verification demo using spot"

# run my example to generate automata
echo "== running tutorial rabinctrl"
./omg_4_rabinctrl 2&> /dev/null 
AUTOMATON1=tmp_omg_4_lspec3
AUTOMATON2=tmp_omg_4_lctrl13
AUTOMATON3=tmp_omg_4_spec3_rabin

# convert to hoa file format
echo "== converting gen -> hoa"
$GEN2HOA $AUTOMATON1.gen $AUTOMATON1.hoa
$GEN2HOA $AUTOMATON2.gen $AUTOMATON2.hoa
$GEN2HOA $AUTOMATON3.gen $AUTOMATON3.hoa

echo "== testing for lloop13 included in spec3"
$AUTFILT --included-in $AUTOMATON3.hoa $AUTOMATON3.hoa --stats '%s states, %e edges' > /dev/null
if [ $? == 0 ]; then
  echo "test passed [expected]"
else
  echo "test failed [test case ERROR]"
fi       

echo "== testing for lspec3 included in lloop13"
$AUTFILT --included-in $AUTOMATON2.hoa $AUTOMATON1.hoa --stats '%s states, %e edges' > /dev/null
if [ $? == 0 ]; then
  echo "test passed [expected]"
else
  echo "test failed [test case ERROR]"
fi       

