/** @file hyb_parameter.cpp  Parameters of linear hybrid automata */

/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2010  Thomas Moor

*/



#include "hyb_parameter.h"

using namespace faudes;


/*
*************************************************************************
*************************************************************************
*************************************************************************

Implementation: Matrix

*************************************************************************
*************************************************************************
*************************************************************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Matrix,Matrix,Type)

// Constructor
Matrix::Matrix(void) : 
  mRC(0),
  mCC(0),
  mpData(0)
{
}

// Constructor
Matrix::Matrix(int rc, int cc) :
  mRC(0),
  mCC(0),
  mpData(0)
{
  if(rc<0 ||  cc<0) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Matrix::Matrix()", errstr.str(), 700);
  }
  mRC=rc;
  mCC=cc;
  if(mRC>0)
  if(mCC>0)
    mpData = new Scalar::Type[mRC*mCC];
}  


// Constructor
Matrix::Matrix(const Matrix& rSrc) : 
  mRC(0),
  mCC(0),
  mpData(0)
{
  DoAssign(rSrc);
}
  
// Constructor
Matrix::Matrix(const std::string& rFileName) :
  mRC(0),
  mCC(0),
  mpData(0)
{
  Read(rFileName);
}


// Destructor
Matrix::~Matrix(void) {
  if(mpData) delete mpData;
}


// virtual Assignment method
Matrix& Matrix::DoAssign(const Matrix& rSrc) {
  mRC=rSrc.mRC;
  mCC=rSrc.mCC;
  if(mpData) delete mpData;
  mpData=0;
  if(mRC>0)
  if(mCC>0)
    mpData = new Scalar::Type[mRC*mCC]; 
  int sz=Size();
  for(int k=0; k<sz; k++) 
    mpData[k]=rSrc.mpData[k];
  return *this; 
}

// virtual compare
bool Matrix::DoEqual(const Matrix& rSrc) const {
  if(mRC!=rSrc.mRC) return false;
  if(mCC!=rSrc.mCC) return false;
  int sz=Size();
  for(int k=0; k<sz; k++) 
    if(mpData[k]!=rSrc.mpData[k]) return false;
  return true;
}


// Clear
void Matrix::Clear(void) {
  mRC=0;
  mCC=0;
  if(mpData) delete mpData;
  mpData=0;
}


// Size
Idx Matrix::Size(void) const {
  return mRC*mCC;
}


// Set Dimension.
void Matrix::RowCount(int rc) {
  if(mRC==rc) return;
  Clear();
  if(rc<0) return;
  mRC=rc;
  if(mRC>0)
  if(mCC>0)
    mpData = new Scalar::Type[mRC*mCC];
}


// Set Dimension.
void Matrix::ColumnCount(int cc) {
  if(mCC==cc) return;
  Clear();
  if(cc<0) return;
  mCC=cc;
  if(mRC>0)
  if(mCC>0)
    mpData = new Scalar::Type[mRC*mCC];
}

// Set Dimension.
void Matrix::Dimension(int rc, int cc) {
  if(mCC==cc && mRC==rc) return;
  Clear();
  if(rc<0) return;
  if(cc<0) return;
  mRC=rc;
  mCC=cc;
  if(mRC>0)
  if(mCC>0)
    mpData = new Scalar::Type[mRC*mCC];
}

// Get Dimension.
int Matrix::RowCount(void) const {
  return mRC;
}

// Get Dimension.
int Matrix::ColumnCount(void) const {
  return mCC;
}

// Zero
void Matrix::Zero(int rc, int cc) {
  if(rc >=0 && rc >=0) Dimension(rc,cc);
  int sz = Size();
  for(int k=0; k<sz; k++) mpData[k]=0;
}


// Zero
void Matrix::Identity(int dim) {
  if(dim >=0) Dimension(dim,dim);
  int k=0;
  for(int i=0; i<mRC; i++) {
    for(int j=0; j<mCC; j++) {
      mpData[k]=0;
      if(i==j) mpData[k]=1;
      k++;
    }
  }
}


// Access
const Scalar::Type& Matrix::At(int i, int j) const {
  if(i<0 || i>= mRC || j<0 || j>= mCC) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Matrix::At", errstr.str(), 700);
  }
  return mpData[i*mCC+j];
}
     
// Access
void Matrix::At(int i, int j, const Scalar::Type& val) {
  if(i<0 || i>= mRC || j<0 || j>= mCC) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Matrix::At", errstr.str(), 700);
  }
  mpData[i*mCC+j]=val;
}
     
  
// Access
const Scalar::Type& Matrix::operator()(int i, int j) const {
  if(i<0 || i>= mRC || j<0 || j>= mCC) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Matrix::At", errstr.str(), 700);
  }
  return mpData[i*mCC+j];
}

// Access
Scalar::Type& Matrix::operator()(int i, int j) {
  if(i<0 || i>= mRC || j<0 || j>= mCC) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Matrix::At", errstr.str(), 700);
  }
  return mpData[i*mCC+j];
}
     

// C array access
void Matrix::CArray(int rc, int cc, const Scalar::Type *data) {
  Dimension(rc,cc);
  int sz=Size();
  for(int k=0; k<sz; k++)
    mpData[k]=data[k];
} 

// C array access
/*
void Matrix::CArray(int rc, int cc, Scalar::Type **data) {
  Clear();
  if(rc<0) return;
  if(cc<0) return;
  mRC=rc;
  mCC=cc;
  mpData=*data;
}
*/ 

