/** @file hyp_compute.h  Polyhedra calculus wrapper */

#ifndef HYB_COMPUTE_H
#define HYB_COMPUTE_H

#include <string>
#include "corefaudes.h"
#include "hyb_parameter.h"


namespace faudes{
	

/** 
 * User data aware assignment 
 *
 * @param src
 *      source
 * @param dst
 *      destination
 * @ingroup HybridPlugin	
 */
void FAUDES_API PolyCopy(const Polyhedron& src, Polyhedron& dst);


/** 
 * Recover faudes parameters from user data
 *
 * @param src
 *      source
 * @ingroup HybridPlugin	
 */
void FAUDES_API PolyFinalise(const Polyhedron& fpoly);

/** 
 * Dump verticies
 *
 * @param src
 *      source
 * @ingroup HybridPlugin	
 */
void FAUDES_API PolyDWrite(const Polyhedron& fpoly);


/** 
 * Intersection
 *
 * @param poly
 *      polyhedron
 * @param res
 *      result gets intersected with poly
 * @ingroup HybridPlugin	
 */
void FAUDES_API PolyIntersection(const Polyhedron& poly, Polyhedron& res);

/** 
 * Test emptyness
 *
 * @param poly
 *      polyhedron
 * @return 
 *      returns tru on empty polyhedra
 * @ingroup HybridPlugin	
 */
bool FAUDES_API PolyIsEmpty(const Polyhedron& poly);

/** 
 * Inclusion
 *
 * @param poly
 *      one polyhedron
 * @param other
 *      other polyhedron
 * @return
        true if poly <= other
 * @ingroup HybridPlugin	
 */
bool FAUDES_API PolyInclusion(const Polyhedron& poly, const Polyhedron& other);

/** 
 * Apply time-elapse on specified polyhedron
 *
 * @param rate
 *      right hand side of diff.incl.
 * @param poly
 *      state set
 * @ingroup HybridPlugin	
 */
void FAUDES_API PolyTimeElapse(const Polyhedron& rate, Polyhedron& poly);

/** 
 * Apply reset relation A'x' + Ax <= B to specified polyhedron 
 *
 * @param relation
 *      linear relation
 * @param poly
 *      state set
 * @ingroup HybridPlugin	
 */
void FAUDES_API PolyLinearRelation(const LinearRelation& reset, Polyhedron& poly);


	
	
}//namespace faudes

#endif
