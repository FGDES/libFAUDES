/** @file pex_altaccess.h Example plugin */

#ifndef FAUDES_PEX_ALTACCESS_H
#define FAUDES_PEX_ALTACCESS_H

#include "corefaudes.h"

namespace faudes {


/**
 * Alternative accessibility algorithm. 
 * We use the alternative accessibility algorithm from tutorial 6 
 * for our example plugin. The implementation uses a todo stack to discover
 * accessible states and finaly remove all other states from the
 * input generator.
 *
 * <h4>Example:</h4>
 * <table width=100%> 
 * <tr>
 * <td> @image html pex_g_notacc.png </td>
 * </tr>
 * </table> 
 *
 * @param rGen
 *   Input generator
 *
 * @ingroup ExamplePlugin 
 *
 */

void AlternativeAccessible(Generator& rGen);


} // namespace faudes

#endif 