// C array access
const Scalar::Type* Matrix::CArray(void) const {
  return mpData;
}

// token IO
void Matrix::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="Matrix";
  // write section
  Token btoken;
  btoken.SetBegin(label);
  btoken.InsAttributeInteger("rows",mRC);
  btoken.InsAttributeInteger("columns",mCC);
  rTw << btoken;
  int oldcols = rTw.Columns();
  rTw.Columns(mCC);
  for(unsigned int k=0; k<Size(); k++) 
    rTw.WriteFloat(mpData[k]);
  rTw.Columns(oldcols);
  // end section
  rTw.WriteEnd(label);
}

// token IO
void Matrix::DoSWrite(TokenWriter& rTw) const {
  // write dimensions
  rTw.WriteComment("Matrix with dimensions " + ToStringInteger(mRC) 
		   + "x" +ToStringInteger(mCC));
}


// token IO
void Matrix::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="Matrix";
  // find section
  Token btoken;
  rTr.ReadBegin(label,btoken);
  // read dimensions
  int rc=btoken.AttributeIntegerValue("rows");
  int cc=btoken.AttributeIntegerValue("columns");
  // test/allocate (token integers are nonnegative anyway .. but this may change)
  if(rc <0 || cc <0) {
    std::stringstream errstr;
    errstr << "Invalid dimension. " << rTr.FileLine();
    throw Exception("Matrix::DoRead", errstr.str(), 700);
  }
  Dimension(rc,cc);
  // read data
  int k=0;
  while(!rTr.Eos(label) && k<(int)Size()) {
    mpData[k]=rTr.ReadFloat();
    k++;
  }
  // test data
  if(k!=(int)Size()) {
    std::stringstream errstr;
    errstr << "Dimension mismatch. " << rTr.FileLine();
    throw Exception("Matrix::DoRead", errstr.str(), 700);
  }
  // end section
  rTr.ReadEnd(label);
}


