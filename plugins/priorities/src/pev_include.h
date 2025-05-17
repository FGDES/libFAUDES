/** @file pev_include.h Include priorities plug-in headers */

/*
 **************************************************** 
 Convenience header file that includes all headers 
 relevant to the priorities plugin. The build system will
 append an include directive  for this file to
 the "allplugins.h" include file.

 (c) Thomas Moor 2025
 ****************************************************
 */

#ifndef FAUDES_PEV_INCLUDE_H
#define FAUDES_PEV_INCLUDE_H

#include "pev_priorities.h"
#include "pev_pgenerator.h"
#include "pev_sparallel.h"
#include "pev_abstraction.h"
#include "pev_verify.h"


#endif


/**
 

@defgroup PrioritiesPlugin Priorities Plug-In


@ingroup AllPlugins


<p>
The priorities plug-in addresses systems with prioritised events. Technically, an event priority
is a non-negatie integer. Semantically, events of higher priority will preempt any event of lower
priority.
</p>

<p>
As its base API, this plug-in provides
- the template faudes::TpEventSet for an alphabet of prioritised events,
- the template faudes::TpGenerator for a generator with prioritised events, and
- the function faudes::ShapePriorities to implement the premting effect for a given generator.
Usage should be fairly intuitive, see also the provided tutorials.
</p>

<p>
Regarding algorithms, we provide the two function
- faudes::IsPNonconflicting and
- faudes::IsPFNonconflicting
Both functions implement a compositional approach to the verfication of non-conflictingness for the
parallel composition of multiple components with a global interpretation of preemption by event
priorities. Thechnical details have been reported in 
</p>

<p>
Y.&nbsp;Tang, T.&nbsp;Moor:
Compositional non-blockingness verification of finite automata with prioritised events,
<i>Discrete-Event Dynamic Systems</i>, 2024.
</p>

<p>

The code base is Yiheng Tang's original work with some later cos,etic extensions to address
consistent integration with libFAUDES and Lua bindings.
</p>


@subsection PevLicense License

<p>
The priosities plugin is distributed with libFAUDES and under the terms of
the LGPL.
</p>
<br>
<p>
Copyright (c) 2024,2025,  Yiheng Tang, Thomas Moor.
</p>


*/
