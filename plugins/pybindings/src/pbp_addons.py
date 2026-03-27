# convenience addons (experimental, unstable)


# #########################################################
# #########################################################
# #########################################################
# Convert Generators from/to Python lists
# #########################################################
# #########################################################
# #########################################################


# initialise Generator or derived class from Python lists
# (use faudes style return-by-reference to be type agnostic)
def __ConvertListsToGenerator(GRes,Q=[],Sigma=[],delta=[],Q0=[],Qm=[]):
    r"""                                                                                               
    Generator.FromLists(Q=[],Sigma=[],delta=[],Q0=[],Qm=[])
    """
    # prepare
    GRes.Clear()
    # insert explicit state indices
    for (x1, ev, x2) in delta:
        if type(x1)==int:
            GRes.InsState(x1)
        if type(x2)==int:
            GRes.InsState(x2)
    for q in Q:
        if type(q)==int:
            GRes.InsState(q)
    for q in Q0:
        if type(q)==int:
            GRes.InsInitState(q)
    for q in Qm:
        if type(q)==int:
            GRes.InsMarkedState(q)
    # insert named states
    for (x1, ev, x2) in delta:
        if type(x1)==str:
            if not GRes.ExistsState(x1):
                GRes.InsState(x1)
        if type(x2)==str:
            if not GRes.ExistsState(x2):
                GRes.InsState(x2)
    for q in Q:
        if type(q)==str:
            if not GRes.ExistsState(q):
                GRes.InsState(q)
    for q in Q0:
        if type(q)==str:
            if not GRes.ExistsState(q):
                GRes.InsState(q)
            GRes.SetInitState(q)
    for q in Qm:
        if type(q)==str:
            if not GRes.ExistsState(q):
                GRes.InsState(q)
            GRes.SetMarkedState(q)    
    # traverse transition relation    
    for (x1, ev, x2) in delta:
        if type(x1)==str:
            x1=GRes.StateIndex(x1)
        GRes.InsEvent(ev)
        if type(ev)==str:
            ev=GRes.EventIndex(ev)
        if type(x2)==str:
            x2=GRes.StateIndex(x2)
        GRes.SetTransition(x1,ev,x2)
    # traverse remaining sets    
    for sig in Sigma:
        GRes.InsEvent(sig)

# wrapper to instantiate a Generator from Python listrs
def __NewGeneratorFromLists(Q=[],Sigma=[],delta=[],Q0=[],Qm=[]):
    r"""                                                                                               
    Generator.NewFromLists(Q=[],Sigma=[],delta=[],Q0=[],Qm=[]) -> Generator
    """
    g=Generator()
    __ConvertListsToGenerator(g,Q,Sigma,delta,Q0,Qm)
    return g

# announce to user
Generator.FromLists=__ConvertListsToGenerator
Generator.NewFromLists=__NewGeneratorFromLists

# convert generator and derived classes to Python lists
def __NewListsFromGenerator(g):
    r"""                                                                                               
    Generator.ToLists(gen) -> Q, Sigma, delta, Q0, Qm 
    """
    # extract stateset
    Q=[]
    sit=g.StatesBegin()
    while sit!=g.StatesEnd():
        q=sit.Index()
        if g.StateName(q) !='':
            q=g.StateName(q)
        Q.append(q)
        sit.Inc()
    # extract alphabet
    Sigma=[]
    eit=g.AlphabetBegin()
    while eit!=g.AlphabetEnd():
        ev=eit.Index()
        ev=g.EventName(ev)
        Sigma.append(ev)
        eit.Inc()
    # extract transiytion relation    
    delta=[]
    tit=g.TransRelBegin()
    while tit!=g.TransRelEnd():
        x1=tit.X1()
        ev=tit.Ev()
        x2=tit.X2()
        if g.StateName(x1)!='': 
            x1=g.StateName(x1)
        ev=g.EventName(ev)
        if g.StateName(x2)!='': 
            x2=g.StateName(x2)
        delta.append((x1,ev,x2))
        tit.Inc()
    # extract initial states
    Q0=[]
    sit=g.InitStatesBegin()
    while sit!=g.InitStatesEnd():
        q=sit.Index()
        if g.StateName(q) !='':
            q=g.StateName(q)
        Q0.append(q)
        sit.Inc()
    # extract marked states
    Qm=[]
    sit=g.MarkedStatesBegin()
    while sit!=g.MarkedStatesEnd():
        q=sit.Index()
        if g.StateName(q) !='':
            q=g.StateName(q)
        Qm.append(q)
        sit.Inc()
    return Q, Sigma, delta, Q0, Qm