/*
*************************************************************************
*************************************************************************
*************************************************************************

Implementation: Vector

*************************************************************************
*************************************************************************
*************************************************************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Vector,Vector,Type)

// Constructor
Vector::Vector(void) : 
  mDim(0),
  mpData(0)
{
}

// Constructor
Vector::Vector(int dim) :
  mDim(0),
  mpData(0)
{
  if(dim<0) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Vector::Vector()", errstr.str(), 700);
  }
  mDim=dim;
  if(mDim>0)
    mpData = new Scalar::Type[mDim];
}  


// Constructor
Vector::Vector(const Vector& rSrc) : 
  mDim(0),
  mpData(0)
{
  DoAssign(rSrc);
}
  
// Constructor
Vector::Vector(const std::string& rFileName) :
  mDim(0),
  mpData(0)
{
  Read(rFileName);
}


// Destructor
Vector::~Vector(void) {
  if(mpData) delete mpData;
}


// virtual Assignment method
Vector& Vector::DoAssign(const Vector& rSrc) {
  mDim=rSrc.mDim;
  if(mpData) delete mpData;
  mpData=0;
  if(mDim>0)
    mpData = new Scalar::Type[mDim];
  for(int k=0; k<mDim; k++) 
    mpData[k]=rSrc.mpData[k];
  return *this; 
}

// virtual compare
bool Vector::DoEqual(const Vector& rSrc) const {
  if(mDim!=rSrc.mDim) return false;
  for(int k=0; k<mDim; k++) 
    if(mpData[k]!=rSrc.mpData[k]) return false;
  return true;
}


// Clear
void Vector::Clear(void) {
  mDim=0;
  if(mpData) delete mpData;
  mpData=0;
}


// Size
Idx Vector::Size(void) const {
  return mDim;
}


// Set Dimension.
void Vector::Dimension(int dim) {
  if(mDim==dim) return;
  Clear();
  if(dim<0) return;
  mDim=dim;
  if(mDim>0)
    mpData = new Scalar::Type[mDim];
}


// Get Dimension.
int Vector::Dimension(void) const {
  return mDim;
}


// Zero
void Vector::Zero(int dim) {
  if(dim >=0) Dimension(dim);
  for(int k=0; k<mDim; k++) mpData[k]=0;
}


// Ones
void Vector::Ones(int dim) {
  if(dim >=0) Dimension(dim);
  for(int k=0; k<mDim; k++) mpData[k]=1;
}


// Access
const Scalar::Type& Vector::At(int i) const {
  if(i<0 || i>= mDim) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Vector::At", errstr.str(), 700);
  }
  return mpData[i];
}
     
// Access
void Vector::At(int i, const Scalar::Type& val) {
  if(i<0 || i>= mDim) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Vector::At", errstr.str(), 700);
  }
  mpData[i]=val;
}
     
  
// Access
const Scalar::Type& Vector::operator()(int i) const {
  if(i<0 || i>= mDim) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Vector::At", errstr.str(), 700);
  }
  return mpData[i];
}

// Access
Scalar::Type& Vector::operator()(int i) {
  if(i<0 || i>= mDim) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Vector::At", errstr.str(), 700);
  }
  return mpData[i];
}
     

// C array access
void Vector::CArray(int dim, const Scalar::Type *data) {
  Dimension(dim);
  for(int k=0; k<dim; k++)
    mpData[k]=data[k];
} 


// C array access
const Scalar::Type* Vector::CArray(void) const {
  return mpData;
}


// token IO
void Vector::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="Vector";
  // write section
  Token btoken;
  btoken.SetBegin(label);
  btoken.InsAttributeInteger("count",mDim);
  rTw << btoken;
  int oldcols = rTw.Columns();
  rTw.Columns(mDim);
  for(int k=0; k<mDim; k++) 
    rTw.WriteFloat(mpData[k]);
  rTw.Columns(oldcols);
  // end section
  rTw.WriteEnd(label);
}

// token IO
void Vector::DoSWrite(TokenWriter& rTw) const {
  // write dimensions
  rTw.WriteComment("Vector with dimension " + ToStringInteger(mDim));
}


// token IO
void Vector::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="Vector";
  // find section
  Token btoken;
  rTr.ReadBegin(label,btoken);
  // read dimensions
  int dim=btoken.AttributeIntegerValue("count");
  // test/allocate (token integers are nonnegative anyway .. but this may change)
  if(dim <0) {
    std::stringstream errstr;
    errstr << "Invalid dimension. " << rTr.FileLine();
    throw Exception("Vector::DoRead", errstr.str(), 700);
  }
  Dimension(dim);
  // read data
  int k=0;
  while(!rTr.Eos(label) && k<(int)Size()) {
    mpData[k]=rTr.ReadFloat();
    k++;
  }
  // test data
  if(k!=(int)Size()) {
    std::stringstream errstr;
    errstr << "Dimension mismatch. " << rTr.FileLine();
    throw Exception("Vector::DoRead", errstr.str(), 700);
  }
  // end section
  rTr.ReadEnd(label);
}


/*
*************************************************************************
*************************************************************************
*************************************************************************

Implementation: Polyhedron

*************************************************************************
*************************************************************************
*************************************************************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(Polyhedron,Polyhedron,Type)

// Constructor
Polyhedron::Polyhedron(void) : 
  mName(),
  mA(0,0),
  mB(0),
  mpUserData(0)
{
}

// Constructor
Polyhedron::Polyhedron(int dim) :
  mName(),
  mA(0,0),
  mB(0),
  mpUserData(0)
{
  if(dim<0) {
    std::stringstream errstr;
    errstr << "Index out of range.";
    throw Exception("Polyhedron::Polyhedron()", errstr.str(), 700);
  }
  mA.Dimension(0,dim);
  mB.Dimension(0);
}  


// Constructor
Polyhedron::Polyhedron(const Polyhedron& rSrc) : 
  mName(),
  mA(0,0),
  mB(0),
  mpUserData(0)
{
  DoAssign(rSrc);
}
  
// Constructor
Polyhedron::Polyhedron(const std::string& rFileName) :
  mName(),
  mA(0,0),
  mB(0),
  mpUserData(0)
{
  Read(rFileName);
}


// Destructor
Polyhedron::~Polyhedron(void) {
  if(mpUserData) delete mpUserData;
}


// virtual Assignment method
Polyhedron& Polyhedron::DoAssign(const Polyhedron& rSrc) {
  mName=rSrc.mName;
  mA=rSrc.mA;
  mB=rSrc.mB;
  if(mpUserData) delete mpUserData;
  mpUserData=0;
  return *this;
}

// virtual compare
bool Polyhedron::DoEqual(const Polyhedron& rSrc) const {
  if(mA!=rSrc.mA) return false;
  if(mB!=rSrc.mB) return false;
  return true;
}


// Clear
void Polyhedron::Clear(void) {
  Dimension(0);
}


// Size
Idx Polyhedron::Size(void) const {
  return mA.RowCount();
}


// Set Dimension.
void Polyhedron::Dimension(int dim) {
  if(dim<0) {
    std::stringstream errstr;
    errstr << "Invalid dimension";
    throw Exception("Polyhedron::Dimension", errstr.str(), 700);
  }
  mA.Dimension(0,dim);
  mB.Dimension(0);
  if(mpUserData) delete mpUserData;
  mpUserData=0;
}


// Get Dimension.
int Polyhedron::Dimension(void) const {
  return mA.ColumnCount();
}


// Access
const Matrix& Polyhedron::A(void) const {
  return mA;
}
     
// Access
const Vector& Polyhedron::B(void) const {
  return mB;
}
     
// Access
void Polyhedron::AB(const Matrix& rA, const Vector& rB) {
  if(rA.RowCount()!=rB.Dimension()) {
    std::stringstream errstr;
    errstr << "Invalid dimansions.";
    throw Exception("Polyhedron::AB", errstr.str(), 700);
  }
  if(mA.ColumnCount()!=rA.ColumnCount()) Dimension(rA.ColumnCount());
  mA=rA;
  mB=rB;
  if(mpUserData) delete mpUserData;
  mpUserData=0;
}
     
  
// User data
void Polyhedron::UserData(Type* data) const {
  Polyhedron* fakeconst= const_cast<Polyhedron*>(this);
  if(mpUserData) delete fakeconst->mpUserData;
  fakeconst->mpUserData=data;
}
    
// User data
Type* Polyhedron::UserData(void) const {
  return mpUserData;
}
    


// token IO
void Polyhedron::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  std::string ftype="Polyhedron";
  if(rLabel=="") label=ftype;
  Token btag;
  btag.SetBegin(label);
  if(label!=ftype) btag.InsAttribute("ftype",ftype);
  if((Name()!=label) && (Name()!="")) btag.InsAttribute("name",Name());
  // write section
  rTw.Write(btag);
  // write data
  mA.Write(rTw,"AMatrix");
  mB.Write(rTw,"BVector");
  // end section
  rTw.WriteEnd(label);
}

// token IO
void Polyhedron::DoSWrite(TokenWriter& rTw) const {
  // write dimensions
  rTw.WriteComment("Polyhedron statistics");
  rTw.WriteComment(" dimension: "+ ToStringInteger(Dimension()));
  rTw.WriteComment(" inequalities: "+ ToStringInteger(Size()));
}


// token IO
void Polyhedron::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="Polyhedron";
  // find section
  Token btag;
  rTr.ReadBegin(label,btag);
  if(btag.ExistsAttributeString("name")) 
    Name(btag.AttributeStringValue("name"));
  // read data
  mA.Read(rTr,"AMatrix");
  mB.Read(rTr,"BVector");
  // test
  if(mA.RowCount()!=mB.Dimension()) {
    std::stringstream errstr;
    errstr << "Dimension mismatch, rows. " << rTr.FileLine();
    throw Exception("Polyhedron::DoRead", errstr.str(), 700);
  }
  // end section
  rTr.ReadEnd(label);
  // note: this should not be nacessary since Read calls Clear before DoRead
  //if(mpUserData) delete mpUserData;
  //mpUserData=0;
}


/*
*************************************************************************
*************************************************************************
*************************************************************************

Implementation: Relation

*************************************************************************
*************************************************************************
*************************************************************************
*/

