/*
 * Polynomial.c
 *
 *  Created on: Feb 5, 2014
 *      Author: dejan
 */

#include "Polynomial.h"

#include "Integer.h"
#include "Variable.h"
#include "VariableOrder.h"
#include "Assignment.h"
#include "Value.h"
#include "Interval.h"
#include "FeasibilitySet.h"

#include "utils.h"
#include "variable_list.h"
#include "sign_condition.h"
#include "feasibility_set.h"
#include "value.h"

#include <structmember.h>

static lp_polynomial_context_t* default_ctx = 0;

const lp_polynomial_context_t* Polynomial_get_default_context(void) {
  if (!default_ctx) {
    default_ctx = lp_polynomial_context_new(0, Variable_get_default_db(), (lp_variable_order_t*) VariableOrder_get_default_order());
  }
  return default_ctx;
}

static void
Polynomial_dealloc(Polynomial* self);

static PyObject*
Polynomial_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

static int
Polynomial_cmp(PyObject* self, PyObject* args);

static PyObject*
Polynomial_richcompare(PyObject* self, PyObject* args, int op);

static PyObject*
Polynomial_degree(PyObject* self);

static PyObject*
Polynomial_coefficients(PyObject* self);

static PyObject*
Polynomial_reductum(PyObject* self, PyObject* args);

static PyObject*
Polynomial_sgn(PyObject* self, PyObject* arguments);

static PyObject*
Polynomial_rem(PyObject* self, PyObject* args);

static PyObject*
Polynomial_prem(PyObject* self, PyObject* args);

static PyObject*
Polynomial_sprem(PyObject* self, PyObject* args);

static PyObject*
Polynomial_gcd(PyObject* self, PyObject* args);

static PyObject*
Polynomial_lcm(PyObject* self, PyObject* args);

static PyObject*
Polynomial_derivative(PyObject* self);

static PyObject*
Polynomial_extended_gcd(PyObject* self, PyObject* args);

static PyObject*
Polynomial_factor(PyObject* self);

static PyObject*
Polynomial_factor_square_free(PyObject* self);

static PyObject*
Polynomial_roots_count(PyObject* self, PyObject* args);

static PyObject*
Polynomial_roots_isolate(PyObject* self, PyObject* args);

static PyObject*
Polynomial_sturm_sequence(PyObject* self);

static PyObject*
Polynomial_str(PyObject* self);

static int
Polynomial_nonzero(PyObject* self);

static PyObject*
Polynomial_add(PyObject* self, PyObject* args);

static PyObject*
Polynomial_neg(PyObject* self);

static PyObject*
Polynomial_sub(PyObject* self, PyObject* args);

static PyObject*
Polynomial_mul(PyObject* self, PyObject* args);

static PyObject*
Polynomial_div(PyObject* self, PyObject* args);

static PyObject*
Polynomial_rem_operator(PyObject* self, PyObject* args);

static PyObject*
Polynomial_divmod(PyObject* self, PyObject* args);

static PyObject*
Polynomial_pow(PyObject* self, PyObject* args);

static PyObject*
Polynomial_resultant(PyObject* self, PyObject* args);

static PyObject*
Polynomial_psc(PyObject* self, PyObject* args);

static PyObject*
Polynomial_evaluate(PyObject* self, PyObject* args);

static PyObject*
Polynomial_vars(PyObject* self);

static PyObject*
Polynomial_feasible_intervals(PyObject* self, PyObject* args);

static PyObject*
Polynomial_feasible_set(PyObject* self, PyObject* args);

