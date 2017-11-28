#ifndef HELPER_H_
#define HELPER_H_


//a: A_exp
//opExp
#define get_opexp_oper(a) (a->u.op.oper)
#define get_opexp_left(a) (a->u.op.left)
#define get_opexp_leftpos(a) (a->u.op.left->pos)
#define get_opexp_right(a) (a->u.op.right)
#define get_opexp_rightpos(a) (a->u.op.right->pos)

//letExp
#define get_letexp_decs(a) (a->u.let.decs)
#define get_letexp_body(a) (a->u.let.body)

//callExp
#define get_callexp_func(a) (a->u.call.func)
#define get_callexp_args(a) (a->u.call.args)

//recordExp
#define get_recordexp_typ(a) (a->u.record.typ)
#define get_recordexp_fields(a) (a->u.record.fields)

//seqExp
#define get_seqexp_seq(a) (a->u.seq)

//assignExp
#define get_assexp_var(a) (a->u.assign.var)
#define get_assexp_exp(a) (a->u.assign.exp) 

//ifexp
#define get_ifexp_test(a) (a->u.iff.test)
#define get_ifexp_then(a) (a->u.iff.then)
#define get_ifexp_else(a) (a->u.iff.elsee)

//whileexp
#define get_whileexp_test(a) (a->u.whilee.test)
#define get_whileexp_body(a) (a->u.whilee.body)

//forexp
#define get_forexp_var(a) (a->u.forr.var)
#define get_forexp_lo(a)  (a->u.forr.lo)
#define get_forexp_hi(a)  (a->u.forr.hi)
#define get_forexp_body(a)  (a->u.forr.body)

//arrayexp
#define get_arrayexp_typ(a) (a->u.array.typ)
#define get_arrayexp_size(a) (a->u.array.size)
#define get_arrayexp_init(a) (a->u.array.init)

//a: A_dec
#define get_vardec_init(a) (a->u.var.init)
#define get_vardec_var(a)  (a->u.var.var)
#define get_vardec_typ(a)  (a->u.var.typ)

#define get_typedec_list(a) (a->u.type)
#define get_funcdec_list(a) (a->u.function)

//a: struct expty
#define get_expty_kind(a) (a.ty->kind)
#define get_array_type(a) (a.ty->u.array->kind)
#define get_array(a) (a.ty->u.array)
#define get_record_fieldlist(a) (a.ty->u.record)

//v: A_var

#define get_simplevar_sym(v) (v->u.simple)
#define get_fieldvar_var(v) (v->u.field.var)
#define get_fieldvar_sym(v) (v->u.field.sym)
#define get_subvar_var(v) (v->u.subscript.var)
#define get_subvar_exp(v) (v->u.subscript.exp)

//a: A_ty
#define get_ty_name(a) (a->u.name)
#define get_ty_record(a) (a->u.record)
#define get_ty_array(a) (a->u.array)

//a: Ty_ty
#define get_array_kind(a) (a->u.array->kind)

//x: E_enventry
#define get_varentry_type(x) (x->u.var.ty)

#define get_func_res(x) (x->u.fun.result)
#define get_func_tylist(x) (x->u.fun.formals)

//------------------ lab5 ----------------------
#define get_var_access(x) (x->u.var.access)
#define get_func_label(x) (x->u.fun.label)
#define get_func_level(x) (x->u.fun.level)



#endif
