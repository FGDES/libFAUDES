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
 

@defgroup PriositiesPlugin Priorities PlugIn


@ingroup AllPlugins

@section Overview


<p>
The priorities plug-in addresses systems with prioritised events. Detailed documentation
will be provieded in due course via the user reference.
</p>

@section License

<p>
The priosities plugin is distributed with libFAUDES and under the terms of
the LGPL.
</p>

@section Contents

<p>
For the methodological background of Yiheng Tang's work on the verfication of non-blockingness for
modular diccrete-event systems with prioritised events was, see:
<p>

<p>
Y.&nbsp;Tang, T.&nbsp;Moor:
Compositional non-blockingness verification of finite automata with prioritised events,
<i>Discrete-Event Dynamic Systems</i>, 2024.
</p>

<p>
Yiheng's original implementation was designed as a
libFAUDES application. At this stage, we are transforming the code base to a
libFAUDES plug-in, incl. integration with the faudes registry and a luaFAUDES interface.
</p>




*/
