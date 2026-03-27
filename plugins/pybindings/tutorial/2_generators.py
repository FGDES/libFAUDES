## Test/demonstrate core faudes generators

## import our module (lazy)
from faudes import *

## ##########################################
## Generator: construct/edit
## ##########################################

## Announce
print("################# Construct a generator")

## Create plain generator
g1 = Generator()                      # empty generator
g2 = Generator(g1)                    # copy construct
g3 = Generator('data/vsmachine.gen')  # construct from file


## Instantiate from Python lists
gen = Generator.NewFromLists(
  delta=[
    ['idle', 'alpha', 'busy'],
    ['busy', 'beta',  'idle']],
  Q0 =[ 'idle' ],
  Qm =[ 'idle' ]
)  

## Have a name
gen.Name('very simple machine');

## Inspect
gen.Write()

## Report
print()
print("################# Extending the generator programmatically")

## Re-name
gen.Name('simple machine incl one lost state');

## Add individual states and events
print('adding states and events')
gen.InsState('down')        # named state
q=gen.InsState()            # anonymous state
gen.StateName(q,'lost')     # name the state
gen.InsEvent('mue')         # event
gen.InsEvent('lambda')      # another event

## Set individual transitions by name
print('setting additional transitions by symolic names')
gen.SetTransition('idle','alpha','busy')
gen.SetTransition('busy','beta','idle')
gen.SetTransition('busy','mue','down')

## Set individual transition as index triplet
print('setting additional transition as index triplet')
t=Transition()
t.X1=gen.StateIndex('down')
t.Ev=gen.EventIndex('lambda')
t.X2=gen.StateIndex('idle')
print(f'Transition {t} (index triplet, symbolic names via Generator {gen.TStr(t)})')
gen.SetTransition(t)

## Report
gen.Write()

## Record test case
TestDump('generator',gen)


## ##########################################
## Generator: graphical output
## ##########################################

## Announce graph output
print('################# Running Graphviz/dot')
print()

## Set dot path (or specify in PATH environment variable)
## eg MacOS ## DotExecPath('/Applications/Graphviz.app/Contents/MacOS/dot')
## eg Linux ## DotExecPath('dot')
## eg MsWin ## DotExecPath('c:\\Programs\Graphviz\dot')

## Run dot to create imgage file
gen.GraphWrite('tmp_simplemachine.jpg')

## Comments: Alternative output formats include <tt>*.png</tt>, <tt>*.svg</tt> 
## and <tt>*.dot</tt>. The latter is particular useful to apply advanced
## dot-processing options to affect e.g. the font or resolution. See also
## the shell script in ./libFAUDES/tools/imgproc

## Inspect image interactively
## gen.GraphShow()

## Comments: The method GraphShow() is meant for quick inspection e.g. in a
## Jupyter notebook. It is implemented in Python an uses the modeul
## mathplotlib to display an image rendered by GraphViz dot. You may inspect
## ./libFAUDES/plygins/pybindings/src/py_addons.py to derive alternive
## variants to best fit your needs


## ##########################################
## Generator: inspect programmatically
## ##########################################


## Announce 
print('################# Container access')

## Entire alphabet (retrieve reference)
sigr = gen.Alphabet()
print(f'alphabet: {sigr}')
## sigr.Clear() ## this will break the generator (!!!)

## Entire alphabet (retrieve copy)
sigc = gen.Alphabet().Copy()
print(f'alphabet: {sigc} (copy)')
sigc.Clear()    ## this is ok since sigc is a copy

## Inspect alphabet (iterate)
print('Iterate over events')
for e in gen.Alphabet():
  print('Event:', e)

## Entire stateset (retrieve reference)
sts=gen.States()
print(f'States: {sts} (plain indicees)')
print('States: show with optional symbiolic names via Generator:')
gen.WriteStateSet(sts)

## Inspect states
print('Iterate over states')
for q in sts:
  print(f'State: {q} (index, optional symbilic name {gen.SStr(q)})')

## Inspect transitions
print('Iterate over transitions')
for t in gen.TransRel():
  print(f'Transition: {t} (with optional symbolic names {gen.TStr(t)}')


## ##########################################
## Generator: example algorithm reachable states
## ##########################################

## say hello
print('Computing reachable states for the simple machine')

## initialize states to care and accumlated result
todo  = gen.InitStates().Copy()        ## copy required to keep generator (!)
reach = StateSet()                     ## nothing yet reached 

## loop for new one-step successors
while not todo.Empty():
  print(f'iterating --- todo: {todo}')
  reach.InsertSet(todo)
  succ=gen.TransRel().SuccessorStates(todo)
  print(f'iterating --- successors: {succ}')
  todo = succ - reach

## log test case
TestDump('reach expect true', reach == gen.AccessibleSet() )

## show result (indices still refer to the generator)
print('Reachable states')
reach.Name('Reachable States')
gen.WriteStateSet(reach)

## retrict generator to reachable states
gen.RestrictStates(reach)

## log
TestDump('simple machine (accessible)',gen)



## ##########################################
## System: construct/edit
## ##########################################

## As with all libFAUDES, System objects can be instatiated empty
## by the copy constructor or from file. For the sake of this tutorial
## we take a copy of the above Generator; i.e., all event attributes
## take default values, which we edit

print('############# cronstruct System from Generator')
sys=System(gen)
sys.Name('simple machine plant model')
sys.SetControllable('alpha')
sys.SetControllable('lambda')


## Entire alphabet with attributes 
print('Alphabet of system incl. attributes')
sig=sys.Alphabet()
sig.Write();

## Entire alphabet with attributes (retrieve a copy)
print('Alphabet of system incl. attributes (copy)')
sig=sys.Alphabet().Copy()
sig.Write()
sig.Clear()  ## this is ok, since sig is a copy
print('Expect 4 events with 2 attributes')
sys.SWrite()

## Record test case
TestDump('system',sys)


## validate test cases
TestDiff()