PyMethodDef Polynomial_methods[] = {
    {"degree", (PyCFunction)Polynomial_degree, METH_NOARGS, "Returns the degree of the polynomial in its top variable"},
    {"coefficients", (PyCFunction)Polynomial_coefficients, METH_NOARGS, "Returns a dictionary from degrees to coefficients"},
    {"reductum", (PyCFunction)Polynomial_reductum, METH_VARARGS, "Returns the reductum of the polynomial"},
    {"sgn", (PyCFunction)Polynomial_sgn, METH_VARARGS, "Returns the sign of the polynomials in the given model"},
    {"rem", (PyCFunction)Polynomial_rem, METH_VARARGS, "Returns the remainder of current and given polynomial"},
    {"prem", (PyCFunction)Polynomial_prem, METH_VARARGS, "Returns the pseudo remainder of current and given polynomial"},
    {"sprem", (PyCFunction)Polynomial_sprem, METH_VARARGS, "Returns the sparse pseudo remainder of current and given polynomial"},
    {"gcd", (PyCFunction)Polynomial_gcd, METH_VARARGS, "Returns the gcd of current and given polynomial"},
    {"lcm", (PyCFunction)Polynomial_lcm, METH_VARARGS, "Returns the lcm of current and given polynomial"},
    {"extended_gcd", (PyCFunction)Polynomial_extended_gcd, METH_VARARGS, "Returns the extended gcd, i.e. (gcd, u, v), of current and given polynomial"},
    {"factor", (PyCFunction)Polynomial_factor, METH_NOARGS, "Returns the factorization of the polynomial"},
    {"factor_square_free", (PyCFunction)Polynomial_factor_square_free, METH_NOARGS, "Returns the square-free factorization of the polynomial"},
    {"roots_count", (PyCFunction)Polynomial_roots_count, METH_VARARGS, "Returns the number of real roots in the given interval"},
    {"roots_isolate", (PyCFunction)Polynomial_roots_isolate, METH_VARARGS, "Returns the list of real roots (has to be univariate modulo the assignment)"},
    {"sturm_sequence", (PyCFunction)Polynomial_sturm_sequence, METH_NOARGS, "Returns the Sturm sequence"},
    {"derivative", (PyCFunction)Polynomial_derivative, METH_NOARGS, "Returns the derivative of the polynomial"},
    {"resultant", (PyCFunction)Polynomial_resultant, METH_VARARGS, "Returns the resultant of the current and given polynomial"},
    {"psc", (PyCFunction)Polynomial_psc, METH_VARARGS, "Returns the principal subresultant coefficients of the current and given polynomial"},
    {"factor_square_free", (PyCFunction)Polynomial_factor_square_free, METH_NOARGS, "Returns the square-free factorization of the polynomial"},
    {"evaluate", (PyCFunction)Polynomial_evaluate, METH_VARARGS, "Returns the value of the polynomial in the given assignment (or null if it doesn't fully evaluate"},
    {"vars", (PyCFunction)Polynomial_vars, METH_NOARGS, "Returns the list of variables in the polynomial"},
    {"feasible_intervals", (PyCFunction)Polynomial_feasible_intervals, METH_VARARGS, "Returns feasible intervals of the polynomial (has to be univariate modulo the assignment)"},
    {"feasible_set", (PyCFunction)Polynomial_feasible_set, METH_VARARGS, "Returns feasible set of the polynomial (has to be univariate modulo the assignment)"},
    {NULL}  /* Sentinel */
};