// faudes Type std
FAUDES_TYPE_IMPLEMENTATION(LinearRelation,LinearRelation,Type)

// Constructor
LinearRelation::LinearRelation(void) : 
  mName(),
  mCase(I),
  mA1(0,0),
  mA2(0,0),
  mB(0),
  mC(0,0),
  mD(0),
  mpUserData(0)
{
}

// Constructor
LinearRelation::LinearRelation(int dim) :
  mName(),
  mCase(I),
  mA1(0,0),
  mA2(0,0),
  mB(0),
  mC(0,0),
  mD(0),
  mpUserData(0)
{
  if(dim<0) {
    std::stringstream errstr;
    errstr << "Dimension out of range.";
    throw Exception("LinearRelation::LinearRelation()", errstr.str(), 700);
  }
  Identity(dim);
}  


// Constructor
LinearRelation::LinearRelation(const LinearRelation& rSrc) : 
  mName(),
  mCase(I),
  mA1(0,0),
  mA2(0,0),
  mB(0),
  mC(0,0),
  mD(0),
  mpUserData(0)
{
  DoAssign(rSrc);
}
  
// Constructor
LinearRelation::LinearRelation(const std::string& rFileName) :
  mName(),
  mCase(I),
  mA1(0,0),
  mA2(0,0),
  mB(0),
  mC(0,0),
  mD(0),
  mpUserData(0)
{
  Read(rFileName);
}


