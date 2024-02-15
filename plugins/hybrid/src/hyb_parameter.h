/** @file hyb_parameter.h  Parameter types for linear hybrid automata */


/* 
   Hybrid systems plug-in  for FAU Discrete Event Systems Library 

   Copyright (C) 2010  Thomas Moor

*/

#ifndef FAUDES_HYB_PARAMETER_H
#define FAUDES_HYB_PARAMETER_H

#include "corefaudes.h"
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <climits>


namespace faudes {

/** 
 * Base type for dense maths. 
 *
 * The hybrid systems plug-in refers to scalar parameters to represent
 * states, constraints, guards and time. The current implementaion uses
 * the floating point representation by the C type "double". As an 
 * alternative, one could consider arbitrary preciscion arithmetic.
 *
 *
 * @ingroup HybridPlugin 
 */
class FAUDES_API Scalar {
 public:
   /** Datatype for scalar */
   typedef double Type;
};


/** 
 *
 * Matrix of scalars.
 * 
 * This is a wrapper for a plain C array to provide a faudes style
 * interface, incl. token IO, access to individual entries. There is also
 * access to the internal C++ array. Note that libFAUDES does not implement
 * any mathematical operations on matrices. For this purpose, one should employ
 * some external library (e.g. BOOST, alglib, ...) via
 * an appropriate set of wrapper functions. 
 *
 * The token format is
 * as follows:
 *
 * @code
 * <Matrix rows="2" columns="5">
 *  1.50    -80   17   90  100
 *  5.23     23    2   -5   13
 * </Matrix>
 * @endcode
 *
 * @ingroup HybridPlugin 
 */
class FAUDES_API Matrix : public Type {
	
FAUDES_TYPE_DECLARATION(Matrix,Matrix,Type)

public:


		
   /** 
    * Default constructor - sets dimension to 0x0
    */
   Matrix(void);

   /** 
    * Constructor by dimension
    *
    *  @param rc
    *    row count
    *  @param cc
    *   column count    
    * 
    */
   Matrix(int rc, int cc);

   /** 
    * Copy-constructor
    */
   Matrix(const Matrix& rSrc);

   /** 
    * Construct from file
    */
   Matrix(const std::string& rFileName);


   /**
    * Destructor
    */
   virtual ~Matrix(void);


   /**
    * Clear all.
    *
    * Sets row- and column count to 0;
    *
    */
   void Clear(void);

   /**
    * Get size.
    *
    * Gets over all number of entries.
    *
    * @return
    *   RowCount() * ColumnCount()
    */
   Idx Size(void) const;


   /**
    * Set dimension.
    * 
    * This will invalidate the entries.
    *
    *  @param rc
    *    row count
    */
   void RowCount(int rc);

   /**
    * Get Dimension.
    * 
    * Set number of rows and invalidate all entries.
    *
    *  @return
    *    row count
    */
   int RowCount(void) const;

   /**
    * Set Dimension.
    * 
    * Set number of columns and invalidate all entries.
    *
    *  @param cc
    *    column count
    */
   void ColumnCount(int cc);

   /**
    * Get Dimension.
    * 
    *  @return
    *    column count
    */
   int ColumnCount(void) const;

   /**
    * Set Dimension.
    * 
    * Set dimensions and invalidate all entries.
    *
    *  @param rc
    *    row count
    *  @param cc
    *    column count
    */
   void Dimension(int rc, int cc);

   /**
    * Set to zero matrix.
    *
    * Either provide dimensions, or use the
    * default -1 to maintain dimensions.
    * 
    *  @param rc
    *    row count
    *  @param cc
    *    column count
    */
   void Zero(int rc=-1, int cc=-1);

   /**
    * Set to identity matrix.
    * 
    * Either provide dimensions, or use the
    * default -1 to maintain dimensions.
    * 
    *  @param dim
    *    row and comlumn count
    */
   void Identity(int dim=-1);

   /**
    *  Get entry.
    *
    *  @param i
    *    row index.
    *  @param j
    *    column index.
    * 
    *  @return
    *    entry A(i,j)
    *  @exception Exception
    *    - index out of range (id 700);
    *
    */
   const Scalar::Type& At(int i, int j) const;
  
   /**
    *  Set entry.
    *
    *  @param i
    *    row index.
    *  @param j
    *    column index.
    *  @param val
    *    new value
    * 
    *  @exception Exception
    *    - index out of range (id 700);
    *
    */
   void At(int i, int j, const Scalar::Type& val);