PyNumberMethods Polynomial_NumberMethods = {
     Polynomial_add, // binaryfunc nb_add;
     Polynomial_sub, // binaryfunc nb_subtract;
     Polynomial_mul, // binaryfunc nb_multiply;
     Polynomial_div, // binaryfunc nb_divide;
     Polynomial_rem_operator, // binaryfunc nb_remainder;
     Polynomial_divmod, // binaryfunc nb_divmod;
     (ternaryfunc)Polynomial_pow, // ternaryfunc nb_power;
     Polynomial_neg, // unaryfunc nb_negative;
     0, // unaryfunc nb_positive;
     0, // unaryfunc nb_absolute;
     Polynomial_nonzero, // inquiry nb_nonzero;       /* Used by PyObject_IsTrue */
     0, // unaryfunc nb_invert;
     0, // binaryfunc nb_lshift;
     0, // binaryfunc nb_rshift;
     0, // binaryfunc nb_and;
     0, // binaryfunc nb_xor;
     0, // binaryfunc nb_or;
     0, // coercion nb_coerce;       /* Used by the coerce() function */
     0, // unaryfunc nb_int;
     0, // unaryfunc nb_long;
     0, // unaryfunc nb_float;
     0, // unaryfunc nb_oct;
     0, // unaryfunc nb_hex;

     /* Added in release 2.0 */
     0, // binaryfunc nb_inplace_add;
     0, // binaryfunc nb_inplace_subtract;
     0, // binaryfunc nb_inplace_multiply;
     0, // binaryfunc nb_inplace_divide;
     0, // binaryfunc nb_inplace_remainder;
     0, // ternaryfunc nb_inplace_power;
     0, // binaryfunc nb_inplace_lshift;
     0, // binaryfunc nb_inplace_rshift;
     0, // binaryfunc nb_inplace_and;
     0, // binaryfunc nb_inplace_xor;
     0, // binaryfunc nb_inplace_or;

     /* Added in release 2.2 */
     0, // binaryfunc nb_floor_divide;
     0, // binaryfunc nb_true_divide;
     0, // binaryfunc nb_inplace_floor_divide;
     0, // binaryfunc nb_inplace_true_divide;

     /* Added in release 2.5 */
     0 // unaryfunc nb_index;
};

PyTypeObject PolynomialType = {
    PyObject_HEAD_INIT(NULL)
    0,                          /*ob_size*/
    "polypy.Polynomial",        /*tp_name*/
    sizeof(Polynomial),       /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)Polynomial_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    Polynomial_cmp,             /*tp_compare*/
    Polynomial_str,             /*tp_repr*/
    &Polynomial_NumberMethods,  /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    Polynomial_str,             /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
    "Polynomial objects",       /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    Polynomial_richcompare,     /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    Polynomial_methods,         /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    Polynomial_new,             /* tp_new */
};

static void
Polynomial_dealloc(Polynomial* self)
{
  if (self->p) {
    lp_polynomial_destruct(self->p);
    free(self->p);
  }
  self->ob_type->tp_free((PyObject*)self);
}

PyObject*
Polynomial_create(lp_polynomial_t* p) {
  Polynomial *self;
  self = (Polynomial*)PolynomialType.tp_alloc(&PolynomialType, 0);
  lp_polynomial_set_external(p);
  self->p = p;
  return (PyObject*) self;
}

static PyObject*
Polynomial_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  return Polynomial_create(0);
}