// Destructor
LinearRelation::~LinearRelation(void) {
  if(mpUserData) delete mpUserData;
}


// virtual Assignment method
LinearRelation& LinearRelation::DoAssign(const LinearRelation& rSrc) {
  mName=rSrc.mName;
  mCase=rSrc.mCase;
  mA1=rSrc.mA1;
  mA2=rSrc.mA2;
  mB=rSrc.mB;
  mC=rSrc.mC;
  mD=rSrc.mD;
  if(mpUserData) delete mpUserData;
  mpUserData=0;
  return *this;
}

// virtual compare
bool LinearRelation::DoEqual(const LinearRelation& rSrc) const {
  if(mCase!=rSrc.mCase) return false;
  if(mA1!=rSrc.mA1) return false;
  if(mA2!=rSrc.mA2) return false;
  if(mB!=rSrc.mB) return false;
  if(mC!=rSrc.mC) return false;
  if(mD!=rSrc.mD) return false;
  return true;
}


// Clear
void LinearRelation::Clear(void) {
  Dimension(0);
}


// Size
Idx LinearRelation::Size(void) const {
  return mB.Dimension();
}


// Set Dimension.
void LinearRelation::Dimension(int dim) {
  if(dim<0) {
    std::stringstream errstr;
    errstr << "Invalid dimension";
    throw Exception("LinearRelation::Dimension", errstr.str(), 700);
  }
  Identity(dim);
}


