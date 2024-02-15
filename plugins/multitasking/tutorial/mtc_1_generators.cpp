/** @file mtc_1_generators.cpp 

Tutorial,  MtcSystem methods.

@ingroup Tutorials 

@include mtc_1_generators.cpp
*/


#include "libfaudes.h"


// make faudes namespace available to our program
using namespace faudes;


/////////////////
// main program
/////////////////

int main() {

  /***************************************************************
   * Constructor and file io
   *
   ***************************************************************/

  // At first we create an empty MtcSystem object
  MtcSystem g1;

  // Insert states
  Idx s1 = g1.InsState("up");
  Idx s2 = g1.InsInitState("middle");

  // Insert events
  Idx e1, e2;
  e1 = g1.InsEvent("go_up");
  e2 = g1.InsEvent("go_down");

  // Set transitions - by name or index
  g1.SetTransition("middle", "go_up", "up");
  g1.SetTransition(s1, e2, s2);

  // Generate further MtcSystem objects by reading a generator from a file...
  MtcSystem g2("data/mtc_generator_1.gen");

  MtcSystem g3;
  g3.Read("data/mtc_generator_1.gen");

  // ...or by copying a generator
  MtcSystem g4=g3;

  // Output of MtcSystem to file
  g4.Write("tmp_mtc_generator_1.gen");
  g1.Write("tmp_mtc_generator_2.gen");

  // Write MtcSystem to console (for debugging)
  std::cout << "##########################################" << std::endl;
  std::cout << "# mtcgenerators - constructors and file io" << std::endl;
  std::cout << std::endl << "# MtcSystem without colors:" << std::endl << std::endl;
  g1.DWrite();
  std::cout << std::endl << "# MtcSystem with colors:" << std::endl << std::endl;
  g2.DWrite();
  std::cout << std::endl << "# same MtcSystem, but read from file using the Read() method:" << std::endl << std::endl;
  g3.DWrite();
  std::cout << "##########################################" << std::endl << std::endl;

  
  /***************************************************************
   * Output to .dot-file for Graphviz visualization
   *
   ***************************************************************/

  // Generator .dot-file for Graphviz dot

  // Output of the MtcSystem to the dot format, where states are colored
  g1.DotWrite("tmp_mtc_generator_1.dot");

  // Graphical output of the MtcSystem, invokes Graphviz dot
  g2.GraphWrite("tmp_mtc_generator_2.png");


  /***************************************************************
   * Editing state attributes - colored states
   *
   ***************************************************************/

  // Insert a new colored state
  Idx s3 = g1.InsColoredState("down", "low");

  // Set further transitions
  g1.SetTransition(s2, e2, s3);
  g1.SetTransition(s3, e1, s2);

  // Set a color for an existing state
  Idx c1 = g1.InsColor(s1, "high");
  g1.InsColor(s2, c1);

  // test output
  g1.Write("tmp_mtc_generator_3.gen");
  g1.GraphWrite("tmp_mtc_generator_3.png");

  // The color label of the second state is wrong
  // ==> delete it
  g1.DelColor(s2, "high");

  // Test output
  g1.Write("tmp_mtc_generator_4.gen");
  g1.GraphWrite("tmp_mtc_generator_4.png");

  // Find out index of a color
  Idx c3 = g1.ColorIndex("high");
  std::cout << "Index of color \"high\": " << c3 << std::endl;

  // Find out name of color
  std::string color3 = g1.ColorName(c3);
  std::cout << "Color name for index " << c3 << ": " << color3 << std::endl;

  // Delete one color from all states
  g1.DelColor(c3);

  // Test output
  g1.Write("tmp_mtc_generator_5.gen");
  g1.GraphWrite("tmp_mtc_generator_5.png");

  // Delete all colors from all states
  g1.ClearStateAttributes();

  // Test output
  g1.Write("tmp_mtc_generator_6.gen");
  g1.GraphWrite("tmp_mtc_generator_6.png");

  // Reinsert color "high"
  g1.InsColor(s1, c3);

  // Test output
  g1.Write("tmp_mtc_generator_7.gen");
  g1.GraphWrite("tmp_mtc_generator_7.png");

  // delete all colors from a particular state
  g1.ClrStateAttribute(s1);

  // test output
  g1.Write("tmp_mtc_generator_8.gen");
  g1.GraphWrite("tmp_mtc_generator_8.png");

  // reinsert color
  g1.InsColor(s1, c1);

  // test output
  g1.Write("tmp_mtc_generator_9.gen");
  g1.GraphWrite("tmp_mtc_generator_9.png");

  // Lookup name of reinserted color
  std::string name = g1.ColorName(c1);

  // Color names are not deleted from symbol table as long as the global
  // color symbol table is used. 
  std::cout << "Color name for index " << c1 << ": " << name << std::endl;
  
  // Check if color exists in generator
  // (therefore iterate over all states -> expensive method)
  if(!g1.ExistsColor(c1)) 
    std::cout << "Color c1 does not exist in g1 anymore" << std::endl;
  else 
    std::cout << "Color c1 exists in g1" << std::endl;

  // Check if a color exists in a particular state
  if(!g1.ExistsColor(s2, c1))
    std::cout << "State s2 is not colored by color c1" << std::endl;
  if(g1.ExistsColor(s1, c1))
    std::cout << "State s1 is colored by color " << g1.CStr(c1) << std::endl;

  // Collect all colors of the generator
  // Directly inserts colors into the specified reference of a color set
  ColorSet allColors1;
  g1.Colors(allColors1);
  
  // Creates temporary color set, inserts all colors,
  // and copies the set to allcolors2 afterwards
  ColorSet allColors2 = g1.Colors();

  // Get all colors of a particular state
  const ColorSet& allStateColors =g1.Colors(s1);

  // Print color to console
  allStateColors.DWrite();
  
  // Clear MtcSystem g1
  g1.Clear();


  /////////////////////////////////////////////////////
  // Local color symbol table
  //
  // In the current implementation, a symbol table is treated local 
  // whenever it is different to the global one. The implementation
  // then tries to keep book and remove symbols that are not referenced.
  // However, this feature in its current form is inefficient and not
  // very useful. It will disapper in a future version.
  //
  /////////////////////////////////////////////////////

  {
    MtcSystem gen, copygen;

    Idx st1 = gen.InsInitState("1");
    Idx st2 = gen.InsState("2");

    Idx eva = gen.InsEvent("a");
    Idx evb = gen.InsEvent("b");
    Idx evc = gen.InsEvent("c");

    Idx c1 = gen.InsColor(st1, "first");
    Idx c2 = gen.InsColor(st2, "second");

    gen.SetTransition(st1, eva, st2);
    gen.SetTransition(st2, evb, st2);
    gen.SetTransition(st2, evc, st1);

    std::cout << "gen: Color name of c1 = " << gen.ColorName(c1) << std::endl;
    std::cout << "gen: Color index of first = " << gen.ColorIndex("first") << std::endl;

    // Generate new color symbol table for gen,
    // all color labels already contained in gen are copied
    gen.NewColorSymbolTable();

    std::cout << "gen: Color name of c1 = " << gen.ColorName(c1) << std::endl;
    std::cout << "gen: Color index of first = " << gen.ColorIndex("first") << std::endl;

    copygen.Assign(gen);

    gen.DelColor(c1);

    std::cout << "copygen: Color name of c1 = " << copygen.ColorName(c1) << std::endl;
    std::cout << "copygen: Color index of first = " << copygen.ColorIndex("first") << std::endl;

    Idx c3 = copygen.InsColor(st1, "New_Color");

    if (!gen.ExistsColor(c1)) std::cout << "gen: Index c1 does not exist!" << std::endl;
    else std::cout << "gen: Index c1 exists!" << std::endl;
    try {
      std::cout << "gen: Color name of c1 = " << gen.ColorName(c1) << std::endl;
    }
    catch (faudes::Exception& exception){
      std::cout << "gen: Color name of c1 does not exist any more" << std::endl;
    }

    if (!gen.ExistsColor(c3)) std::cout << "gen: Index c3 does not exist!" << std::endl;
    else std::cout << "gen: Index c3 exists!" << std::endl;
    try {
      std::cout << "gen: Color name of c3 = " << gen.ColorName(c1) << std::endl;
    }
    catch (faudes::Exception& exception){
      std::cout << "gen: Color name of c3 does not exist any more" << std::endl;
    }

    if (!copygen.ExistsColor(c1)) std::cout << "copygen: Index c1 does not exist!" << std::endl;
    else std::cout << "copygen: Index c1 exists!" << std::endl;
    try {
      std::cout << "copygen: Color name of c1 = " << copygen.ColorName(c1) << std::endl;
    }
    catch (faudes::Exception& exception){
      std::cout << "copygen: Color name of c1 does not exist any more" << std::endl;
    }

    if (!copygen.ExistsColor(c3)) std::cout << "copygen: Index c3 does not exist!" << std::endl;
    else std::cout << "copygen: Index c3 exists!" << std::endl;
    try {
      std::cout << "copygen: Color name of c3 = " << copygen.ColorName(c1) << std::endl;
    }
    catch (faudes::Exception& exception){
      std::cout << "copygen: Color name of c3 does not exist any more" << std::endl;
    }

  }

  return 0;
}