   /**
    *  Cosmetic access operator.
    *
    *  @param i
    *    row index.
    *  @param j
    *    column index.
    * 
    *  @exception Exception
    *    - index out of range (id 700);
    *
    */
   const Scalar::Type& operator()(int i, int j) const;

   /**
    *  Cosmetic access operator.
    *
    *  @param i
    *    row index.
    *  @param j
    *    column index.
    * 
    *  @exception Exception
    *    - index out of range (id 700);
    *
    */
   Scalar::Type& operator()(int i, int j);


   /**
    * Set from std C array.
    * 
    * The provided data must be organized as
    * A(i,j)=data[i*cc+j]. This version takes a
    * copy of the provided data.
    * 
    *  @param rc
    *    row count
    *  @param cc
    *    column count
    *  @param data
    *    pointer to C array of entries.
    */
   void CArray(int rc, int cc, const Scalar::Type *data);

   /**
    * Access std C array.
    * 
    * The data field is organized as
    * A(i,j)=data[i*column_count+j]. Note that
    * for zero row- or column count, this function returns
    * NULL.
    *
    * @return
    *   const pointer to internal C array
    * 
    */
   const Scalar::Type* CArray(void) const;

 protected:

   /* Matrix: dimensions */
   int mRC;
   int mCC;

   /* Matrix: data */
   Scalar::Type* mpData;

  /**
   * Assign from other matrix.
   *
   * @param rSrc
   *   source to copy from
   * @return
   *   ref to this matrix
   */
  virtual Matrix& DoAssign(const Matrix& rSrc);


  /**
   * Test equality.
   *
   * @param rOther 
   *    Other object to compare with.
   * @return 
   *   True on match.
   */
  virtual bool DoEqual(const Matrix& rOther) const;

  /**
   * Write to TokenWriter, see Type::Write for public wrappers
   *
   * @param tw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label of the section to write, defaults to name of set or "Matrix"
   * @param pContext
   *   Write context to provide contextual information (ignores)
   *
   * @exception Exception
   *   - IO errors (id 2)
   */
  virtual void DoWrite(TokenWriter& tw, const std::string& rLabel="", const Type* pContext=0) const;
        
  /** Write statistic info to TokenWriter, see Type::SWrite for public wrapper.
   *
   * @param tw
   *   Reference to TokenWriter
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoSWrite(TokenWriter& tw) const;

  /**
   * Read from TokenReader, see Type::Read for public wrappers. 
   * The method invokes TokenReader::ReadBegin() to seek the specified
   * section, reads subsequent symbols, and calls matching TokenReader::ReadEnd(). 
   * If no section is specified, the section is assumed to start at the current position
   * of the token stream. If the current position is no begin token, 
   * the section "Matrix" is read.
   *
   * @param tr
   *   Reference to TokenReader
   * @param rLabel
   *   Label to read, defaults to current begin label or else "Matrix"
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception
   *   - IO errors (id 1)
   *   - token mismatch (id 50)
   */
  virtual void DoRead(TokenReader& tr, const std::string& rLabel = "", const Type* pContext=0);

};




/** 
 *
 * Vector of scalars.
 * 
 * This is a wrapper for a plain C array to provide a faudes style
 * interface, incl. token IO, access to individual entries. There is also
 * access to the internal C++ array. Note that libFAUDES does not implement
 * any mathematical operations on vectors. For this purpose, one should employ
 * some external library (e.g. BOOST, alglib, ...) via
 * an appropriate set of wrapper functions. 
 *
 * The token format is
 * as follows:
 *
 * @code
 * <Vector count="5">
 *  1.50    -80   17   90  100
 * </Vector>
 * @endcode
 *
 * @ingroup HybridPlugin 
 */
class FAUDES_API Vector : public Type {
	
FAUDES_TYPE_DECLARATION(Vector, Vector,Type)

public:


		
   /** 
    * Default constructor - sets dimension to 0
    */
   Vector(void);

   /** 
    * Constructor by dimension
    *
    *  @param dim
    *    dimension
    * 
    */
   Vector(int dim);

   /** 
    * Copy-constructor
    */
   Vector(const Vector& rSrc);

   /** 
    * Construct from file
    */
   Vector(const std::string& rFileName);


   /**
    * Destructor
    */
   virtual ~Vector(void);


   /**
    * Clear all.
    *
    * Sets dimension to 0
    *
    */
   void Clear(void);