// Get Dimension.
int LinearRelation::Dimension(void) const {
  if(Relation()) return mA1.ColumnCount();
  return mC.ColumnCount();
}



// Test case
bool LinearRelation::Identity(void) const {
  if(mCase==I) return true;
  return false;
}

// Test case
bool LinearRelation::Map(void) const {
  if(mCase!=R) return true;
  return false;
}

// Test case
bool LinearRelation::Relation(void) const {
  return true;
}

// Access
const Matrix& LinearRelation::A1(void) const {
  return mA1;
}
     
// Access
const Matrix& LinearRelation::A2(void) const {
  return mA2;
}
     
// Access
const Vector& LinearRelation::B(void) const {
  return mB;
}
     
// Access
const Matrix& LinearRelation::C(void) const {
  return mC;
}
     
// Access
const Vector& LinearRelation::D(void) const {
  return mD;
}
     
// Access
void LinearRelation::Relation(const Matrix& rA1, const Matrix& rA2, const Vector& rB) {
  if(rA1.RowCount()!=rB.Dimension()) {
    std::stringstream errstr;
    errstr << "Invalid dimensions.";
    throw Exception("LinearRelation::Relation", errstr.str(), 700);
  }
  if(rA2.RowCount()!=rB.Dimension()) {
    std::stringstream errstr;
    errstr << "Invalid dimensions.";
    throw Exception("LinearRelation::Relation", errstr.str(), 700);
  }
  if(rA1.ColumnCount()!=rA1.ColumnCount()) {
    std::stringstream errstr;
    errstr << "Invalid dimensions.";
    throw Exception("LinearRelation::Relation", errstr.str(), 700);
  }
  mCase=R;
  mA1=rA1;
  mA2=rA2;
  mB=rB;
  mC.Clear();
  mD.Clear();
  if(mpUserData) delete mpUserData;
  mpUserData=0;
}
     
// Access
void LinearRelation::Map(const Matrix& rC, const Vector& rD) {
  if(rC.RowCount()!=rC.ColumnCount()) {
    std::stringstream errstr;
    errstr << "Invalid dimensions.";
    throw Exception("LinearRelation::Map", errstr.str(), 700);
  }
  if(rC.RowCount()!=rD.Dimension()) {
    std::stringstream errstr;
    errstr << "Invalid dimensions.";
    throw Exception("LinearRelation::Map", errstr.str(), 700);
  }
  mCase=M;
  mC=rC;
  mD=rD;
  // fix other data
  //
  // -C x1 + I x2 <=  d
  //  C x1 - I x2 <= -d
  //
  int n=rC.RowCount();
  mA1.Zero(2*n,n);
  mA2.Zero(2*n,n);
  mB.Dimension(2*n);
  for(int i =0; i<n; i++) {
    for(int j =0; j<n; j++) {
      mA1(i,j)=  -mC(i,j);
      mA1(i+n,j)= mC(i,j);
      if(i==j) {
        mA2(i,j)=1;
        mA2(i+n,j)=-1;
      }
    }
    mB(i)=mD(i);
    mB(i+n)=-mD(i);
  }      
  if(mpUserData) delete mpUserData;
  mpUserData=0;
}

