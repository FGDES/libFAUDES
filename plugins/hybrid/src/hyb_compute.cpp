/** @file hyb_compute.cpp  Polyhedron caluculs via PPL */

#include "hyb_compute.h"
#include <ppl.hh>



namespace faudes{

/** user data type for polyhedra */	
class pdata_t : public Type {
public:
  Parma_Polyhedra_Library::C_Polyhedron mPPLpoly;
  bool mChanged;
}; 

  
/** 
convert a faudes style polyhedron "A x <= B" to a PPL polyhedron,
and keep the conversion result in the UserData entry
*/
Parma_Polyhedra_Library::C_Polyhedron& UserData(const Polyhedron& fpoly) {
  pdata_t* pdata= dynamic_cast<pdata_t*>(fpoly.UserData());
  if(!pdata) {
    pdata = new pdata_t();
    Parma_Polyhedra_Library::C_Polyhedron& ppoly = pdata->mPPLpoly;
    ppoly.add_space_dimensions_and_embed(fpoly.Dimension());
    for(Idx i = 0; i < fpoly.Size(); i++) {
      Parma_Polyhedra_Library::Linear_Expression e;
      for(int j = 0; j < fpoly.Dimension(); j++) {
	Parma_Polyhedra_Library::Variable xj(j);
	e += fpoly.A().At(i,j) * xj;
       }
       ppoly.add_constraint(e <= fpoly.B().At(i));
    }
    pdata->mChanged=false;
    fpoly.UserData(pdata);
    pdata= dynamic_cast<pdata_t*>(fpoly.UserData());
  }
  if(!pdata) {
    std::stringstream errstr;
    errstr << "Internal Computaional Error (Convert to PPL).";
    throw Exception("UserData", errstr.str(), 700);
  }
  return pdata->mPPLpoly;
}

/** 
convert PPL polyhedron back to faudes data structures;
this is required if we manipulate a polyhedron and like
to access it from libFAUDES
*/ 
void PolyFinalise(const Polyhedron& fpoly){
  // nothing to do if no user data
  if(fpoly.UserData()==NULL) return;
  // get user data
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(fpoly);
  Parma_Polyhedra_Library::Constraint_System consys=ppoly.minimized_constraints();
  // sanity check
  if(fpoly.Dimension()!=0)
  if(ppoly.space_dimension()!= ((unsigned int) fpoly.Dimension())) {
    std::stringstream errstr;
    errstr << "Internal Computaional Error (Conversion from PPL, dimension mismatch).";
    throw Exception("PolyFinalise", errstr.str(), 700);
  }
  /*
  if(consys.has_equalities()){
    std::stringstream errstr;
    errstr << "Internal Computaional Error (Conversion from PPL, cannot hanlde equalities).";
    throw Exception("PolyFinalise", errstr.str(), 700);
  }
  */
  if(consys.has_strict_inequalities()){
    std::stringstream errstr;
    errstr << "Internal Computaional Error (Conversion from PPL, cannot hanlde strict inequalities).";
    throw Exception("PolyFinalise", errstr.str(), 700);
  }
  // extract constraints parameters
  Parma_Polyhedra_Library::Constraint_System::const_iterator cit = consys.begin();
  Parma_Polyhedra_Library::Constraint_System::const_iterator cit_end = consys.end();
  int count=0;
  for(;cit!=cit_end;++cit) { ++count; if(cit->is_equality()) ++count; }
  Matrix A(count,ppoly.space_dimension());
  Vector B(count);
  int i=0;
  for(cit = consys.begin();cit!=cit_end;++cit) {
    if(cit->is_nonstrict_inequality()) {
      for(unsigned int j = 0; j < ppoly.space_dimension(); j++) {
        Parma_Polyhedra_Library::Variable xj(j);
        A.At(i,j,-1* cit->coefficient(xj).get_d());
      }
      B.At(i,cit->inhomogeneous_term().get_d());
      ++i;
    }
    if(cit->is_equality()) {
      for(unsigned int j = 0; j < ppoly.space_dimension(); j++) {
        Parma_Polyhedra_Library::Variable xj(j);
        A.At(i,j,-1*cit->coefficient(xj).get_d());
        A.At(i+1,j,cit->coefficient(xj).get_d());
      }
      B.At(i,cit->inhomogeneous_term().get_d());
      B.At(i+1,-1* cit->inhomogeneous_term().get_d());
      i+=2;
    }
  }
  const_cast<Polyhedron&>(fpoly).AB(A,B);
}

/** user data type for reset relation (fake faudes type) */	
class rdata_t : public Type {
public:
  Parma_Polyhedra_Library::C_Polyhedron mPPLrelation;
}; 

/** 
convert a faudes style linear relation "A' x' + A x <= B" to a PPL polyhedron,
and keep the conversion result in the UserData entry
*/
Parma_Polyhedra_Library::C_Polyhedron& UserData(const LinearRelation& frelation) {
  rdata_t* rdata= dynamic_cast<rdata_t*>(frelation.UserData());
  if(!rdata) {
    int dim=frelation.Dimension();
    rdata = new rdata_t();
    Parma_Polyhedra_Library::C_Polyhedron& prelation = rdata->mPPLrelation;
    prelation.add_space_dimensions_and_embed(2*dim);
    for(Idx i = 0; i < frelation.Size(); i++) {
      Parma_Polyhedra_Library::Linear_Expression e;
      for(int j = 0; j < dim; j++) {
	Parma_Polyhedra_Library::Variable xj(j);
	e += frelation.A1().At(i,j) * xj;
      }
      for(int j = 0; j < dim; j++) {
	Parma_Polyhedra_Library::Variable xjprime(dim+j);
	e += frelation.A2().At(i,j) * xjprime;
      }
      prelation.add_constraint(e <= frelation.B().At(i));
    }
    frelation.UserData(rdata);
    rdata= dynamic_cast<rdata_t*>(frelation.UserData());
  }
  if(!rdata) {
    std::stringstream errstr;
    errstr << "Internal Computaional Error (Convert to PPL).";
    throw Exception("UserData", errstr.str(), 700);
  }
  return rdata->mPPLrelation;
}





/** poly dump */
void PolyDWrite(const Polyhedron& fpoly){
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(fpoly);
  Parma_Polyhedra_Library::Generator_System gensys = ppoly.minimized_generators(); 
  gensys.ascii_dump();
}


/** copy method */  
void PolyCopy(const Polyhedron& src, Polyhedron& dst) {
  // copy faudes data
  dst.Assign(src);
  // bail out if there is no user data
  pdata_t* spdata= dynamic_cast<pdata_t*>(src.UserData());
  if(!spdata) return;  
  // copy user data
  pdata_t* dpdata = new pdata_t();
  dpdata->mChanged=spdata->mChanged;
  dpdata->mPPLpoly=spdata->mPPLpoly;
  dst.UserData(dpdata);
}

/** intersection */
void PolyIntersection(const Polyhedron& poly, Polyhedron& res) {
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(poly);
  Parma_Polyhedra_Library::C_Polyhedron& pres=UserData(res);
  pres.intersection_assign(ppoly);
  dynamic_cast<pdata_t*>(res.UserData())->mChanged=true;
}

  

/** test emptyness */
bool PolyIsEmpty(const Polyhedron& poly) {
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(poly);
  return ppoly.is_empty();
}

/** inclusion */
bool PolyInclusion(const Polyhedron& poly, const Polyhedron& other) {
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(poly);
  Parma_Polyhedra_Library::C_Polyhedron& opoly=UserData(other);
  return opoly.contains(ppoly);
}

/** scratch */
void PolyScratch(void) {
  // diamond wint center (0,1)
  FD_WARN("setting up diamond");
  Parma_Polyhedra_Library::C_Polyhedron ppoly(2);
  Parma_Polyhedra_Library::Variable x1(0);
  Parma_Polyhedra_Library::Variable x2(1);
  ppoly.add_constraint(x2 <= x1 + 2);
  ppoly.add_constraint(x2 >= x1 );
  ppoly.add_constraint(x2 <= -x1 + 2);
  ppoly.add_constraint(x2 >= -x1 );
  Parma_Polyhedra_Library::Generator_System gensys = ppoly.minimized_generators(); 
  gensys.ascii_dump();
  // encode reset bloat
  Parma_Polyhedra_Library::C_Polyhedron preset = ppoly;
  preset.add_space_dimensions_and_embed(4);
  Parma_Polyhedra_Library::Variable z1(2);
  Parma_Polyhedra_Library::Variable z2(3);
  preset.add_constraint(z1 - 2*x1 <= 2 +100);
  preset.add_constraint(-z1 + 2*x1 <= 2 - 100);
  preset.add_constraint(z2 - 2*x2 <= 2 - 2 );
  preset.add_constraint(-z2 + 2*x2 <= 2 + 2 );
  // project to z1/z2  
  preset.remove_space_dimensions(Parma_Polyhedra_Library::Variables_Set(x1,x2));
  preset.remove_higher_space_dimensions(2);
  gensys = preset.minimized_generators(); 
  gensys.ascii_dump();
}

/** apply reset relation A'x' + Ax <= B */
void PolyLinearRelation(const LinearRelation& reset, Polyhedron& poly) {
  // bail out on default 
  if(reset.Identity()) return;
  Parma_Polyhedra_Library::C_Polyhedron& preset=UserData(reset);
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(poly);
  // sanity check
  unsigned int dim=ppoly.space_dimension();
  if(preset.space_dimension() != 2 * dim) {
    std::stringstream errstr;
    errstr << "Internal Computaional Error (dimension mismatch).";
    throw Exception("PolyLinearRelation", errstr.str(), 700);
  }
  // include reset constraints with polyhedron
  ppoly.add_space_dimensions_and_embed(dim);
  ppoly.add_constraints(preset.constraints());
  // drop original x variables in favour of x'
  Parma_Polyhedra_Library::Variables_Set xvars;
  for(unsigned int j=0; j<dim;++j) xvars.insert(Parma_Polyhedra_Library::Variable(j));
  ppoly.remove_space_dimensions(xvars);
  ppoly.remove_higher_space_dimensions(dim);
}

/** time elapse */
void PolyTimeElapse(const Polyhedron& rate, Polyhedron& poly) {
  Parma_Polyhedra_Library::C_Polyhedron& prate=UserData(rate);
  Parma_Polyhedra_Library::C_Polyhedron& ppoly=UserData(poly);
  if(prate.is_empty()) {
    //FD_WARN("PolyTimeElapse(...): empty rate");
  } else {
    ppoly.time_elapse_assign(prate);
  }
  dynamic_cast<pdata_t*>(poly.UserData())->mChanged=true;
}




}//namespace faudes