   /**
    * Get size.
    *
    * Gets over all number of entries. Same as Dimension().
    *
    * @return
    *   Dimension()
    */
   Idx Size(void) const;


   /**
    * Get dimension.
    * 
    *  @return
    *    Number of entries
    */
   int Dimension(void) const;

   /**
    * Set dimension.
    * 
    * This will invalidate the entries.
    *
    *  @param dim
    *    New dimension
    */
   void Dimension(int dim);

   /**
    * Set to zero vector.
    *
    * Either provide dimension, or use the
    * default -1 to maintain the dimension.
    * 
    *  @param dim
    *    number of entries
    */
   void Zero(int dim=-1);

   /**
    * Set to ones.
    * 
    * Either provide dimension, or use the
    * default -1 to maintain the dimension.
    * 
    *  @param dim
    *    number of entries
    */
   void Ones(int dim=-1);

   /**
    *  Get entry.
    *
    *  @param i
    *    index.
    *  @return
    *    entry V(i)
    *  @exception Exception
    *    - index out of range (id 700);
    *
    */
   const Scalar::Type& At(int i) const;
  
   /**
    *  Set entry.
    *
    *  @param i
    *    index
    *  @param val
    *    value
    * 
    *  @exception Exception
    *    - index out of range (id 700);
    *
    */
   void At(int i, const Scalar::Type& val);

   /**
    *  Cosmetic access operator.
    *
    *  @param i
    *    index.
    * 
    *  @exception Exception
    *    - index out of range (id 700);
    */
   const Scalar::Type& operator()(int i) const;

   /**
    *  Cosmetic access operator.
    *
    *  @param i
    *    index.
    *  @exception Exception
    *    - index out of range (id 700);
    */
   Scalar::Type& operator()(int i);

   /**
    * Set from std C array.
    * 
    * This version takes a
    * copy of the provided data.
    * 
    *  @param dim
    *    dimension
    *  @param data
    *    pointer to array of entries.
    */
   void CArray(int dim, const Scalar::Type *data);

   /**
    * Access std C array.
    * 
    * Note that for zero dimension, this function returns
    * NULL.
    *
    * @return
    *   Const ref to internal C array
    * 
    */
   const Scalar::Type* CArray(void) const;


 protected:

   /* Vector: dimensions */
   int mDim;

   /* Vector: data */
   Scalar::Type* mpData;

  /**
   * Assign from other vector
   *
   * @param rSrc
   *   Source to copy from
   * @return
   *   Ref to this vector
   */
  virtual Vector& DoAssign(const Vector& rSrc);


  /**
   * Test equality.
   *
   * @param rOther 
   *    Other object to compare with.
   * @return 
   *   True on match.
   */
  virtual bool DoEqual(const Vector& rOther) const;

  /**
   * Write to TokenWriter, see Type::Write for public wrappers
   *
   * @param tw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label of the section to write, defaults to name of set or "Vector"
   * @param pContext
   *   Write context to provide contextual information (ignores)
   *
   * @exception Exception
   *   - IO errors (id 2)
   */
  virtual void DoWrite(TokenWriter& tw, const std::string& rLabel="", const Type* pContext=0) const;
        
  /** Write statistic info to TokenWriter, see Type::SWrite for public wrapper.
   *
   * @param tw
   *   Reference to TokenWriter
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoSWrite(TokenWriter& tw) const;

  /**
   * Read from TokenReader, see Type::Read for public wrappers. 
   * The method invokes TokenReader::ReadBegin() to seek the specified
   * section, reads subsequent symbols, and calls matching TokenReader::ReadEnd(). 
   * If no section is specified, the section is assumed to start at the current position
   * of the token stream. If the current position is no begin token, 
   * the section "Vector" is read.
   *
   * @param tr
   *   Reference to TokenReader
   * @param rLabel
   *   Label to read, defaults to current begin label or else "Vector"
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception
   *   - IO errors (id 1)
   *   - token mismatch (id 50)
   */
  virtual void DoRead(TokenReader& tr, const std::string& rLabel = "", const Type* pContext=0);

};



/** 
 *
 * Polyhedron in R^n
 * 
 * This class is a container to hold a matrix A and a vector B to represent
 * the polyhedron {x| Ax<=b}. It provides basic member access and token IO.
 *
 * To facilitate the ussage of an external library (e.g. PPL) that implements 
 * operations on polyhedra, the Polyhedron container can record untyped user
 * data in a (void*) entry. See hyb_compute.cpp for a PLL based implemenation
 * of a reachability analysis.
 *
 * The token format is as follows:
 *
 * @code
 * <Polyhedron>
 * <AMatrix rows="4" columns="2">
 *  1   0
 * -1   0
 *  0   1
 *  0  -1
 * </AMatrix>
 * <BVector count="4">
 *  1    
 *  0
 *  1
 *  0
 * </BVector>
 * </Polyhedron>
 * @endcode
 *
 * @ingroup HybridPlugin y
 */
class FAUDES_API Polyhedron : public Type {
	
FAUDES_TYPE_DECLARATION(Polyhedron,Polyhedron,Type)

public:


		
   /** 
    * Default constructor - sets dimension to 0
    */
   Polyhedron(void);

