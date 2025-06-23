# libFAUDES - A Discrete-Event Systems Library 



## Overview


libFAUDES is a C++ library that implements data structures for the representation 
of finite automata and algorithms for supervisory control within the framework 
originally proposed by P.J. Ramadge and W.M. Wonham. The library provides an 
implementation and evaluation basis for the further development of 
algorithms for the analysis and synthesis of DESs.

The core library sources are provided for free under conditions of the GNU Lesser 
General Public License.  Different terms may apply to libFAUDES plug-ins.




## Documentation

libFAUDES ships with auto-generated HTML documentaion which can be
accessed via ./doc/index.html. An online version is available at

https://fgdes.tf.fau.de/faudes

For your convenience, precompiled binaries are distributed at

https://fgdes.tf.fau.de/download.html

If you want to build your own, see the build process documentation

https://fgdes.tf.fau.de/faudes/faudes_build.html


## Versioning/Branching

The <i>main</i> branch of this repository is at least in a state to compile all default 
plug-ins and pass the test cases. It should be suitable for the devlopment of libFAUDES 
based applications. We add tags to <i>main</i> whenever the version (as specified in the 
file `VERSION`) number is incremented. In this event validation in Linux, Mac OS amd 
Windows environments is trigered automaticaly.  

The <i>dev</i> branch is used for our ongoing development process. We typically only commit
when test cases pass, but there may be temporally exceptions and we rarely validate in a 
Windows environment. We wont bother to increment version numbers or to set tags either. The 
<i>dev</i> branch is only of interest, if you plan to contribute to libFAUDES and want to 
inspect latest features. In that case you most likely want to have your personal branch/fork 
based on the <i>dev</i> branch. You will inherit the test cases and can trigger automatic 
validation on all supported envorinments by setting/commiting/pushing a tag; see our yml-files
for GitHub actions.



## Authors/Copyright

libFAUDES is distributed under terms of the LGPL v2.1. Over time, many students and colleguous have contributed to the code base; see  https://fgdes.tf.fau.de/faudes for a complete list.

- Copyright (C) 2006, Bernd Opitz.
- Copyright (C) 2008 - 2010, Thomas Moor, Klaus Schmidt, Sebastian Perk. 
- Copyright (C) 2011 - 2025, Thomas Moor, Klaus Schmidt

