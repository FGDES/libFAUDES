/** @file pbp_include.h Includes all luabindings plug-in headers */

/*
 **************************************************** 
 Convenience header file that includes all headers
 relevant to the pybindings plug-in. 

 (c) Thomas Moor 2023, 2024
 ****************************************************
 */


#ifndef FAUDES_PBP_INCLUDE_H
#define FAUDES_PBP_INCLUDE_H



/**
 
@defgroup PybindingsPlugin Python Bindings Plug-In


@ingroup AllPlugins

<p>
This plug-in generates libFAUDES bindings for the scripting language Python; 
see http://www.python.org. 
With this plug-in, the most essential libFAUDES data types and functions
can be accesed from the Python interpreter. 
</p>

<p>
Disclaimer: the primary scripting laguage for libFAUDES is Lua; see the Lua bindings plug-in.
Specifically, Lua bindings provide a more comprehensive interface, which is systematically
tested on a more regular basis. If you experience short commings of libFAUDES Python bindings,
please do not hesitate to report back to us.
</p>

<p>
Restrictions:

- The build system allows for either Lua bindings or Python bindings; building both
  simultaneously is not supported.


- For most plug-ins, bindings are automatically generated via SWIG based on so called
  interface files (residing in <tt>libfaudes/plugins/`*`/registry/`*`.i</tt>);
  regarding Python, only the plug-ins Synthesis and Observer have been tested.
  
</p>

@subsection SecPybindingsIntro1 Example Script

<p>
In large, libFAUDES Python bindings follow the same conventions as the Lua bindingins.
Thus, you may want to inspect the documentation of the latter. For further inspiration,
tutorials can be found at <tt>pybindings/tutorial</tt>.
</p>

<p>
For the following example, run Python in/from a directoty in which the
libFAUDES bindings reside (files <tt>_faudes.so</tt> and <tt>faudes.py</tt>).
</p>

@code{.unparsed}

# load libFAUDES bindings
import faudes

# test
faudes.Version()

# machine 1
gL1=faudes.Generator()
gL1.InsInitState("Idle")
gL1.SetMarkedState("Idle")
gL1.InsState("Busy")
gL1.InsEvent("alpha1")
gL1.InsEvent("beta1")
gL1.SetTransition("Idle","alpha1","Busy")
gL1.SetTransition("Busy","beta1","Idle")

# machine 2
gL2=faudes.Generator()
gL2.InsInitState("Idle")
gL2.SetMarkedState("Idle")
gL2.InsState("Busy")
gL2.InsEvent("alpha2")
gL2.InsEvent("beta2")
gL2.SetTransition("Idle","alpha2","Busy")
gL2.SetTransition("Busy","beta2","Idle")

# overall plant
gL=faudes.Generator()
faudes.Parallel(gL1,gL2,gL)

# controllable events
sCtrl=faudes.EventSet()
sCtrl.Insert("alpha1")
sCtrl.Insert("alpha2")

# specification aka buffer
gE=faudes.Generator()
gE.InsInitState("Empty")
gE.SetMarkedState("Empty")
gE.InsState("Full")
gE.InsEvent("beta1")
gE.InsEvent("alpha2")
gE.SetTransition("Empty","beta1","Full")
gE.SetTransition("Full","alpha2","Empty")

# lift specification to overall eventset
sAll=faudes.EventSet()
sAll=gL.Alphabet()
faudes.InvProject(gE,sAll)

# supremal closed loop
gK=faudes.Generator()
faudes.SupConNB(gL,sCtrl,gE,gK)

# show result on console
gK.Write()

# save result as graphics
gK.GraphWrite("K.png")

@endcode

<p>
Note: for graphics output you must have installed <tt>dot</tt> from the GraphViz package.
If <tt>dot</tt> is not in the systems path, you may direct libfaudes via
<tt>faudes.DocExecPath("/wherever_dot_is/dot")</tt>.
</p>



@subsection SecLuabindingsIntro3 Build System

<p>
To use python bindings, you will need to configure and recompile libFAUDES;
see also <a href="../faudes_build.html">build-system documentation</a> for
an overview.
</p>

<p>
Notes:

- You should configure libFAUDES to use no further plug-ins than Synthesis and Observer.
  This is done in the main Makefile.
  If you require Python bindings other plug-ins, please let us know.


- The build system is best supported on Linux and OS X. For Windows we recommend to  MSYS2 or equivalents.


- The build process relies on the developer package of your Python distribution. It
  needs to be pointed to the paths of <tt>Python.h</tt> and the
  Python shared object <tt>libpython.so</tt>. There is an attempt to figure those
  paths  automatically, however, your milage may vary; 
  see  <tt>pybindings/Makefile.plugin</tt> for manual configuration.


- After building libFAUDES as a shared object, copy the files <tt>pybindings/tutorial/faudes.py</tt>
  and <tt>pybindings/tutorial/_faudes.so</tt> to reside in the same
  directory as your Python project. Test your settings by starting a Python command promt  within your
  project directory and try, e.g. <tt>import faudes</tt> and <tt>faudes.Version()</tt>.


- After re-compiling libFAUDES and importing to Python, make shure to clear all caches;
  i.e., start with a clean directory after each iteration of re-compilation.  
</p>


@subsection PyLicense License

The Python bindings plug-in is distributed with libFAUDES. 
All code is  provided under terms of the LGPL. 


Copyright (c) 2023, 2024 Thomas Moor.



*/



#endif