// Access
void LinearRelation::Identity(int dim) {
  mCase=I;
  mA1.Clear();
  mA2.Clear();
  mB.Clear();
  mC.Identity(dim);
  mD.Zero(dim);
  // fix other data
  //
  //  -I x1 +  I x2 <=  0
  //   I x1 -  I x2 <=  0
  //
  int n=mC.RowCount();
  mA1.Zero(2*n,n);
  mA2.Zero(2*n,n);
  mB.Dimension(2*n);
  for(int i =0; i<n; i++) {
    mA1(i,i)=   -1;
    mA1(i+n,i)=  1;
    mA2(i,i)=    1;
    mA2(i+n,i)= -1;
  }
  if(mpUserData) delete mpUserData;
  mpUserData=0;
}
     
  
// User data
void LinearRelation::UserData(Type* data) const {
  LinearRelation* fakeconst= const_cast<LinearRelation*>(this);
  if(mpUserData) delete fakeconst->mpUserData;
  fakeconst->mpUserData=data;
}
    
// User data
Type* LinearRelation::UserData(void) const {
  return mpUserData;
}
    

// token IO
void LinearRelation::DoWrite(TokenWriter& rTw, const std::string& rLabel, const Type* pContext) const {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="LinearRelation";
  // write section
  Token stoken;
  stoken.SetBegin(label);
  stoken.InsAttributeInteger("dim",Dimension());
  rTw.Write(stoken);
  // write data
  if(Map()) {
    mC.Write(rTw,"CMatrix");
    mD.Write(rTw,"DVector");
  } else {
    mA1.Write(rTw,"A1Matrix");
    mA2.Write(rTw,"A2Matrix");
    mB.Write(rTw,"BVector");
  }
  // end section
  rTw.WriteEnd(label);
}

// token IO
void LinearRelation::DoSWrite(TokenWriter& rTw) const {
  // write dimensions
  rTw.WriteComment("LinearRelation statistics");
  rTw.WriteComment(" dimension: "+ ToStringInteger(Dimension()));
  if(mCase==I)
    rTw.WriteComment(" case: identity");
  if(mCase==M)
    rTw.WriteComment(" case: map");
  if(mCase==R)
    rTw.WriteComment(" inequalities: "+ ToStringInteger(Size()));
}


// token IO
void LinearRelation::DoRead(TokenReader& rTr, const std::string& rLabel, const Type* pContext) {
  // ignore context
  (void) pContext;
  // figure section
  std::string label=rLabel;
  if(rLabel=="") label="LinearRelation";
  // find section
  Token stoken;
  rTr.ReadBegin(label,stoken);
  int dim=-1;
  if(stoken.ExistsAttributeInteger("dim")) 
    dim=stoken.AttributeIntegerValue("dim");
  // figure case
  Token token;
  rTr.Peek(token);
  // case: identity
  if(token.IsEnd(label)){ 
   Identity(dim);
  }
  // case: map
  if(token.IsBegin("CMatrix")) {
    Matrix c;
    Vector d;
    c.Read(rTr,"CMatrix");
    d.Read(rTr,"DVector");
    Map(c,d);
  }
  // case: relation
  if(token.IsBegin("A1Matrix")) {
    Matrix a1, a2;
    Vector b;
    a1.Read(rTr,"A1Matrix");
    a2.Read(rTr,"A2Matrix");
    b.Read(rTr,"BVector");
    Relation(a1,a2,b);
  }
  // should trace errors ...
  // end section
  rTr.ReadEnd(label);
  // .. done by virtual clear
  //if(mpUserData) delete mpUserData;
  //mpUserData=0;
}

