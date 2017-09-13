/*
  FFTW interface for Numo::NArray
  Copyright(C) 2017 Masahiro TANAKA

  FFTW: C subroutine library for computing the discrete Fourier transform (DFT)
  http://www.fftw.org/
*/

#include <fftw3.h>

#include <ruby.h>
#include "numo/narray.h"
#include "numo/template.h"
#define cDC numo_cDComplex
#define cDF numo_cDFloat

static ID id_cast;

<%
def argmap(arg)
  case arg
  when Numeric
    arg = 1..arg
  end
  arg.map do |x|
    yield(x)
  end.join(",")
end
$funcs = []
%>

<% (1..3).each do |d| %>
<% $funcs.push func="dft_#{d}d" %>
static void
iter_fftw_<%=func%>(na_loop_t *const lp)
{
    char  *p1, *p2;
    int   sign;
    int   <%=argmap(d){|i|"n#{i}"}%>;
    fftw_plan plan;

<% (1..d).each do |i| %>
    n<%=i%> = lp->args[0].shape[<%=i-1%>];
<% end %>
    p1 = ((lp)->args[0]).ptr + ((lp)->args[0].iter[0]).pos;
    p2 = ((lp)->args[1]).ptr + ((lp)->args[1].iter[0]).pos;

    sign = *(int*)(lp->opt_ptr);
    plan = fftw_plan_dft_<%=d%>d(<%=argmap(d){|i|"n#{i}"}%>,
                                 (fftw_complex*)p1, (fftw_complex*)p2,
                                 sign, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

/*
  <%=d%>-dimentional Complex Discrete Fourier Transform
  @overload dft_<%=d%>d(narray,sign)
  @param [Numo::DComplex] narray Input NArray with at least <%=d%> dimension<%="s" if d>1%>.
  @param [Numeric] sign  the sign of the exponent in the formula
    that defines the Fourier transform.
    It can be -1 (= FFTW_FORWARD) or +1 (= FFTW_BACKWARD).
  @return [Numo::DComplex] Result DComplex NArray with
    `shape = [.., <%if d>2%>nz, <%end;if d>1%>ny, <%end%>nx]`.
*/
static VALUE
numo_fftw_<%=func%>(VALUE mod, VALUE vna, VALUE vsign)
{
    narray_t *na;
    VALUE vans;
    int ndim;
    int sign=-1;
    size_t shape[<%=d%>];
    ndfunc_arg_in_t ain[1] = {{cDC,<%=d%>}};
    ndfunc_arg_out_t aout[1] = {{cDC,<%=d%>,shape}};
    ndfunc_t ndf = { iter_fftw_<%=func%>, NO_LOOP|NDF_INPLACE, 1, 1, ain, aout };

    sign = NUM2INT(vsign);
    if (sign != 1 && sign != -1) {
        rb_raise(rb_eArgError,"second argument (sign) must be 1 or -1");
    }
    GetNArray(vna,na);
    ndim = NA_NDIM(na);
    if (ndim<<%=d%>) {
        rb_raise(nary_eDimensionError,"ndim(=%d) should >= <%=d%>",ndim);
    }
<% (1..d).each do |i| %>
    shape[<%=d-i%>] = NA_SHAPE(na)[ndim-<%=i%>];
<% end %>
    vans = na_ndloop3(&ndf, &sign, 1, vna);
    RB_GC_GUARD(vna);
    return vans;
}
<% end %>

<% $funcs.push func="dft" %>
/*
  Multi-dimentional Complex Discrete Fourier Transform
  @overload dft(narray,sign)
  @param [Numo::DComplex] narray Input NArray with any dimension.
  @param [Numeric] sign  the sign of the exponent in the formula
    that defines the Fourier transform.
    It can be -1 (= FFTW_FORWARD) or +1 (= FFTW_BACKWARD).
  @return [Numo::DComplex] Result DComplex NArray with the same shape
    as input NArray.
*/
static VALUE
numo_fftw_<%=func%>(VALUE mod, VALUE vna, VALUE vsign)
{
    VALUE vans;
    int sign;
    narray_t *na;
    int ndim, i;
    int *shape;
    fftw_complex *in, *out;
    fftw_plan plan;

    sign = NUM2INT(vsign);
    if (sign != 1 && sign != -1) {
        rb_raise(rb_eArgError,"second argument (sign) must be 1 or -1");
    }
    if (CLASS_OF(vna) != numo_cDComplex) {
        vna = rb_funcall(numo_cDComplex, id_cast, 1, vna);
    }
    if (!RTEST(nary_check_contiguous(vna))) {
        vna = nary_dup(vna);
    }
    GetNArray(vna,na);
    ndim = NA_NDIM(na);
    shape = ALLOCA_N(int,ndim);
    for (i=0; i<ndim; i++) {
        shape[i] = NA_SHAPE(na)[i];
    }
    if (TEST_INPLACE(vna)) {
        vans = vna;
        in = out = (fftw_complex*)na_get_pointer_for_read_write(vna);
    } else {
        vans = nary_s_new_like(numo_cDComplex, vna);
        in = (fftw_complex*)na_get_pointer_for_read(vna);
        out = (fftw_complex*)na_get_pointer_for_write(vans);
    }
    plan = fftw_plan_dft(ndim, shape, in, out, sign, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);

    RB_GC_GUARD(vna);
    return vans;
}

void
Init_fftw()
{
    VALUE mNumo,mFFTW;

    rb_require("numo/narray");
    mNumo = rb_define_module("Numo");
    mFFTW = rb_define_module_under(mNumo,"FFTW");

<% $funcs.each do |f| %>
    rb_define_module_function(mFFTW,"<%=f%>",numo_fftw_<%=f%>,2);
<% end %>

    id_cast    = rb_intern("cast");
}