static PyObject*
Polynomial_richcompare(PyObject* self, PyObject* other, int op) {
  PyObject *result = 0;

  const lp_polynomial_context_t* ctx = 0;

  // One of them is a polynomial
  if (PyPolynomial_CHECK(self)) {
    ctx = lp_polynomial_get_context(((Polynomial*) self)->p);
  } else {
    ctx = lp_polynomial_get_context(((Polynomial*) other)->p);
  }

  int dec_other = 0;
  int dec_self = 0;

  // Check arguments
  if (!PyPolynomial_CHECK(self)) {
    if (PyVariable_CHECK(self)) {
      self = PyPolynomial_FromVariable(self, ctx);
      dec_self = 1;
    } else if (PyLong_or_Int_Check(self)) {
      self = PyPolynomial_FromLong_or_Int(self, ctx);
      dec_self = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // Check arguments
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  lp_polynomial_t* self_p = ((Polynomial*) self)->p;
  lp_polynomial_t* other_p = ((Polynomial*) other)->p;

  int cmp = lp_polynomial_cmp(self_p, other_p);

  switch (op) {
  case Py_LT:
    result = cmp < 0 ? Py_True : Py_False;
    break;
  case Py_LE:
    result = cmp <= 0 ? Py_True : Py_False;
    break;
  case Py_EQ:
    result = cmp == 0 ? Py_True : Py_False;
    break;
  case Py_NE:
    result = cmp != 0 ? Py_True : Py_False;
    break;
  case Py_GT:
    result = cmp > 0 ? Py_True : Py_False;
    break;
  case Py_GE:
    result = cmp >= 0 ? Py_True : Py_False;
    break;
  }

  if (dec_self) {
    Py_DECREF(self);
  }
  if (dec_other) {
    Py_DECREF(other);
  }

  Py_INCREF(result);
  return result;
}

static int
Polynomial_cmp(PyObject* self, PyObject* other) {

  // Check arguments
  if (!PyPolynomial_CHECK(self) || !PyPolynomial_CHECK(other)) {
    // should return -1 and set an exception condition when an error occurred
    return -1;
  }
  // Get arguments
  Polynomial* p1 = (Polynomial*) self;
  Polynomial* p2 = (Polynomial*) other;
  // Compare
  int cmp = lp_polynomial_cmp(p1->p, p2->p);
  return cmp > 0 ? 1 : cmp < 0 ? -1 : 0;
}

static PyObject* Polynomial_str(PyObject* self) {
  Polynomial* p = (Polynomial*) self;
  if (p) {
    char* p_str = lp_polynomial_to_string(p->p);
    PyObject* str = PyString_FromString(p_str);
    free(p_str);
    return str;
  } else {
    Py_RETURN_NONE;
  }
}

PyObject*
PyPolynomial_FromVariable(PyObject* variable, const lp_polynomial_context_t* ctx) {
  // The variable
  lp_variable_t x = ((Variable*) variable)->x;

  // The constant
  lp_integer_t one;
  lp_integer_construct_from_int(ctx->K, &one, 1);

  // The x polynomial
  lp_polynomial_t* p_x = lp_polynomial_alloc();
  lp_polynomial_construct_simple(p_x, ctx, &one, x, 1);

  // Remove temps
  lp_integer_destruct(&one);

  // Return the polynomial
  PyObject* result = Polynomial_create(p_x);
  Py_INCREF(result);
  return result;
}

PyObject*
PyPolynomial_FromLong_or_Int(PyObject* number, const lp_polynomial_context_t* ctx) {
  // The constants
  lp_integer_t c;
  PyLong_or_Int_to_integer(number, 0, &c);

  // The c polynomial
  lp_polynomial_t* p_c = lp_polynomial_alloc();
  lp_polynomial_construct_simple(p_c, ctx, &c, 0, 0);

  // Remove temps
  lp_integer_destruct(&c);

  // Return the polynomial
  PyObject* result = Polynomial_create(p_c);
  Py_INCREF(result);
  return result;
}

static PyObject*
Polynomial_add(PyObject* self, PyObject* other) {

  int dec_other = 0;

  if (!PyPolynomial_CHECK(self)) {
    return Polynomial_add(other, self);
  }

  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  // Check argument
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // Get arguments
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (p1_ctx != p2_ctx) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Add the polynomials
  lp_polynomial_t* sum = lp_polynomial_new(p1_ctx);
  lp_polynomial_add(sum, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(sum);
}

static PyObject*
Polynomial_neg(PyObject* self) {
  Polynomial* p = (Polynomial*) self;
  const lp_polynomial_context_t* p_ctx = lp_polynomial_get_context(p->p);
  lp_polynomial_t* neg = lp_polynomial_new(p_ctx);
  lp_polynomial_neg(neg, p->p);
  return Polynomial_create(neg);
}

static PyObject*
Polynomial_sub(PyObject* self, PyObject* other) {

  int dec_other = 0;

  if (!PyPolynomial_CHECK(self)) {
    Polynomial* sub = (Polynomial*) Polynomial_sub(other, self);
    lp_polynomial_neg(sub->p, sub->p);
    return (PyObject*) sub;
  }

  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  // Check argument
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // Get arguments
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (p1_ctx != p2_ctx) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Subtract the polynomials
  lp_polynomial_t* sub = lp_polynomial_new(p1_ctx);
  lp_polynomial_sub(sub, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(sub);
}

static PyObject*
Polynomial_mul(PyObject* self, PyObject* other) {

  int dec_other = 0;

  if (!PyPolynomial_CHECK(self)) {
    return Polynomial_mul(other, self);
  }

  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  // Check argument
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // Get arguments
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* mul = lp_polynomial_new(p1_ctx);
  lp_polynomial_mul(mul, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(mul);
}


static PyObject*
Polynomial_pow(PyObject* self, PyObject* other) {
  // Check arguments
  if (!PyPolynomial_CHECK(self) || !PyInt_Check(other)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }
  // Get arguments
  Polynomial* p = (Polynomial*) self;
  long n = PyInt_AsLong(other);
  if (n < 0) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }
  const lp_polynomial_context_t* p_ctx = lp_polynomial_get_context(p->p);
  // Compute
  lp_polynomial_t* pow = lp_polynomial_new(p_ctx);
  lp_polynomial_pow(pow, p->p, n);
  // Return the result
  return Polynomial_create(pow);
}

static PyObject*
Polynomial_div(PyObject* self, PyObject* other) {

  int dec_other = 0;

  if (!PyPolynomial_CHECK(self)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // other can be a variable or a number
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* div = lp_polynomial_new(p1_ctx);
  lp_polynomial_div(div, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(div);
}

static PyObject*
Polynomial_rem_operator(PyObject* self, PyObject* other) {
  int dec_other = 0;

  if (!PyPolynomial_CHECK(self)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // other can be a variable or a number
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* rem = lp_polynomial_new(p1_ctx);
  lp_polynomial_rem(rem, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(rem);
}

static PyObject*
Polynomial_divmod(PyObject* self, PyObject* other) {

  int dec_other = 0;

  if (!PyPolynomial_CHECK(self)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // other can be a variable or a number
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* rem = lp_polynomial_new(p1_ctx);
  lp_polynomial_t* div = lp_polynomial_new(p1_ctx);
  lp_polynomial_divrem(div, rem, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  PyObject* pair = PyTuple_New(2);
  PyObject* divObj = Polynomial_create(div);
  PyObject* remObj = Polynomial_create(rem);
  Py_INCREF(divObj);
  Py_INCREF(remObj);
  PyTuple_SetItem(pair, 0, divObj);
  PyTuple_SetItem(pair, 1, remObj);
  return pair;
}

static int
Polynomial_nonzero(PyObject* self) {
  // Get arguments
  Polynomial* p = (Polynomial*) self;
  // Return the result
  return !lp_polynomial_is_zero(p->p);
}

enum rem_type {
  REM_EXACT,
  REM_PSEUDO,
  REM_SPARSE_PSEUDO
};

static PyObject*
Polynomial_rem_general(PyObject* self, PyObject* args, enum rem_type type) {
  int dec_other = 0;

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* other = PyTuple_GetItem(args, 0);

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // other can be a variable or a number
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* rem = lp_polynomial_new(p1_ctx);
  switch (type) {
  case REM_EXACT:
    lp_polynomial_rem(rem, p1->p, p2->p);
    break;
  case REM_PSEUDO:
    lp_polynomial_prem(rem, p1->p, p2->p);
    break;
  case REM_SPARSE_PSEUDO:
    lp_polynomial_sprem(rem, p1->p, p2->p);
    break;
  }

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(rem);
}

static PyObject*
Polynomial_rem(PyObject* self, PyObject* other) {
  return Polynomial_rem_general(self, other, REM_EXACT);
}

static PyObject*
Polynomial_prem(PyObject* self, PyObject* other) {
  return Polynomial_rem_general(self, other, REM_PSEUDO);
}

static PyObject*
Polynomial_sprem(PyObject* self, PyObject* other) {
  return Polynomial_rem_general(self, other, REM_SPARSE_PSEUDO);
}

static PyObject*
Polynomial_gcd(PyObject* self, PyObject* args) {

  int dec_other = 0;

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* other = PyTuple_GetItem(args, 0);

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // other can be a variable or a number
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* gcd = lp_polynomial_new(p1_ctx);
  lp_polynomial_gcd(gcd, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(gcd);
}

static PyObject*
Polynomial_lcm(PyObject* self, PyObject* args) {

  int dec_other = 0;

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* other = PyTuple_GetItem(args, 0);

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // other can be a variable or a number
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Multiply the polynomials
  lp_polynomial_t* lcm = lp_polynomial_new(p1_ctx);
  lp_polynomial_lcm(lcm, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(lcm);
}

static PyObject*
Polynomial_psc(PyObject* self, PyObject* args) {

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* other = PyTuple_GetItem(args, 0);

  int dec_other = 0;

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // Othe polynomial
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Check the arguments (must be same top variable)
  if (lp_polynomial_is_constant(p1->p) || lp_polynomial_is_constant(p2->p)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  if (lp_polynomial_top_variable(p1->p) != lp_polynomial_top_variable(p2->p)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Allocate the polynomials for the sequence
  size_t p1_deg = lp_polynomial_degree(p1->p);
  size_t p2_deg = lp_polynomial_degree(p2->p);
  int size = p1_deg > p2_deg ? p2_deg + 1 : p1_deg + 1;

  lp_polynomial_t** psc = malloc(sizeof(lp_polynomial_t*)*size);
  int i;
  for (i = 0; i < size; ++ i) {
    psc[i] = lp_polynomial_new(p1_ctx);
  }

  // Compute the psc
  lp_polynomial_psc(psc, p1->p, p2->p);

  // Copy the polynomials into a list
  PyObject* list = PyList_New(size);
  for (i = 0; i < size; ++i) {
    PyObject* p = Polynomial_create(psc[i]);
    PyList_SetItem(list, i, p);
  }

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return list;
}

static PyObject*
Polynomial_resultant(PyObject* self, PyObject* args) {

  // self is always a polynomial
  Polynomial* p1 = (Polynomial*) self;
  const lp_polynomial_context_t* p1_ctx = lp_polynomial_get_context(p1->p);

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* other = PyTuple_GetItem(args, 0);

  int dec_other = 0;

  // Make sure other is a polynomial
  if (!PyPolynomial_CHECK(other)) {
    if (PyVariable_CHECK(other)) {
      other = PyPolynomial_FromVariable(other, p1_ctx);
      dec_other = 1;
    } else if (PyLong_or_Int_Check(other)) {
      other = PyPolynomial_FromLong_or_Int(other, p1_ctx);
      dec_other = 1;
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  }

  // Othe polynomial
  Polynomial* p2 = (Polynomial*) other;
  const lp_polynomial_context_t* p2_ctx = lp_polynomial_get_context(p2->p);
  if (!lp_polynomial_context_equal(p1_ctx, p2_ctx)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Check the arguments (must be same top variable)
  if (lp_polynomial_is_constant(p1->p) || lp_polynomial_is_constant(p2->p)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  if (lp_polynomial_top_variable(p1->p) != lp_polynomial_top_variable(p2->p)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Allocate the resultant
  lp_polynomial_t* resultant = lp_polynomial_new(p1_ctx);

  // Compute the psc
  lp_polynomial_resultant(resultant, p1->p, p2->p);

  if (dec_other) {
    Py_DECREF(other);
  }

  // Return the result
  return Polynomial_create(resultant);
}


static PyObject*
Polynomial_extended_gcd(PyObject* self, PyObject* args) {
  return 0;
}

static PyObject*
Polynomial_factor(PyObject* self) {
  return 0;
}

// Creates a python list from the factors, taking over the polynomials
PyObject* factors_to_PyList(lp_polynomial_t** factors, size_t* multiplicities, size_t size) {
  // Construct the result
  PyObject* factors_list = PyList_New(size);

  // Copy the constant
    // Copy over the factors
  int i;
  for (i = 0; i < size; ++ i) {
    PyObject* p_i = Polynomial_create(factors[i]);
    Py_INCREF(p_i);
    PyObject* d = PyInt_FromSize_t(multiplicities[i]);
    PyObject* pair = PyTuple_New(2);
    PyTuple_SetItem(pair, 0, p_i);
    PyTuple_SetItem(pair, 1, d);
    PyList_SetItem(factors_list, i, pair);
  }

  // Return the list
  return factors_list;
}

static PyObject*
Polynomial_factor_square_free(PyObject* self) {
  // Get arguments
  Polynomial* p = (Polynomial*) self;
  // Factor
  lp_polynomial_t** factors = 0;
  size_t* multiplicities = 0;
  size_t factors_size = 0;
  lp_polynomial_factor_square_free(p->p, &factors, &multiplicities, &factors_size);
  // Create the list
  PyObject* factors_list = factors_to_PyList(factors, multiplicities, factors_size);
  // Get rid of the factors (not the polynomials)
  free(factors);
  free(multiplicities);
  // Return the list
  return factors_list;
}

static PyObject*
Polynomial_roots_count(PyObject* self, PyObject* args) {
  return 0;
}

static PyObject*
Polynomial_roots_isolate(PyObject* self, PyObject* args) {

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* assignment_obj = PyTuple_GetItem(args, 0);

  if (!PyAssignment_CHECK(assignment_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  lp_polynomial_t* p = ((Polynomial*) self)->p;
  lp_assignment_t* assignment = ((Assignment*) assignment_obj)->assignment;

  // Check that the top variable is unassigned
  lp_variable_t x = lp_polynomial_top_variable(p);
  if (lp_assignment_get_value(assignment, x)->type != LP_VALUE_NONE) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Get the degree of the polynomial and allocate the values
  lp_value_t* roots = malloc(sizeof(lp_value_t)*lp_polynomial_degree(p));
  size_t roots_size = 0;

  // Get the roots
  lp_polynomial_roots_isolate(p, assignment, roots, &roots_size);

  // Generate a list of roots
  PyObject* list = PyList_New(roots_size);

  int i;
  for (i = 0; i < roots_size; ++ i) {
    PyObject* c = PyValue_create(roots + i);
    PyList_SetItem(list, i, c);
  }

  // Get rid of the temporaries
  for (i = 0; i < roots_size; ++ i) {
    lp_value_destruct(roots + i);
  }
  free(roots);

  // Return the list
  return list;
}

static PyObject*
Polynomial_derivative(PyObject* self) {
  lp_polynomial_t* p = ((Polynomial*) self)->p;
  lp_polynomial_t* p_derivative = lp_polynomial_new(lp_polynomial_get_context(p));
  lp_polynomial_derivative(p_derivative, p);
  return Polynomial_create(p_derivative);
}

static PyObject*
Polynomial_sturm_sequence(PyObject* self) {
  return 0;
}

static PyObject*
Polynomial_degree(PyObject* self) {
  Polynomial* p = (Polynomial*) self;
  return PyInt_FromLong(lp_polynomial_degree(p->p));
}

static PyObject*
Polynomial_coefficients(PyObject* self) {
  int i;

  lp_polynomial_t* p = ((Polynomial*) self)->p;
  size_t size = lp_polynomial_degree(p) + 1;

  // Get the coefficients
  const lp_polynomial_context_t* ctx = lp_polynomial_get_context(p);

  // Copy the polynomials into a list
  PyObject* list = PyList_New(size);
  for (i = 0; i < size; ++i) {
    lp_polynomial_t* c_p = lp_polynomial_new(ctx);
    lp_polynomial_get_coefficient(c_p, p, i);
    PyObject* c = Polynomial_create(c_p);
    PyList_SetItem(list, i, c);
  }

  return list;
}

static PyObject*
Polynomial_vars(PyObject* self) {

  lp_polynomial_t* p = ((Polynomial*) self)->p;

  lp_variable_list_t p_vars;
  lp_variable_list_construct(&p_vars);

  // Copy the polynomials into a list
  PyObject* list = PyList_New(p_vars.list_size);
  size_t i;
  for (i = 0; i < p_vars.list_size; ++i) {
    PyObject* c = PyVariable_create(p_vars.list[i]);
    PyList_SetItem(list, i, c);
  }

  lp_variable_list_destruct(&p_vars);

  return list;
}


static PyObject*
Polynomial_reductum(PyObject* self, PyObject* args) {
  lp_polynomial_t* p = ((Polynomial*) self)->p;
  const lp_polynomial_context_t* ctx = lp_polynomial_get_context(p);

  if (!PyTuple_Check(args) || PyTuple_Size(args) > 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  lp_assignment_t* assignment = 0;

  if (PyTuple_Size(args) == 1) {
    PyObject* assignment_obj = PyTuple_GetItem(args, 0);
    if (!PyAssignment_CHECK(assignment_obj)) {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    } else {
      assignment = ((Assignment*) assignment_obj)->assignment;
    }
  }

  lp_polynomial_t* result = lp_polynomial_new(ctx);
  if (assignment) {
    lp_polynomial_reductum_m(result, p, assignment);
  } else {
    lp_polynomial_reductum(result, p);
  }

  return Polynomial_create(result);
}

static PyObject*
Polynomial_sgn(PyObject* self, PyObject* args) {

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* assignment_obj = PyTuple_GetItem(args, 0);

  if (!PyAssignment_CHECK(assignment_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  lp_polynomial_t* p = ((Polynomial*) self)->p;
  lp_assignment_t* assignment = ((Assignment*) assignment_obj)->assignment;

  int sgn = lp_polynomial_sgn(p, assignment);

  return PyInt_FromLong(sgn);
}


static PyObject*
Polynomial_evaluate(PyObject* self, PyObject* args) {

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* assignment_obj = PyTuple_GetItem(args, 0);

  if (!PyAssignment_CHECK(assignment_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  lp_polynomial_t* p = ((Polynomial*) self)->p;
  lp_assignment_t* assignment = ((Assignment*) assignment_obj)->assignment;

  lp_value_t* value = lp_polynomial_evaluate(p, assignment);
  PyObject* value_obj = PyValue_create(value);
  lp_value_delete(value);

  return value_obj;
}

static PyObject*
Polynomial_feasible_intervals(PyObject* self, PyObject* args) {

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 2) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* assignment_obj = PyTuple_GetItem(args, 0);
  if (!PyAssignment_CHECK(assignment_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* sgn_condition_obj = PyTuple_GetItem(args, 1);
  if (!PyInt_Check(sgn_condition_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

    // Get the arguments
  lp_polynomial_t* p = ((Polynomial*) self)->p;
  lp_assignment_t* assignment = ((Assignment*) assignment_obj)->assignment;
  lp_sign_condition_t sgn_condition = PyInt_AsLong(sgn_condition_obj);

  // Check if all but the top variable are unassigned
  if (!lp_polynomial_is_univariate_m(p, assignment)) {
    Py_IncRef(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Get the feasible intervals
  lp_feasibility_set_t* feasible = lp_polynomial_constraint_get_feasible_set(p, sgn_condition, 0, assignment);

  // The list where we return the arguments
  PyObject* list = PyList_New(feasible->size);
  // Copy over to the list
  size_t i;
  for (i = 0; i < feasible->size; ++i) {
    PyObject* p = PyInterval_create(feasible->intervals + i);
    PyList_SetItem(list, i, p);
  }
  // Remove temp
  lp_feasibility_set_delete(feasible);
  // Return the list
  return list;
}

static PyObject*
Polynomial_feasible_set(PyObject* self, PyObject* args) {

  if (!PyTuple_Check(args) || PyTuple_Size(args) != 2) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* assignment_obj = PyTuple_GetItem(args, 0);
  if (!PyAssignment_CHECK(assignment_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyObject* sgn_condition_obj = PyTuple_GetItem(args, 1);
  if (!PyInt_Check(sgn_condition_obj)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

    // Get the arguments
  lp_polynomial_t* p = ((Polynomial*) self)->p;
  lp_assignment_t* assignment = ((Assignment*) assignment_obj)->assignment;
  lp_sign_condition_t sgn_condition = PyInt_AsLong(sgn_condition_obj);

  // Check if all but the top variable are unassigned
  if (!lp_polynomial_is_univariate_m(p, assignment)) {
    Py_IncRef(Py_NotImplemented);
    return Py_NotImplemented;
  }

  // Get the feasible intervals
  lp_feasibility_set_t* feasible = lp_polynomial_constraint_get_feasible_set(p, sgn_condition, 0, assignment);

  // Return the list
  return PyFeasibilitySet_create(feasible);
}