# announce to user
Generator.ToLists=__NewListsFromGenerator


# #########################################################
# #########################################################
# #########################################################
# Convert Systems from/to Python lists
# #########################################################
# #########################################################
# #########################################################

# wrapper to instantiate a System from Python lists
def __NewSystemFromLists(Q=[],Sigma=[],Sigmac=[],delta=[],Q0=[],Qm=[]):
    r"""                                                                                               
    System.FromLists(Q=[],Sigma=[],Sigmac=[],delta=[],Q0=[],Qm=[])
    """
    g=System()
    __ConvertListsToGenerator(g,Q,Sigma,delta,Q0,Qm)
    for e in Sigmac:
      g.InsEvent(e)
      g.SetControllable(e)
    return g

# announce to user
System.FromLists=__NewSystemFromLists

# wrapper to convert a System to Python lists
def __NewListsFromSystem(g):
    r"""                                                                                               
    System.ToLists(gen) -> Q, Sigma, Sigmac, delta, Q0, Qm 
    """
    Q, Sigma, delta, Q0, Qm = __NewListsFromGenerator(g)
    # extract controllable events
    Sigmac=[]
    fsigc=g.ControllableEvents()
    eit=fsigc.Begin()
    while eit!=fsigc.End():
        ev=eit.Index()
        ev=g.EventName(ev)
        Sigmac.append(ev)
        eit.Inc()
    return Q, Sigma, Sigmac, delta, Q0, Qm

# announce to user
System.ToLists=__NewListsFromSystem


# #########################################################
# #########################################################
# Wrapper for graphical output
# #########################################################
# #########################################################

# get what we need
from matplotlib import pyplot
from PIL import Image
import tempfile
import subprocess
from pathlib import PurePath

# have my own dot runner
# (the libFAUDES implementation will fail in e.g. macOS/IDLE)
def __ProcessDot(dotfile,imgfile, opts=[]):
  # test dor dot
  if not DotReady():
    raise Exception("__ProcessDot: dot executable not found.")      
  # figure output format from suffix and have some nice defaults  
  if len(opts) == 0:
    opts=['-Efontname=Arial', '-Nfontname=Arial', '-Gbgcolor=transparent'] 
    suff=PurePath(imgfile).suffix
    if suff=='.png':
       opts+=['-Tpng']
    elif suff=='.jpg':
       opts+=['-Tjpeg']
    elif suff=='.jpeg':
       opts+=['-Tjpeg']
    elif suff=='.svg':
       opts+=['-Tsvg']
    elif suff=='.eps':
       opts+=['-Teps']
    elif suff=='.pdf':
       opts+=['-Tpdf']
    else:
      raise Exception("__ProcessDot: cannot figure image format.")
  # assemble command and run
  dotcmd= [DotExecPath()]
  dotcmd += opts
  dotcmd += [dotfile,'-o', imgfile]
  subprocess.run(dotcmd)


# worker function
def __GeneratorGraphShow(g):
  # have a temp files    
  with tempfile.NamedTemporaryFile(suffix=".dot", delete=False) as tmp:
    dotfile = tmp.name
  with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
    imgfile = tmp.name
  # sproduce the dot file and run dot
  g.DotWrite(dotfile)
  __ProcessDot(dotfile,imgfile)
  # read back
  img = Image.open(imgfile)
  # organise plot
  w, h = img.size
  pyplot.figure(figsize=(w/100, h/100+0.5), dpi=100)
  pyplot.imshow(img)
  pyplot.axis('off')
  pyplot.title(g.Name(),loc='left')
  pyplot.show()


# advertise to user
Generator.GraphShow=__GeneratorGraphShow