   /** 
    * Constructor by dimension
    *
    *  @param dim
    *    dimension
    * 
    */
   Polyhedron(int dim);

   /** 
    * Copy-constructor
    */
   Polyhedron(const Polyhedron& rSrc);

   /** 
    * Construct from file
    */
   Polyhedron(const std::string& rFileName);


   /**
    * Destructor
    */
   virtual ~Polyhedron(void);

   /**
    * Object name
    *
    * @return name 
    */
   virtual const std::string& Name(void) const {return mName;};

   /**
    * Object name
    *
    * @param name
    *   Set to specifies name 
    */
   virtual void Name(const std::string& name)  {mName=name;};


   /**
    * Clear all.
    *
    * Sets dimension to 0
    *
    */
   void Clear(void);

   /**
    * Get size.
    *
    * Gets the number of inequalities (number of rows of A).
    *
    * @return
    *   number of equations
    */
   Idx Size(void) const;


   /**
    * Get dimension.
    * Gets the dimension of the polyhedron (number of columns of A).
    * 
    *  @return
    *    dimension
    */
   int Dimension(void) const;

   /**
    * Set dimension.
    * 
    * This will invalidate A and B.
    *
    *  @param dim
    *    New dimension
    */
   void Dimension(int dim);

   /**
    *  Get A matrix.
    *
    */
   const Matrix& A(void) const;
  
   /**
    *  Get B vector.
    *
    */
   const Vector& B(void) const;
  
   /**
    *  Set A matrix and B vector.
    *
    *  @param rA
    *    A matrix
    *  @param rB
    *    B vector
    * 
    *  @exception Exception
    *    - dimension missmatch (id 700);
    *
    */
   void AB(const Matrix& rA, const Vector& rB);

   /**
    * Set user data.
    *
    * The polyhedron takes ownership of
    * the provided memory. On write access on the
    * defining parameters, the user data gets
    * invalidated by means of "delete". The internal pointer
    * is then set to NULL.
    * 
    */
   void UserData(Type* data) const;
    
   /**
    * Get user data.
    *
    * @return
    *   Pointer to user data, or NULL if no valid data is available
    * 
    */
   Type* UserData(void) const;
    

 protected:


   /** name */
   std::string mName;

   /** Polyhedron: data */
   Matrix mA;

   /** Polyhedron: data */
   Vector mB;

   /** User data */
   Type* mpUserData;

  /**
   * Assign from other polyhedron
   *
   * @param rSrc
   *   Source to copy from
   * @return
   *   Ref to this polyhedron
   */
  virtual Polyhedron& DoAssign(const Polyhedron& rSrc);


  /**
   * Test equality.
   *
   * Note: this test refers to the plain parameters as oposed to the
   * actual polyhedron.
   *
   * @param rOther 
   *    Other object to compare with.
   * @return 
   *   True on match.
   */
  virtual bool DoEqual(const Polyhedron& rOther) const;

  /**
   * Write to TokenWriter, see Type::Write for public wrappers
   *
   * @param tw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label of the section to write, defaults to name of set or "Polyhedron"
   * @param pContext
   *   Write context to provide contextual information (ignores)
   *
   * @exception Exception
   *   - IO errors (id 2)
   */
  virtual void DoWrite(TokenWriter& tw, const std::string& rLabel="", const Type* pContext=0) const;
        
