/************************************************
 ************************************************
 ************************************************/

/**
 
@mainpage libFAUDES C++ API


<p>
As a C++ library, the natural application interface of libFAUDES is given by C++ class and
function declarations. The API documentation is generated directly from the C++ source code 
and supplemented by <a href="group__Tutorials.html">tutorials</a> that illustrate the intended
ussage by simple example programs. 
</p>
<p>
The C++ API is organized as follows:
<ul class="l2">
<li><p>
<a href="group__ContainerClasses.html">container classes</a> 
for sets of events, states and transitions; 
</p></li>
<li><p>
<a href="group__GeneratorClasses.html">generator classes</a> 
to model finite state machines; 
</p></li>
<li><p>
<a href="group__GeneratorFunctions.html">functions</a> 
that implement general purpose algorithms on generators; and 
</p></li>
<li><p>
<a href="group__AllPlugins.html">plug-ins</a> 
to provide specialised algorithms and derived generator classes.
</p></li>
<li><p>
<a href="group__RunTimeInterface.html">run-time interface (RTI)</a> 
for transparent extensions (advanced topic)
<li><p>
<a href="group__TokenIO.html">token-IO</a> 
to serialize faudes typed objects (advanced topic)
</p></li>
</ul>
</p>


<p>
In addition to the C++ API, libFAUDES provides a 
<a href="../reference/index.html">user-reference</a> to address the typical user 
of a libFAUDES application without bothering with C++ implementation technicalities.
</p>


<br>

@section SecHomeCompile Compile


<p>
libFAUDES is coded in posix C++ and should compile within any recent C++ 
development environment. The supplied
<tt>Makefile</tt> is targeted for the GNU-Tool-Chain and provides the
following build targets:
</p>

<ul class="l2">
<li><p>
<tt>make configure</tt> to configure the library; e.g., select plug-ins, choose dynamic or static linking, etc.
</p></li>
<li><p>
<tt>make</tt> to build the library and utility executables;
</p></li>
<li><p>
<tt>make tutorial</tt> to compile examples from the tutorial section.
</p></li>
<li><p>
<tt>make test</tt> to evaluate included test cases
</p></li>
</ul>

<p>
For more information on how to compile libFAUDES,
see the 
<a href="../faudes_build.html">build-system documentation</a>.
</p>

<br/>


@section SecCoding Coding Conventions


<p>
In order to ease communication, libFAUDES sources try 
to follow some notational conventions. 
They are extracted from 
<a href="http://www.possibility.com/Cpp/CppCodingStandard.html" target="_top">
www.possibility.com/Cpp/CppCodingStandard.html</a>.
</p>

<ul class="l2">
<li><p>
member variable names are upper case  with lower case prefix that
indicates ownership, i.e. 
<tt>m</tt> for ordinary members,
<tt>p</tt> for pointer to variables controlled somewhere else,
<tt>r</tt> for refernces to variables controlled somewhere else, and 
<tt>mp</tt> for pointers to variables controlled here;
</p></li>
<li><p>
function-local stack variables are lower case,  
</p></li>
<li><p>
class names are upper case, with prefix T for class templates.
</p></li>
<li><p>
keep local stuff in the .cpp file, minimize .h files
</p></li>
</ul>

<p>
Moreover, we
</p>
<ul class="l2">
<li><p>
avoid the Set/Get prefixes of members functions and use overloading
instead;
</p></li>
<li><p>
throw exceptions rather than abort() or assert();
</p></li>
<li><p>
use FAUDES_filename_H macro in headers;
</p></li>
<li><p>
use the provided macros in cfl_definitions FD_WARN etc for any console output;
</p></li>
<li><p>
use 2-space indent to have shorter lines of code;
</p></li>
<li><p>
never ever have "using" statements in headers.
</p></li>
</ul>

<br/>

@section SecHomeLicense License 


<p>
The core libFAUDES sources incl. the C++ API 
are provided for free under conditions of the GNU Lesser General Public License. 
Note that different and perhaps more restrictive license terms may apply to the plug-ins that reside in the plug-in directory.
</p>




<br>
<br>
<br>


<p>
Copyright (C) 2006 Bernd Opitz <br>
Copyright (C) 2008 - 2010 Thomas Moor, Klaus Schmidt, Sebastian Perk <br>
Copyright (C) 2010 - 2024 Thomas Moor, Klaus Schmidt<br>
</p>
*/

/************************************************
 ************************************************
 ************************************************/


/** @defgroup GeneratorClasses  Generator Classes

A generator is a tupel G = (Q, Sigma, Delta, Qo, Qm). Generators are used
to represent the closed and the marked language L(G) 
and L_m(G) respectively. The core members of FAUDS generator classes are
- mpStates, the stateset Q, type faudes::StateSet;
- mpAlphabet,  the Alphabet Sigma, type faudes::EventSet;
- mpTransRel, the transitionrelation Delta, type faudes::TransSet; 
- mInitStates, the set of initial states Qo, type faudes::StateSet;
- mMarkedStates, the set of marked states Qm, type faudes::StateSet.

The base class for all libFAUDES generator classes is the faudes::vGenerator,
also accessible as typedef faudes::Generator.
It provides functions for read and write access to the core members and file IO. 
Derived generator classes provide attibutes attached to events, states and transitions 
in order to extend basic generator semantics. 
Examples for attributes are controllability flags for events; 
see also faudes::TcGenerator or faudes::System.

*/

/************************************************
 ************************************************
 ************************************************/

