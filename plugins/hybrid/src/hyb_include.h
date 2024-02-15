/** @file hyp_include.h Include hybrid plugin headers */

/*
 **************************************************** 

 Convenience header file that includes all headers 
 relevant to the hybrid systems plugin

 The make system will append an include directive 
 for this file to the "allplugins.h" include file.

 (c) 2010-2017 Thomas Moor

 ****************************************************
 */

#ifndef FAUDES_HYB_INCLUDE_H
#define FAUDES_HYB_INCLUDE_H

#include "hyb_parameter.h"
#include "hyb_compute.h"
#include "hyb_attributes.h"
#include "hyb_hgenerator.h"
#include "hyb_reachability.h"
#include "hyb_experiment.h"
#include "hyb_abstraction.h"

#endif


/**
 

@defgroup HybridPlugin Hybrid Systems PlugIn


@ingroup AllPlugins

@section Overview


<p>
This plug-in provides basic support for linear hybrid automata.
The long term perspective is to implement abstraction based
synthesis methods. The current implementation supports the C++ API only,
it provides neither dedicated reference pages nor lua-bindings. 
</p>

@section License

<p>
The hybrid systems plug-in is distributed with libFAUDES and under the terms of
the LGPL. 
</p>
<p>
The current implementation links to the GPL licensed libraries GMP and PPL.
Effectively, this renders any derived work/application that includes libFAUDES with
the hyprid systems plug-in GPL licensed (as opposed to LGPL). 
</p>



@section Contents
@brief



*/