  /** Write statistic info to TokenWriter, see Type::SWrite for public wrapper.
   *
   * @param tw
   *   Reference to TokenWriter
   *
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoSWrite(TokenWriter& tw) const;

  /**
   * Read from TokenReader, see Type::Read for public wrappers. 
   * The method invokes TokenReader::ReadBegin() to seek the specified
   * section, reads subsequent symbols, and calls matching TokenReader::ReadEnd(). 
   * If no section is specified, the section is assumed to start at the current position
   * of the token stream. If the current position is no begin token, 
   * the section "Polyhedron" is read.
   *
   * @param tr
   *   Reference to TokenReader
   * @param rLabel
   *   Label to read, defaults to current begin label or else "Polyhedron"
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception
   *   - IO errors (id 1)
   *   - token mismatch (id 50)
   */
  virtual void DoRead(TokenReader& tr, const std::string& rLabel = "", const Type* pContext=0);




};


/** 
 *
 * Linear relation on R^n
 * 
 * This class is a container to hold matrices A1 and A2 and a vector B to represent
 * the relation 
 *
 *    {(x1,x2) | A1 x1 + A2 x2 <= B}. 
 *
 * Effectively, a linear relation on R^n is a polyhedron on R^(2n).
 * For an alternative parametrisation for a linear relation consider an affine map "x -> C x + d" and a 
 * subsequent bloat by a polyhedral offset "F x <= E", to obtain:
 *
 *    {(x1,x2) |  F ( x2 - (C x1 + D) ) <= E}.
 *
 * to convert the second from to the first, we have the correspondence
 * 
 *   A1 = -F C; A2 = F; B = E + F D ;  
 *
 * The class maintains a flags to indicate the special cases {(x1,x2) | x2 = C x1 + D } 
 * and  {(x1,x2) | x2 = x1 }. These flags, however, are purely syntatic, i.e., they need
 * to be set by using the respective initialisers. 
 *
 * To facilitate the ussage of an external library that implements transformations
 * on polyhedra, the Relation container can record untyped user data in a (void*) entry. 
 * See hyb_compute.cpp for a PLL based implemenation of a reachability analysis.
 *
 * The token format is
 * as follows:
 *
 * @code
 * % general relation
 * <Relation>
 * <A1Matrix>
 * ...
 * </A1Matrix>
 * <A2Matrix>
 * ...
 * </A2Matrix>
 * <BVector>
 * ...
 * </BVector>
 * </Relation>
 * @endcode
 *
 *
 * @code
 * % affine map
 * <Relation>
 * <CMatrix>
 * ...
 * </CMatrix>
 * <DVector>
 * ...
 * </DVector>
 * </Relation>
 * @endcode
 *
 * @code
 * % identity
 * <Relation dimension="2"/>
 * @endcode
 *
 * @ingroup HybridPlugin 
 */
class FAUDES_API LinearRelation : public Type {
	
FAUDES_TYPE_DECLARATION(LinearRelation, LinearRelation,Type)

public:

   /** 
    * Default constructor.
    * Sets dimension to 0 and the relation to be the identity map.
    */
   LinearRelation(void);

   /** 
    * Constructor by dimension. 
    * Sets the relation to be the  identity map.
    *
    *  @param dim
    *    Dimension
    * 
    */
   LinearRelation(int dim);

   /** 
    * Copy-constructor
    */
   LinearRelation(const LinearRelation& rSrc);

   /** 
    * Construct from file
    */
   LinearRelation(const std::string& rFileName);


   /**
    * Destructor
    */
   virtual ~LinearRelation(void);


   /**
    * Object name
    *
    * @return name 
    */
   virtual const std::string& Name(void) const {return mName;};

   /**
    * Object name
    *
    * @param name
    *   Set to specifies name 
    */
   virtual void Name(const std::string& name)  {mName=name;};

   /**
    * Clear all.
    *
    * Sets dimension to 0 with identity
    *
    */
   void Clear(void);

   /**
    * Get size.
    *
    * Number of inequalities.
    *
    * @return
    *   Eq count
    */
   Idx Size(void) const;


   /**
    * Get dimension.
    * 
    *  @return
    *    Dimension
    */
   int Dimension(void) const;

   /**
    * Set dimension.
    * 
    * Defaults to identity map.
    *
    *  @param dim
    *    New dimension
    */
   void Dimension(int dim);


   /**
    * Test for data format.
    * 
    * The current implementation allway sets up the matrices
    * A1, A2 and B and, hence, allways returns true.
    */
   bool Relation(void) const;


   /**
    * Test for data format.
    * True when initialsed via Map or Identity. When initialised by
    * Relation, the map data format is not available.
    */
   bool Map(void) const;