/** @defgroup ContainerClasses State, Event and Transition Set

This module collects all basic container classes aka state- event- 
and transition-sets, e.g.,

- TBaseSet   base class for all libFaudes containers,
- IndexSet   set of plain indices,
- TaIndexSet set of indices with attributes, 
- NameSet    set of indices with symbolic names, 
- TaNameSet  set of indices with attributes and symbolic names, 
- TTransSet  set of transitions in a sepcific order.

All libFAUDES container classes share the same basic API 
similar to the one known from STL containers. In addition,
they implement file IO via faudes token streams and/or XML.
Most commonly used is the plain faudes::EventSet and the faudes::Alphabet.
Plug-ins introduce specialized sets for their purposes. 

*/

/************************************************
 ************************************************
 ************************************************/

/** @defgroup GeneratorFunctions  General Purpose Functions

This module collects general purpose functions on 
Generator, System, EventSet and Alphabet typed data. 
It includes functions related to regular expressions,
projection, parallel composition, etc. 

*/


/************************************************
 ************************************************
 ************************************************/

/** @defgroup Tutorials  Tutorials

This module collects all tutorials. The tutorials are seen as a pragmatic
introduction to supplement the doxygen generated html documentaion of 
class and function definitions. Plug-Ins are meant to contribute additional tutorials 
to illustrate their intended usage.  

To compile the tutorials call make with target &quot;tutorial&quot; 
in the &quot;libfaudes&quot; directory of the source tree:
<pre>$ make tutorial
</pre>


*/

/************************************************
 ************************************************
 ************************************************/


/** @defgroup AllPlugins PlugIns

List of all plug-ins that are installed and enabled
in this copy of libFAUDES.

To install a plug-in, copy its sources into the plug directory
of the libfaudes source tree. To enable a plug-in, either edit the
main Makefile or add its name to the FAUDES_PLUGINS environment variable.
The <a href="../faudes_algorithm_example.html">example plug-in</a> 
further illustrates the plug-in infrastructure.

For instructions on how to compile libFAUDES, see the 
<a href="../faudes_build.html">build-system</a> documentation.

*/

/************************************************
 ************************************************
 ************************************************/

/** @defgroup TokenIO  Faudes object serialization.

Objects of a class derived from the universal base faudes::Type
provide a uniform interface for serialization, which are to be
implemented for derived classes. This is supported by 
- the faudes::Token class to model atomic data units like integers, sections tags and strings.
- the faudes::TokenReader class to read a tokenized character stream from file or string
- the faudes::TokenWriter class to write a tokenized character stream to file or string

A token hast value types attached, e.g., type string, number, and/or tag. 
Tags indicate the beginning or end of a section, must balance, and must be XML compliant.
The character data inbetween the tags is interpreted as a sequence of strings and numbers,
to e.g. conveniently represent a list of events, transition or states. While the Token class
implements reading and writing individual tokens from a C++ stream, the TokenReader and TokenWriter
classes keep track of nested sections. Thus, opening a file via a TokenReader allows to scan for
a particular section or to loop over all tokens from a particular section.

Tokens by example
- <tt>abcd</tt>          the string "abcd"
- <tt>"ab cd"</tt>       the same string containing a blank
- <tt>'ad\"cd';</tt>       the same string containing a double quote
- <tt>abc\\d</tt>         the same string containing a backslash
- <tt>"123"</tt>         the string 123
- <tt>123</tt>           the non-negative integer 123 
- <tt>0xFF</tt>          the non-negative integer 255 (hexadecimal notation)
- <tt>1.5</tt>           the float number 1.5
- <tt>&lt;Start&gt;</tt>  tag to indicate begin of section Start (so called XML element)
- <tt>&lt;/Start&gt;</tt> tag to indicate end of section Start (so called XML element)
- <tt>&lt;Start value="x12"&gt;</tt>  begin tag with XML attribute

Faudes serialisation example
@code
<Generator name="simple machine">

<Alphabet>                         
alpha +C+ beta mue lambda      
</Alphabet>

<States>                           
idle busy down
</States>

<TransRel>
idle alpha busy
busy beta idle
busy mue down
down lambda idle
</TransRel>

<InitStates>
idle
</InitStates>

<MarkedStates>
idle
</MarkedStates>

% this is a comment: it will be ignored by TokenReader

<!-- another comment, will be ignored by TokenReader and other XML parsers -->

</Generator>
@endcode

<br>

Note that the special characters <tt>&lt;</tt>, <tt>&gt;</tt> and <tt>&amp;</tt> must be
quoted as entities, i.e., represented as <tt>&amp;xxx;</tt> where the <tt>xxx</tt> refers to the
character. This style of quoting is also required when the respective character appears within
a quoted string or a faudes-style comment. This restriction is required for XML compliance.

<br>
<br>



<b>Technical Note: alternative file format</b>

libFAUDES token formats are intended to be human editable. However, the
specific interpretation of the character data between the begin and end elements
can not be covered a formal XML document model. The design objective here
was a balance between a formal model and human editable data input.

Since the initial design, however, a number of faudes types where inroduced to 
configure the run-time behaviour of tools like the simulator or hardware access. 
Theese kind of objects are seldomly edited and we feel that a more rigoruos 
XML format is required to make use of professional XML editors/tools.
Therefore we started in libFAUDES 2.16f to implement an alternative serialization 
format. Recent versions of libFAUDES still read the old format and provide
the  utility <tt>libfaudes/bin/fts2ftx</tt> for file conversion. 
Please let us know if you experience problems with data in the old format.

Typical user data like generators and alphabets by default continue to use
the original file format for token output. However, for applications that require 
the more rigoruos XML format, the base class Type provides the additional 
interface XWrite().

*/

/************************************************
 ************************************************
 ************************************************/

/** @namespace faudes

libFAUDES resides within the namespace faudes. Plug-Ins may use the same namespace.

*/