   /**
    * Test for data format.
    * True only if initialsed Identity.
    * 
    */
   bool Identity(void) const;



   /**
    *  Get A1 matrix.
    *
    */
   const Matrix& A1(void) const;
  
   /**
    *  Get A2 matrix.
    *
    */
   const Matrix& A2(void) const;
  
   /**
    *  Get B vector.
    *
    */
   const Vector& B(void) const;
  
   /**
    *  Get C matrix.
    *
    *  Note: this is only defined for the cases Map and Identity.
    *
    */
   const Matrix& C(void) const;
  
   /**
    *  Get D vector.
    *
    *  Note: this is only defined for the cases Map and Identity.
    */
   const Vector& D(void) const;
  
   /**
    *  Set by A1, A2 and B
    *
    *  @param rA1
    *    A1 matrix
    *  @param rA2
    *    A2 matrix
    *  @param rB
    *    B vector
    * 
    *  @exception Exception
    *    - dimension missmatch (id 700);
    *
    */
   void Relation(const Matrix& rA1, const Matrix& rA2, const Vector& rB);

   /**
    *  Set to affine map Cx+D 
    *
    *  @param rC
    *    C matrix
    *  @param rB
    *    D vector
    * 
    *  @exception Exception
    *    - dimension missmatch (id 700);
    *
    */
   void Map(const Matrix& rC, const Vector& rD);

   /**
    * Set to identity
    *
    *  @param dim
    *   dimension
    */
   void Identity(int dim);


   /**
    * Set user data.
    *
    * The relation takes ownership of
    * the provided memory. On write access on the
    * defining parameters the user data gets
    * invalidated by means of "delete". The internal pointer
    * is then set to NULL.
    * 
    */
   void UserData(Type* data) const;
    
   /**
    * Get user data.
    *
    * @return
    *   Pointer to user data, or NULL if no internal data available
    * 
    */
   Type* UserData(void) const;
    
 protected:

   /** have a name */
   std::string mName;

   /** Case enum */
   typedef enum{R,M,I} TCase;

   /** Case flag */
   TCase mCase;

   /** LinearRelation: data */
   Matrix mA1;
   Matrix mA2;
   Vector mB;
   Matrix mC;
   Vector mD;

   /** User data */
   Type* mpUserData;

  /**
   * Assign from other polyhedron
   *
   * @param rSrc
   *   Source to copy from
   * @return
   *   Ref to this polyhedron
   */
  virtual LinearRelation& DoAssign(const LinearRelation& rSrc);


  /**
   * Test equality.
   *
   * Note: this test refers to the paramtrisation as oposed to the
   * actual relation.
   *
   * @param rOther 
   *    Other object to compare with.
   * @return 
   *   True on match.
   */
  virtual bool DoEqual(const LinearRelation& rOther) const;

  /**
   * Write to TokenWriter, see Type::Write for public wrappers
   *
   * @param tw
   *   Reference to TokenWriter
   * @param rLabel
   *   Label of the section to write, defaults to name of set or "LinearRelation"
   * @param pContext
   *   Write context to provide contextual information (ignores)
   *
   * @exception Exception
   *   - IO errors (id 2)
   */
  virtual void DoWrite(TokenWriter& tw, const std::string& rLabel="", const Type* pContext=0) const;
        
  /** Write statistic info to TokenWriter, see Type::SWrite for public wrapper.
   *
   * @param tw
   *   Reference to TokenWriter
   *
   * @exception Exception 
   *   - IO errors (id 2)
   */
  virtual void DoSWrite(TokenWriter& tw) const;

  /**
   * Read from TokenReader, see Type::Read for public wrappers. 
   * The method invokes TokenReader::ReadBegin() to seek the specified
   * section, reads subsequent symbols, and calls matching TokenReader::ReadEnd(). 
   * If no section is specified, the section is assumed to start at the current position
   * of the token stream. If the current position is no begin token, 
   * the section "LinearRelation" is read.
   *
   * @param tr
   *   Reference to TokenReader
   * @param rLabel
   *   Label to read, defaults to current begin label or else "LinearRelation"
   * @param pContext
   *   Write context to provide contextual information
   *
   * @exception Exception
   *   - IO errors (id 1)
   *   - token mismatch (id 50)
   */
  virtual void DoRead(TokenReader& tr, const std::string& rLabel = "", const Type* pContext=0);




};





}  // name space
#endif  

