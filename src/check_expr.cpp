enum CallArgumentError {
	CallArgumentError_None,
	CallArgumentError_NoneProcedureType,
	CallArgumentError_WrongTypes,
	CallArgumentError_NonVariadicExpand,
	CallArgumentError_VariadicTuple,
	CallArgumentError_MultipleVariadicExpand,
	CallArgumentError_AmbiguousPolymorphicVariadic,
	CallArgumentError_ArgumentCount,
	CallArgumentError_TooFewArguments,
	CallArgumentError_TooManyArguments,
	CallArgumentError_InvalidFieldValue,
	CallArgumentError_ParameterNotFound,
	CallArgumentError_ParameterMissing,
	CallArgumentError_DuplicateParameter,
	CallArgumentError_NoneConstantParameter,

	CallArgumentError_MAX,
};
char const *CallArgumentError_strings[CallArgumentError_MAX] = {
	"None",
	"NoneProcedureType",
	"WrongTypes",
	"NonVariadicExpand",
	"VariadicTuple",
	"MultipleVariadicExpand",
	"AmbiguousPolymorphicVariadic",
	"ArgumentCount",
	"TooFewArguments",
	"TooManyArguments",
	"InvalidFieldValue",
	"ParameterNotFound",
	"ParameterMissing",
	"DuplicateParameter",
	"NoneConstantParameter",
};


enum CallArgumentErrorMode {
	CallArgumentMode_NoErrors,
	CallArgumentMode_ShowErrors,
};

struct CallArgumentData {
	Entity *gen_entity;
	i64     score;
	Type *  result_type;
};

struct PolyProcData {
	Entity *  gen_entity;
	ProcInfo *proc_info;
};

struct ValidIndexAndScore {
	isize index;
	i64   score;
};

int valid_index_and_score_cmp(void const *a, void const *b) {
	i64 si = (cast(ValidIndexAndScore const *)a)->score;
	i64 sj = (cast(ValidIndexAndScore const *)b)->score;
	return sj < si ? -1 : sj > si;
}



#define CALL_ARGUMENT_CHECKER(name) CallArgumentError name(CheckerContext *c, Ast *call, Type *proc_type, Entity *entity, Array<Operand> operands, CallArgumentErrorMode show_error_mode, CallArgumentData *data)
typedef CALL_ARGUMENT_CHECKER(CallArgumentCheckerType);



void     check_expr                     (CheckerContext *c, Operand *operand, Ast *expression);
void     check_multi_expr               (CheckerContext *c, Operand *operand, Ast *expression);
void     check_multi_expr_or_type       (CheckerContext *c, Operand *operand, Ast *expression);
void     check_multi_expr_with_type_hint(CheckerContext *c, Operand *o, Ast *e, Type *type_hint);
void     check_expr_or_type             (CheckerContext *c, Operand *operand, Ast *expression, Type *type_hint);
ExprKind check_expr_base                (CheckerContext *c, Operand *operand, Ast *expression, Type *type_hint);
void     check_expr_with_type_hint      (CheckerContext *c, Operand *o, Ast *e, Type *t);
Type *   check_type                     (CheckerContext *c, Ast *expression);
Type *   check_type_expr                (CheckerContext *c, Ast *expression, Type *named_type);
Type *   make_optional_ok_type          (Type *value, bool typed=true);
Entity * check_selector                 (CheckerContext *c, Operand *operand, Ast *node, Type *type_hint);
Entity * check_ident                    (CheckerContext *c, Operand *o, Ast *n, Type *named_type, Type *type_hint, bool allow_import_name);
Entity * find_polymorphic_record_entity (CheckerContext *c, Type *original_type, isize param_count, Array<Operand> const &ordered_operands, bool *failure);
void     check_not_tuple                (CheckerContext *c, Operand *operand);
void     convert_to_typed               (CheckerContext *c, Operand *operand, Type *target_type);
gbString expr_to_string                 (Ast *expression);
void     check_proc_body                (CheckerContext *c, Token token, DeclInfo *decl, Type *type, Ast *body);
void     update_untyped_expr_type       (CheckerContext *c, Ast *e, Type *type, bool final);
bool     check_is_terminating           (Ast *node, String const &label);
bool     check_has_break                (Ast *stmt, String const &label, bool implicit);
void     check_stmt                     (CheckerContext *c, Ast *node, u32 flags);
void     check_stmt_list                (CheckerContext *c, Slice<Ast *> const &stmts, u32 flags);
void     check_init_constant            (CheckerContext *c, Entity *e, Operand *operand);
bool     check_representable_as_constant(CheckerContext *c, ExactValue in_value, Type *type, ExactValue *out_value);
bool     check_procedure_type           (CheckerContext *c, Type *type, Ast *proc_type_node, Array<Operand> *operands = nullptr);
void     check_struct_type              (CheckerContext *c, Type *struct_type, Ast *node, Array<Operand> *poly_operands,
                                         Type *named_type = nullptr, Type *original_type_for_poly = nullptr);
void     check_union_type               (CheckerContext *c, Type *union_type, Ast *node, Array<Operand> *poly_operands,
                                         Type *named_type = nullptr, Type *original_type_for_poly = nullptr);

CallArgumentData check_call_arguments   (CheckerContext *c, Operand *operand, Type *proc_type, Ast *call);
Type *           check_init_variable    (CheckerContext *c, Entity *e, Operand *operand, String context_name);


void check_assignment_error_suggestion(CheckerContext *c, Operand *o, Type *type);
void add_map_key_type_dependencies(CheckerContext *ctx, Type *key);

Type *make_soa_struct_slice(CheckerContext *ctx, Ast *array_typ_expr, Ast *elem_expr, Type *elem);
Type *make_soa_struct_dynamic_array(CheckerContext *ctx, Ast *array_typ_expr, Ast *elem_expr, Type *elem);

bool check_builtin_procedure(CheckerContext *c, Operand *operand, Ast *call, i32 id, Type *type_hint);

void check_promote_optional_ok(CheckerContext *c, Operand *x, Type **val_type_, Type **ok_type_);

void check_or_else_right_type(CheckerContext *c, Ast *expr, String const &name, Type *right_type);
void check_or_else_split_types(CheckerContext *c, Operand *x, String const &name, Type **left_type_, Type **right_type_);
void check_or_else_expr_no_value_error(CheckerContext *c, String const &name, Operand const &x, Type *type_hint);
void check_or_return_split_types(CheckerContext *c, Operand *x, String const &name, Type **left_type_, Type **right_type_);


void check_did_you_mean_print(DidYouMeanAnswers *d, char const *prefix = "") {
	auto results = did_you_mean_results(d);
	if (results.count != 0) {
		error_line("\tSuggestion: Did you mean?\n");
		for_array(i, results) {
			String const &target = results[i].target;
			error_line("\t\t%s%.*s\n", prefix, LIT(target));
			// error_line("\t\t%.*s %td\n", LIT(target), results[i].distance);
		}
	}
}

void check_did_you_mean_type(String const &name, Array<Entity *> const &fields, char const *prefix = "") {
	ERROR_BLOCK();

	DidYouMeanAnswers d = did_you_mean_make(heap_allocator(), fields.count, name);
	defer (did_you_mean_destroy(&d));

	for_array(i, fields) {
		did_you_mean_append(&d, fields[i]->token.string);
	}
	check_did_you_mean_print(&d, prefix);
}

void check_did_you_mean_type(String const &name, Slice<Entity *> const &fields, char const *prefix = "") {
	ERROR_BLOCK();

	DidYouMeanAnswers d = did_you_mean_make(heap_allocator(), fields.count, name);
	defer (did_you_mean_destroy(&d));

	for_array(i, fields) {
		did_you_mean_append(&d, fields[i]->token.string);
	}
	check_did_you_mean_print(&d, prefix);
}

void check_did_you_mean_scope(String const &name, Scope *scope, char const *prefix = "") {
	ERROR_BLOCK();

	DidYouMeanAnswers d = did_you_mean_make(heap_allocator(), scope->elements.entries.count, name);
	defer (did_you_mean_destroy(&d));

	mutex_lock(&scope->mutex);
	for_array(i, scope->elements.entries) {
		Entity *e = scope->elements.entries[i].value;
		did_you_mean_append(&d, e->token.string);
	}
	mutex_unlock(&scope->mutex);
	check_did_you_mean_print(&d, prefix);
}

Entity *entity_from_expr(Ast *expr) {
	expr = unparen_expr(expr);
	switch (expr->kind) {
	case Ast_Ident:
		return expr->Ident.entity;
	case Ast_SelectorExpr:
		return entity_from_expr(expr->SelectorExpr.selector);
	}
	return nullptr;
}

void error_operand_not_expression(Operand *o) {
	if (o->mode == Addressing_Type) {
		gbString err = expr_to_string(o->expr);
		error(o->expr, "'%s' is not an expression but a type", err);
		gb_string_free(err);
		o->mode = Addressing_Invalid;
	}
}

void error_operand_no_value(Operand *o) {
	if (o->mode == Addressing_NoValue) {
		gbString err = expr_to_string(o->expr);
		Ast *x = unparen_expr(o->expr);
		if (x->kind == Ast_CallExpr) {
			error(o->expr, "'%s' call does not return a value and cannot be used as a value", err);
		} else {
			error(o->expr, "'%s' used as a value", err);
		}
		gb_string_free(err);
		o->mode = Addressing_Invalid;
	}
}


void check_scope_decls(CheckerContext *c, Slice<Ast *> const &nodes, isize reserve_size) {
	Scope *s = c->scope;

	check_collect_entities(c, nodes);

	for_array(i, s->elements.entries) {
		Entity *e = s->elements.entries[i].value;
		switch (e->kind) {
		case Entity_Constant:
		case Entity_TypeName:
		case Entity_Procedure:
			break;
		default:
			continue;
		}
		DeclInfo *d = decl_info_of_entity(e);
		if (d != nullptr) {
			check_entity_decl(c, e, d, nullptr);
		}
	}
}

bool find_or_generate_polymorphic_procedure(CheckerContext *old_c, Entity *base_entity, Type *type,
                                            Array<Operand> *param_operands, Ast *poly_def_node, PolyProcData *poly_proc_data) {
	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// TODO CLEANUP(bill): This procedure is very messy and hacky. Clean this!!! //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	CheckerInfo *info = old_c->info;

	if (base_entity == nullptr) {
		return false;
	}

	if (!is_type_proc(base_entity->type)) {
		return false;
	}

	String name = base_entity->token.string;

	Type *src = base_type(base_entity->type);
	Type *dst = nullptr;
	if (type != nullptr) {
		dst = base_type(type);
	}

	if (param_operands == nullptr) {
		GB_ASSERT(dst != nullptr);
	}
	if (param_operands != nullptr) {
		GB_ASSERT(dst == nullptr);
	}

	mutex_lock(&info->gen_procs_mutex);
	defer (mutex_unlock(&info->gen_procs_mutex));

	if (!src->Proc.is_polymorphic || src->Proc.is_poly_specialized) {
		return false;
	}

	if (dst != nullptr) {
		if (dst->Proc.is_polymorphic) {
			return false;
		}

		if (dst->Proc.param_count  != src->Proc.param_count ||
		    dst->Proc.result_count != src->Proc.result_count) {
		    return false;
		}
	}


	DeclInfo *old_decl = decl_info_of_entity(base_entity);
	if (old_decl == nullptr) {
		return false;
	}


	gbAllocator a = heap_allocator();

	Array<Operand> operands = {};
	if (param_operands) {
		operands = *param_operands;
	} else {
		operands = array_make<Operand>(a, 0, dst->Proc.param_count);
		for (isize i = 0; i < dst->Proc.param_count; i++) {
			Entity *param = dst->Proc.params->Tuple.variables[i];
			Operand o = {Addressing_Value};
			o.type = param->type;
			array_add(&operands, o);
		}
	}

	defer (if (param_operands == nullptr) {
		array_free(&operands);
	});


	CheckerContext nctx = *old_c;

	nctx.procs_to_check_queue = old_c->procs_to_check_queue;

	Scope *scope = create_scope(info, base_entity->scope);
	scope->flags |= ScopeFlag_Proc;
	nctx.scope = scope;
	nctx.allow_polymorphic_types = true;
	if (nctx.polymorphic_scope == nullptr) {
		nctx.polymorphic_scope = scope;
	}


	auto *pt = &src->Proc;

	// NOTE(bill): This is slightly memory leaking if the type already exists
	// Maybe it's better to check with the previous types first?
	Type *final_proc_type = alloc_type_proc(scope, nullptr, 0, nullptr, 0, false, pt->calling_convention);
	bool success = check_procedure_type(&nctx, final_proc_type, pt->node, &operands);

	if (!success) {
		return false;
	}

	auto *found_gen_procs = map_get(&info->gen_procs, base_entity->identifier.load());
	if (found_gen_procs) {
		auto procs = *found_gen_procs;
		for_array(i, procs) {
			Entity *other = procs[i];
			Type *pt = base_type(other->type);
			if (are_types_identical(pt, final_proc_type)) {
				if (poly_proc_data) {
					poly_proc_data->gen_entity = other;
				}
				return true;
			}
		}
	}

	{
		// LEAK TODO(bill): This is technically a memory leak as it has to generate the type twice
		bool prev_no_polymorphic_errors = nctx.no_polymorphic_errors;
		defer (nctx.no_polymorphic_errors = prev_no_polymorphic_errors);
		nctx.no_polymorphic_errors = false;

		// NOTE(bill): Reset scope from the failed procedure type
		scope_reset(scope);

		// LEAK TODO(bill): Cloning this AST may be leaky
		Ast *cloned_proc_type_node = clone_ast(pt->node);
		success = check_procedure_type(&nctx, final_proc_type, cloned_proc_type_node, &operands);

		if (!success) {
			return false;
		}

		if (found_gen_procs) {
			auto procs = *found_gen_procs;
			for_array(i, procs) {
				Entity *other = procs[i];
				Type *pt = base_type(other->type);
				if (are_types_identical(pt, final_proc_type)) {
					if (poly_proc_data) {
						poly_proc_data->gen_entity = other;
					}
					return true;
				}
			}
		}
	}


	Ast *proc_lit = clone_ast(old_decl->proc_lit);
	ast_node(pl, ProcLit, proc_lit);
	// NOTE(bill): Associate the scope declared above withinth this procedure declaration's type
	add_scope(&nctx, pl->type, final_proc_type->Proc.scope);
	final_proc_type->Proc.is_poly_specialized = true;
	final_proc_type->Proc.is_polymorphic = true;


	for (isize i = 0; i < operands.count; i++) {
		Operand o = operands[i];
		if (final_proc_type == o.type ||
		    base_entity->type == o.type) {
			// NOTE(bill): Cycle
			final_proc_type->Proc.is_poly_specialized = false;
			break;
		}
	}

	u64 tags = base_entity->Procedure.tags;
	Ast *ident = clone_ast(base_entity->identifier);
	Token token = ident->Ident.token;
	DeclInfo *d = make_decl_info(scope, old_decl->parent);
	d->gen_proc_type = final_proc_type;
	d->type_expr = pl->type;
	d->proc_lit = proc_lit;
	d->proc_checked = false;

	Entity *entity = alloc_entity_procedure(nullptr, token, final_proc_type, tags);
	entity->identifier = ident;

	add_entity_and_decl_info(&nctx, ident, entity, d);
	// NOTE(bill): Set the scope afterwards as this is not real overloading
	entity->scope = scope->parent;
	entity->file = base_entity->file;
	entity->pkg = base_entity->pkg;
	entity->flags &= ~EntityFlag_ProcBodyChecked;

	AstFile *file = nullptr;
	{
		Scope *s = entity->scope;
		while (s != nullptr && s->file == nullptr) {
			file = s->file;
			s = s->parent;
		}
	}

	ProcInfo *proc_info = gb_alloc_item(permanent_allocator(), ProcInfo);
	proc_info->file  = file;
	proc_info->token = token;
	proc_info->decl  = d;
	proc_info->type  = final_proc_type;
	proc_info->body  = pl->body;
	proc_info->tags  = tags;
	proc_info->generated_from_polymorphic = true;
	proc_info->poly_def_node = poly_def_node;

	if (found_gen_procs) {
		array_add(found_gen_procs, entity);
	} else {
		auto array = array_make<Entity *>(heap_allocator());
		array_add(&array, entity);
		map_set(&info->gen_procs, base_entity->identifier.load(), array);
	}

	if (poly_proc_data) {
		poly_proc_data->gen_entity = entity;
		poly_proc_data->proc_info  = proc_info;
		entity->Procedure.generated_from_polymorphic = proc_info->generated_from_polymorphic;
	}

	// NOTE(bill): Check the newly generated procedure body
	check_procedure_later(&nctx, proc_info);

	return true;
}

bool check_polymorphic_procedure_assignment(CheckerContext *c, Operand *operand, Type *type, Ast *poly_def_node, PolyProcData *poly_proc_data) {
	if (operand->expr == nullptr) return false;
	Entity *base_entity = entity_of_node(operand->expr);
	if (base_entity == nullptr) return false;
	return find_or_generate_polymorphic_procedure(c, base_entity, type, nullptr, poly_def_node, poly_proc_data);
}

bool find_or_generate_polymorphic_procedure_from_parameters(CheckerContext *c, Entity *base_entity, Array<Operand> *operands, Ast *poly_def_node, PolyProcData *poly_proc_data) {
	return find_or_generate_polymorphic_procedure(c, base_entity, nullptr, operands, poly_def_node, poly_proc_data);
}

bool check_type_specialization_to(CheckerContext *c, Type *specialization, Type *type, bool compound, bool modify_type);
bool is_polymorphic_type_assignable(CheckerContext *c, Type *poly, Type *source, bool compound, bool modify_type);
bool check_cast_internal(CheckerContext *c, Operand *x, Type *type);

#define MAXIMUM_TYPE_DISTANCE 10

i64 check_distance_between_types(CheckerContext *c, Operand *operand, Type *type) {
	if (c == nullptr) {
		GB_ASSERT(operand->mode == Addressing_Value);
		GB_ASSERT(is_type_typed(operand->type));
	}
	if (operand->mode == Addressing_Invalid ||
	    type == t_invalid) {
		return -1;
	}

	if (operand->mode == Addressing_Builtin) {
		return -1;
	}

	if (operand->mode == Addressing_Type) {
		if (is_type_typeid(type)) {
			add_type_info_type(c, operand->type);
			return 4;
		}
		return -1;
	}

	Type *s = operand->type;

	if (are_types_identical(s, type)) {
		return 0;
	}

	Type *src = base_type(s);
	Type *dst = base_type(type);

	if (is_type_untyped_undef(src)) {
		if (type_has_undef(dst)) {
			return 1;
		}
		return -1;
	}

	if (is_type_untyped_nil(src)) {
		if (type_has_nil(dst)) {
			return 1;
		}
		return -1;
	}
	if (is_type_untyped(src)) {
		if (is_type_any(dst)) {
			// NOTE(bill): Anything can cast to 'Any'
			add_type_info_type(c, s);
			return MAXIMUM_TYPE_DISTANCE;
		}
		if (dst->kind == Type_Basic) {
			if (operand->mode == Addressing_Constant) {
				if (check_representable_as_constant(c, operand->value, dst, nullptr)) {
					if (is_type_typed(dst) && src->kind == Type_Basic) {
						switch (src->Basic.kind) {
						case Basic_UntypedRune:
							if (is_type_integer(dst) || is_type_rune(dst)) {
								return 1;
							}
							break;
						case Basic_UntypedInteger:
							if (is_type_integer(dst) || is_type_rune(dst)) {
								return 1;
							}
							break;
						case Basic_UntypedFloat:
							if (is_type_float(dst)) {
								return 1;
							}
							break;
						case Basic_UntypedComplex:
							if (is_type_complex(dst)) {
								return 1;
							}
							if (is_type_quaternion(dst)) {
								return 2;
							}
							break;
						case Basic_UntypedQuaternion:
							if (is_type_quaternion(dst)) {
								return 1;
							}
							break;
						}
					}
					return 2;
				}
				return -1;
			}
			if (src->kind == Type_Basic && src->Basic.kind == Basic_UntypedRune) {
				if (is_type_integer(dst) || is_type_rune(dst)) {
					if (is_type_typed(type)) {
						return 2;
					}
					return 1;
				}
				return -1;
			}
			if (src->kind == Type_Basic && src->Basic.kind == Basic_UntypedBool) {
				if (is_type_boolean(dst)) {
					if (is_type_typed(type)) {
						return 2;
					}
					return 1;
				}
				return -1;
			}
		}
	}

	if (is_type_enum(dst) && are_types_identical(dst->Enum.base_type, operand->type)) {
		if (c->in_enum_type) {
			return 3;
		}
	}


	{
		isize subtype_level = check_is_assignable_to_using_subtype(operand->type, type);
		if (subtype_level > 0) {
			return 4 + subtype_level;
		}
	}

	// ^T <- rawptr
#if 0
	// TODO(bill): Should C-style (not C++) pointer cast be allowed?
	if (is_type_pointer(dst) && is_type_rawptr(src)) {
	    return true;
	}
#endif
#if 1


	// rawptr <- ^T
	if (are_types_identical(type, t_rawptr) && is_type_pointer(src)) {
		return 5;
	}
	// rawptr <- [^]T
	if (are_types_identical(type, t_rawptr) && is_type_multi_pointer(src)) {
		return 5;
	}
	// ^T <- [^]T
	if (dst->kind == Type_Pointer && src->kind == Type_MultiPointer) {
		if (are_types_identical(dst->Pointer.elem, src->MultiPointer.elem)) {
			return 4;
		}
	}
	// [^]T <- ^T
	if (dst->kind == Type_MultiPointer && src->kind == Type_Pointer) {
		if (are_types_identical(dst->MultiPointer.elem, src->Pointer.elem)) {
			return 4;
		}
	}
#endif

	if (is_type_polymorphic(dst) && !is_type_polymorphic(src)) {
		bool modify_type = !c->no_polymorphic_errors;
		if (is_polymorphic_type_assignable(c, type, s, false, modify_type)) {
			return 2;
		}
	}

	if (is_type_union(dst)) {
		for_array(i, dst->Union.variants) {
			Type *vt = dst->Union.variants[i];
			if (are_types_identical(vt, s)) {
				return 1;
			}
		}

		// TODO(bill): Determine which rule is a better on in practice
		#if 1
			if (dst->Union.variants.count == 1) {
				Type *vt = dst->Union.variants[0];
				i64 score = check_distance_between_types(c, operand, vt);
				if (score >= 0) {
					return score+2;
				}
			}
		#else
			// NOTE(bill): check to see you can assign to it with one of the variants?
			i64 prev_lowest_score = -1;
			i64 lowest_score = -1;
			for_array(i, dst->Union.variants) {
				Type *vt = dst->Union.variants[i];
				i64 score = check_distance_between_types(c, operand, vt);
				if (score >= 0) {
					if (lowest_score < 0) {
						lowest_score = score;
					} else {
						if (prev_lowest_score < 0) {
							prev_lowest_score = lowest_score;
						} else {
							prev_lowest_score = gb_min(prev_lowest_score, lowest_score);
						}
						lowest_score = gb_min(lowest_score, score);
					}
				}
			}
			if (lowest_score >= 0) {
				if (prev_lowest_score != lowest_score) { // remove possible ambiguities
					return lowest_score+2;
				}
			}
		#endif
	}

	if (is_type_relative_pointer(dst)) {
		i64 score = check_distance_between_types(c, operand, dst->RelativePointer.pointer_type);
		if (score >= 0) {
			return score+2;
		}
	}

	if (is_type_relative_slice(dst)) {
		i64 score = check_distance_between_types(c, operand, dst->RelativeSlice.slice_type);
		if (score >= 0) {
			return score+2;
		}
	}

	if (is_type_proc(dst)) {
		if (are_types_identical(src, dst)) {
			return 3;
		}
		PolyProcData poly_proc_data = {};
		if (check_polymorphic_procedure_assignment(c, operand, type, operand->expr, &poly_proc_data)) {
			Entity *e = poly_proc_data.gen_entity;
			add_type_and_value(c->info, operand->expr, Addressing_Value, e->type, {});
			add_entity_use(c, operand->expr, e);
			return 4;
		}
	}
	
	if (is_type_complex_or_quaternion(dst)) {
		Type *elem = base_complex_elem_type(dst);
		if (are_types_identical(elem, base_type(src))) {
			return 5;
		}
	}

	if (is_type_array(dst)) {
		Type *elem = base_array_type(dst);
		i64 distance = check_distance_between_types(c, operand, elem);
		if (distance >= 0) {
			return distance + 6;
		}
	}
	
	if (is_type_matrix(dst)) {
		Type *dst_elem = base_array_type(dst);
		i64 distance = check_distance_between_types(c, operand, dst_elem);
		if (distance >= 0) {
			return distance + 7;
		}
	}

	if (is_type_any(dst)) {
		if (!is_type_polymorphic(src)) {
			if (operand->mode == Addressing_Context && operand->type == t_context) {
				return -1;
			} else {
				// NOTE(bill): Anything can cast to 'Any'
				add_type_info_type(c, s);
				return MAXIMUM_TYPE_DISTANCE;
			}
		}
	}

	Ast *expr = unparen_expr(operand->expr);
	if (expr != nullptr) {
		if (expr->kind == Ast_AutoCast) {
			Operand x = *operand;
			x.expr = expr->AutoCast.expr;
			if (check_cast_internal(c, &x, type)) {
				return MAXIMUM_TYPE_DISTANCE;
			}
		}
	}

	return -1;
}


i64 assign_score_function(i64 distance, bool is_variadic=false) {
	// 3*x^2 + 1 > x^2 + x + 1 (for positive x)
	i64 const c = 3*MAXIMUM_TYPE_DISTANCE*MAXIMUM_TYPE_DISTANCE + 1;

	// TODO(bill): A decent score function
	i64 d = distance*distance; // x^2
	if (is_variadic && d >= 0) {
		d += distance + 1; // x^2 + x + 1
	}
	return gb_max(c - d, 0);
}


bool check_is_assignable_to_with_score(CheckerContext *c, Operand *operand, Type *type, i64 *score_, bool is_variadic=false) {
	i64 score = 0;
	i64 distance = check_distance_between_types(c, operand, type);
	bool ok = distance >= 0;
	if (ok) {
		score = assign_score_function(distance, is_variadic);
	}
	if (score_) *score_ = score;
	return ok;
}


bool check_is_assignable_to(CheckerContext *c, Operand *operand, Type *type) {
	i64 score = 0;
	return check_is_assignable_to_with_score(c, operand, type, &score);
}

bool internal_check_is_assignable_to(Type *src, Type *dst) {
	Operand x = {};
	x.type = src;
	x.mode = Addressing_Value;
	return check_is_assignable_to(nullptr, &x, dst);
}

AstPackage *get_package_of_type(Type *type) {
	for (;;) {
		if (type == nullptr) {
			return nullptr;
		}
		switch (type->kind) {
		case Type_Basic:
			return builtin_pkg;
		case Type_Named:
			if (type->Named.type_name != nullptr) {
				return type->Named.type_name->pkg;
			}
			return nullptr;
		case Type_Pointer:
			type = type->Pointer.elem;
			continue;
		case Type_Array:
			type = type->Array.elem;
			continue;
		case Type_Slice:
			type = type->Slice.elem;
			continue;
		case Type_DynamicArray:
			type = type->DynamicArray.elem;
			continue;
		case Type_RelativePointer:
			type = type->RelativePointer.pointer_type;
			continue;
		case Type_RelativeSlice:
			type = type->RelativeSlice.slice_type;
			continue;
		}
		return nullptr;
	}
}


// NOTE(bill): 'content_name' is for debugging and error messages
void check_assignment(CheckerContext *c, Operand *operand, Type *type, String context_name) {
	check_not_tuple(c, operand);
	if (operand->mode == Addressing_Invalid) {
		return;
	}

	if (is_type_untyped(operand->type)) {
		Type *target_type = type;
		if (type == nullptr || is_type_any(type)) {
			if (type == nullptr && is_type_untyped_nil(operand->type)) {
				error(operand->expr, "Use of untyped nil in %.*s", LIT(context_name));
				operand->mode = Addressing_Invalid;
				return;
			}
			if (type == nullptr && is_type_untyped_undef(operand->type)) {
				error(operand->expr, "Use of --- in %.*s", LIT(context_name));
				operand->mode = Addressing_Invalid;
				return;
			}
			target_type = default_type(operand->type);
			if (type != nullptr && !is_type_any(type)) {
				GB_ASSERT_MSG(is_type_typed(target_type), "%s", type_to_string(type));
			}
			add_type_info_type(c, type);
			add_type_info_type(c, target_type);
		}

		convert_to_typed(c, operand, target_type);
		if (operand->mode == Addressing_Invalid) {
			return;
		}
	}


	if (type == nullptr) {
		return;
	}

	if (operand->mode == Addressing_ProcGroup) {
		Array<Entity *> procs = proc_group_entities(c, *operand);
		bool good = false;
		// NOTE(bill): These should be done
		for_array(i, procs) {
			Type *t = base_type(procs[i]->type);
			if (t == t_invalid) {
				continue;
			}
			Operand x = {};
			x.mode = Addressing_Value;
			x.type = t;
			if (check_is_assignable_to(c, &x, type)) {
				Entity *e = procs[i];
				add_entity_use(c, operand->expr, e);
				good = true;
				break;
			}
		}

		if (!good) {
			gbString expr_str    = expr_to_string(operand->expr);
			gbString op_type_str = type_to_string(operand->type);
			gbString type_str    = type_to_string(type);

			defer (gb_string_free(type_str));
			defer (gb_string_free(op_type_str));
			defer (gb_string_free(expr_str));

			// TODO(bill): is this a good enough error message?
			error(operand->expr,
			      "Cannot assign overloaded procedure '%s' to '%s' in %.*s",
			      expr_str,
			      op_type_str,
			      LIT(context_name));
			operand->mode = Addressing_Invalid;
		}

		convert_to_typed(c, operand, type);
		return;
	}

	if (check_is_assignable_to(c, operand, type)) {
		if (operand->mode == Addressing_Type && is_type_typeid(type)) {
			add_type_info_type(c, operand->type);
			add_type_and_value(c->info, operand->expr, Addressing_Value, type, exact_value_typeid(operand->type));
		}
	} else {
		gbString expr_str    = expr_to_string(operand->expr);
		gbString op_type_str = type_to_string(operand->type);
		gbString type_str    = type_to_string(type);

		defer (gb_string_free(type_str));
		defer (gb_string_free(op_type_str));
		defer (gb_string_free(expr_str));

		switch (operand->mode) {
		case Addressing_Builtin:
			// TODO(bill): Actually allow built in procedures to be passed around and thus be created on use
			error(operand->expr,
			      "Cannot assign built-in procedure '%s' in %.*s",
			      expr_str,
			      LIT(context_name));
			break;
		case Addressing_Type:
			error(operand->expr,
			      "Cannot assign '%s' which is a type in %.*s",
			      op_type_str,
			      LIT(context_name));
			break;
		default:
			// TODO(bill): is this a good enough error message?
			{
				gbString op_type_extra = gb_string_make(heap_allocator(), "");
				gbString type_extra = gb_string_make(heap_allocator(), "");
				defer (gb_string_free(op_type_extra));
				defer (gb_string_free(type_extra));

				isize on = gb_string_length(op_type_str);
				isize tn = gb_string_length(type_str);
				if (on == tn && gb_strncmp(op_type_str, type_str, on) == 0) {
					AstPackage *op_pkg = get_package_of_type(operand->type);
					AstPackage *type_pkg = get_package_of_type(type);
					if (op_pkg != nullptr) {
						op_type_extra = gb_string_append_fmt(op_type_extra, " (package %.*s)", LIT(op_pkg->name));
					}
					if (type_pkg != nullptr) {
						type_extra = gb_string_append_fmt(type_extra, " (package %.*s)", LIT(type_pkg->name));
					}
				}
				error(operand->expr,
				      "Cannot assign value '%s' of type '%s%s' to '%s%s' in %.*s",
				      expr_str,
				      op_type_str, op_type_extra,
				      type_str, type_extra,
				      LIT(context_name));
				check_assignment_error_suggestion(c, operand, type);
			}
			break;
		}
		operand->mode = Addressing_Invalid;

		return;
	}
}

bool polymorphic_assign_index(Type **gt_, i64 *dst_count, i64 source_count) {
	Type *gt = *gt_;
	
	GB_ASSERT(gt->kind == Type_Generic);
	Entity *e = scope_lookup(gt->Generic.scope, gt->Generic.name);
	GB_ASSERT(e != nullptr);
	if (e->kind == Entity_TypeName) {
		*gt_ = nullptr;
		*dst_count = source_count;

		e->kind = Entity_Constant;
		e->Constant.value = exact_value_i64(source_count);
		e->type = t_untyped_integer;
		return true;
	} else if (e->kind == Entity_Constant) {
		*gt_ = nullptr;
		if (e->Constant.value.kind != ExactValue_Integer) {
			return false;
		}
		i64 count = big_int_to_i64(&e->Constant.value.value_integer);
		if (count != source_count) {
			return false;
		}
		*dst_count = source_count;
		return true;
	}
	return false;
}

bool is_polymorphic_type_assignable(CheckerContext *c, Type *poly, Type *source, bool compound, bool modify_type) {
	Operand o = {Addressing_Value};
	o.type = source;
	switch (poly->kind) {
	case Type_Basic:
		if (compound) return are_types_identical(poly, source);
		return check_is_assignable_to(c, &o, poly);

	case Type_Named: {
		if (check_type_specialization_to(c, poly, source, compound, modify_type)) {
			return true;
		}
		if (compound || !is_type_generic(poly)) {
			return are_types_identical(poly, source);
		}
		return check_is_assignable_to(c, &o, poly);
	}

	case Type_Generic: {
		if (poly->Generic.specialized != nullptr) {
			Type *s = poly->Generic.specialized;
			if (!check_type_specialization_to(c, s, source, compound, modify_type)) {
				return false;
			}
		}
		if (modify_type) {
			Type *ds = default_type(source);
			gb_memmove(poly, ds, gb_size_of(Type));
		}
		return true;
	}
	case Type_Pointer:
		if (source->kind == Type_Pointer) {
			isize level = check_is_assignable_to_using_subtype(source->Pointer.elem, poly->Pointer.elem);
			if (level > 0) {
				return true;
			}
			return is_polymorphic_type_assignable(c, poly->Pointer.elem, source->Pointer.elem, true, modify_type);
		}
		return false;

	case Type_MultiPointer:
		if (source->kind == Type_MultiPointer) {
			isize level = check_is_assignable_to_using_subtype(source->MultiPointer.elem, poly->MultiPointer.elem);
			if (level > 0) {
				return true;
			}
			return is_polymorphic_type_assignable(c, poly->MultiPointer.elem, source->MultiPointer.elem, true, modify_type);
		}
		return false;
	case Type_Array:
		if (source->kind == Type_Array) {
			if (poly->Array.generic_count != nullptr) {
				if (!polymorphic_assign_index(&poly->Array.generic_count, &poly->Array.count, source->Array.count)) {
					return false;
				}
			}
			if (poly->Array.count == source->Array.count) {
				return is_polymorphic_type_assignable(c, poly->Array.elem, source->Array.elem, true, modify_type);
			}
		} else if (source->kind == Type_EnumeratedArray) {
			if (poly->Array.generic_count != nullptr) {
				Type *gt = poly->Array.generic_count;
				GB_ASSERT(gt->kind == Type_Generic);
				Entity *e = scope_lookup(gt->Generic.scope, gt->Generic.name);
				GB_ASSERT(e != nullptr);
				if (e->kind == Entity_TypeName) {
					Type *index = source->EnumeratedArray.index;
					Type *it = base_type(index);
					if (it->kind != Type_Enum) {
						return false;
					}

					poly->kind = Type_EnumeratedArray;
					poly->cached_size  = -1;
					poly->cached_align = -1;
					poly->flags.exchange(source->flags);
					poly->failure      = false;
					poly->EnumeratedArray.elem      = source->EnumeratedArray.elem;
					poly->EnumeratedArray.index     = source->EnumeratedArray.index;
					poly->EnumeratedArray.min_value = source->EnumeratedArray.min_value;
					poly->EnumeratedArray.max_value = source->EnumeratedArray.max_value;
					poly->EnumeratedArray.count     = source->EnumeratedArray.count;
					poly->EnumeratedArray.op        = source->EnumeratedArray.op;

					e->kind = Entity_TypeName;
					e->TypeName.is_type_alias = true;
					e->type = index;

					if (poly->EnumeratedArray.count == source->EnumeratedArray.count) {
						return is_polymorphic_type_assignable(c, poly->EnumeratedArray.elem, source->EnumeratedArray.elem, true, modify_type);
					}
				}
			}
		}
		return false;
	case Type_EnumeratedArray:
		if (source->kind == Type_EnumeratedArray) {
			if (poly->EnumeratedArray.op != source->EnumeratedArray.op) {
				return false;
			}
			if (poly->EnumeratedArray.op) {
				if (poly->EnumeratedArray.count != source->EnumeratedArray.count) {
					return false;
				}
				if (compare_exact_values(Token_NotEq, *poly->EnumeratedArray.min_value, *source->EnumeratedArray.min_value)) {
					return false;
				}
				if (compare_exact_values(Token_NotEq, *poly->EnumeratedArray.max_value, *source->EnumeratedArray.max_value)) {
					return false;
				}
				return is_polymorphic_type_assignable(c, poly->EnumeratedArray.index, source->EnumeratedArray.index, true, modify_type);
			}
			bool index = is_polymorphic_type_assignable(c, poly->EnumeratedArray.index, source->EnumeratedArray.index, true, modify_type);
			bool elem  = is_polymorphic_type_assignable(c, poly->EnumeratedArray.elem, source->EnumeratedArray.elem, true, modify_type);
			return index || elem;
		}
		return false;

	case Type_DynamicArray:
		if (source->kind == Type_DynamicArray) {
			return is_polymorphic_type_assignable(c, poly->DynamicArray.elem, source->DynamicArray.elem, true, modify_type);
		}
		return false;
	case Type_Slice:
		if (source->kind == Type_Slice) {
			return is_polymorphic_type_assignable(c, poly->Slice.elem, source->Slice.elem, true, modify_type);
		}
		return false;

	case Type_Enum:
		return false;

	case Type_BitSet:
		if (source->kind == Type_BitSet) {
			if (!is_polymorphic_type_assignable(c, poly->BitSet.elem, source->BitSet.elem, true, modify_type)) {
				return false;
			}
			if (poly->BitSet.underlying == nullptr) {
				if (modify_type) {
					poly->BitSet.underlying = source->BitSet.underlying;
				}
			} else if (!is_polymorphic_type_assignable(c, poly->BitSet.underlying, source->BitSet.underlying, true, modify_type)) {
				return false;
			}
			return true;
		}
		return false;

	case Type_Union:
		if (source->kind == Type_Union) {
			TypeUnion *x = &poly->Union;
			TypeUnion *y = &source->Union;
			if (x->variants.count != y->variants.count) {
				return false;
			}
			for_array(i, x->variants) {
				Type *a = x->variants[i];
				Type *b = y->variants[i];
				bool ok = is_polymorphic_type_assignable(c, a, b, false, modify_type);
				if (!ok) return false;
			}
			return true;
		}
		return false;

	case Type_Struct:
		if (source->kind == Type_Struct) {
			if (poly->Struct.soa_kind == source->Struct.soa_kind &&
			    poly->Struct.soa_kind != StructSoa_None) {
				bool ok = is_polymorphic_type_assignable(c, poly->Struct.soa_elem, source->Struct.soa_elem, true, modify_type);
				if (ok) switch (source->Struct.soa_kind) {
				case StructSoa_Fixed:
				default:
					GB_PANIC("Unhandled SOA Kind");
					break;

				case StructSoa_Slice:
					if (modify_type) {
						Type *type = make_soa_struct_slice(c, nullptr, poly->Struct.node, poly->Struct.soa_elem);
						gb_memmove(poly, type, gb_size_of(*type));
					}
					break;
				case StructSoa_Dynamic:
					if (modify_type) {
						Type *type = make_soa_struct_dynamic_array(c, nullptr, poly->Struct.node, poly->Struct.soa_elem);
						gb_memmove(poly, type, gb_size_of(*type));
					}
					break;
				}
				return ok;

			}
			// return check_is_assignable_to(c, &o, poly);
		}
		return false;
	case Type_Tuple:
		GB_PANIC("This should never happen");
		return false;
	case Type_Proc:
		if (source->kind == Type_Proc) {
			// return check_is_assignable_to(c, &o, poly);
			// TODO(bill): Polymorphic type assignment
			#if 1
			TypeProc *x = &poly->Proc;
			TypeProc *y = &source->Proc;
			if (x->calling_convention != y->calling_convention) {
				return false;
			}
			if (x->c_vararg != y->c_vararg) {
				return false;
			}
			if (x->variadic != y->variadic) {
				return false;
			}
			if (x->param_count != y->param_count) {
				return false;
			}
			if (x->result_count != y->result_count) {
				return false;
			}

			for (isize i = 0; i < x->param_count; i++) {
				Entity *a = x->params->Tuple.variables[i];
				Entity *b = y->params->Tuple.variables[i];
				bool ok = is_polymorphic_type_assignable(c, a->type, b->type, false, modify_type);
				if (!ok) return false;
			}
			for (isize i = 0; i < x->result_count; i++) {
				Entity *a = x->results->Tuple.variables[i];
				Entity *b = y->results->Tuple.variables[i];
				bool ok = is_polymorphic_type_assignable(c, a->type, b->type, false, modify_type);
				if (!ok) return false;
			}

			return true;
			#endif
		}
		return false;
	case Type_Map:
		if (source->kind == Type_Map) {
			bool key   = is_polymorphic_type_assignable(c, poly->Map.key, source->Map.key, true, modify_type);
			bool value = is_polymorphic_type_assignable(c, poly->Map.value, source->Map.value, true, modify_type);
			return key || value;
		}
		return false;
		
	case Type_Matrix:
		if (source->kind == Type_Matrix) {
			if (poly->Matrix.generic_row_count != nullptr) {
				poly->Matrix.stride_in_bytes = 0;
				if (!polymorphic_assign_index(&poly->Matrix.generic_row_count, &poly->Matrix.row_count, source->Matrix.row_count)) {
					return false;
				}
			}
			if (poly->Matrix.generic_column_count != nullptr) {
				poly->Matrix.stride_in_bytes = 0;
				if (!polymorphic_assign_index(&poly->Matrix.generic_column_count, &poly->Matrix.column_count, source->Matrix.column_count)) {
					return false;
				}
			}
			if (poly->Matrix.row_count == source->Matrix.row_count &&
			    poly->Matrix.column_count == source->Matrix.column_count) {
				return is_polymorphic_type_assignable(c, poly->Matrix.elem, source->Matrix.elem, true, modify_type);
			}
		} 
		return false;
	}
	return false;
}

bool check_cycle(CheckerContext *c, Entity *curr, bool report) {
	if (curr->state != EntityState_InProgress) {
		return false;
	}
	for_array(i, *c->type_path) {
		Entity *prev = (*c->type_path)[i];
		if (prev == curr) {
			if (report) {
				error(curr->token, "Illegal declaration cycle of `%.*s`", LIT(curr->token.string));
				for (isize j = i; j < c->type_path->count; j++) {
					Entity *curr = (*c->type_path)[j];
					error(curr->token, "\t%.*s refers to", LIT(curr->token.string));
				}
				error(curr->token, "\t%.*s", LIT(curr->token.string));
				curr->type = t_invalid;
			}
			return true;
		}
	}
	return false;
}

Entity *check_ident(CheckerContext *c, Operand *o, Ast *n, Type *named_type, Type *type_hint, bool allow_import_name) {
	GB_ASSERT(n->kind == Ast_Ident);
	o->mode = Addressing_Invalid;
	o->expr = n;
	String name = n->Ident.token.string;

	Entity *e = scope_lookup(c->scope, name);
	if (e == nullptr) {
		if (is_blank_ident(name)) {
			error(n, "'_' cannot be used as a value");
		} else {
			error(n, "Undeclared name: %.*s", LIT(name));
		}
		o->type = t_invalid;
		o->mode = Addressing_Invalid;
		if (named_type != nullptr) {
			set_base_type(named_type, t_invalid);
		}
		return nullptr;
	}

	GB_ASSERT((e->flags & EntityFlag_Overridden) == 0);

	if (e->parent_proc_decl != nullptr &&
	    e->parent_proc_decl != c->curr_proc_decl) {
		if (e->kind == Entity_Variable) {
			if ((e->flags & EntityFlag_Static) == 0) {
				error(n, "Nested procedures do not capture its parent's variables: %.*s", LIT(name));
				return nullptr;
			}
		} else if (e->kind == Entity_Label) {
			error(n, "Nested procedures do not capture its parent's labels: %.*s", LIT(name));
			return nullptr;
		}
	}

	if (e->kind == Entity_ProcGroup) {
		auto *pge = &e->ProcGroup;

		DeclInfo *d = decl_info_of_entity(e);
		check_entity_decl(c, e, d, nullptr);


		Array<Entity *> procs = pge->entities;
		bool skip = false;

		if (type_hint != nullptr) {
			// NOTE(bill): These should be done
			for_array(i, procs) {
				Type *t = base_type(procs[i]->type);
				if (t == t_invalid) {
					continue;
				}
				Operand x = {};
				x.mode = Addressing_Value;
				x.type = t;
				if (check_is_assignable_to(c, &x, type_hint)) {
					e = procs[i];
					add_entity_use(c, n, e);
					skip = true;
					break;
				}
			}
		}

		if (!skip) {
			o->mode       = Addressing_ProcGroup;
			o->type       = t_invalid;
			o->proc_group = e;
			return nullptr;
		}
	}

	add_entity_use(c, n, e);
	if (e->state == EntityState_Unresolved) {
		check_entity_decl(c, e, nullptr, named_type);
	}
	if (e->type == nullptr) {
		// TODO(bill): Which is correct? return or compiler_error?
		// compiler_error("How did this happen? type: %s; identifier: %.*s\n", type_to_string(e->type), LIT(name));
		return nullptr;
	}

	e->flags |= EntityFlag_Used;

	Type *type = e->type;

	o->type = type;

	switch (e->kind) {
	case Entity_Constant:
		if (type == t_invalid) {
			o->type = t_invalid;
			return e;
		}
		o->value = e->Constant.value;
		if (o->value.kind == ExactValue_Invalid) {
			return e;
		}
		if (o->value.kind == ExactValue_Procedure) {
			Entity *proc = strip_entity_wrapping(o->value.value_procedure);
			if (proc != nullptr) {
				o->mode = Addressing_Value;
				o->type = proc->type;
				return proc;
			}
		}
		o->mode = Addressing_Constant;
		break;

	case Entity_Variable:
		e->flags |= EntityFlag_Used;
		if (type == t_invalid) {
			o->type = t_invalid;
			return e;
		}
		o->mode = Addressing_Variable;
		if (e->flags & EntityFlag_Value) {
			o->mode = Addressing_Value;
		}
		break;

	case Entity_Procedure:
		o->mode = Addressing_Value;
		o->value = exact_value_procedure(n);
		break;

	case Entity_Builtin:
		o->builtin_id = cast(BuiltinProcId)e->Builtin.id;
		o->mode = Addressing_Builtin;
		break;

	case Entity_TypeName:
		o->mode = Addressing_Type;
		if (check_cycle(c, e, true)) {
			o->type = t_invalid;
		}
		if (o->type != nullptr && type->kind == Type_Named && o->type->Named.type_name->TypeName.is_type_alias) {
			o->type = base_type(o->type);
		}

		break;

	case Entity_ImportName:
		if (!allow_import_name) {
			error(n, "Use of import '%.*s' not in selector", LIT(name));
		}
		return e;
	case Entity_LibraryName:
		error(n, "Use of library '%.*s' not in foreign block", LIT(name));
		return e;

	case Entity_Label:
		o->mode = Addressing_NoValue;
		break;

	case Entity_Nil:
		o->mode = Addressing_Value;
		break;

	default:
		compiler_error("Unknown EntityKind %.*s", LIT(entity_strings[e->kind]));
		break;
	}

	return e;
}


bool check_unary_op(CheckerContext *c, Operand *o, Token op) {
	if (o->type == nullptr) {
		gbString str = expr_to_string(o->expr);
		error(o->expr, "Expression has no value '%s'", str);
		gb_string_free(str);
		return false;
	}
	// TODO(bill): Handle errors correctly
	Type *type = base_type(core_array_type(o->type));
	gbString str = nullptr;
	switch (op.kind) {
	case Token_Add:
	case Token_Sub:
		if (!is_type_numeric(type)) {
			str = expr_to_string(o->expr);
			error(op, "Operator '%.*s' is not allowed with '%s'", LIT(op.string), str);
			gb_string_free(str);
		}
		break;

	case Token_Xor:
		if (!is_type_integer(type) && !is_type_boolean(type) && !is_type_bit_set(type)) {
			error(op, "Operator '%.*s' is only allowed with integers, booleans, or bit sets", LIT(op.string));
		}
		break;

	case Token_Not:
		if (!is_type_boolean(type)) {
			str = expr_to_string(o->expr);
			error(op, "Operator '%.*s' is only allowed on boolean expression", LIT(op.string));
			gb_string_free(str);
		}
		break;

	default:
		error(op, "Unknown operator '%.*s'", LIT(op.string));
		return false;
	}

	return true;
}

bool check_binary_op(CheckerContext *c, Operand *o, Token op) {
	Type *main_type = o->type;
	// TODO(bill): Handle errors correctly
	Type *type = base_type(core_array_type(main_type));
	Type *ct = core_type(type);
	switch (op.kind) {
	case Token_Sub:
	case Token_SubEq:
		if (is_type_bit_set(type)) {
			return true;
		} else if (!is_type_numeric(type)) {
			error(op, "Operator '%.*s' is only allowed with numeric expressions", LIT(op.string));
			return false;
		}
		break;

	case Token_Quo:
	case Token_QuoEq:
		if (is_type_matrix(main_type)) {
			error(op, "Operator '%.*s' is only allowed with matrix types", LIT(op.string));
			return false;
		}
		/*fallthrough*/
	case Token_Mul:
	case Token_MulEq:
	case Token_AddEq:
		if (is_type_bit_set(type)) {
			return true;
		} else if (!is_type_numeric(type)) {
			error(op, "Operator '%.*s' is only allowed with numeric expressions", LIT(op.string));
			return false;
		}
		break;

	case Token_Add:
		if (is_type_string(type)) {
			if (o->mode == Addressing_Constant) {
				return true;
			}
			error(op, "String concatenation is only allowed with constant strings");
			return false;
		} else if (is_type_bit_set(type)) {
			return true;
		} else if (!is_type_numeric(type)) {
			error(op, "Operator '%.*s' is only allowed with numeric expressions", LIT(op.string));
			return false;
		}
		break;

	case Token_And:
	case Token_Or:
	case Token_AndEq:
	case Token_OrEq:
	case Token_Xor:
	case Token_XorEq:
		if (!is_type_integer(ct) && !is_type_boolean(ct) && !is_type_bit_set(ct)) {
			error(op, "Operator '%.*s' is only allowed with integers, booleans, or bit sets", LIT(op.string));
			return false;
		}
		break;

	case Token_Mod:
	case Token_ModMod:
	case Token_ModEq:
	case Token_ModModEq:
		if (is_type_matrix(main_type)) {
			error(op, "Operator '%.*s' is only allowed with matrix types", LIT(op.string));
			return false;
		}
		if (!is_type_integer(type)) {
			error(op, "Operator '%.*s' is only allowed with integers", LIT(op.string));
			return false;
		}
		if (is_type_simd_vector(o->type)) {
			switch (op.kind) {
			case Token_ModMod:
			case Token_ModModEq:
				error(op, "Operator '%.*s' is only allowed with integers", LIT(op.string));
				return false;
			}
		}
		break;

	case Token_AndNot:
	case Token_AndNotEq:
		if (!is_type_integer(ct) && !is_type_bit_set(ct)) {
			error(op, "Operator '%.*s' is only allowed with integers and bit sets", LIT(op.string));
			return false;
		}
		if (is_type_simd_vector(o->type)) {
			switch (op.kind) {
			case Token_AndNot:
			case Token_AndNotEq:
				error(op, "Operator '%.*s' is only allowed with integers", LIT(op.string));
				return false;
			}
		}
		break;

	case Token_CmpAnd:
	case Token_CmpOr:
	case Token_CmpAndEq:
	case Token_CmpOrEq:
		if (!is_type_boolean(type)) {
			error(op, "Operator '%.*s' is only allowed with boolean expressions", LIT(op.string));
			return false;
		}
		break;

	default:
		error(op, "Unknown operator '%.*s'", LIT(op.string));
		return false;
	}

	return true;

}


bool check_representable_as_constant(CheckerContext *c, ExactValue in_value, Type *type, ExactValue *out_value) {
	if (in_value.kind == ExactValue_Invalid) {
		// NOTE(bill): There's already been an error
		return true;
	}

	type = core_type(type);
	if (type == t_invalid) {
		return false;
	} else if (is_type_boolean(type)) {
		return in_value.kind == ExactValue_Bool;
	} else if (is_type_string(type)) {
		return in_value.kind == ExactValue_String;
	} else if (is_type_integer(type) || is_type_rune(type)) {
		if (in_value.kind == ExactValue_Bool) {
			return false;
		}
		ExactValue v = exact_value_to_integer(in_value);
		if (v.kind != ExactValue_Integer) {
			return false;
		}
		if (out_value) *out_value = v;


		if (is_type_untyped(type)) {
			return true;
		}

		BigInt i = v.value_integer;

		i64 bit_size = type_size_of(type);
		BigInt umax = {};
		BigInt imin = {};
		BigInt imax = {};

		if (bit_size < 16) {
			big_int_from_u64(&umax, unsigned_integer_maxs[bit_size]);
			big_int_from_i64(&imin, signed_integer_mins[bit_size]);
			big_int_from_i64(&imax, signed_integer_maxs[bit_size]);
		} else {
			big_int_from_u64(&umax, 1);
			big_int_from_i64(&imin, 1);
			big_int_from_i64(&imax, 1);

			BigInt bi128 = {};
			BigInt bi127 = {};
			big_int_from_i64(&bi128, 128);
			big_int_from_i64(&bi127, 127);

			big_int_shl_eq(&umax, &bi128);
			mp_decr(&umax);

			big_int_shl_eq(&imin, &bi127);
			big_int_neg(&imin, &imin);

			big_int_shl_eq(&imax, &bi127);
			mp_decr(&imax);
		}

		switch (type->Basic.kind) {
		case Basic_rune:
		case Basic_i8:
		case Basic_i16:
		case Basic_i32:
		case Basic_i64:
		case Basic_i128:
		case Basic_int:

		case Basic_i16le:
		case Basic_i32le:
		case Basic_i64le:
		case Basic_i128le:
		case Basic_i16be:
		case Basic_i32be:
		case Basic_i64be:
		case Basic_i128be:
			{
				// return imin <= i && i <= imax;
				int a = big_int_cmp(&imin, &i);
				int b = big_int_cmp(&i, &imax);
				return (a <= 0) && (b <= 0);
			}

		case Basic_u8:
		case Basic_u16:
		case Basic_u32:
		case Basic_u64:
		case Basic_u128:
		case Basic_uint:
		case Basic_uintptr:

		case Basic_u16le:
		case Basic_u32le:
		case Basic_u64le:
		case Basic_u128le:
		case Basic_u16be:
		case Basic_u32be:
		case Basic_u64be:
		case Basic_u128be:
			{
				// return 0ull <= i && i <= umax;
				int b = big_int_cmp(&i, &umax);
				return !i.sign && (b <= 0);
			}

		case Basic_UntypedInteger:
			return true;

		default: GB_PANIC("Compiler error: Unknown integer type!"); break;
		}
	} else if (is_type_float(type)) {
		ExactValue v = exact_value_to_float(in_value);
		if (v.kind != ExactValue_Float) {
			return false;
		}
		if (out_value) *out_value = v;

		switch (type->Basic.kind) {
		case Basic_f16:
		case Basic_f32:
		case Basic_f64:
			return true;

		case Basic_f16le:
		case Basic_f16be:
		case Basic_f32le:
		case Basic_f32be:
		case Basic_f64le:
		case Basic_f64be:
			return true;

		case Basic_UntypedFloat:
			return true;

		default: GB_PANIC("Compiler error: Unknown float type!"); break;
		}
	} else if (is_type_complex(type)) {
		ExactValue v = exact_value_to_complex(in_value);
		if (v.kind != ExactValue_Complex) {
			return false;
		}

		switch (type->Basic.kind) {
		case Basic_complex32:
		case Basic_complex64:
		case Basic_complex128: {
			ExactValue real = exact_value_real(v);
			ExactValue imag = exact_value_imag(v);
			if (real.kind != ExactValue_Invalid &&
			    imag.kind != ExactValue_Invalid) {
				if (out_value) *out_value = exact_value_complex(exact_value_to_f64(real), exact_value_to_f64(imag));
				return true;
			}
			break;
		}
		case Basic_UntypedComplex:
			return true;

		default: GB_PANIC("Compiler error: Unknown complex type!"); break;
		}

		return false;
	} else if (is_type_quaternion(type)) {
		ExactValue v = exact_value_to_quaternion(in_value);
		if (v.kind != ExactValue_Quaternion) {
			return false;
		}

		switch (type->Basic.kind) {
		case Basic_quaternion64:
		case Basic_quaternion128:
		case Basic_quaternion256: {
			ExactValue real = exact_value_real(v);
			ExactValue imag = exact_value_imag(v);
			ExactValue jmag = exact_value_jmag(v);
			ExactValue kmag = exact_value_kmag(v);
			if (real.kind != ExactValue_Invalid &&
			    imag.kind != ExactValue_Invalid) {
				if (out_value) *out_value = exact_value_quaternion(exact_value_to_f64(real), exact_value_to_f64(imag), exact_value_to_f64(jmag), exact_value_to_f64(kmag));
				return true;
			}
			break;
		}
		case Basic_UntypedComplex:
			if (out_value) *out_value = exact_value_to_quaternion(*out_value);
			return true;
		case Basic_UntypedQuaternion:
			return true;

		default: GB_PANIC("Compiler error: Unknown complex type!"); break;
		}

		return false;
	} else if (is_type_pointer(type)) {
		if (in_value.kind == ExactValue_Pointer) {
			return true;
		}
		if (in_value.kind == ExactValue_Integer) {
			return false;
			// return true;
		}
		if (in_value.kind == ExactValue_String) {
			return false;
		}
		if (out_value) *out_value = in_value;
	} else if (is_type_bit_set(type)) {
		if (in_value.kind == ExactValue_Integer) {
			return true;
		}
	}

	return false;
}


void check_assignment_error_suggestion(CheckerContext *c, Operand *o, Type *type) {
	gbString a = expr_to_string(o->expr);
	gbString b = type_to_string(type);
	defer(
		gb_string_free(b);
		gb_string_free(a);
	);

	Type *src = base_type(o->type);
	Type *dst = base_type(type);

	if (is_type_array(src) && is_type_slice(dst)) {
		Type *s = src->Array.elem;
		Type *d = dst->Slice.elem;
		if (are_types_identical(s, d)) {
			error_line("\tSuggestion: the array expression may be sliced with %s[:]\n", a);
		}
	} else if (are_types_identical(src, dst)) {
		error_line("\tSuggestion: the expression may be directly casted to type %s\n", b);
	} else if (are_types_identical(src, t_string) && is_type_u8_slice(dst)) {
		error_line("\tSuggestion: a string may be transmuted to %s\n", b);
	} else if (is_type_u8_slice(src) && are_types_identical(dst, t_string)) {
		error_line("\tSuggestion: the expression may be casted to %s\n", b);
	}
}

void check_cast_error_suggestion(CheckerContext *c, Operand *o, Type *type) {
	gbString a = expr_to_string(o->expr);
	gbString b = type_to_string(type);
	defer(
		gb_string_free(b);
		gb_string_free(a);
	);

	Type *src = base_type(o->type);
	Type *dst = base_type(type);

	if (is_type_array(src) && is_type_slice(dst)) {
		Type *s = src->Array.elem;
		Type *d = dst->Slice.elem;
		if (are_types_identical(s, d)) {
			error_line("\tSuggestion: the array expression may be sliced with %s[:]\n", a);
		}
	} else if (is_type_pointer(o->type) && is_type_integer(type)) {
		if (is_type_uintptr(type)) {
			error_line("\tSuggestion: a pointer may be directly casted to %s\n", b);
		} else {
			error_line("\tSuggestion: for a pointer to be casted to an integer, it must be converted to 'uintptr' first\n");
			i64 x = type_size_of(o->type);
			i64 y = type_size_of(type);
			if (x != y) {
				error_line("\tNote: the type of expression and the type of the cast have a different size in bytes, %lld vs %lld\n", x, y);
			}
		}
	} else if (is_type_integer(o->type) && is_type_pointer(type)) {
		if (is_type_uintptr(o->type)) {
			error_line("\tSuggestion: %a may be directly casted to %s\n", a, b);
		} else {
			error_line("\tSuggestion: for an integer to be casted to a pointer, it must be converted to 'uintptr' first\n");
		}
	} else if (are_types_identical(src, t_string) && is_type_u8_slice(dst)) {
		error_line("\tSuggestion: a string may be transmuted to %s\n", b);
	} else if (is_type_u8_slice(src) && are_types_identical(dst, t_string)) {
		error_line("\tSuggestion: the expression may be casted to %s\n", b);
	}
}


bool check_is_expressible(CheckerContext *ctx, Operand *o, Type *type) {
	GB_ASSERT(o->mode == Addressing_Constant);
	ExactValue out_value = o->value;
	if (is_type_constant_type(type) && check_representable_as_constant(ctx, o->value, type, &out_value)) {
		o->value = out_value;
		return true;
	} else {
		o->value = out_value;

		gbString a = expr_to_string(o->expr);
		gbString b = type_to_string(type);
		gbString c = type_to_string(o->type);
		defer(
			gb_string_free(c);
			gb_string_free(b);
			gb_string_free(a);
			o->mode = Addressing_Invalid;
		);

		if (is_type_numeric(o->type) && is_type_numeric(type)) {
			if (!is_type_integer(o->type) && is_type_integer(type)) {
				error(o->expr, "'%s' truncated to '%s'", a, b);
			} else {
				error(o->expr, "Cannot convert numeric value '%s' to '%s' from '%s", a, b, c);
				check_assignment_error_suggestion(ctx, o, type);
			}
		} else {
			error(o->expr, "Cannot convert '%s' to '%s' from '%s", a, b, c);
			check_assignment_error_suggestion(ctx, o, type);
		}
		return false;
	}
}

bool check_is_not_addressable(CheckerContext *c, Operand *o) {
	if (o->mode == Addressing_OptionalOk) {
		Ast *expr = unselector_expr(o->expr);
		if (expr->kind != Ast_TypeAssertion) {
			return true;
		}
		ast_node(ta, TypeAssertion, expr);
		TypeAndValue tv = ta->expr->tav;
		if (is_type_pointer(tv.type)) {
			return false;
		}
		if (is_type_union(tv.type) && tv.mode == Addressing_Variable) {
			return false;
		}
		if (is_type_any(tv.type)) {
			return false;
		}
		return true;
	}
	if (o->mode == Addressing_MapIndex) {
		return false;
	}

	Ast *expr = unparen_expr(o->expr);
	if (expr->kind == Ast_CompoundLit) {
		return false;
	}

	return o->mode != Addressing_Variable;
}

void check_unary_expr(CheckerContext *c, Operand *o, Token op, Ast *node) {
	switch (op.kind) {
	case Token_And: { // Pointer address
		if (check_is_not_addressable(c, o)) {
			if (ast_node_expect(node, Ast_UnaryExpr)) {
				ast_node(ue, UnaryExpr, node);
				gbString str = expr_to_string(ue->expr);
				defer (gb_string_free(str));

				Entity *e = entity_of_node(o->expr);
				if (e != nullptr && (e->flags & EntityFlag_Param) != 0) {
					error(op, "Cannot take the pointer address of '%s' which is a procedure parameter", str);
				} else {
					switch (o->mode) {
					case Addressing_SoaVariable:
						error(op, "Cannot take the pointer address of '%s' as it is an indirect index of an SOA struct", str);
						break;
					case Addressing_Constant:
						error(op, "Cannot take the pointer address of '%s' which is a constant", str);
						break;
					case Addressing_SwizzleValue:
					case Addressing_SwizzleVariable:
						error(op, "Cannot take the pointer address of '%s' which is a swizzle intermediate array value", str);
						break;
					default:
						{
							begin_error_block();
							defer (end_error_block());
							error(op, "Cannot take the pointer address of '%s'", str);
							if (e != nullptr && (e->flags & EntityFlag_ForValue) != 0) {
								error_line("\tSuggestion: Did you want to pass the iterable value to the for statement by pointer to get addressable semantics?\n");
							}
							if (e != nullptr && (e->flags & EntityFlag_SwitchValue) != 0) {
								error_line("\tSuggestion: Did you want to pass the value to the switch statement by pointer to get addressable semantics?\n");
							}
						}
						break;
					}
				}
			}
			o->mode = Addressing_Invalid;
			return;
		}

		o->type = alloc_type_pointer(o->type);

		switch (o->mode) {
		case Addressing_OptionalOk:
		case Addressing_MapIndex:
			o->mode = Addressing_OptionalOkPtr;
			break;
		default:
			o->mode = Addressing_Value;
			break;
		}

		return;
	}
	}

	if (!check_unary_op(c, o, op)) {
		o->mode = Addressing_Invalid;
		return;
	}

	if (o->mode == Addressing_Constant) {
		Type *type = base_type(o->type);
		if (!is_type_constant_type(o->type)) {
			gbString xt = type_to_string(o->type);
			gbString err_str = expr_to_string(node);
			error(op, "Invalid type, '%s', for constant unary expression '%s'", xt, err_str);
			gb_string_free(err_str);
			gb_string_free(xt);
			o->mode = Addressing_Invalid;
			return;
		}



		if (op.kind == Token_Xor && is_type_untyped(type)) {
			gbString err_str = expr_to_string(node);
			error(op, "Bitwise not cannot be applied to untyped constants '%s'", err_str);
			gb_string_free(err_str);
			o->mode = Addressing_Invalid;
			return;
		}
		if (op.kind == Token_Sub && is_type_unsigned(type)) {
			gbString err_str = expr_to_string(node);
			error(op, "A unsigned constant cannot be negated '%s'", err_str);
			gb_string_free(err_str);
			o->mode = Addressing_Invalid;
			return;
		}

		i32 precision = 0;
		if (is_type_typed(type)) {
			precision = cast(i32)(8 * type_size_of(type));
		}

		bool is_unsigned = is_type_unsigned(type);
		if (is_type_rune(type)) {
			GB_ASSERT(!is_unsigned);
		}

		o->value = exact_unary_operator_value(op.kind, o->value, precision, is_unsigned);

		if (is_type_typed(type)) {
			if (node != nullptr) {
				o->expr = node;
			}
			check_is_expressible(c, o, type);
		}
		return;
	}

	o->mode = Addressing_Value;
}

void add_comparison_procedures_for_fields(CheckerContext *c, Type *t) {
	if (t == nullptr) {
		return;
	}
	t = base_type(t);
	if (!is_type_comparable(t)) {
		return;
	}
	switch (t->kind) {
	case Type_Basic:
		switch (t->Basic.kind) {
		case Basic_complex32:
			add_package_dependency(c, "runtime", "complex32_eq");
			add_package_dependency(c, "runtime", "complex32_ne");
			break;
		case Basic_complex64:
			add_package_dependency(c, "runtime", "complex64_eq");
			add_package_dependency(c, "runtime", "complex64_ne");
			break;
		case Basic_complex128:
			add_package_dependency(c, "runtime", "complex128_eq");
			add_package_dependency(c, "runtime", "complex128_ne");
			break;
		case Basic_quaternion64:
			add_package_dependency(c, "runtime", "quaternion64_eq");
			add_package_dependency(c, "runtime", "quaternion64_ne");
			break;
		case Basic_quaternion128:
			add_package_dependency(c, "runtime", "quaternion128_eq");
			add_package_dependency(c, "runtime", "quaternion128_ne");
			break;
		case Basic_quaternion256:
			add_package_dependency(c, "runtime", "quaternion256_eq");
			add_package_dependency(c, "runtime", "quaternion256_ne");
			break;
		case Basic_cstring:
			add_package_dependency(c, "runtime", "cstring_to_string");
			/*fallthrough*/
		case Basic_string:
			add_package_dependency(c, "runtime", "string_eq");
			add_package_dependency(c, "runtime", "string_ne");
			break;
		}
		break;
	case Type_Struct:
		for (Entity *field : t->Struct.fields) {
			add_comparison_procedures_for_fields(c, field->type);
		}
		break;
	}
}


void check_comparison(CheckerContext *c, Operand *x, Operand *y, TokenKind op) {
	if (x->mode == Addressing_Type && y->mode == Addressing_Type) {
		bool comp = are_types_identical(x->type, y->type);
		switch (op) {
		case Token_CmpEq: /* comp = comp; */ break;
		case Token_NotEq: comp = !comp; break;
		}
		x->mode  = Addressing_Constant;
		x->type  = t_untyped_bool;
		x->value = exact_value_bool(comp);
		return;
	}

	if (x->mode == Addressing_Type && is_type_typeid(y->type)) {
		add_type_info_type(c, x->type);
		add_type_info_type(c, y->type);
		add_type_and_value(c->info, x->expr, Addressing_Value, y->type, exact_value_typeid(x->type));

		x->mode = Addressing_Value;
		x->type = t_untyped_bool;
		return;
	} else if (is_type_typeid(x->type) && y->mode == Addressing_Type) {
		add_type_info_type(c, x->type);
		add_type_info_type(c, y->type);
		add_type_and_value(c->info, y->expr, Addressing_Value, x->type, exact_value_typeid(y->type));

		x->mode = Addressing_Value;
		x->type = t_untyped_bool;
		return;
	}


	gbString err_str = nullptr;

	if (check_is_assignable_to(c, x, y->type) ||
	    check_is_assignable_to(c, y, x->type)) {
		Type *err_type = x->type;
		bool defined = false;
		switch (op) {
		case Token_CmpEq:
		case Token_NotEq:
			defined = (is_type_comparable(x->type) && is_type_comparable(y->type)) ||
			          (is_operand_nil(*x) && type_has_nil(y->type)) ||
			          (is_operand_nil(*y) && type_has_nil(x->type));
			break;
		case Token_Lt:
		case Token_Gt:
		case Token_LtEq:
		case Token_GtEq:
			if (are_types_identical(x->type, y->type) && is_type_bit_set(x->type)) {
				defined = true;
			} else {
				defined = is_type_ordered(x->type) && is_type_ordered(y->type);
			}
			break;
		}

		if (!defined) {
			if (x->type == err_type && is_operand_nil(*x)) {
				err_type = y->type;
			}
			gbString type_string = type_to_string(err_type);
			defer (gb_string_free(type_string));
			err_str = gb_string_make(temporary_allocator(),
				gb_bprintf("operator '%.*s' not defined for type '%s'", LIT(token_strings[op]), type_string));
		} else {
			Type *comparison_type = x->type;
			if (x->type == err_type && is_operand_nil(*x)) {
				comparison_type = y->type;
			}

			add_comparison_procedures_for_fields(c, comparison_type);
		}
	} else {
		gbString xt, yt;
		if (x->mode == Addressing_ProcGroup) {
			xt = gb_string_make(heap_allocator(), "procedure group");
		} else {
			xt = type_to_string(x->type);
		}
		if (y->mode == Addressing_ProcGroup) {
			yt = gb_string_make(heap_allocator(), "procedure group");
		} else {
			yt = type_to_string(y->type);
		}
		err_str = gb_string_make(temporary_allocator(), gb_bprintf("mismatched types '%s' and '%s'", xt, yt));
		gb_string_free(yt);
		gb_string_free(xt);
	}

	if (err_str != nullptr) {
		error(x->expr, "Cannot compare expression, %s", err_str);
		x->type = t_untyped_bool;
	} else {
		if (x->mode == Addressing_Constant &&
		    y->mode == Addressing_Constant) {
			if (is_type_constant_type(x->type)) {
				if (is_type_bit_set(x->type)) {
					switch (op) {
					case Token_CmpEq:
					case Token_NotEq:
						x->value = exact_value_bool(compare_exact_values(op, x->value, y->value));
						break;
					case Token_Lt:
					case Token_LtEq:
						{
							ExactValue lhs = x->value;
							ExactValue rhs = y->value;
							ExactValue res = exact_binary_operator_value(Token_And, lhs, rhs);
							res = exact_value_bool(compare_exact_values(op, res, lhs));
							if (op == Token_Lt) {
								res = exact_binary_operator_value(Token_And, res, exact_value_bool(compare_exact_values(op, lhs, rhs)));
							}
							x->value = res;
							break;
						}
					case Token_Gt:
					case Token_GtEq:
						{
							ExactValue lhs = x->value;
							ExactValue rhs = y->value;
							ExactValue res = exact_binary_operator_value(Token_And, lhs, rhs);
							res = exact_value_bool(compare_exact_values(op, res, rhs));
							if (op == Token_Gt) {
								res = exact_binary_operator_value(Token_And, res, exact_value_bool(compare_exact_values(op, lhs, rhs)));
							}
							x->value = res;
							break;
						}
					}
				} else {
					x->value = exact_value_bool(compare_exact_values(op, x->value, y->value));
				}
			} else {
				x->mode = Addressing_Value;
			}
		} else {
			x->mode = Addressing_Value;

			update_untyped_expr_type(c, x->expr, default_type(x->type), true);
			update_untyped_expr_type(c, y->expr, default_type(y->type), true);

			i64 size = 0;
			if (!is_type_untyped(x->type)) size = gb_max(size, type_size_of(x->type));
			if (!is_type_untyped(y->type)) size = gb_max(size, type_size_of(y->type));

			if (is_type_string(x->type) || is_type_string(y->type)) {
				switch (op) {
				case Token_CmpEq: add_package_dependency(c, "runtime", "string_eq"); break;
				case Token_NotEq: add_package_dependency(c, "runtime", "string_ne"); break;
				case Token_Lt:    add_package_dependency(c, "runtime", "string_lt"); break;
				case Token_Gt:    add_package_dependency(c, "runtime", "string_gt"); break;
				case Token_LtEq:  add_package_dependency(c, "runtime", "string_le"); break;
				case Token_GtEq:  add_package_dependency(c, "runtime", "string_gt"); break;
				}
			} else if (is_type_complex(x->type) || is_type_complex(y->type)) {
				switch (op) {
				case Token_CmpEq:
					switch (8*size) {
					case 64:  add_package_dependency(c, "runtime", "complex64_eq");  break;
					case 128: add_package_dependency(c, "runtime", "complex128_eq"); break;
					}
					break;
				case Token_NotEq:
					switch (8*size) {
					case 64:  add_package_dependency(c, "runtime", "complex64_ne");  break;
					case 128: add_package_dependency(c, "runtime", "complex128_ne"); break;
					}
					break;
				}
			} else if (is_type_quaternion(x->type) || is_type_quaternion(y->type)) {
				switch (op) {
				case Token_CmpEq:
					switch (8*size) {
					case 128: add_package_dependency(c, "runtime", "quaternion128_eq");  break;
					case 256: add_package_dependency(c, "runtime", "quaternion256_eq"); break;
					}
					break;
				case Token_NotEq:
					switch (8*size) {
					case 128: add_package_dependency(c, "runtime", "quaternion128_ne");  break;
					case 256: add_package_dependency(c, "runtime", "quaternion256_ne"); break;
					}
					break;
				}
			}
		}

		x->type = t_untyped_bool;
	}

}

void check_shift(CheckerContext *c, Operand *x, Operand *y, Ast *node, Type *type_hint) {
	GB_ASSERT(node->kind == Ast_BinaryExpr);
	ast_node(be, BinaryExpr, node);

	ExactValue x_val = {};
	if (x->mode == Addressing_Constant) {
		x_val = exact_value_to_integer(x->value);
	}

	bool x_is_untyped = is_type_untyped(x->type);
	if (!(is_type_integer(x->type) || (x_is_untyped && x_val.kind == ExactValue_Integer))) {
		gbString err_str = expr_to_string(x->expr);
		error(node, "Shifted operand '%s' must be an integer", err_str);
		gb_string_free(err_str);
		x->mode = Addressing_Invalid;
		return;
	}

	if (is_type_unsigned(y->type)) {

	} else if (is_type_untyped(y->type)) {
		convert_to_typed(c, y, t_untyped_integer);
		if (y->mode == Addressing_Invalid) {
			x->mode = Addressing_Invalid;
			return;
		}
	} else {
		gbString err_str = expr_to_string(y->expr);
		error(node, "Shift amount '%s' must be an unsigned integer", err_str);
		gb_string_free(err_str);
		x->mode = Addressing_Invalid;
		return;
	}


	if (x->mode == Addressing_Constant) {
		if (y->mode == Addressing_Constant) {
			ExactValue y_val = exact_value_to_integer(y->value);
			if (y_val.kind != ExactValue_Integer) {
				gbString err_str = expr_to_string(y->expr);
				error(node, "Shift amount '%s' must be an unsigned integer", err_str);
				gb_string_free(err_str);
				x->mode = Addressing_Invalid;
				return;
			}

			BigInt max_shift = {};
			big_int_from_u64(&max_shift, MAX_BIG_INT_SHIFT);

			if (big_int_cmp(&y_val.value_integer, &max_shift) > 0) {
				gbString err_str = expr_to_string(y->expr);
				error(node, "Shift amount too large: '%s'", err_str);
				gb_string_free(err_str);
				x->mode = Addressing_Invalid;
				return;
			}

			if (!is_type_integer(x->type)) {
				// NOTE(bill): It could be an untyped float but still representable
				// as an integer
				x->type = t_untyped_integer;
			}

			x->value = exact_value_shift(be->op.kind, x_val, y_val);

			if (is_type_typed(x->type)) {
				check_is_expressible(c, x, base_type(x->type));
			}
			return;
		}

		TokenPos pos = ast_token(x->expr).pos;
		if (x_is_untyped) {
			if (x->expr != nullptr) {
				x->expr->tav.is_lhs = true;
			}
			x->mode = Addressing_Value;
			if (type_hint && is_type_integer(type_hint)) {
				x->type = type_hint;
			}
			// x->value = x_val;
			return;
		}
	}

	if (y->mode == Addressing_Constant && big_int_is_neg(&y->value.value_integer)) {
		gbString err_str = expr_to_string(y->expr);
		error(node, "Shift amount cannot be negative: '%s'", err_str);
		gb_string_free(err_str);
	}

	if (!is_type_integer(x->type)) {
		gbString err_str = expr_to_string(y->expr);
		error(node, "Shift operand '%s' must be an integer", err_str);
		gb_string_free(err_str);
		x->mode = Addressing_Invalid;
		return;
	}

	if (is_type_untyped(y->type)) {
		convert_to_typed(c, y, t_uint);
	}

	x->mode = Addressing_Value;
}



bool check_is_castable_to(CheckerContext *c, Operand *operand, Type *y) {
	if (check_is_assignable_to(c, operand, y)) {
		return true;
	}

	bool is_constant = operand->mode == Addressing_Constant;

	Type *x = operand->type;
	Type *src = core_type(x);
	Type *dst = core_type(y);
	if (are_types_identical(src, dst)) {
		return true;
	}

	// if (is_type_tuple(src)) {
	// 	Ast *expr = unparen_expr(operand->expr);
	// 	if (expr && expr->kind == Ast_CallExpr) {
	// 		// NOTE(bill, 2021-04-19): Allow casting procedure calls with #optional_ok
	// 		ast_node(ce, CallExpr, expr);
	// 		Type *pt = base_type(type_of_expr(ce->proc));
	// 		if (pt->kind == Type_Proc && pt->Proc.optional_ok) {
	// 			if (pt->Proc.result_count > 0) {
	// 				Operand op = *operand;
	// 				op.type = pt->Proc.results->Tuple.variables[0]->type;
	// 				bool ok = check_is_castable_to(c, &op, y);
	// 				if (ok) {
	// 					ce->optional_ok_one = true;
	// 				}
	// 				return ok;
	// 			}
	// 		}
	// 	}
	// }

	if (is_constant && is_type_untyped(src) && is_type_string(src)) {
		if (is_type_u8_array(dst)) {
			String s = operand->value.value_string;
			return s.len == dst->Array.count;
		}
		if (is_type_rune_array(dst)) {
			String s = operand->value.value_string;
			return gb_utf8_strnlen(s.text, s.len) == dst->Array.count;
		}
	}


	if (dst->kind == Type_Array && src->kind == Type_Array) {
		if (are_types_identical(dst->Array.elem, src->Array.elem)) {
			return dst->Array.count == src->Array.count;
		}
	}

	if (dst->kind == Type_Slice && src->kind == Type_Slice) {
		return are_types_identical(dst->Slice.elem, src->Slice.elem);
	}

	// Cast between booleans and integers
	if (is_type_boolean(src) || is_type_integer(src)) {
		if (is_type_boolean(dst) || is_type_integer(dst)) {
			return true;
		}
	}

	// Cast between numbers
	if (is_type_integer(src) || is_type_float(src)) {
		if (is_type_integer(dst) || is_type_float(dst)) {
			return true;
		}
	}

	if (is_type_integer(src) && is_type_rune(dst)) {
		return true;
	}
	if (is_type_rune(src) && is_type_integer(dst)) {
		return true;
	}

	if (is_type_complex(src) && is_type_complex(dst)) {
		return true;
	}

	if (is_type_float(src) && is_type_complex(dst)) {
		return true;
	}
	if (is_type_float(src) && is_type_quaternion(dst)) {
		return true;
	}
	if (is_type_complex(src) && is_type_quaternion(dst)) {
		return true;
	}

	if (is_type_quaternion(src) && is_type_quaternion(dst)) {
		return true;
	}
	
	if (is_type_matrix(src) && is_type_matrix(dst)) {
		GB_ASSERT(src->kind == Type_Matrix);
		GB_ASSERT(dst->kind == Type_Matrix);
		Operand op = *operand;
		op.type = src->Matrix.elem;
		if (!check_is_castable_to(c, &op, dst->Matrix.elem)) {
			return false;
		}
		
		if (src->Matrix.row_count != src->Matrix.column_count) {
			i64 src_count = src->Matrix.row_count*src->Matrix.column_count;
			i64 dst_count = dst->Matrix.row_count*dst->Matrix.column_count;
			return src_count == dst_count;
		}
		
		return is_matrix_square(dst) && is_matrix_square(src);
	}


	// Cast between pointers
	if (is_type_pointer(src) && is_type_pointer(dst)) {
		return true;
	}
	if (is_type_multi_pointer(src) && is_type_multi_pointer(dst)) {
		return true;
	}
	if (is_type_multi_pointer(src) && is_type_pointer(dst)) {
		return true;
	}
	if (is_type_pointer(src) && is_type_multi_pointer(dst)) {
		return true;
	}

	// uintptr <-> pointer
	if (is_type_uintptr(src) && is_type_pointer(dst)) {
		return true;
	}
	if (is_type_pointer(src) && is_type_uintptr(dst)) {
		return true;
	}
	if (is_type_uintptr(src) && is_type_multi_pointer(dst)) {
		return true;
	}
	if (is_type_multi_pointer(src) && is_type_uintptr(dst)) {
		return true;
	}

	// []byte/[]u8 <-> string (not cstring)
	if (is_type_u8_slice(src) && (is_type_string(dst) && !is_type_cstring(dst))) {
		return true;
	}

	// cstring -> string
	if (are_types_identical(src, t_cstring) && are_types_identical(dst, t_string)) {
		if (operand->mode != Addressing_Constant) {
			add_package_dependency(c, "runtime", "cstring_to_string");
		}
		return true;
	}
	// cstring -> ^u8
	if (are_types_identical(src, t_cstring) && is_type_u8_ptr(dst)) {
		return !is_constant;
	}
	// cstring -> [^]u8
	if (are_types_identical(src, t_cstring) && is_type_u8_multi_ptr(dst)) {
		return !is_constant;
	}
	// cstring -> rawptr
	if (are_types_identical(src, t_cstring) && is_type_rawptr(dst)) {
		return !is_constant;
	}

	// ^u8 -> cstring
	if (is_type_u8_ptr(src) && are_types_identical(dst, t_cstring)) {
		return !is_constant;
	}
	// [^]u8 -> cstring
	if (is_type_u8_multi_ptr(src) && are_types_identical(dst, t_cstring)) {
		return !is_constant;
	}
	// rawptr -> cstring
	if (is_type_rawptr(src) && are_types_identical(dst, t_cstring)) {
		return !is_constant;
	}
	// proc <-> proc
	if (is_type_proc(src) && is_type_proc(dst)) {
		return true;
	}

	// proc -> rawptr
	if (is_type_proc(src) && is_type_rawptr(dst)) {
		return true;
	}
	// rawptr -> proc
	if (is_type_rawptr(src) && is_type_proc(dst)) {
		return true;
	}

	return false;
}

bool check_cast_internal(CheckerContext *c, Operand *x, Type *type) {
	bool is_const_expr = x->mode == Addressing_Constant;

	Type *bt = base_type(type);
	if (is_const_expr && is_type_constant_type(bt)) {
		if (core_type(bt)->kind == Type_Basic) {
			if (check_representable_as_constant(c, x->value, bt, &x->value)) {
				return true;
			} else if (check_is_castable_to(c, x, type)) {
				if (is_type_pointer(type)) {
					return true;
				}
			}
		} else if (check_is_castable_to(c, x, type)) {
			x->value = {};
			x->mode = Addressing_Value;
			return true;
		}
	} else if (check_is_castable_to(c, x, type)) {
		if (x->mode != Addressing_Constant) {
			x->mode = Addressing_Value;
		} else if (is_type_slice(type) && is_type_string(x->type)) {
			x->mode = Addressing_Value;
		} else if (is_type_union(type)) {
			x->mode = Addressing_Value;
		}
		if (x->mode == Addressing_Value) {
			x->value = {};
		}
		return true;
	}
	return false;

}

void check_cast(CheckerContext *c, Operand *x, Type *type) {
	if (!is_operand_value(*x)) {
		error(x->expr, "Only values can be casted");
		x->mode = Addressing_Invalid;
		return;
	}

	bool is_const_expr = x->mode == Addressing_Constant;
	bool can_convert = check_cast_internal(c, x, type);

	if (!can_convert) {
		gbString expr_str = expr_to_string(x->expr);
		gbString to_type  = type_to_string(type);
		gbString from_type = type_to_string(x->type);
		error(x->expr, "Cannot cast '%s' as '%s' from '%s'", expr_str, to_type, from_type);
		gb_string_free(from_type);
		gb_string_free(to_type);
		gb_string_free(expr_str);

		check_cast_error_suggestion(c, x, type);

		x->mode = Addressing_Invalid;
		return;
	}

	if (is_type_untyped(x->type)) {
		Type *final_type = type;
		if (is_const_expr && !is_type_constant_type(type)) {
			final_type = default_type(x->type);
		}
		update_untyped_expr_type(c, x->expr, final_type, true);
	}

	if (build_context.vet_extra) {
		if (are_types_identical(x->type, type)) {
			gbString str = type_to_string(type);
			warning(x->expr, "Unneeded cast to the same type '%s'", str);
			gb_string_free(str);
		}
	}

	x->type = type;
}

bool check_transmute(CheckerContext *c, Ast *node, Operand *o, Type *t) {
	if (!is_operand_value(*o)) {
		error(o->expr, "'transmute' can only be applied to values");
		o->mode = Addressing_Invalid;
		return false;
	}

	if (o->mode == Addressing_Constant) {
		gbString expr_str = expr_to_string(o->expr);
		error(o->expr, "Cannot transmute a constant expression: '%s'", expr_str);
		gb_string_free(expr_str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return false;
	}

	if (is_type_untyped(o->type)) {
		gbString expr_str = expr_to_string(o->expr);
		error(o->expr, "Cannot transmute untyped expression: '%s'", expr_str);
		gb_string_free(expr_str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return false;
	}

	Type *dst_bt = base_type(t);
	if (dst_bt == nullptr || dst_bt == t_invalid) {
		GB_ASSERT(global_error_collector.count != 0);

		o->mode = Addressing_Invalid;
		o->expr = node;
		return false;
	}

	Type *src_bt = base_type(o->type);
	if (src_bt == nullptr || src_bt == t_invalid) {
		// NOTE(bill): this should be an error
		GB_ASSERT(global_error_collector.count != 0);
		o->mode = Addressing_Value;
		o->expr = node;
		o->type = t;
		return true;
	}

	i64 srcz = type_size_of(o->type);
	i64 dstz = type_size_of(t);
	if (srcz != dstz) {
		gbString expr_str = expr_to_string(o->expr);
		gbString type_str = type_to_string(t);
		error(o->expr, "Cannot transmute '%s' to '%s', %lld vs %lld bytes", expr_str, type_str, srcz, dstz);
		gb_string_free(type_str);
		gb_string_free(expr_str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return false;
	}

	if (build_context.vet_extra) {
		if (are_types_identical(o->type, t)) {
			gbString str = type_to_string(t);
			warning(o->expr, "Unneeded transmute to the same type '%s'", str);
			gb_string_free(str);
		}
	}

	o->mode = Addressing_Value;
	o->type = t;
	return true;
}

bool check_binary_array_expr(CheckerContext *c, Token op, Operand *x, Operand *y) {
	if (is_type_array(x->type) && !is_type_array(y->type)) {
		if (check_is_assignable_to(c, y, x->type)) {
			if (check_binary_op(c, x, op)) {
				return true;
			}
		}
	}
	return false;
}

bool is_ise_expr(Ast *node) {
	node = unparen_expr(node);
	return node->kind == Ast_ImplicitSelectorExpr;
}

bool can_use_other_type_as_type_hint(bool use_lhs_as_type_hint, Type *other_type) {
	if (use_lhs_as_type_hint) { // RHS in this case
		return other_type != nullptr && other_type != t_invalid && is_type_typed(other_type);
	}
	return false;
}

Type *check_matrix_type_hint(Type *matrix, Type *type_hint) {
	Type *xt = base_type(matrix);
	if (type_hint != nullptr) {
		Type *th = base_type(type_hint);
		if (are_types_identical(th, xt)) {
			return type_hint;
		} else if (xt->kind == Type_Matrix && th->kind == Type_Array) {
			if (!are_types_identical(xt->Matrix.elem, th->Array.elem)) {
				// ignore
			} else if (xt->Matrix.row_count == 1 && xt->Matrix.column_count == th->Array.count) {
				return type_hint;
			} else if (xt->Matrix.column_count == 1 && xt->Matrix.row_count == th->Array.count) {
				return type_hint;
			}
		}
	}
	return matrix;
}


void check_binary_matrix(CheckerContext *c, Token const &op, Operand *x, Operand *y, Type *type_hint, bool use_lhs_as_type_hint) {
	if (!check_binary_op(c, x, op)) {
		x->mode = Addressing_Invalid;
		return;
	}
		
	Type *xt = base_type(x->type);
	Type *yt = base_type(y->type);
	
	if (is_type_matrix(x->type)) {
		GB_ASSERT(xt->kind == Type_Matrix);
		if (op.kind == Token_Mul) {
			if (yt->kind == Type_Matrix) {
				if (!are_types_identical(xt->Matrix.elem, yt->Matrix.elem)) {
					goto matrix_error;
				}
				
				if (xt->Matrix.column_count != yt->Matrix.row_count) {
					goto matrix_error;
				}
				x->mode = Addressing_Value;
				x->type = alloc_type_matrix(xt->Matrix.elem, xt->Matrix.row_count, yt->Matrix.column_count);
				goto matrix_success;
			} else if (yt->kind == Type_Array) {
				if (!are_types_identical(xt->Matrix.elem, yt->Array.elem)) {
					goto matrix_error;
				}
				
				if (xt->Matrix.column_count != yt->Array.count) {
					goto matrix_error;
				}
				
				// Treat arrays as column vectors
				x->mode = Addressing_Value;
				if (type_hint == nullptr && xt->Matrix.row_count == yt->Array.count) {
					x->type = y->type;
				} else {
					x->type = alloc_type_matrix(xt->Matrix.elem, xt->Matrix.row_count, 1);
				}
				goto matrix_success;
			}
		}
		if (!are_types_identical(xt, yt)) {
			goto matrix_error;
		}
		x->mode = Addressing_Value;
		x->type = xt;
		goto matrix_success;
	} else {
		GB_ASSERT(is_type_matrix(yt));
		GB_ASSERT(!is_type_matrix(xt));
		
		if (op.kind == Token_Mul) {
			// NOTE(bill): no need to handle the matrix case here since it should be handled above
			if (xt->kind == Type_Array) {
				if (!are_types_identical(yt->Matrix.elem, xt->Array.elem)) {
					goto matrix_error;
				}
				
				if (xt->Array.count != yt->Matrix.row_count) {
					goto matrix_error;
				}
				
				// Treat arrays as row vectors
				x->mode = Addressing_Value;
				if (type_hint == nullptr && yt->Matrix.column_count == xt->Array.count) {
					x->type = x->type;
				} else {
					x->type = alloc_type_matrix(yt->Matrix.elem, 1, yt->Matrix.column_count);
				}
				goto matrix_success;
			}
		}
		if (!are_types_identical(xt, yt)) {
			goto matrix_error;
		}
		x->mode = Addressing_Value;
		x->type = xt;
		goto matrix_success;
	}

matrix_success:
	x->type = check_matrix_type_hint(x->type, type_hint);
	
	return;
	
	
matrix_error:
	gbString xts = type_to_string(x->type);
	gbString yts = type_to_string(y->type);
	gbString expr_str = expr_to_string(x->expr);
	error(op, "Mismatched types in binary matrix expression '%s' for operator '%.*s' : '%s' vs '%s'", expr_str, LIT(op.string), xts, yts);
	gb_string_free(expr_str);
	gb_string_free(yts);
	gb_string_free(xts);
	x->type = t_invalid;
	x->mode = Addressing_Invalid;
	return;
	
}


void check_binary_expr(CheckerContext *c, Operand *x, Ast *node, Type *type_hint, bool use_lhs_as_type_hint=false) {
	GB_ASSERT(node->kind == Ast_BinaryExpr);
	Operand y_ = {}, *y = &y_;

	ast_node(be, BinaryExpr, node);

	defer({
		node->viral_state_flags |= be->left->viral_state_flags;
		node->viral_state_flags |= be->right->viral_state_flags;
	});

	Token op = be->op;
	switch (op.kind) {
	case Token_CmpEq:
	case Token_NotEq: {
		// NOTE(bill): Allow comparisons between types
		if (is_ise_expr(be->left)) {
			// Evalute the right before the left for an '.X' expression
			check_expr_or_type(c, y, be->right, type_hint);
			check_expr_or_type(c, x, be->left, y->type);
		} else {
			check_expr_or_type(c, x, be->left, type_hint);
			check_expr_or_type(c, y, be->right, x->type);
		}
		bool xt = x->mode == Addressing_Type;
		bool yt = y->mode == Addressing_Type;
		// If only one is a type, this is an error
		if (xt ^ yt) {
			GB_ASSERT(xt != yt);
			if (xt) {
				if (!is_type_typeid(y->type)) {
					error_operand_not_expression(x);
				}
			}
			if (yt) {
				if (!is_type_typeid(x->type)) {
					error_operand_not_expression(y);
				}
			}
		}

		break;
	}

	case Token_in:
	case Token_not_in:
	{
		// IMPORTANT NOTE(bill): This uses right-left evaluation in type checking only no in
		check_expr(c, y, be->right);
		Type *rhs_type = type_deref(y->type);

		if (is_type_bit_set(rhs_type)) {
			Type *elem = base_type(rhs_type)->BitSet.elem;
			check_expr_with_type_hint(c, x, be->left, elem);
		} else if (is_type_map(rhs_type)) {
			Type *key = base_type(rhs_type)->Map.key;
			check_expr_with_type_hint(c, x, be->left, key);
		} else {
			check_expr(c, x, be->left);
		}

		if (x->mode == Addressing_Invalid) {
			return;
		}
		if (y->mode == Addressing_Invalid) {
			x->mode = Addressing_Invalid;
			x->expr = y->expr;
			return;
		}

		if (is_type_map(rhs_type)) {
			Type *yt = base_type(rhs_type);
			if (op.kind == Token_in) {
				check_assignment(c, x, yt->Map.key, str_lit("map 'in'"));
			} else {
				check_assignment(c, x, yt->Map.key, str_lit("map 'not_in'"));
			}

			add_package_dependency(c, "runtime", "__dynamic_map_get");
		} else if (is_type_bit_set(rhs_type)) {
			Type *yt = base_type(rhs_type);

			if (op.kind == Token_in) {
				check_assignment(c, x, yt->BitSet.elem, str_lit("bit_set 'in'"));
			} else {
				check_assignment(c, x, yt->BitSet.elem, str_lit("bit_set 'not_in'"));
			}
			if (x->mode == Addressing_Constant && y->mode == Addressing_Constant) {
				ExactValue k = exact_value_to_integer(x->value);
				ExactValue v = exact_value_to_integer(y->value);
				GB_ASSERT(k.kind == ExactValue_Integer);
				GB_ASSERT(v.kind == ExactValue_Integer);
				i64 key = big_int_to_i64(&k.value_integer);
				i64 lower = yt->BitSet.lower;
				i64 upper = yt->BitSet.upper;

				if (lower <= key && key <= upper) {
					i64 bit = 1ll<<key;
					i64 bits = big_int_to_i64(&v.value_integer);

					x->mode = Addressing_Constant;
					x->type = t_untyped_bool;
					if (op.kind == Token_in) {
						x->value = exact_value_bool((bit & bits) != 0);
					} else {
						x->value = exact_value_bool((bit & bits) == 0);
					}
					x->expr = node;
					return;
				} else {
					error(x->expr, "key '%lld' out of range of bit set, %lld..%lld", key, lower, upper);
					x->mode = Addressing_Invalid;
				}
			}
		} else {
			gbString t = type_to_string(y->type);
			error(x->expr, "expected either a map or bitset for 'in', got %s", t);
			gb_string_free(t);
			x->expr = node;
			x->mode = Addressing_Invalid;
			return;
		}
		if (x->mode != Addressing_Invalid) {
			x->mode = Addressing_Value;
			x->type = t_untyped_bool;
		}
		x->expr = node;

		return;
	}

	default:
		if (is_ise_expr(be->left)) {
			// Evalute the right before the left for an '.X' expression
			check_expr_or_type(c, y, be->right, type_hint);

			if (can_use_other_type_as_type_hint(use_lhs_as_type_hint, y->type)) { // RHS in this case
				check_expr_or_type(c, x, be->left, y->type);
			} else {
				check_expr_with_type_hint(c, x, be->left, type_hint);
			}
		} else {
			check_expr_with_type_hint(c, x, be->left, type_hint);
			if (can_use_other_type_as_type_hint(use_lhs_as_type_hint, x->type)) {
				check_expr_with_type_hint(c, y, be->right, x->type);
			} else {
				check_expr_with_type_hint(c, y, be->right, type_hint);
			}
		}
		break;
	}
	if (x->mode == Addressing_Invalid) {
		return;
	}
	if (y->mode == Addressing_Invalid) {
		x->mode = Addressing_Invalid;
		x->expr = y->expr;
		return;
	}

	if (x->mode == Addressing_Builtin) {
		x->mode = Addressing_Invalid;
		error(x->expr, "built-in expression in binary expression");
		return;
	}
	if (y->mode == Addressing_Builtin) {
		x->mode = Addressing_Invalid;
		error(y->expr, "built-in expression in binary expression");
		return;
	}

	if (token_is_shift(op.kind)) {
		check_shift(c, x, y, node, type_hint);
		return;
	}

	convert_to_typed(c, x, y->type);
	if (x->mode == Addressing_Invalid) {
		return;
	}
	convert_to_typed(c, y, x->type);
	if (y->mode == Addressing_Invalid) {
		x->mode = Addressing_Invalid;
		return;
	}



	if (token_is_comparison(op.kind)) {
		check_comparison(c, x, y, op.kind);
		return;
	}

	if (check_binary_array_expr(c, op, x, y)) {
		x->mode = Addressing_Value;
		x->type = x->type;
		return;
	}
	if (check_binary_array_expr(c, op, y, x)) {
		x->mode = Addressing_Value;
		x->type = y->type;
		return;
	}
	if (is_type_matrix(x->type) || is_type_matrix(y->type)) {
		check_binary_matrix(c, op, x, y, type_hint, use_lhs_as_type_hint);
		x->expr = node;
		return;
	}

	
	if (!are_types_identical(x->type, y->type)) {
		if (x->type != t_invalid &&
		    y->type != t_invalid) {
			gbString xt = type_to_string(x->type);
			gbString yt = type_to_string(y->type);
			gbString expr_str = expr_to_string(x->expr);
			error(op, "Mismatched types in binary expression '%s' : '%s' vs '%s'", expr_str, xt, yt);
			gb_string_free(expr_str);
			gb_string_free(yt);
			gb_string_free(xt);
		}
		x->mode = Addressing_Invalid;
		return;
	}

	if (!check_binary_op(c, x, op)) {
		x->mode = Addressing_Invalid;
		return;
	}

	switch (op.kind) {
	case Token_Quo:
	case Token_Mod:
	case Token_ModMod:
	case Token_QuoEq:
	case Token_ModEq:
	case Token_ModModEq:
		if ((x->mode == Addressing_Constant || is_type_integer(x->type)) &&
		    y->mode == Addressing_Constant) {
			bool fail = false;
			switch (y->value.kind) {
			case ExactValue_Integer:
				if (big_int_is_zero(&y->value.value_integer)) {
					fail = true;
				}
				break;
			case ExactValue_Float:
				if (y->value.value_float == 0.0) {
					fail = true;
				}
				break;
			}

			if (fail) {
				error(y->expr, "Division by zero not allowed");
				x->mode = Addressing_Invalid;
				return;
			}
		}
		break;

	case Token_CmpAnd:
	case Token_CmpOr:
		if (be->left->viral_state_flags & ViralStateFlag_ContainsDeferredProcedure) {
			error(be->left, "Procedure calls that have an associated deferred procedure are not allowed within logical binary expressions");
		}
		if (be->right->viral_state_flags & ViralStateFlag_ContainsDeferredProcedure) {
			error(be->right, "Procedure calls that have an associated deferred procedure are not allowed within logical binary expressions");
		}
		break;

	}

	if (x->mode == Addressing_Constant &&
	    y->mode == Addressing_Constant) {
		ExactValue a = x->value;
		ExactValue b = y->value;

		if (!is_type_constant_type(x->type)) {
		#if 0
			gbString xt = type_to_string(x->type);
			gbString err_str = expr_to_string(node);
			error(op, "Invalid type, '%s', for constant binary expression '%s'", xt, err_str);
			gb_string_free(err_str);
			gb_string_free(xt);
			x->mode = Addressing_Invalid;
		#else
			// NOTE(bill, 2021-04-21): The above is literally a useless error message.
			// Why did I add it in the first place?!
			x->mode = Addressing_Value;
		#endif
			return;
		}

		if (op.kind == Token_Quo && is_type_integer(x->type)) {
			op.kind = Token_QuoEq; // NOTE(bill): Hack to get division of integers
		}

		if (is_type_bit_set(x->type)) {
			switch (op.kind) {
			case Token_Add: op.kind = Token_Or;     break;
			case Token_Sub: op.kind = Token_AndNot; break;
			}
		}

		x->value = exact_binary_operator_value(op.kind, a, b);

		if (is_type_typed(x->type)) {
			if (node != nullptr) {
				x->expr = node;
			}
			check_is_expressible(c, x, x->type);
		}
		return;
	} else if (is_type_string(x->type)) {
		error(node, "String concatenation is only allowed with constant strings");
		x->mode = Addressing_Invalid;
		return;
	}

	if (op.kind == Token_Quo || op.kind == Token_QuoEq) {
		Type *bt = base_type(x->type);
		if (bt->kind == Type_Basic) switch (bt->Basic.kind) {
		case Basic_complex32:     add_package_dependency(c, "runtime", "quo_complex32");     break;
		case Basic_complex64:     add_package_dependency(c, "runtime", "quo_complex64");     break;
		case Basic_complex128:    add_package_dependency(c, "runtime", "quo_complex128");    break;
		case Basic_quaternion64:  add_package_dependency(c, "runtime", "quo_quaternion64");  break;
		case Basic_quaternion128: add_package_dependency(c, "runtime", "quo_quaternion128"); break;
		case Basic_quaternion256: add_package_dependency(c, "runtime", "quo_quaternion256"); break;
		}
	} else if (op.kind == Token_Mul || op.kind == Token_MulEq) {
		Type *bt = base_type(x->type);
		if (bt->kind == Type_Basic) switch (bt->Basic.kind) {
		case Basic_quaternion64:  add_package_dependency(c, "runtime", "mul_quaternion64"); break;
		case Basic_quaternion128: add_package_dependency(c, "runtime", "mul_quaternion128"); break;
		case Basic_quaternion256: add_package_dependency(c, "runtime", "mul_quaternion256"); break;
		}
	}

	x->mode = Addressing_Value;
}

Operand make_operand_from_node(Ast *node) {
	GB_ASSERT(node != nullptr);
	Operand x = {};
	x.expr  = node;
	x.mode  = node->tav.mode;
	x.type  = node->tav.type;
	x.value = node->tav.value;
	return x;
}


void update_untyped_expr_type(CheckerContext *c, Ast *e, Type *type, bool final) {
	GB_ASSERT(e != nullptr);
	ExprInfo *old = check_get_expr_info(c, e);
	if (old == nullptr) {
		if (type != nullptr && type != t_invalid) {
			if (e->tav.type == nullptr || e->tav.type == t_invalid) {
				add_type_and_value(c->info, e, e->tav.mode, type ? type : e->tav.type, e->tav.value);
				if (e->kind == Ast_TernaryIfExpr) {
					update_untyped_expr_type(c, e->TernaryIfExpr.x, type, final);
					update_untyped_expr_type(c, e->TernaryIfExpr.y, type, final);
				}
			}
		}
		return;
	}

	switch (e->kind) {
	case_ast_node(ue, UnaryExpr, e);
		if (old->value.kind != ExactValue_Invalid) {
			// NOTE(bill): if 'e' is constant, the operands will be constant too.
			// They don't need to be updated as they will be updated later and
			// checked at the end of general checking stage.
			break;
		}
		update_untyped_expr_type(c, ue->expr, type, final);
	case_end;

	case_ast_node(be, BinaryExpr, e);
		if (old->value.kind != ExactValue_Invalid) {
			// See above note in UnaryExpr case
			break;
		}
		if (token_is_comparison(be->op.kind)) {
			// NOTE(bill): Do nothing as the types are fine
		} else if (token_is_shift(be->op.kind)) {
			update_untyped_expr_type(c, be->left, type, final);
		} else {
			update_untyped_expr_type(c, be->left,  type, final);
			update_untyped_expr_type(c, be->right, type, final);
		}
	case_end;

	case_ast_node(te, TernaryIfExpr, e);
		if (old->value.kind != ExactValue_Invalid) {
			// See above note in UnaryExpr case
			break;
		}
		
		// NOTE(bill): This is a bit of a hack to get around the edge cases of ternary if expressions
		// having an untyped value
		Operand x = make_operand_from_node(te->x);
		Operand y = make_operand_from_node(te->y);		
		if (x.mode != Addressing_Constant || check_is_expressible(c, &x, type)) {
			update_untyped_expr_type(c, te->x, type, final);
		}
		if (y.mode != Addressing_Constant || check_is_expressible(c, &y, type)) {
			update_untyped_expr_type(c, te->y, type, final);
		}
		
	case_end;

	case_ast_node(te, TernaryWhenExpr, e);
		if (old->value.kind != ExactValue_Invalid) {
			// See above note in UnaryExpr case
			break;
		}

		update_untyped_expr_type(c, te->x, type, final);
		update_untyped_expr_type(c, te->y, type, final);
	case_end;

	case_ast_node(ore, OrReturnExpr, e);
		if (old->value.kind != ExactValue_Invalid) {
			// See above note in UnaryExpr case
			break;
		}

		update_untyped_expr_type(c, ore->expr, type, final);
	case_end;

	case_ast_node(oee, OrElseExpr, e);
		if (old->value.kind != ExactValue_Invalid) {
			// See above note in UnaryExpr case
			break;
		}

		update_untyped_expr_type(c, oee->x, type, final);
		update_untyped_expr_type(c, oee->y, type, final);
	case_end;

	case_ast_node(pe, ParenExpr, e);
		update_untyped_expr_type(c, pe->expr, type, final);
	case_end;
	}

	if (!final && is_type_untyped(type)) {
		old->type = base_type(type);
		return;
	}

	// We need to remove it and then give it a new one
	check_remove_expr_info(c, e);

	if (old->is_lhs && !is_type_integer(type)) {
		gbString expr_str = expr_to_string(e);
		gbString type_str = type_to_string(type);
		error(e, "Shifted operand %s must be an integer, got %s", expr_str, type_str);
		gb_string_free(type_str);
		gb_string_free(expr_str);
		return;
	}

	add_type_and_value(c->info, e, old->mode, type, old->value);
}

void update_untyped_expr_value(CheckerContext *c, Ast *e, ExactValue value) {
	GB_ASSERT(e != nullptr);
	ExprInfo *found = check_get_expr_info(c, e);
	if (found) {
		found->value = value;
	}
}

void convert_untyped_error(CheckerContext *c, Operand *operand, Type *target_type) {
	gbString expr_str = expr_to_string(operand->expr);
	gbString type_str = type_to_string(target_type);
	gbString from_type_str = type_to_string(operand->type);
	char const *extra_text = "";

	if (operand->mode == Addressing_Constant) {
		if (big_int_is_zero(&operand->value.value_integer)) {
			if (make_string_c(expr_str) != "nil") { // HACK NOTE(bill): Just in case
				// NOTE(bill): Doesn't matter what the type is as it's still zero in the union
				extra_text = " - Did you want 'nil'?";
			}
		}
	}
	ERROR_BLOCK();

	error(operand->expr, "Cannot convert untyped value '%s' to '%s' from '%s'%s", expr_str, type_str, from_type_str, extra_text);
	if (operand->value.kind == ExactValue_String) {
		String key = operand->value.value_string;
		if (is_type_string(operand->type) && is_type_enum(target_type)) {
			Type *et = base_type(target_type);
			check_did_you_mean_type(key, et->Enum.fields, ".");
		}
	}

	gb_string_free(from_type_str);
	gb_string_free(type_str);
	gb_string_free(expr_str);
	operand->mode = Addressing_Invalid;
}

ExactValue convert_exact_value_for_type(ExactValue v, Type *type) {
	Type *t = core_type(type);
	if (is_type_boolean(t)) {
		// v = exact_value_to_boolean(v);
	} else if (is_type_float(t)) {
		v = exact_value_to_float(v);
	} else if (is_type_integer(t)) {
		v = exact_value_to_integer(v);
	} else if (is_type_pointer(t)) {
		v = exact_value_to_integer(v);
	} else if (is_type_complex(t)) {
		v = exact_value_to_complex(v);
	} else if (is_type_quaternion(t)) {
		v = exact_value_to_quaternion(v);
	}
	return v;
}

void convert_to_typed(CheckerContext *c, Operand *operand, Type *target_type) {
	GB_ASSERT_NOT_NULL(target_type);
	if (operand->mode == Addressing_Invalid ||
	    operand->mode == Addressing_Type ||
	    is_type_typed(operand->type) ||
	    target_type == t_invalid) {
		return;
	}

	if (is_type_untyped(target_type)) {
		GB_ASSERT(operand->type->kind == Type_Basic);
		GB_ASSERT(target_type->kind == Type_Basic);
		BasicKind x_kind = operand->type->Basic.kind;
		BasicKind y_kind = target_type->Basic.kind;
		if (is_type_numeric(operand->type) && is_type_numeric(target_type)) {
			if (x_kind < y_kind) {
				operand->type = target_type;
				update_untyped_expr_type(c, operand->expr, target_type, false);
			}
		} else if (x_kind != y_kind) {
			operand->mode = Addressing_Invalid;
			convert_untyped_error(c, operand, target_type);
			return;
		}
		return;
	}

	Type *t = base_type(target_type);
	if (c->in_enum_type) {
		t = core_type(target_type);
	}

	switch (t->kind) {
	case Type_Basic:
		if (operand->mode == Addressing_Constant) {
			check_is_expressible(c, operand, t);
			if (operand->mode == Addressing_Invalid) {
				return;
			}
			update_untyped_expr_value(c, operand->expr, operand->value);
		} else {
			switch (operand->type->Basic.kind) {
			case Basic_UntypedBool:
				if (!is_type_boolean(target_type)) {
					operand->mode = Addressing_Invalid;
					convert_untyped_error(c, operand, target_type);
					return;
				}
				break;
			case Basic_UntypedInteger:
			case Basic_UntypedFloat:
			case Basic_UntypedComplex:
			case Basic_UntypedQuaternion:
			case Basic_UntypedRune:
				if (!is_type_numeric(target_type)) {
					operand->mode = Addressing_Invalid;
					convert_untyped_error(c, operand, target_type);
					return;
				}
				break;

			case Basic_UntypedNil:
				if (is_type_any(target_type)) {
					// target_type = t_untyped_nil;
				} else if (is_type_cstring(target_type)) {
					// target_type = t_untyped_nil;
				} else if (!type_has_nil(target_type)) {
					operand->mode = Addressing_Invalid;
					convert_untyped_error(c, operand, target_type);
					return;
				}
				break;
			}
		}
		break;

	case Type_Array: {
		Type *elem = base_array_type(t);
		if (check_is_assignable_to(c, operand, elem)) {
			operand->mode = Addressing_Value;
		} else {
			if (operand->value.kind == ExactValue_String) {
				String s = operand->value.value_string;
				if (is_type_u8_array(t)) {
					if (s.len == t->Array.count) {
						break;
					}
				} else if (is_type_rune_array(t)) {
					isize rune_count = gb_utf8_strnlen(s.text, s.len);
					if (rune_count == t->Array.count) {
						break;
					}
				}
			}
			operand->mode = Addressing_Invalid;
			convert_untyped_error(c, operand, target_type);
			return;
		}

		break;
	}
	
	case Type_Matrix: {
		Type *elem = base_array_type(t);
		if (check_is_assignable_to(c, operand, elem)) {
			if (t->Matrix.row_count != t->Matrix.column_count) {
				operand->mode = Addressing_Invalid;
				begin_error_block();
				defer (end_error_block());
				
				convert_untyped_error(c, operand, target_type);
				error_line("\tNote: Only a square matrix types can be initialized with a scalar value\n");
				return;
			} else {
				operand->mode = Addressing_Value;
			}
		} else {
			operand->mode = Addressing_Invalid;
			convert_untyped_error(c, operand, target_type);
			return;
		}
		break;
	}
		

	case Type_Union:
		if (!is_operand_nil(*operand) && !is_operand_undef(*operand)) {
			isize count = t->Union.variants.count;
			ValidIndexAndScore *valids = gb_alloc_array(temporary_allocator(), ValidIndexAndScore, count);
			isize valid_count = 0;
			isize first_success_index = -1;
			for_array(i, t->Union.variants) {
				Type *vt = t->Union.variants[i];
				i64 score = 0;
				if (check_is_assignable_to_with_score(c, operand, vt, &score)) {
					valids[valid_count].index = i;
					valids[valid_count].score = score;
					valid_count += 1;
					if (first_success_index < 0) {
						first_success_index = i;
					}
				}
			}

			if (valid_count > 1) {
				gb_sort_array(valids, valid_count, valid_index_and_score_cmp);
				i64 best_score = valids[0].score;
				for (isize i = 1; i < valid_count; i++) {
					auto v = valids[i];
					if (best_score > v.score) {
						valid_count = i;
						break;
					}
					best_score = v.score;
				}
				first_success_index = valids[0].index;
			}

			gbString type_str = type_to_string(target_type);
			defer (gb_string_free(type_str));

			if (valid_count == 1) {
				operand->mode = Addressing_Value;
				operand->type = t->Union.variants[first_success_index];
				target_type = t->Union.variants[first_success_index];
				break;
			} else if (valid_count > 1) {
				begin_error_block();
				defer (end_error_block());

				GB_ASSERT(first_success_index >= 0);
				operand->mode = Addressing_Invalid;
				convert_untyped_error(c, operand, target_type);

				error_line("Ambiguous type conversion to '%s', which variant did you mean:\n\t", type_str);
				i32 j = 0;
				for (i32 i = 0; i < valid_count; i++) {
					ValidIndexAndScore valid = valids[i];
					if (j > 0 && valid_count > 2) error_line(", ");
					if (j == valid_count-1) {
						if (valid_count == 2) error_line(" ");
						error_line("or ");
					}
					gbString str = type_to_string(t->Union.variants[valid.index]);
					error_line("'%s'", str);
					gb_string_free(str);
					j++;
				}
				error_line("\n\n");

				return;
			} else if (is_type_untyped_undef(operand->type) && type_has_undef(target_type)) {
				target_type = t_untyped_undef;
			} else if (!is_type_untyped_nil(operand->type) || !type_has_nil(target_type)) {
				begin_error_block();
				defer (end_error_block());

				operand->mode = Addressing_Invalid;
				convert_untyped_error(c, operand, target_type);
				if (count > 0) {
					error_line("'%s' is a union which only excepts the following types:\n", type_str);
					error_line("\t");
					for (i32 i = 0; i < count; i++) {
						Type *v = t->Union.variants[i];
						if (i > 0 && count > 2) error_line(", ");
						if (i == count-1) {
							if (count == 2) error_line(" ");
							if (count > 1) {
								error_line("or ");
							}
						}
						gbString str = type_to_string(v);
						error_line("'%s'", str);
						gb_string_free(str);
					}
					error_line("\n\n");

				}
				return;
			}
		}
		/* fallthrough */


	default:
		if (is_type_untyped_undef(operand->type) && type_has_undef(target_type)) {
			target_type = t_untyped_undef;
		} else if (is_type_untyped_nil(operand->type) && type_has_nil(target_type)) {
			target_type = t_untyped_nil;
		} else {
			operand->mode = Addressing_Invalid;
			convert_untyped_error(c, operand, target_type);
			return;
		}
		break;
	}
	
	if (is_type_any(target_type) && is_type_untyped(operand->type)) {
		if (is_type_untyped_nil(operand->type) && is_type_untyped_undef(operand->type)) {
				
		} else {
			target_type = default_type(operand->type);
		}
	}

	update_untyped_expr_type(c, operand->expr, target_type, true);
	operand->type = target_type;
}

bool check_index_value(CheckerContext *c, Type *main_type, bool open_range, Ast *index_value, i64 max_count, i64 *value, Type *type_hint=nullptr) {
	Operand operand = {Addressing_Invalid};
	check_expr_with_type_hint(c, &operand, index_value, type_hint);
	if (operand.mode == Addressing_Invalid) {
		if (value) *value = 0;
		return false;
	}

	Type *index_type = t_int;
	if (type_hint != nullptr) {
		index_type = type_hint;
	}
	convert_to_typed(c, &operand, index_type);
	if (operand.mode == Addressing_Invalid) {
		if (value) *value = 0;
		return false;
	}

	if (type_hint != nullptr) {
		if (!check_is_assignable_to(c, &operand, type_hint)) {
			gbString expr_str = expr_to_string(operand.expr);
			gbString index_type_str = type_to_string(type_hint);
			error(operand.expr, "Index '%s' must be an enum of type '%s'", expr_str, index_type_str);
			gb_string_free(index_type_str);
			gb_string_free(expr_str);
			if (value) *value = 0;
			return false;
		}
	} else if (!is_type_integer(operand.type) && !is_type_enum(operand.type)) {
		gbString expr_str = expr_to_string(operand.expr);
		error(operand.expr, "Index '%s' must be an integer", expr_str);
		gb_string_free(expr_str);
		if (value) *value = 0;
		return false;
	}

	if (operand.mode == Addressing_Constant &&
	    (c->state_flags & StateFlag_no_bounds_check) == 0) {
		BigInt i = exact_value_to_integer(operand.value).value_integer;
		if (i.sign && !is_type_enum(index_type) && !is_type_multi_pointer(main_type)) {
			gbString expr_str = expr_to_string(operand.expr);
			error(operand.expr, "Index '%s' cannot be a negative value", expr_str);
			gb_string_free(expr_str);
			if (value) *value = 0;
			return false;
		}

		if (max_count >= 0) {
			if (is_type_enum(index_type)) {
				Type *bt = base_type(index_type);
				GB_ASSERT(bt->kind == Type_Enum);
				ExactValue const &lo = *bt->Enum.min_value;
				ExactValue const &hi = *bt->Enum.max_value;
				String lo_str = {};
				String hi_str = {};
				if (bt->Enum.fields.count > 0) {
					isize lo_idx = gb_clamp(bt->Enum.min_value_index, 0, bt->Enum.fields.count - 1);
					isize hi_idx = gb_clamp(bt->Enum.max_value_index, 0, bt->Enum.fields.count - 1);

					lo_str = bt->Enum.fields[lo_idx]->token.string;
					hi_str = bt->Enum.fields[hi_idx]->token.string;
				}

				bool out_of_bounds = false;

				if (compare_exact_values(Token_Lt, operand.value, lo) || compare_exact_values(Token_Gt, operand.value, hi)) {
					out_of_bounds = true;
				}

				if (out_of_bounds) {
					gbString expr_str = expr_to_string(operand.expr);
					if (lo_str.len > 0) {
						error(operand.expr, "Index '%s' is out of bounds range %.*s .. %.*s", expr_str, LIT(lo_str), LIT(hi_str));
					} else {
						gbString index_type_str = type_to_string(index_type);
						error(operand.expr, "Index '%s' is out of bounds range of enum type %s", expr_str, index_type_str);
						gb_string_free(index_type_str);
					}
					gb_string_free(expr_str);
					return false;
				}

				if (value) *value = exact_value_to_i64(exact_value_sub(operand.value, lo));

				return true;

			} else { // NOTE(bill): Do array bound checking
				i64 v = -1;
				if (i.used <= 1) {
					v = big_int_to_i64(&i);
				}
				if (value) *value = v;
				bool out_of_bounds = false;
				if (v < 0) {
					out_of_bounds = true;
				} else if (open_range) {
					out_of_bounds = v > max_count;
				} else {
					out_of_bounds = v >= max_count;
				}

				if (out_of_bounds) {
					gbString expr_str = expr_to_string(operand.expr);
					error(operand.expr, "Index '%s' is out of bounds range 0..<%lld", expr_str, max_count);
					gb_string_free(expr_str);
					return false;
				}


				return true;
			}
		} else {
			if (value) *value = exact_value_to_i64(operand.value);
			return true;
		}
	}

	// NOTE(bill): It's alright :D
	if (value) *value = -1;
	return true;
}

ExactValue get_constant_field_single(CheckerContext *c, ExactValue value, i32 index, bool *success_, bool *finish_) {
	if (value.kind == ExactValue_String) {
		GB_ASSERT(0 <= index && index < value.value_string.len);
		u8 val = value.value_string[index];
		if (success_) *success_ = true;
		if (finish_) *finish_ = true;
		return exact_value_u64(val);
	}
	if (value.kind != ExactValue_Compound) {
		if (success_) *success_ = true;
		if (finish_) *finish_ = true;
		return value;
	}


	Ast *node = value.value_compound;
	switch (node->kind) {
	case_ast_node(cl, CompoundLit, node);
		if (cl->elems.count == 0) {
			if (success_) *success_ = true;
			if (finish_) *finish_ = true;
			return empty_exact_value;
		}

		if (cl->elems[0]->kind == Ast_FieldValue) {
			if (is_type_struct(node->tav.type)) {
				for_array(i, cl->elems) {
					Ast *elem = cl->elems[i];
					if (elem->kind != Ast_FieldValue) {
						continue;
					}
					ast_node(fv, FieldValue, elem);
					String name = fv->field->Ident.token.string;
					Selection sub_sel = lookup_field(node->tav.type, name, false);
					defer (array_free(&sub_sel.index));
					if (sub_sel.index[0] == index) {
						value = fv->value->tav.value;
						break;
					}
				}
			} else if (is_type_array(node->tav.type) || is_type_enumerated_array(node->tav.type)) {
				for_array(i, cl->elems) {
					Ast *elem = cl->elems[i];
					if (elem->kind != Ast_FieldValue) {
						continue;
					}
					ast_node(fv, FieldValue, elem);
					if (is_ast_range(fv->field)) {
						ast_node(ie, BinaryExpr, fv->field);
						TypeAndValue lo_tav = ie->left->tav;
						TypeAndValue hi_tav = ie->right->tav;
						GB_ASSERT(lo_tav.mode == Addressing_Constant);
						GB_ASSERT(hi_tav.mode == Addressing_Constant);

						TokenKind op = ie->op.kind;
						i64 lo = exact_value_to_i64(lo_tav.value);
						i64 hi = exact_value_to_i64(hi_tav.value);

						i64 corrected_index = index;

						if (is_type_enumerated_array(node->tav.type)) {
							Type *bt = base_type(node->tav.type);
							GB_ASSERT(bt->kind == Type_EnumeratedArray);
							corrected_index = index + exact_value_to_i64(*bt->EnumeratedArray.min_value);
						}
						if (op != Token_RangeHalf) {
							if (lo <= corrected_index && corrected_index <= hi) {
								TypeAndValue tav = fv->value->tav;
								if (success_) *success_ = true;
								if (finish_) *finish_ = false;
								return tav.value;
							}
						} else {
							if (lo <= corrected_index && corrected_index < hi) {
								TypeAndValue tav = fv->value->tav;
								if (success_) *success_ = true;
								if (finish_) *finish_ = false;
								return tav.value;
							}
						}
					} else {
						TypeAndValue index_tav = fv->field->tav;
						GB_ASSERT(index_tav.mode == Addressing_Constant);
						ExactValue index_value = index_tav.value;
						if (is_type_enumerated_array(node->tav.type)) {
							Type *bt = base_type(node->tav.type);
							GB_ASSERT(bt->kind == Type_EnumeratedArray);
							index_value = exact_value_sub(index_value, *bt->EnumeratedArray.min_value);
						}

						i64 field_index = exact_value_to_i64(index_value);
						if (index == field_index) {
							TypeAndValue tav = fv->value->tav;
							if (success_) *success_ = true;
							if (finish_) *finish_ = false;
							return tav.value;;
						}
					}

				}
			}
		} else {
			i32 count = (i32)cl->elems.count;
			if (count < index) {
				if (success_) *success_ = false;
				if (finish_) *finish_ = true;
				return empty_exact_value;
			}
			if (cl->elems.count <= index) {
				if (success_) *success_ = false;
				if (finish_) *finish_ = false;
				return value;
			}

			TypeAndValue tav = cl->elems[index]->tav;
			if (tav.mode == Addressing_Constant) {
				if (success_) *success_ = true;
				if (finish_) *finish_ = false;
				return tav.value;
			} else {
				GB_ASSERT(is_type_untyped_nil(tav.type));
				if (success_) *success_ = true;
				if (finish_) *finish_ = false;
				return tav.value;
			}
		}

	case_end;

	default:
		// TODO(bill): Should this be a general fallback?
		if (success_) *success_ = true;
		if (finish_) *finish_ = true;
		return empty_exact_value;
	}

	if (finish_) *finish_ = false;
	return value;
}



ExactValue get_constant_field(CheckerContext *c, Operand const *operand, Selection sel, bool *success_) {
	if (operand->mode != Addressing_Constant) {
		if (success_) *success_ = false;
		return empty_exact_value;
	}

	if (sel.indirect) {
		if (success_) *success_ = false;
		return empty_exact_value;
	}

	if (sel.index.count == 0) {
		if (success_) *success_ = false;
		return empty_exact_value;
	}


	ExactValue value = operand->value;
	if (value.kind == ExactValue_Compound) {
		while (sel.index.count > 0) {
			i32 index = sel.index[0];
			sel = sub_selection(sel, 1);

			bool finish = false;
			value = get_constant_field_single(c, value, index, success_, &finish);
			if (finish) {
				return value;
			}
		}

		if (success_) *success_ = true;
		return value;
	} else if (value.kind == ExactValue_Quaternion) {
		// @QuaternionLayout
		Quaternion256 q = *value.value_quaternion;
		GB_ASSERT(sel.index.count == 1);

		switch (sel.index[0]) {
		case 3: // w
			if (success_) *success_ = true;
			return exact_value_float(q.real);

		case 0: // x
			if (success_) *success_ = true;
			return exact_value_float(q.imag);

		case 1: // y
			if (success_) *success_ = true;
			return exact_value_float(q.jmag);

		case 2: // z
			if (success_) *success_ = true;
			return exact_value_float(q.kmag);
		}

		if (success_) *success_ = false;
		return empty_exact_value;
	} else if (value.kind == ExactValue_Complex) {
		// @QuaternionLayout
		Complex128 c = *value.value_complex;
		GB_ASSERT(sel.index.count == 1);

		switch (sel.index[0]) {
		case 0: // real
			if (success_) *success_ = true;
			return exact_value_float(c.real);

		case 1: // imag
			if (success_) *success_ = true;
			return exact_value_float(c.imag);
		}

		if (success_) *success_ = false;
		return empty_exact_value;
	}

	if (success_) *success_ = true;
	return empty_exact_value;
}

Type *determine_swizzle_array_type(Type *original_type, Type *type_hint, isize new_count) {
	Type *array_type = base_type(type_deref(original_type));
	GB_ASSERT(array_type->kind == Type_Array);
	Type *elem_type = array_type->Array.elem;

	Type *swizzle_array_type = nullptr;
	Type *bth = base_type(type_deref(type_hint));
	if (bth != nullptr && bth->kind == Type_Array &&
	    bth->Array.count == new_count &&
	    are_types_identical(bth->Array.elem, elem_type)) {
		swizzle_array_type = type_hint;
	} else {
		i64 max_count = array_type->Array.count;
		if (new_count == max_count) {
			swizzle_array_type = original_type;
		} else {
			swizzle_array_type = alloc_type_array(elem_type, new_count);
		}
	}
	return swizzle_array_type;
}


bool is_entity_declared_for_selector(Entity *entity, Scope *import_scope, bool *allow_builtin) {
	bool is_declared = entity != nullptr;
	if (is_declared) {
		if (entity->kind == Entity_Builtin) {
			// NOTE(bill): Builtin's are in the universal scope which is part of every scopes hierarchy
			// This means that we should just ignore the found result through it
			*allow_builtin = entity->scope == import_scope || entity->scope != builtin_pkg->scope;
		} else if ((entity->scope->flags&ScopeFlag_Global) == ScopeFlag_Global && (import_scope->flags&ScopeFlag_Global) == 0) {
			is_declared = false;
		}
	}
	return is_declared;
}

// NOTE(bill, 2022-02-03): see `check_const_decl` for why it exists reasoning
Entity *check_entity_from_ident_or_selector(CheckerContext *c, Ast *node, bool ident_only) {
	if (node->kind == Ast_Ident) {
		String name = node->Ident.token.string;
		return scope_lookup(c->scope, name);
	} else if (!ident_only) if (node->kind == Ast_SelectorExpr) {
		ast_node(se, SelectorExpr, node);
		if (se->token.kind == Token_ArrowRight) {
			return nullptr;
		}

		Ast *op_expr  = se->expr;
		Ast *selector = unparen_expr(se->selector);
		if (selector == nullptr) {
			return nullptr;
		}
		if (selector->kind != Ast_Ident) {
			return nullptr;
		}

		Entity *entity = nullptr;
		Entity *expr_entity = nullptr;
		bool check_op_expr = true;

		if (op_expr->kind == Ast_Ident) {
			String op_name = op_expr->Ident.token.string;
			Entity *e = scope_lookup(c->scope, op_name);
			if (e == nullptr) {
				return nullptr;
			}
			add_entity_use(c, op_expr, e);
			expr_entity = e;

			if (e != nullptr && e->kind == Entity_ImportName && selector->kind == Ast_Ident) {
				// IMPORTANT NOTE(bill): This is very sloppy code but it's also very fragile
				// It pretty much needs to be in this order and this way
				// If you can clean this up, please do but be really careful
				String import_name = op_name;
				Scope *import_scope = e->ImportName.scope;
				String entity_name = selector->Ident.token.string;

				check_op_expr = false;
				entity = scope_lookup_current(import_scope, entity_name);
				bool allow_builtin = false;
				if (!is_entity_declared_for_selector(entity, import_scope, &allow_builtin)) {
					return nullptr;
				}

				check_entity_decl(c, entity, nullptr, nullptr);
				if (entity->kind == Entity_ProcGroup) {
					return entity;
				}
				GB_ASSERT_MSG(entity->type != nullptr, "%.*s (%.*s)", LIT(entity->token.string), LIT(entity_strings[entity->kind]));
			}
		}

		Operand operand = {};
		if (check_op_expr) {
			check_expr_base(c, &operand, op_expr, nullptr);
			if (operand.mode == Addressing_Invalid) {
				return nullptr;
			}
		}

		if (entity == nullptr && selector->kind == Ast_Ident) {
			String field_name = selector->Ident.token.string;
			if (is_type_dynamic_array(type_deref(operand.type))) {
				init_mem_allocator(c->checker);
			}
			auto sel = lookup_field(operand.type, field_name, operand.mode == Addressing_Type);
			entity = sel.entity;
		}

		if (entity != nullptr) {
			return entity;
		}
	}
	return nullptr;
}


Entity *check_selector(CheckerContext *c, Operand *operand, Ast *node, Type *type_hint) {
	ast_node(se, SelectorExpr, node);

	bool check_op_expr = true;
	Entity *expr_entity = nullptr;
	Entity *entity = nullptr;
	Selection sel = {}; // NOTE(bill): Not used if it's an import name

	if (!c->allow_arrow_right_selector_expr && se->token.kind == Token_ArrowRight) {
		error(node, "Illegal use of -> selector shorthand outside of a call");
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	operand->expr = node;

	Ast *op_expr  = se->expr;
	Ast *selector = unparen_expr(se->selector);
	if (selector == nullptr) {
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	if (selector->kind != Ast_Ident) {
		error(selector, "Illegal selector kind: '%.*s'", LIT(ast_strings[selector->kind]));
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	if (op_expr->kind == Ast_Ident) {
		String op_name = op_expr->Ident.token.string;
		Entity *e = scope_lookup(c->scope, op_name);
		add_entity_use(c, op_expr, e);
		expr_entity = e;

		if (e != nullptr && e->kind == Entity_ImportName && selector->kind == Ast_Ident) {
			// IMPORTANT NOTE(bill): This is very sloppy code but it's also very fragile
			// It pretty much needs to be in this order and this way
			// If you can clean this up, please do but be really careful
			String import_name = op_name;
			Scope *import_scope = e->ImportName.scope;
			String entity_name = selector->Ident.token.string;

			check_op_expr = false;
			entity = scope_lookup_current(import_scope, entity_name);
			bool allow_builtin = false;
			if (!is_entity_declared_for_selector(entity, import_scope, &allow_builtin)) {
				error(op_expr, "'%.*s' is not declared by '%.*s'", LIT(entity_name), LIT(import_name));
				operand->mode = Addressing_Invalid;
				operand->expr = node;

				check_did_you_mean_scope(entity_name, import_scope);
				return nullptr;
			}

			check_entity_decl(c, entity, nullptr, nullptr);
			if (entity->kind == Entity_ProcGroup) {
				operand->mode = Addressing_ProcGroup;
				operand->proc_group = entity;

				add_type_and_value(c->info, operand->expr, operand->mode, operand->type, operand->value);
				return entity;
			}
			GB_ASSERT_MSG(entity->type != nullptr, "%.*s (%.*s)", LIT(entity->token.string), LIT(entity_strings[entity->kind]));

			if (!is_entity_exported(entity, allow_builtin)) {
				gbString sel_str = expr_to_string(selector);
				error(op_expr, "'%s' is not exported by '%.*s'", sel_str, LIT(import_name));
				gb_string_free(sel_str);
				// NOTE(bill): make the state valid still, even if it's "invalid"
				// operand->mode = Addressing_Invalid;
				// operand->expr = node;
				// return nullptr;
			}

			if (entity->kind == Entity_ProcGroup) {
				Array<Entity *> procs = entity->ProcGroup.entities;
				bool skip = false;
				for_array(i, procs) {
					Entity *p = procs[i];
					Type *t = base_type(p->type);
					if (t == t_invalid) {
						continue;
					}

					Operand x = {};
					x.mode = Addressing_Value;
					x.type = t;
					if (type_hint != nullptr) {
						if (check_is_assignable_to(c, &x, type_hint)) {
							entity = p;
							skip = true;
							break;
						}
					}
				}

				if (!skip) {
					GB_ASSERT(entity != nullptr);
					operand->mode       = Addressing_ProcGroup;
					operand->type       = t_invalid;
					operand->expr       = node;
					operand->proc_group = entity;
					return entity;
				}
			}
		}
	}

	if (check_op_expr) {
		check_expr_base(c, operand, op_expr, nullptr);
		if (operand->mode == Addressing_Invalid) {
			operand->mode = Addressing_Invalid;
			operand->expr = node;
			return nullptr;
		}
	}


	if (entity == nullptr && selector->kind == Ast_Ident) {
		String field_name = selector->Ident.token.string;
		if (is_type_dynamic_array(type_deref(operand->type))) {
			init_mem_allocator(c->checker);
		}
		sel = lookup_field(operand->type, field_name, operand->mode == Addressing_Type);
		entity = sel.entity;

		// NOTE(bill): Add type info needed for fields like 'names'
		if (entity != nullptr && (entity->flags&EntityFlag_TypeField)) {
			add_type_info_type(c, operand->type);
		}
		if (is_type_enum(operand->type)) {
			add_type_info_type(c, operand->type);
		}
	}

	if (entity == nullptr && selector->kind == Ast_Ident && is_type_array(type_deref(operand->type))) {
		// TODO(bill): Simd_Vector swizzling

		String field_name = selector->Ident.token.string;
		if (1 < field_name.len && field_name.len <= 4) {
			u8 swizzles_xyzw[4] = {'x', 'y', 'z', 'w'};
			u8 swizzles_rgba[4] = {'r', 'g', 'b', 'a'};
			bool found_xyzw = false;
			bool found_rgba = false;
			for (isize i = 0; i < field_name.len; i++) {
				bool valid = false;
				for (isize j = 0; j < 4; j++) {
					if (field_name.text[i] == swizzles_xyzw[j]) {
						found_xyzw = true;
						valid = true;
						break;
					}
					if (field_name.text[i] == swizzles_rgba[j]) {
						found_rgba = true;
						valid = true;
						break;
					}
				}
				if (!valid) {
					goto end_of_array_selector_swizzle;
				}
			}

			u8 *swizzles = nullptr;

			u8 index_count = cast(u8)field_name.len;
			if (found_xyzw && found_rgba) {
				gbString op_str = expr_to_string(op_expr);
				error(op_expr, "Mixture of swizzle kinds for field index, got %s", op_str);
				gb_string_free(op_str);
				operand->mode = Addressing_Invalid;
				operand->expr = node;
				return nullptr;
			}
			u8 indices = 0;

			if (found_xyzw) {
				swizzles = swizzles_xyzw;
			} else if (found_rgba) {
				swizzles = swizzles_rgba;
			}
			for (isize i = 0; i < field_name.len; i++) {
				for (isize j = 0; j < 4; j++) {
					if (field_name.text[i] == swizzles[j]) {
						indices |= cast(u8)(j)<<(i*2);
						break;
					}
				}
			}

			Type *original_type = operand->type;
			Type *array_type = base_type(type_deref(original_type));
			GB_ASSERT(array_type->kind == Type_Array);
			i64 array_count = array_type->Array.count;
			for (u8 i = 0; i < index_count; i++) {
				u8 idx = indices>>(i*2) & 3;
				if (idx >= array_count) {
					char c = 0;
					if (found_xyzw) {
						c = swizzles_xyzw[idx];
					} else if (found_rgba) {
						c = swizzles_rgba[idx];
					} else {
						GB_PANIC("unknown swizzle kind");
					}
					error(selector->Ident.token, "Swizzle value is out of bounds, got %c, max count %lld", c, array_count);
					break;
				}
			}

			se->swizzle_count = index_count;
			se->swizzle_indices = indices;


			AddressingMode prev_mode = operand->mode;
			operand->mode = Addressing_SwizzleValue;
			operand->type = determine_swizzle_array_type(original_type, type_hint, index_count);
			operand->expr = node;

			switch (prev_mode) {
			case Addressing_Variable:
			case Addressing_SoaVariable:
			case Addressing_SwizzleVariable:
				operand->mode = Addressing_SwizzleVariable;
				break;
			}

			Entity *swizzle_entity = alloc_entity_variable(nullptr, make_token_ident(field_name), operand->type, EntityState_Resolved);
			add_type_and_value(c->info, operand->expr, operand->mode, operand->type, operand->value);
			return swizzle_entity;
		}
	end_of_array_selector_swizzle:;
	}

	if (entity == nullptr) {
		gbString op_str   = expr_to_string(op_expr);
		gbString type_str = type_to_string(operand->type);
		gbString sel_str  = expr_to_string(selector);
		error(op_expr, "'%s' of type '%s' has no field '%s'", op_str, type_str, sel_str);

		if (operand->type != nullptr && selector->kind == Ast_Ident) {
			String const &name = selector->Ident.token.string;
			Type *bt = base_type(operand->type);
			if (bt->kind == Type_Struct) {
				check_did_you_mean_type(name, bt->Struct.fields);
			} else if (bt->kind == Type_Enum) {
				check_did_you_mean_type(name, bt->Enum.fields);
			}
		}

		gb_string_free(sel_str);
		gb_string_free(type_str);
		gb_string_free(op_str);
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	if (expr_entity != nullptr && expr_entity->kind == Entity_Constant && entity->kind != Entity_Constant) {
		bool success = false;
		ExactValue field_value = get_constant_field(c, operand, sel, &success);
		if (success) {
			operand->mode = Addressing_Constant;
			operand->expr = node;
			operand->value = field_value;
			operand->type = entity->type;
			add_entity_use(c, selector, entity);
			add_type_and_value(c->info, operand->expr, operand->mode, operand->type, operand->value);
			return entity;
		}

		gbString op_str   = expr_to_string(op_expr);
		gbString type_str = type_to_string(operand->type);
		gbString sel_str  = expr_to_string(selector);
		error(op_expr, "Cannot access non-constant field '%s' from '%s'", sel_str, op_str);
		gb_string_free(sel_str);
		gb_string_free(type_str);
		gb_string_free(op_str);
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	if (operand->mode == Addressing_Constant && entity->kind != Entity_Constant) {
		bool success = false;
		ExactValue field_value = get_constant_field(c, operand, sel, &success);
		if (success) {
			operand->mode = Addressing_Constant;
			operand->expr = node;
			operand->value = field_value;
			operand->type = entity->type;
			add_entity_use(c, selector, entity);
			add_type_and_value(c->info, operand->expr, operand->mode, operand->type, operand->value);
			return entity;
		}

		gbString op_str   = expr_to_string(op_expr);
		gbString type_str = type_to_string(operand->type);
		gbString sel_str  = expr_to_string(selector);
		error(op_expr, "Cannot access non-constant field '%s' from '%s'", sel_str, op_str);
		gb_string_free(sel_str);
		gb_string_free(type_str);
		gb_string_free(op_str);
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	if (expr_entity != nullptr && is_type_polymorphic(expr_entity->type)) {
		gbString op_str   = expr_to_string(op_expr);
		gbString type_str = type_to_string(operand->type);
		gbString sel_str  = expr_to_string(selector);
		error(op_expr, "Cannot access field '%s' from non-specialized polymorphic type '%s'", sel_str, op_str);
		gb_string_free(sel_str);
		gb_string_free(type_str);
		gb_string_free(op_str);
		operand->mode = Addressing_Invalid;
		operand->expr = node;
		return nullptr;
	}

	add_entity_use(c, selector, entity);

	operand->type = entity->type;
	operand->expr = node;

	switch (entity->kind) {
	case Entity_Constant:
		operand->value = entity->Constant.value;
		operand->mode = Addressing_Constant;
		if (operand->value.kind == ExactValue_Procedure) {
			Entity *proc = strip_entity_wrapping(operand->value.value_procedure);
			if (proc != nullptr) {
				operand->mode = Addressing_Value;
				operand->type = proc->type;
			}
		}
		break;
	case Entity_Variable:
		if (sel.indirect) {
			operand->mode = Addressing_Variable;
		} else if (operand->mode == Addressing_Context) {
			// Do nothing
		} else if (operand->mode == Addressing_MapIndex) {
			operand->mode = Addressing_Value;
		} else if (entity->flags & EntityFlag_SoaPtrField) {
			operand->mode = Addressing_SoaVariable;
		} else if (operand->mode == Addressing_OptionalOk || operand->mode == Addressing_OptionalOkPtr) {
			operand->mode = Addressing_Value;
		} else if (operand->mode == Addressing_SoaVariable) {
			operand->mode = Addressing_Variable;
		} else if (operand->mode != Addressing_Value) {
			operand->mode = Addressing_Variable;
		} else {
			operand->mode = Addressing_Value;
		}
		break;
	case Entity_TypeName:
		operand->mode = Addressing_Type;
		break;
	case Entity_Procedure:
		operand->mode = Addressing_Value;
		operand->value = exact_value_procedure(node);
		break;
	case Entity_Builtin:
		operand->mode = Addressing_Builtin;
		operand->builtin_id = cast(BuiltinProcId)entity->Builtin.id;
		break;

	case Entity_ProcGroup:
		operand->mode = Addressing_ProcGroup;
		operand->proc_group = entity;
		break;

	// NOTE(bill): These cases should never be hit but are here for sanity reasons
	case Entity_Nil:
		operand->mode = Addressing_Value;
		break;
	}

	add_type_and_value(c->info, operand->expr, operand->mode, operand->type, operand->value);

	return entity;
}

bool is_type_normal_pointer(Type *ptr, Type **elem) {
	ptr = base_type(ptr);
	if (is_type_pointer(ptr)) {
		if (is_type_rawptr(ptr)) {
			return false;
		}
		if (elem) *elem = ptr->Pointer.elem;
		return true;
	}
	return false;
}

bool check_identifier_exists(Scope *s, Ast *node, bool nested = false, Scope **out_scope = nullptr) {
	switch (node->kind) {
	case_ast_node(i, Ident, node);
		String name = i->token.string;
		if (nested) {
			Entity *e = scope_lookup_current(s, name);
			if (e != nullptr) {
				if (out_scope) *out_scope = e->scope;
				return true;
			}
		} else {
			Entity *e = scope_lookup(s, name);
			if (e != nullptr) {
				if (out_scope) *out_scope = e->scope;
				return true;
			}
		}
	case_end;
	case_ast_node(se, SelectorExpr, node);
		Ast *lhs = se->expr;
		Ast *rhs = se->selector;
		Scope *lhs_scope = nullptr;
		if (check_identifier_exists(s, lhs, nested, &lhs_scope)) {
			return check_identifier_exists(lhs_scope, rhs, true);
		}
	case_end;
	}
	return false;
}

isize add_dependencies_from_unpacking(CheckerContext *c, Entity **lhs, isize lhs_count, isize tuple_index, isize tuple_count) {
	if (lhs != nullptr && c->decl != nullptr) {
		mutex_lock(&c->info->deps_mutex);

		for (isize j = 0; (tuple_index + j) < lhs_count && j < tuple_count; j++) {
			Entity *e = lhs[tuple_index + j];
			if (e != nullptr) {
				DeclInfo *decl = decl_info_of_entity(e);
				if (decl != nullptr) {
					for_array(k, decl->deps.entries) {
						Entity *dep = decl->deps.entries[k].ptr;
						ptr_set_add(&c->decl->deps, dep);
					}
				}
			}
		}

		mutex_unlock(&c->info->deps_mutex);
	}
	return tuple_count;
}


bool check_assignment_arguments(CheckerContext *ctx, Array<Operand> const &lhs, Array<Operand> *operands, Slice<Ast *> const &rhs) {
	bool optional_ok = false;
	isize tuple_index = 0;
	for_array(i, rhs) {
		CheckerContext c_ = *ctx;
		CheckerContext *c = &c_;

		Operand o = {};

		Type *type_hint = nullptr;

		if (tuple_index < lhs.count) {
			type_hint = lhs[tuple_index].type;
		}

		check_expr_base(c, &o, rhs[i], type_hint);
		if (o.mode == Addressing_NoValue) {
			error_operand_no_value(&o);
			o.mode = Addressing_Invalid;
		}

		if (o.type == nullptr || o.type->kind != Type_Tuple) {
			if (lhs.count == 2 && rhs.count == 1 &&
			    (o.mode == Addressing_MapIndex || o.mode == Addressing_OptionalOk || o.mode == Addressing_OptionalOkPtr)) {
				Ast *expr = unparen_expr(o.expr);

				Operand val0 = o;
				Operand val1 = o;
				val0.mode = Addressing_Value;
				val1.mode = Addressing_Value;
				val1.type = t_untyped_bool;

				check_promote_optional_ok(c, &o, nullptr, &val1.type);

				if (expr->kind == Ast_TypeAssertion &&
				    (o.mode == Addressing_OptionalOk || o.mode == Addressing_OptionalOkPtr)) {
					// NOTE(bill): Used only for optimizations in the backend
					if (is_blank_ident(lhs[0].expr)) {
						expr->TypeAssertion.ignores[0] = true;
					}
					if (is_blank_ident(lhs[1].expr)) {
						expr->TypeAssertion.ignores[1] = true;
					}
				}

				array_add(operands, val0);
				array_add(operands, val1);
				optional_ok = true;
				tuple_index += 2;
			} else if (o.mode == Addressing_OptionalOk && is_type_tuple(o.type)) {
				Type *tuple = o.type;
				GB_ASSERT(tuple->Tuple.variables.count == 2);
				Ast *expr = unparen_expr(o.expr);
				if (expr->kind == Ast_CallExpr) {
					expr->CallExpr.optional_ok_one = true;
				}
				Operand val = o;
				val.type = tuple->Tuple.variables[0]->type;
				val.mode = Addressing_Value;
				array_add(operands, val);
				tuple_index += tuple->Tuple.variables.count;
			} else {
				array_add(operands, o);
				tuple_index += 1;
			}
		} else {
			TypeTuple *tuple = &o.type->Tuple;
			for_array(j, tuple->variables) {
				o.type = tuple->variables[j]->type;
				array_add(operands, o);
			}

			tuple_index += tuple->variables.count;
		}
	}

	return optional_ok;
}



bool check_unpack_arguments(CheckerContext *ctx, Entity **lhs, isize lhs_count, Array<Operand> *operands, Slice<Ast *> const &rhs, bool allow_ok, bool is_variadic) {
	bool optional_ok = false;
	isize tuple_index = 0;
	for_array(i, rhs) {
		CheckerContext c_ = *ctx;
		CheckerContext *c = &c_;

		Operand o = {};

		Type *type_hint = nullptr;

		if (lhs != nullptr && tuple_index < lhs_count) {
			// NOTE(bill): override DeclInfo for dependency
			Entity *e = lhs[tuple_index];
			if (e != nullptr) {
				// DeclInfo *decl = decl_info_of_entity(e);
				// if (decl) c->decl = decl;
				type_hint = e->type;
				if (e->flags & EntityFlag_Ellipsis) {
					GB_ASSERT(is_type_slice(e->type));
					GB_ASSERT(e->type->kind == Type_Slice);
					type_hint = e->type->Slice.elem;
				}
			}
		} else if (lhs != nullptr && tuple_index >= lhs_count && is_variadic) {
			// NOTE(bill): override DeclInfo for dependency
			Entity *e = lhs[lhs_count-1];
			if (e != nullptr) {
				// DeclInfo *decl = decl_info_of_entity(e);
				// if (decl) c->decl = decl;
				type_hint = e->type;
				if (e->flags & EntityFlag_Ellipsis) {
					GB_ASSERT(is_type_slice(e->type));
					GB_ASSERT(e->type->kind == Type_Slice);
					type_hint = e->type->Slice.elem;
				}
			}
		}

		check_expr_base(c, &o, rhs[i], type_hint);
		if (o.mode == Addressing_NoValue) {
			error_operand_no_value(&o);
			o.mode = Addressing_Invalid;
		}

		if (o.type == nullptr || o.type->kind != Type_Tuple) {
			if (allow_ok && lhs_count == 2 && rhs.count == 1 &&
			    (o.mode == Addressing_MapIndex || o.mode == Addressing_OptionalOk || o.mode == Addressing_OptionalOkPtr)) {
				Ast *expr = unparen_expr(o.expr);

				Operand val0 = o;
				Operand val1 = o;
				val0.mode = Addressing_Value;
				val1.mode = Addressing_Value;
				val1.type = t_untyped_bool;

				check_promote_optional_ok(c, &o, nullptr, &val1.type);

				if (expr->kind == Ast_TypeAssertion &&
				    (o.mode == Addressing_OptionalOk || o.mode == Addressing_OptionalOkPtr)) {
					// NOTE(bill): Used only for optimizations in the backend
					if (is_blank_ident(lhs[0]->token)) {
						expr->TypeAssertion.ignores[0] = true;
					}
					if (is_blank_ident(lhs[1]->token)) {
						expr->TypeAssertion.ignores[1] = true;
					}
				}

				array_add(operands, val0);
				array_add(operands, val1);
				optional_ok = true;
				tuple_index += add_dependencies_from_unpacking(c, lhs, lhs_count, tuple_index, 2);
			} else {
				array_add(operands, o);
				tuple_index += 1;
			}
		} else {
			TypeTuple *tuple = &o.type->Tuple;
			for_array(j, tuple->variables) {
				o.type = tuple->variables[j]->type;
				array_add(operands, o);
			}

			isize count = tuple->variables.count;
			tuple_index += add_dependencies_from_unpacking(c, lhs, lhs_count, tuple_index, count);
		}
	}

	return optional_ok;
}


bool is_expr_constant_zero(Ast *expr) {
	GB_ASSERT(expr != nullptr);
	auto v = exact_value_to_integer(expr->tav.value);
	if (v.kind == ExactValue_Integer) {
		return big_int_cmp_zero(&v.value_integer) == 0;
	}
	return false;
}


CALL_ARGUMENT_CHECKER(check_call_arguments_internal) {
	ast_node(ce, CallExpr, call);
	GB_ASSERT(is_type_proc(proc_type));
	proc_type = base_type(proc_type);
	TypeProc *pt = &proc_type->Proc;

	isize param_count = 0;
	isize param_count_excluding_defaults = 0;
	bool variadic = pt->variadic;
	bool vari_expand = (ce->ellipsis.pos.line != 0);
	i64 score = 0;
	bool show_error = show_error_mode == CallArgumentMode_ShowErrors;


	TypeTuple *param_tuple = nullptr;

	if (pt->params != nullptr) {
		param_tuple = &pt->params->Tuple;

		param_count = param_tuple->variables.count;
		if (variadic) {
			for (isize i = param_count-1; i >= 0; i--) {
				Entity *e = param_tuple->variables[i];
				if (e->kind == Entity_TypeName) {
					break;
				}

				if (e->kind == Entity_Variable) {
					if (e->Variable.param_value.kind != ParameterValue_Invalid) {
						param_count--;
						continue;
					}
				}
				break;
			}
			param_count--;
		}
	}

	param_count_excluding_defaults = param_count;
	if (param_tuple != nullptr) {
		for (isize i = param_count-1; i >= 0; i--) {
			Entity *e = param_tuple->variables[i];
			if (e->kind == Entity_TypeName) {
				break;
			}

			if (e->kind == Entity_Variable) {
				if (e->Variable.param_value.kind != ParameterValue_Invalid) {
					param_count_excluding_defaults--;
					continue;
				}
			}
			break;
		}
	}

	CallArgumentError err = CallArgumentError_None;
	Type *final_proc_type = proc_type;
	Entity *gen_entity = nullptr;

	if (vari_expand && !variadic) {
		if (show_error) {
			error(ce->ellipsis,
			      "Cannot use '..' in call to a non-variadic procedure: '%.*s'",
			      LIT(ce->proc->Ident.token.string));
		}
		err = CallArgumentError_NonVariadicExpand;
	} else if (vari_expand && pt->c_vararg) {
		if (show_error) {
			error(ce->ellipsis,
			      "Cannot use '..' in call to a '#c_vararg' variadic procedure: '%.*s'",
			      LIT(ce->proc->Ident.token.string));
		}
		err = CallArgumentError_NonVariadicExpand;
	} else if (operands.count == 0 && param_count_excluding_defaults == 0) {
		err = CallArgumentError_None;

		if (variadic) {
			GB_ASSERT(param_tuple != nullptr && param_tuple->variables.count > 0);
			Type *t = param_tuple->variables[0]->type;
			if (is_type_polymorphic(t)) {
				error(call, "Ambiguous call to a polymorphic variadic procedure with no variadic input");
				err = CallArgumentError_AmbiguousPolymorphicVariadic;
			}
		}
	} else {
		i32 error_code = 0;
		if (operands.count < param_count_excluding_defaults) {
			error_code = -1;
		} else if (!variadic && operands.count > param_count) {
			error_code = +1;
		}
		if (error_code != 0) {
			err = CallArgumentError_TooManyArguments;
			char const *err_fmt = "Too many arguments for '%s', expected %td arguments, got %td";
			if (error_code < 0) {
				err = CallArgumentError_TooFewArguments;
				err_fmt = "Too few arguments for '%s', expected %td arguments, got %td";
			}

			if (show_error) {
				gbString proc_str = expr_to_string(ce->proc);
				defer (gb_string_free(proc_str));
				error(call, err_fmt, proc_str, param_count_excluding_defaults, operands.count);

				#if 0
				error_line("\t");
				for_array(i, operands) {
					if (i > 0) {
						error_line(", ");
					}
					gbString s = expr_to_string(operands[i].expr);
					error_line("%s", s);
					gb_string_free(s);
				}
				error_line("\n");
				#endif
			}
		} else {
			// NOTE(bill): Generate the procedure type for this generic instance
			if (pt->is_polymorphic && !pt->is_poly_specialized) {
				PolyProcData poly_proc_data = {};
				if (find_or_generate_polymorphic_procedure_from_parameters(c, entity, &operands, call, &poly_proc_data)) {
					gen_entity = poly_proc_data.gen_entity;
					GB_ASSERT(is_type_proc(gen_entity->type));
					final_proc_type = gen_entity->type;
				} else {
					err = CallArgumentError_WrongTypes;
				}
			}

			GB_ASSERT(is_type_proc(final_proc_type));
			TypeProc *pt = &final_proc_type->Proc;

			GB_ASSERT(pt->params != nullptr);
			auto sig_params = pt->params->Tuple.variables;
			isize operand_index = 0;
			isize max_operand_count = gb_min(param_count, operands.count);
			for (; operand_index < max_operand_count; operand_index++) {
				Entity *e = sig_params[operand_index];
				Type *t = e->type;
				Operand o = operands[operand_index];
				if (o.expr != nullptr) {
					call->viral_state_flags |= o.expr->viral_state_flags;
				}

				if (e->kind == Entity_TypeName) {
					// GB_ASSERT(!variadic);
					if (o.mode == Addressing_Invalid) {
						continue;
					} else if (o.mode != Addressing_Type) {
						if (show_error) {
							error(o.expr, "Expected a type for the argument '%.*s'", LIT(e->token.string));
						}
						err = CallArgumentError_WrongTypes;
					}

					if (are_types_identical(e->type, o.type)) {
						score += assign_score_function(1);
					} else {
						score += assign_score_function(MAXIMUM_TYPE_DISTANCE);
					}

					continue;
				}

				bool param_is_variadic = pt->variadic && pt->variadic_index == operand_index;

				i64 s = 0;
				if (!check_is_assignable_to_with_score(c, &o, t, &s, param_is_variadic)) {
					bool ok = false;
					if (e->flags & EntityFlag_AutoCast) {
						ok = check_is_castable_to(c, &o, t);
					} else if (e->flags & EntityFlag_AnyInt) {
						if (is_type_integer(t)) {
							ok = check_is_castable_to(c, &o, t);
						}
					}
					if (ok) {
						s = assign_score_function(MAXIMUM_TYPE_DISTANCE);
					} else {
						if (show_error) {
							check_assignment(c, &o, t, str_lit("argument"));
						}
						// TODO(bill, 2021-05-05): Is this incorrect logic to only fail if there is ambiguity for definite?
						if (o.mode == Addressing_Invalid) {
							err = CallArgumentError_WrongTypes;
						}
					}
				} else if (show_error) {
					check_assignment(c, &o, t, str_lit("argument"));
				}
				score += s;

				if (e->flags & EntityFlag_ConstInput) {
					if (o.mode != Addressing_Constant) {
						if (show_error) {
							error(o.expr, "Expected a constant value for the argument '%.*s'", LIT(e->token.string));
						}
						err = CallArgumentError_NoneConstantParameter;
					}
				}

				if (o.mode == Addressing_Type && is_type_typeid(e->type)) {
					add_type_info_type(c, o.type);
					add_type_and_value(c->info, o.expr, Addressing_Value, e->type, exact_value_typeid(o.type));
				} else if (show_error && is_type_untyped(o.type)) {
					update_untyped_expr_type(c, o.expr, t, true);
				}

			}

			if (variadic) {
				bool variadic_expand = false;
				Type *slice = sig_params[param_count]->type;
				GB_ASSERT(is_type_slice(slice));
				Type *elem = base_type(slice)->Slice.elem;
				Type *t = elem;

				if (is_type_polymorphic(t)) {
					error(call, "Ambiguous call to a polymorphic variadic procedure with no variadic input");
					err = CallArgumentError_AmbiguousPolymorphicVariadic;
				}

				for (; operand_index < operands.count; operand_index++) {
					Operand o = operands[operand_index];
					if (vari_expand) {
						variadic_expand = true;
						t = slice;
						if (operand_index != param_count) {
							if (show_error) {
								error(o.expr, "'..' in a variadic procedure can only have one variadic argument at the end");
							}
							if (data) {
								data->score = score;
								data->result_type = final_proc_type->Proc.results;
								data->gen_entity = gen_entity;
							}
							return CallArgumentError_MultipleVariadicExpand;
						}
					}
					i64 s = 0;
					if (!check_is_assignable_to_with_score(c, &o, t, &s, true)) {
						if (show_error) {
							check_assignment(c, &o, t, str_lit("argument"));
						}
						err = CallArgumentError_WrongTypes;
					} else if (show_error) {
						check_assignment(c, &o, t, str_lit("argument"));
					}
					score += s;
					if (is_type_any(elem)) {
						add_type_info_type(c, o.type);
					}
					if (o.mode == Addressing_Type && is_type_typeid(t)) {
						add_type_info_type(c, o.type);
						add_type_and_value(c->info, o.expr, Addressing_Value, t, exact_value_typeid(o.type));
					} else if (show_error && is_type_untyped(o.type)) {
						update_untyped_expr_type(c, o.expr, t, true);
					}
				}
			}
		}
	}

	if (data) {
		data->score = score;
		data->result_type = final_proc_type->Proc.results;
		data->gen_entity = gen_entity;
		add_type_and_value(c->info, ce->proc, Addressing_Value, final_proc_type, {});
	}

	return err;
}

bool is_call_expr_field_value(AstCallExpr *ce) {
	GB_ASSERT(ce != nullptr);

	if (ce->args.count == 0) {
		return false;
	}
	return ce->args[0]->kind == Ast_FieldValue;
}

isize lookup_procedure_parameter(TypeProc *pt, String parameter_name) {
	isize param_count = pt->param_count;
	for (isize i = 0; i < param_count; i++) {
		Entity *e = pt->params->Tuple.variables[i];
		String name = e->token.string;
		if (is_blank_ident(name)) {
			continue;
		}
		if (name == parameter_name) {
			return i;
		}
	}
	return -1;
}
isize lookup_procedure_result(TypeProc *pt, String result_name) {
	isize result_count = pt->result_count;
	for (isize i = 0; i < result_count; i++) {
		Entity *e = pt->results->Tuple.variables[i];
		String name = e->token.string;
		if (is_blank_ident(name)) {
			continue;
		}
		if (name == result_name) {
			return i;
		}
	}
	return -1;
}

CALL_ARGUMENT_CHECKER(check_named_call_arguments) {
	ast_node(ce, CallExpr, call);
	GB_ASSERT(is_type_proc(proc_type));
	proc_type = base_type(proc_type);
	TypeProc *pt = &proc_type->Proc;

	i64 score = 0;
	bool show_error = show_error_mode == CallArgumentMode_ShowErrors;
	CallArgumentError err = CallArgumentError_None;

	isize param_count = pt->param_count;
	bool *visited = gb_alloc_array(temporary_allocator(), bool, param_count);
	auto ordered_operands = array_make<Operand>(temporary_allocator(), param_count);
	defer ({
		for_array(i, ordered_operands) {
			Operand const &o = ordered_operands[i];
			if (o.expr != nullptr) {
				call->viral_state_flags |= o.expr->viral_state_flags;
			}
		}
	});

	for_array(i, ce->args) {
		Ast *arg = ce->args[i];
		ast_node(fv, FieldValue, arg);
		if (fv->field->kind != Ast_Ident) {
			if (show_error) {
				gbString expr_str = expr_to_string(fv->field);
				error(arg, "Invalid parameter name '%s' in procedure call", expr_str);
				gb_string_free(expr_str);
			}
			err = CallArgumentError_InvalidFieldValue;
			continue;
		}
		String name = fv->field->Ident.token.string;
		isize index = lookup_procedure_parameter(pt, name);
		if (index < 0) {
			if (show_error) {
				error(arg, "No parameter named '%.*s' for this procedure type", LIT(name));
			}
			err = CallArgumentError_ParameterNotFound;
			continue;
		}
		if (visited[index]) {
			if (show_error) {
				error(arg, "Duplicate parameter '%.*s' in procedure call", LIT(name));
			}
			err = CallArgumentError_DuplicateParameter;
			continue;
		}

		visited[index] = true;
		ordered_operands[index] = operands[i];
	}

	// NOTE(bill): Check for default values and missing parameters
	isize param_count_to_check = param_count;
	if (pt->variadic) {
		param_count_to_check--;
	}
	for (isize i = 0; i < param_count_to_check; i++) {
		if (!visited[i]) {
			Entity *e = pt->params->Tuple.variables[i];
			if (is_blank_ident(e->token)) {
				continue;
			}
			if (e->kind == Entity_Variable) {
				if (e->Variable.param_value.kind != ParameterValue_Invalid) {
					score += assign_score_function(1);
					continue;
				}
			}

			if (show_error) {
				if (e->kind == Entity_TypeName) {
					error(call, "Type parameter '%.*s' is missing in procedure call",
					      LIT(e->token.string));
				} else if (e->kind == Entity_Constant && e->Constant.value.kind != ExactValue_Invalid) {
					// Ignore
				} else {
					gbString str = type_to_string(e->type);
					error(call, "Parameter '%.*s' of type '%s' is missing in procedure call",
					      LIT(e->token.string), str);
					gb_string_free(str);
				}
			}
			err = CallArgumentError_ParameterMissing;
		}
	}

	Entity *gen_entity = nullptr;
	if (pt->is_polymorphic && !pt->is_poly_specialized && err == CallArgumentError_None) {
		PolyProcData poly_proc_data = {};
		if (find_or_generate_polymorphic_procedure_from_parameters(c, entity, &ordered_operands, call, &poly_proc_data)) {
			gen_entity = poly_proc_data.gen_entity;
			Type *gept = base_type(gen_entity->type);
			GB_ASSERT(is_type_proc(gept));
			proc_type = gept;
			pt = &gept->Proc;
		}
	}


	for (isize i = 0; i < param_count; i++) {
		Entity *e = pt->params->Tuple.variables[i];
		Operand *o = &ordered_operands[i];
		bool param_is_variadic = pt->variadic && pt->variadic_index == i;


		if (o->mode == Addressing_Invalid) {
			if (param_is_variadic) {
				Type *slice = e->type;
				GB_ASSERT(is_type_slice(slice));
				Type *elem = base_type(slice)->Slice.elem;
				if (is_type_polymorphic(elem)) {
					error(call, "Ambiguous call to a polymorphic variadic procedure with no variadic input");
					err = CallArgumentError_AmbiguousPolymorphicVariadic;
					return err;
				}
			}
			continue;
		}

		if (e->kind == Entity_TypeName) {
			GB_ASSERT(pt->is_polymorphic);
			if (o->mode != Addressing_Type) {
				if (show_error) {
					error(o->expr, "Expected a type for the argument '%.*s'", LIT(e->token.string));
				}
				err = CallArgumentError_WrongTypes;
			}
			if (are_types_identical(e->type, o->type)) {
				score += assign_score_function(1);
			} else {
				score += assign_score_function(MAXIMUM_TYPE_DISTANCE);
			}
		} else {
			i64 s = 0;
			if (!check_is_assignable_to_with_score(c, o, e->type, &s, param_is_variadic)) {
				bool ok = false;
				if (e->flags & EntityFlag_AutoCast) {
					ok = check_is_castable_to(c, o, e->type);
				}
				if (ok) {
					s = assign_score_function(MAXIMUM_TYPE_DISTANCE);
				} else {
					if (show_error) {
						check_assignment(c, o, e->type, str_lit("procedure argument"));
					}
					err = CallArgumentError_WrongTypes;
				}

				if (e->flags & EntityFlag_ConstInput) {
					if (o->mode != Addressing_Constant) {
						if (show_error) {
							error(o->expr, "Expected a constant value for the argument '%.*s'", LIT(e->token.string));
						}
						err = CallArgumentError_NoneConstantParameter;
					}
				}
			} else if (show_error) {
				check_assignment(c, o, e->type, str_lit("procedure argument"));
			}
			score += s;
		}

		if (o->mode == Addressing_Type && is_type_typeid(e->type)) {
			add_type_info_type(c, o->type);
			add_type_and_value(c->info, o->expr, Addressing_Value, e->type, exact_value_typeid(o->type));
		}
	}

	if (data) {
		data->score = score;
		data->result_type = pt->results;
		data->gen_entity = gen_entity;
		add_type_and_value(c->info, ce->proc, Addressing_Value, proc_type, {});
	}

	return err;
}

Entity **populate_proc_parameter_list(CheckerContext *c, Type *proc_type, isize *lhs_count_, bool *is_variadic) {
	Entity **lhs = nullptr;
	isize lhs_count = -1;

	if (proc_type == nullptr) {
		return nullptr;
	}

	GB_ASSERT(is_type_proc(proc_type));
	TypeProc *pt = &base_type(proc_type)->Proc;
	*is_variadic = pt->variadic;

	if (!pt->is_polymorphic || pt->is_poly_specialized) {
		if (pt->params != nullptr) {
			lhs = pt->params->Tuple.variables.data;
			lhs_count = pt->params->Tuple.variables.count;
		}
	} else {
		// NOTE(bill): Create 'lhs' list in order to ignore parameters which are polymorphic
		if (pt->params == nullptr)  {
			lhs_count = 0;
		} else {
			lhs_count = pt->params->Tuple.variables.count;
		}
		lhs = gb_alloc_array(permanent_allocator(), Entity *, lhs_count);
		for (isize i = 0; i < lhs_count; i++) {
			Entity *e = pt->params->Tuple.variables[i];
			if (!is_type_polymorphic(e->type)) {
				lhs[i] = e;
			}
		}
	}

	if (lhs_count_) *lhs_count_ = lhs_count;

	return lhs;
}


bool evaluate_where_clauses(CheckerContext *ctx, Ast *call_expr, Scope *scope, Slice<Ast *> *clauses, bool print_err) {
	if (clauses != nullptr) {
		for_array(i, *clauses) {
			Ast *clause = (*clauses)[i];
			Operand o = {};
			check_expr(ctx, &o, clause);
			if (o.mode != Addressing_Constant) {
				if (print_err) error(clause, "'where' clauses expect a constant boolean evaluation");
				if (print_err && call_expr) error(call_expr, "at caller location");
				return false;
			} else if (o.value.kind != ExactValue_Bool) {
				if (print_err) error(clause, "'where' clauses expect a constant boolean evaluation");
				if (print_err && call_expr) error(call_expr, "at caller location");
				return false;
			} else if (!o.value.value_bool) {
				if (print_err) {
					ERROR_BLOCK();
					
					gbString str = expr_to_string(clause);
					error(clause, "'where' clause evaluated to false:\n\t%s", str);
					gb_string_free(str);

					if (scope != nullptr) {
						isize print_count = 0;
						for_array(j, scope->elements.entries) {
							Entity *e = scope->elements.entries[j].value;
							switch (e->kind) {
							case Entity_TypeName: {
								if (print_count == 0) error_line("\n\tWith the following definitions:\n");

								gbString str = type_to_string(e->type);
								error_line("\t\t%.*s :: %s;\n", LIT(e->token.string), str);
								gb_string_free(str);
								print_count += 1;
								break;
							}
							case Entity_Constant: {
								if (print_count == 0) error_line("\n\tWith the following definitions:\n");

								gbString str = exact_value_to_string(e->Constant.value);
								if (is_type_untyped(e->type)) {
									error_line("\t\t%.*s :: %s;\n", LIT(e->token.string), str);
								} else {
									gbString t = type_to_string(e->type);
									error_line("\t\t%.*s : %s : %s;\n", LIT(e->token.string), t, str);
									gb_string_free(t);
								}
								gb_string_free(str);

								print_count += 1;
								break;
							}
							}
						}
					}

					if (call_expr) error(call_expr, "at caller location");
				}
				return false;
			}
		}
	}

	return true;
}


CallArgumentData check_call_arguments(CheckerContext *c, Operand *operand, Type *proc_type, Ast *call, Slice<Ast *> const &args) {
	ast_node(ce, CallExpr, call);

	CallArgumentCheckerType *call_checker = check_call_arguments_internal;
	Array<Operand> operands = {};
	defer (array_free(&operands));

	Type *result_type = t_invalid;

	if (is_call_expr_field_value(ce)) {
		call_checker = check_named_call_arguments;

		operands = array_make<Operand>(heap_allocator(), args.count);

		// NOTE(bill): This is give type hints for the named parameters
		// in order to improve the type inference system

		StringMap<Type *> type_hint_map = {}; // Key: String
		string_map_init(&type_hint_map, heap_allocator(), 2*args.count);
		defer (string_map_destroy(&type_hint_map));

		Type *ptype = nullptr;
		bool single_case = true;

		if (operand->mode == Addressing_ProcGroup) {
			single_case = false;
			Array<Entity *> procs = proc_group_entities(c, *operand);
			if (procs.count == 1) {
				ptype = procs[0]->type;
				single_case = true;
			}
		} else {
			ptype = proc_type;
		}

		if (single_case) {
			Type *bptype = base_type(ptype);
			if (is_type_proc(bptype)) {
				TypeProc *pt = &bptype->Proc;
				TypeTuple *param_tuple = nullptr;
				if (pt->params != nullptr) {
					param_tuple = &pt->params->Tuple;
				}
				if (param_tuple != nullptr) {
					for_array(i, param_tuple->variables) {
						Entity *e = param_tuple->variables[i];
						if (is_blank_ident(e->token)) {
							continue;
						}
						string_map_set(&type_hint_map, e->token.string, e->type);
					}
				}
			}
		} else {
			Array<Entity *> procs = proc_group_entities(c, *operand);
			for_array(j, procs) {
				Type *proc_type = base_type(procs[j]->type);
				if (is_type_proc(proc_type)) {
					TypeProc *pt = &proc_type->Proc;
					TypeTuple *param_tuple = nullptr;
					if (pt->params != nullptr) {
						param_tuple = &pt->params->Tuple;
					}
					if (param_tuple == nullptr) {
						continue;
					}
					for_array(i, param_tuple->variables) {
						Entity *e = param_tuple->variables[i];
						if (is_blank_ident(e->token)) {
							continue;
						}
						StringHashKey key = string_hash_string(e->token.string);
						Type **found = string_map_get(&type_hint_map, key);
						if (found) {
							Type *t = *found;
							if (t == nullptr) {
								// NOTE(bill): Ambiguous named parameter across all types
								continue;
							}
							if (are_types_identical(t, e->type)) {
								// NOTE(bill): No need to set again
							} else {
								// NOTE(bill): Ambiguous named parameter across all types so set it to a nullptr
								string_map_set(&type_hint_map, key, cast(Type *)nullptr);
							}
						} else {
							string_map_set(&type_hint_map, key, e->type);
						}
					}
				}
			}

		}


		for_array(i, args) {
			Ast *arg = args[i];
			ast_node(fv, FieldValue, arg);
			Ast *field = fv->field;

			Type *type_hint = nullptr;

			if (field != nullptr && field->kind == Ast_Ident) {
				String key = field->Ident.token.string;
				Type **found = string_map_get(&type_hint_map, key);
				if (found) {
					type_hint = *found;
				}
			}
			check_expr_or_type(c, &operands[i], fv->value, type_hint);
		}
	} else {
		operands = array_make<Operand>(heap_allocator(), 0, 2*args.count);
		Entity **lhs = nullptr;
		isize lhs_count = -1;
		bool is_variadic = false;
		if (proc_type != nullptr && is_type_proc(proc_type)) {
			lhs = populate_proc_parameter_list(c, proc_type, &lhs_count, &is_variadic);
		}
		if (operand->mode != Addressing_ProcGroup) {
			check_unpack_arguments(c, lhs, lhs_count, &operands, args, false, is_variadic);
		}
	}

	if (operand->mode == Addressing_ProcGroup) {
		check_entity_decl(c, operand->proc_group, nullptr, nullptr);

		Array<Entity *> procs = proc_group_entities(c, *operand);

		if (procs.count == 1) {
			Ast *ident = operand->expr;
			while (ident->kind == Ast_SelectorExpr) {
				Ast *s = ident->SelectorExpr.selector;
				ident = s;
			}

			Entity *e = procs[0];

			Entity **lhs = nullptr;
			isize lhs_count = -1;
			bool is_variadic = false;
			lhs = populate_proc_parameter_list(c, e->type, &lhs_count, &is_variadic);
			check_unpack_arguments(c, lhs, lhs_count, &operands, args, false, is_variadic);

			CallArgumentData data = {};
			CallArgumentError err = call_checker(c, call, e->type, e, operands, CallArgumentMode_ShowErrors, &data);
			if (err != CallArgumentError_None) {
				// handle error
			}
			Entity *entity_to_use = data.gen_entity != nullptr ? data.gen_entity : e;
			add_entity_use(c, ident, entity_to_use);
			if (entity_to_use != nullptr) {
				update_untyped_expr_type(c, operand->expr, entity_to_use->type, true);
			}
			return data;
		}

		Entity **lhs = nullptr;
		isize lhs_count = -1;

		{
			// NOTE(bill, 2019-07-13): This code is used to improve the type inference for procedure groups
			// where the same positional parameter has the same type value (and ellipsis)
			bool proc_arg_count_all_equal = true;
			isize proc_arg_count = -1;
			for_array(i, procs) {
				Entity *p = procs[i];
				Type *pt = base_type(p->type);
				if (pt != nullptr && is_type_proc(pt)) {
					if (proc_arg_count < 0) {
						proc_arg_count = pt->Proc.param_count;
					} else {
						if (proc_arg_count != pt->Proc.param_count) {
							proc_arg_count_all_equal = false;
							break;
						}
					}
				}
			}



			if (proc_arg_count >= 0 && proc_arg_count_all_equal) {
				lhs_count = proc_arg_count;
				if (lhs_count > 0)  {
					lhs = gb_alloc_array(heap_allocator(), Entity *, lhs_count);
					for (isize param_index = 0; param_index < lhs_count; param_index++) {
						Entity *e = nullptr;
						for_array(j, procs) {
							Entity *p = procs[j];
							Type *pt = base_type(p->type);
							if (pt != nullptr && is_type_proc(pt)) {
								if (e == nullptr) {
									e = pt->Proc.params->Tuple.variables[param_index];
								} else {
									Entity *f = pt->Proc.params->Tuple.variables[param_index];
									if (e == f) {
										continue;
									}
									if (are_types_identical(e->type, f->type)) {
										bool ee = (e->flags & EntityFlag_Ellipsis) != 0;
										bool fe = (f->flags & EntityFlag_Ellipsis) != 0;
										if (ee == fe) {
											continue;
										}
									}
									// NOTE(bill): Entities are not close enough to be used
									e = nullptr;
									break;
								}
							}
						}
						lhs[param_index] = e;
					}
				}
			}
		}


		check_unpack_arguments(c, lhs, lhs_count, &operands, args, false, false);

		if (lhs != nullptr) {
			gb_free(heap_allocator(), lhs);
		}

		auto valids = array_make<ValidIndexAndScore>(heap_allocator(), 0, procs.count);
		defer (array_free(&valids));

		auto proc_entities = array_make<Entity *>(heap_allocator(), 0, procs.count*2 + 1);
		defer (array_free(&proc_entities));
		for_array(i, procs) {
			array_add(&proc_entities, procs[i]);
		}


		gbString expr_name = expr_to_string(operand->expr);
		defer (gb_string_free(expr_name));

		for_array(i, procs) {
			Entity *p = procs[i];
			Type *pt = base_type(p->type);
			if (pt != nullptr && is_type_proc(pt)) {
				CallArgumentError err = CallArgumentError_None;
				CallArgumentData data = {};
				CheckerContext ctx = *c;

				ctx.no_polymorphic_errors = true;
				ctx.allow_polymorphic_types = is_type_polymorphic(pt);
				ctx.hide_polymorphic_errors = true;

				err = call_checker(&ctx, call, pt, p, operands, CallArgumentMode_NoErrors, &data);
				if (err != CallArgumentError_None) {
					continue;
				}
				isize index = i;

				if (data.gen_entity != nullptr) {
					Entity *e = data.gen_entity;
					DeclInfo *decl = data.gen_entity->decl_info;
					ctx.scope = decl->scope;
					ctx.decl = decl;
					ctx.proc_name = e->token.string;
					ctx.curr_proc_decl = decl;
					ctx.curr_proc_sig  = e->type;

					GB_ASSERT(decl->proc_lit->kind == Ast_ProcLit);
					if (!evaluate_where_clauses(&ctx, call, decl->scope, &decl->proc_lit->ProcLit.where_clauses, false)) {
						continue;
					}

					array_add(&proc_entities, data.gen_entity);
					index = proc_entities.count-1;
				}

				ValidIndexAndScore item = {};
				item.index = index;
				item.score = data.score;
				array_add(&valids, item);
			}
		}

		if (valids.count > 1) {
			gb_sort_array(valids.data, valids.count, valid_index_and_score_cmp);
			i64 best_score = valids[0].score;
			Entity *best_entity = proc_entities[valids[0].index];
			GB_ASSERT(best_entity != nullptr);
			for (isize i = 1; i < valids.count; i++) {
				if (best_score > valids[i].score) {
					valids.count = i;
					break;
				}
				if (best_entity == proc_entities[valids[i].index]) {
					valids.count = i;
					break;
				}
			}
		}


		if (valids.count == 0) {
			begin_error_block();
			defer (end_error_block());

			error(operand->expr, "No procedures or ambiguous call for procedure group '%s' that match with the given arguments", expr_name);
			if (operands.count == 0) {
				error_line("\tNo given arguments\n");
			} else {
				error_line("\tGiven argument types: (");
				for_array(i, operands) {
					Operand o = operands[i];
					if (i > 0) error_line(", ");
					gbString type = type_to_string(o.type);
					defer (gb_string_free(type));
					error_line("%s", type);
				}
				error_line(")\n");
			}

			if (procs.count > 0) {
				error_line("Did you mean to use one of the following:\n");
			}
			for_array(i, procs) {
				Entity *proc = procs[i];
				TokenPos pos = proc->token.pos;
				Type *t = base_type(proc->type);
				if (t == t_invalid) continue;
				GB_ASSERT(t->kind == Type_Proc);
				gbString pt;
				defer (gb_string_free(pt));
				if (t->Proc.node != nullptr) {
					pt = expr_to_string(t->Proc.node);
				} else {
					pt = type_to_string(t);
				}
				String prefix = {};
				String prefix_sep = {};
				if (proc->pkg) {
					prefix = proc->pkg->name;
					prefix_sep = str_lit(".");
				}
				String name = proc->token.string;

				char const *sep = "::";
				if (proc->kind == Entity_Variable) {
					sep = ":=";
				}
				error_line("\t%.*s%.*s%.*s %s %s at %s\n", LIT(prefix), LIT(prefix_sep), LIT(name), sep, pt, token_pos_to_string(pos));
			}
			if (procs.count > 0) {
				error_line("\n");
			}

			result_type = t_invalid;
		} else if (valids.count > 1) {
			begin_error_block();
			defer (end_error_block());

			error(operand->expr, "Ambiguous procedure group call '%s' that match with the given arguments", expr_name);
			error_line("\tGiven argument types: (");
			for_array(i, operands) {
				Operand o = operands[i];
				if (i > 0) error_line(", ");
				gbString type = type_to_string(o.type);
				defer (gb_string_free(type));
				error_line("%s", type);
			}
			error_line(")\n");

			for (isize i = 0; i < valids.count; i++) {
				Entity *proc = proc_entities[valids[i].index];
				GB_ASSERT(proc != nullptr);
				TokenPos pos = proc->token.pos;
				Type *t = base_type(proc->type); GB_ASSERT(t->kind == Type_Proc);
				gbString pt = nullptr;
				defer (gb_string_free(pt));
				if (t->Proc.node != nullptr) {
					pt = expr_to_string(t->Proc.node);
				} else {
					pt = type_to_string(t);
				}
				String name = proc->token.string;
				char const *sep = "::";
				if (proc->kind == Entity_Variable) {
					sep = ":=";
				}
				error_line("\t%.*s %s %s ", LIT(name), sep, pt);
				if (proc->decl_info->proc_lit != nullptr) {
					GB_ASSERT(proc->decl_info->proc_lit->kind == Ast_ProcLit);
					auto *pl = &proc->decl_info->proc_lit->ProcLit;
					if (pl->where_token.kind != Token_Invalid) {
						error_line("\n\t\twhere ");
						for_array(j, pl->where_clauses) {
							Ast *clause = pl->where_clauses[j];
							if (j != 0) {
								error_line("\t\t      ");
							}
							gbString str = expr_to_string(clause);
							error_line("%s", str);
							gb_string_free(str);

							if (j != pl->where_clauses.count-1) {
								error_line(",");
							}
						}
						error_line("\n\t");
					}
				}
				error_line("at %s\n", token_pos_to_string(pos));
			}
			result_type = t_invalid;
		} else {
			Ast *ident = operand->expr;
			while (ident->kind == Ast_SelectorExpr) {
				Ast *s = ident->SelectorExpr.selector;
				ident = s;
			}

			Entity *e = proc_entities[valids[0].index];
			GB_ASSERT(e != nullptr);

			proc_type = e->type;
			CallArgumentData data = {};
			CallArgumentError err = call_checker(c, call, proc_type, e, operands, CallArgumentMode_ShowErrors, &data);
			gb_unused(err);
			Entity *entity_to_use = data.gen_entity != nullptr ? data.gen_entity : e;
			add_entity_use(c, ident, entity_to_use);
			if (entity_to_use != nullptr) {
				update_untyped_expr_type(c, operand->expr, entity_to_use->type, true);
			}

			if (data.gen_entity != nullptr) {
				Entity *e = data.gen_entity;
				DeclInfo *decl = data.gen_entity->decl_info;
				CheckerContext ctx = *c;
				ctx.scope = decl->scope;
				ctx.decl = decl;
				ctx.proc_name = e->token.string;
				ctx.curr_proc_decl = decl;
				ctx.curr_proc_sig  = e->type;

				GB_ASSERT(decl->proc_lit->kind == Ast_ProcLit);
				bool ok = evaluate_where_clauses(&ctx, call, decl->scope, &decl->proc_lit->ProcLit.where_clauses, true);
				decl->where_clauses_evaluated = true;

				if (ok && (data.gen_entity->flags & EntityFlag_ProcBodyChecked) == 0) {
					check_procedure_later(c, e->file, e->token, decl, e->type, decl->proc_lit->ProcLit.body, decl->proc_lit->ProcLit.tags);
				}
			}
			return data;
		}
	} else {
		Ast *ident = operand->expr;
		while (ident->kind == Ast_SelectorExpr) {
			Ast *s = ident->SelectorExpr.selector;
			ident = s;
		}

		Entity *e = entity_of_node(ident);


		CallArgumentData data = {};
		CallArgumentError err = call_checker(c, call, proc_type, e, operands, CallArgumentMode_ShowErrors, &data);
		gb_unused(err);
		Entity *entity_to_use = data.gen_entity != nullptr ? data.gen_entity : e;
		add_entity_use(c, ident, entity_to_use);
		if (entity_to_use != nullptr) {
			update_untyped_expr_type(c, operand->expr, entity_to_use->type, true);
		}
		if (data.gen_entity != nullptr) {
			Entity *e = data.gen_entity;
			DeclInfo *decl = data.gen_entity->decl_info;
			CheckerContext ctx = *c;
			ctx.scope = decl->scope;
			ctx.decl = decl;
			ctx.proc_name = e->token.string;
			ctx.curr_proc_decl = decl;
			ctx.curr_proc_sig  = e->type;

			GB_ASSERT(decl->proc_lit->kind == Ast_ProcLit);
			bool ok = evaluate_where_clauses(&ctx, call, decl->scope, &decl->proc_lit->ProcLit.where_clauses, true);
			decl->where_clauses_evaluated = true;

			if (ok && (data.gen_entity->flags & EntityFlag_ProcBodyChecked) == 0) {
				check_procedure_later(c, e->file, e->token, decl, e->type, decl->proc_lit->ProcLit.body, decl->proc_lit->ProcLit.tags);
			}
		}
		return data;
	}


	CallArgumentData data = {};
	data.result_type = t_invalid;
	return data;
}


isize lookup_polymorphic_record_parameter(Type *t, String parameter_name) {
	if (!is_type_polymorphic_record(t)) {
		return -1;
	}

	TypeTuple *params = get_record_polymorphic_params(t);
	if (params == nullptr) {
		return -1;
	}
	for_array(i, params->variables) {
		Entity *e = params->variables[i];
		String name = e->token.string;
		if (is_blank_ident(name)) {
			continue;
		}
		if (name == parameter_name) {
			return i;
		}
	}
	return -1;
}


CallArgumentError check_polymorphic_record_type(CheckerContext *c, Operand *operand, Ast *call) {
	ast_node(ce, CallExpr, call);

	Type *original_type = operand->type;
	GB_ASSERT(is_type_polymorphic_record(original_type));

	bool show_error = true;

	Array<Operand> operands = {};
	defer (array_free(&operands));

	bool named_fields = false;
	{
		// NOTE(bill, 2019-10-26): Allow a cycle in the parameters but not in the fields themselves
		auto prev_type_path = c->type_path;
		c->type_path = new_checker_type_path();
		defer ({
			destroy_checker_type_path(c->type_path);
			c->type_path = prev_type_path;
		});

		if (is_call_expr_field_value(ce)) {
			named_fields = true;
			operands = array_make<Operand>(heap_allocator(), ce->args.count);
			for_array(i, ce->args) {
				Ast *arg = ce->args[i];
				ast_node(fv, FieldValue, arg);

				if (fv->field->kind == Ast_Ident) {
					String name = fv->field->Ident.token.string;
					isize index = lookup_polymorphic_record_parameter(original_type, name);
					if (index >= 0) {
						TypeTuple *params = get_record_polymorphic_params(original_type);
						Entity *e = params->variables[i];
						if (e->kind == Entity_Constant) {
							check_expr_with_type_hint(c, &operands[i], fv->value, e->type);
							continue;
						}
					}

				}
				check_expr_or_type(c, &operands[i], fv->value);
			}

			bool vari_expand = (ce->ellipsis.pos.line != 0);
			if (vari_expand) {
				error(ce->ellipsis, "Invalid use of '..' in a polymorphic type call'");
			}

		} else {
			operands = array_make<Operand>(heap_allocator(), 0, 2*ce->args.count);

			Entity **lhs = nullptr;
			isize lhs_count = -1;

			TypeTuple *params = get_record_polymorphic_params(original_type);
			if (params != nullptr) {
				lhs = params->variables.data;
				lhs_count = params->variables.count;
			}

			check_unpack_arguments(c, lhs, lhs_count, &operands, ce->args, false, false);
		}

	}

	CallArgumentError err = CallArgumentError_None;

	TypeTuple *tuple = get_record_polymorphic_params(original_type);
	isize param_count = tuple->variables.count;
	isize minimum_param_count = param_count;
	for (; minimum_param_count > 0; minimum_param_count--) {
		Entity *e = tuple->variables[minimum_param_count-1];
		if (e->kind != Entity_Constant) {
			break;
		}
		if (e->Constant.param_value.kind == ParameterValue_Invalid) {
			break;
		}
	}


	Array<Operand> ordered_operands = operands;
	if (!named_fields) {
		ordered_operands = array_make<Operand>(permanent_allocator(), param_count);
		array_copy(&ordered_operands, operands, 0);
	} else {
		bool *visited = gb_alloc_array(temporary_allocator(), bool, param_count);

		// LEAK(bill)
		ordered_operands = array_make<Operand>(permanent_allocator(), param_count);

		for_array(i, ce->args) {
			Ast *arg = ce->args[i];
			ast_node(fv, FieldValue, arg);
			if (fv->field->kind != Ast_Ident) {
				if (show_error) {
					gbString expr_str = expr_to_string(fv->field);
					error(arg, "Invalid parameter name '%s' in polymorphic type call", expr_str);
					gb_string_free(expr_str);
				}
				err = CallArgumentError_InvalidFieldValue;
				continue;
			}
			String name = fv->field->Ident.token.string;
			isize index = lookup_polymorphic_record_parameter(original_type, name);
			if (index < 0) {
				if (show_error) {
					error(arg, "No parameter named '%.*s' for this polymorphic type", LIT(name));
				}
				err = CallArgumentError_ParameterNotFound;
				continue;
			}
			if (visited[index]) {
				if (show_error) {
					error(arg, "Duplicate parameter '%.*s' in polymorphic type", LIT(name));
				}
				err = CallArgumentError_DuplicateParameter;
				continue;
			}

			visited[index] = true;
			ordered_operands[index] = operands[i];
		}

		for (isize i = 0; i < param_count; i++) {
			if (!visited[i]) {
				Entity *e = tuple->variables[i];
				if (is_blank_ident(e->token)) {
					continue;
				}

				if (show_error) {
					if (e->kind == Entity_TypeName) {
						error(call, "Type parameter '%.*s' is missing in polymorphic type call",
						      LIT(e->token.string));
					} else {
						gbString str = type_to_string(e->type);
						error(call, "Parameter '%.*s' of type '%s' is missing in polymorphic type call",
						      LIT(e->token.string), str);
						gb_string_free(str);
					}
				}
				err = CallArgumentError_ParameterMissing;
			}
		}
	}

	if (err != 0) {
		operand->mode = Addressing_Invalid;
		return err;
	}

	while (ordered_operands.count > 0) {
		if (ordered_operands[ordered_operands.count-1].expr != nullptr) {
			break;
		}
		array_pop(&ordered_operands);
	}

	if (minimum_param_count != param_count) {
		if (param_count < ordered_operands.count) {
			error(call, "Too many polymorphic type arguments, expected a maximum of %td, got %td", param_count, ordered_operands.count);
			err = CallArgumentError_TooManyArguments;
		} else if (minimum_param_count > ordered_operands.count) {
			error(call, "Too few polymorphic type arguments, expected a minimum of %td, got %td", minimum_param_count, ordered_operands.count);
			err = CallArgumentError_TooFewArguments;
		}
	} else {
		if (param_count < ordered_operands.count) {
			error(call, "Too many polymorphic type arguments, expected %td, got %td", param_count, ordered_operands.count);
			err = CallArgumentError_TooManyArguments;
		} else if (param_count > ordered_operands.count) {
			error(call, "Too few polymorphic type arguments, expected %td, got %td", param_count, ordered_operands.count);
			err = CallArgumentError_TooFewArguments;
		}
	}

	if (err != 0) {
		return err;
	}

	if (minimum_param_count != param_count) {
		array_resize(&ordered_operands, param_count);

		isize missing_count = 0;
		// NOTE(bill): Replace missing operands with the default values (if possible)
		for_array(i, ordered_operands) {
			Operand *o = &ordered_operands[i];
			if (o->expr == nullptr) {
				Entity *e = tuple->variables[i];
				if (e->kind == Entity_Constant) {
					missing_count += 1;
					o->mode = Addressing_Constant;
					o->type = default_type(e->type);
					o->expr = unparen_expr(e->Constant.param_value.original_ast_expr);
					if (e->Constant.param_value.kind == ParameterValue_Constant) {
						o->value = e->Constant.param_value.value;
					}
				} else if (e->kind == Entity_TypeName) {
					missing_count += 1;
					o->mode = Addressing_Type;
					o->type = e->type;
					o->expr = e->identifier;
				}
			}
		}
	}

	isize oo_count = gb_min(param_count, ordered_operands.count);
	i64 score = 0;
	for (isize i = 0; i < oo_count; i++) {
		Entity *e = tuple->variables[i];
		Operand *o = &ordered_operands[i];
		if (o->mode == Addressing_Invalid) {
			continue;
		}
		if (e->kind == Entity_TypeName) {
			if (o->mode != Addressing_Type) {
				if (show_error) {
					error(o->expr, "Expected a type for the argument '%.*s'", LIT(e->token.string));
				}
				err = CallArgumentError_WrongTypes;
			}
			if (are_types_identical(e->type, o->type)) {
				score += assign_score_function(1);
			} else {
				score += assign_score_function(MAXIMUM_TYPE_DISTANCE);
			}
		} else {
			i64 s = 0;
			if (o->type->kind == Type_Generic) {
				// Polymorphic name!
				score += assign_score_function(1);
				continue;
			} else if (!check_is_assignable_to_with_score(c, o, e->type, &s)) {
				if (show_error) {
					check_assignment(c, o, e->type, str_lit("polymorphic type argument"));
				}
				err = CallArgumentError_WrongTypes;
			}
			o->type = e->type;
			if (o->mode != Addressing_Constant) {
				bool valid = false;
				if (is_type_proc(o->type)) {
					Entity *proc_entity = entity_from_expr(o->expr);
					valid = proc_entity != nullptr;
				}
				if (!valid) {
					if (show_error) {
						error(o->expr, "Expected a constant value for this polymorphic type argument");
					}
					err = CallArgumentError_NoneConstantParameter;
				}
			}
			score += s;
		}

		// NOTE(bill): Add type info the parameters
		// TODO(bill, 2022-01-23): why was this line added in the first place? I'm commenting it out for the time being
		// add_type_info_type(c, o->type);
	}

	{
		bool failure = false;
		Entity *found_entity = find_polymorphic_record_entity(c, original_type, param_count, ordered_operands, &failure);
		if (found_entity) {
			operand->mode = Addressing_Type;
			operand->type = found_entity->type;
			return err;
		}

		String generated_name = make_string_c(expr_to_string(call));

		CheckerContext ctx = *c;
		// NOTE(bill): We need to make sure the lookup scope for the record is the same as where it was created
		ctx.scope = polymorphic_record_parent_scope(original_type);
		GB_ASSERT(ctx.scope != nullptr);

		Type *named_type = alloc_type_named(generated_name, nullptr, nullptr);
		Type *bt = base_type(original_type);
		if (bt->kind == Type_Struct) {
			Ast *node = clone_ast(bt->Struct.node);
			Type *struct_type = alloc_type_struct();
			struct_type->Struct.node = node;
			struct_type->Struct.polymorphic_parent = original_type;
			set_base_type(named_type, struct_type);

			check_open_scope(&ctx, node);
			check_struct_type(&ctx, struct_type, node, &ordered_operands, named_type, original_type);
			check_close_scope(&ctx);
		} else if (bt->kind == Type_Union) {
			Ast *node = clone_ast(bt->Union.node);
			Type *union_type = alloc_type_union();
			union_type->Union.node = node;
			union_type->Union.polymorphic_parent = original_type;
			set_base_type(named_type, union_type);

			check_open_scope(&ctx, node);
			check_union_type(&ctx, union_type, node, &ordered_operands, named_type, original_type);
			check_close_scope(&ctx);
		} else {
			GB_PANIC("Unsupported parametric polymorphic record type");
		}

		operand->mode = Addressing_Type;
		operand->type = named_type;
	}
	return err;
}




ExprKind check_call_expr(CheckerContext *c, Operand *operand, Ast *call, Ast *proc, Slice<Ast *> const &args, ProcInlining inlining, Type *type_hint) {
	if (proc != nullptr &&
	    proc->kind == Ast_BasicDirective) {
		ast_node(bd, BasicDirective, proc);
		String name = bd->name.string;
		if (
		    name == "location" || 
		    name == "assert" || 
		    name == "panic" || 
		    name == "defined" || 
		    name == "config" || 
		    name == "load" ||
		    name == "load_hash" ||
		    name == "load_or"
		) {
			operand->mode = Addressing_Builtin;
			operand->builtin_id = BuiltinProc_DIRECTIVE;
			operand->expr = proc;
			operand->type = t_invalid;
			add_type_and_value(c->info, proc, operand->mode, operand->type, operand->value);
		} else {
			error(proc, "Unknown directive: #%.*s", LIT(name));
			operand->expr = proc;
			operand->type = t_invalid;
			operand->mode = Addressing_Invalid;
			return Expr_Expr;
		}
		if (inlining != ProcInlining_none) {
			error(call, "Inlining operators are not allowed on built-in procedures");
		}
	} else {
		if (proc != nullptr) {
			check_expr_or_type(c, operand, proc);
		} else {
			GB_ASSERT(operand->expr != nullptr);
		}
	}

	if (args.count > 0) {
		bool fail = false;
		bool first_is_field_value = (args[0]->kind == Ast_FieldValue);
		for_array(i, args) {
			Ast *arg = args[i];
			bool mix = false;
			if (first_is_field_value) {
				mix = arg->kind != Ast_FieldValue;
			} else {
				mix = arg->kind == Ast_FieldValue;
			}
			if (mix) {
				error(arg, "Mixture of 'field = value' and value elements in a procedure call is not allowed");
				fail = true;
			}
		}

		if (fail) {
			operand->mode = Addressing_Invalid;
			operand->expr = call;
			return Expr_Stmt;
		}
	}

	if (operand->mode == Addressing_Invalid) {
		for_array(i, args) {
			Ast *arg = args[i];
			if (arg->kind == Ast_FieldValue) {
				arg = arg->FieldValue.value;
			}
			check_expr_base(c, operand, arg, nullptr);
		}
		operand->mode = Addressing_Invalid;
		operand->expr = call;
		return Expr_Stmt;
	}

	if (operand->mode == Addressing_Type) {
		Type *t = operand->type;
		if (is_type_polymorphic_record(t)) {
			if (!is_type_named(t)) {
				gbString s = expr_to_string(operand->expr);
				error(call, "Illegal use of an unnamed polymorphic record, %s", s);
				gb_string_free(s);
				operand->mode = Addressing_Invalid;
				operand->type = t_invalid;;
				return Expr_Expr;
			}
			auto err = check_polymorphic_record_type(c, operand, call);
			if (err == 0) {
				Ast *ident = operand->expr;
				while (ident->kind == Ast_SelectorExpr) {
					Ast *s = ident->SelectorExpr.selector;
					ident = s;
				}
				Type *ot = operand->type;
				GB_ASSERT(ot->kind == Type_Named);
				Entity *e = ot->Named.type_name;
				add_entity_use(c, ident, e);
				add_type_and_value(c->info, call, Addressing_Type, ot, empty_exact_value);
			} else {
				operand->mode = Addressing_Invalid;
				operand->type = t_invalid;
			}
		} else {
			gbString str = type_to_string(t);
			defer (gb_string_free(str));

			operand->mode = Addressing_Invalid;
			isize arg_count = args.count;
			switch (arg_count) {
			case 0:  error(call, "Missing argument in conversion to '%s'", str);   break;
			default: error(call, "Too many arguments in conversion to '%s'", str); break;
			case 1: {
				Ast *arg = args[0];
				if (arg->kind == Ast_FieldValue) {
					error(call, "'field = value' cannot be used in a type conversion");
					arg = arg->FieldValue.value;
					// NOTE(bill): Carry on the cast regardless
				}
				check_expr_with_type_hint(c, operand, arg, t);
				if (operand->mode != Addressing_Invalid) {
					if (is_type_polymorphic(t)) {
						error(call, "A polymorphic type cannot be used in a type conversion");
					} else {
						// NOTE(bill): Otherwise the compiler can override the polymorphic type
						// as it assumes it is determining the type
						check_cast(c, operand, t);
					}
				}
				operand->type = t;
				operand->expr = call;
				if (operand->mode != Addressing_Invalid) {
					update_untyped_expr_type(c, arg, t, false);
				}
				break;
			}
			}
		}
		return Expr_Expr;
	}

	if (operand->mode == Addressing_Builtin) {
		i32 id = operand->builtin_id;
		if (!check_builtin_procedure(c, operand, call, id, type_hint)) {
			operand->mode = Addressing_Invalid;
			operand->type = t_invalid;
		}
		operand->expr = call;
		return builtin_procs[id].kind;
	}

	Entity *initial_entity = entity_of_node(operand->expr);

	if (initial_entity != nullptr && initial_entity->kind == Entity_Procedure) {
		if (initial_entity->Procedure.deferred_procedure.entity != nullptr) {
			call->viral_state_flags |= ViralStateFlag_ContainsDeferredProcedure;
		}
	}

	Type *proc_type = base_type(operand->type);
	if (operand->mode != Addressing_ProcGroup) {
		bool valid_type = (proc_type != nullptr) && is_type_proc(proc_type);
		bool valid_mode = is_operand_value(*operand);
		if (!valid_type || !valid_mode) {
			Ast *e = operand->expr;
			gbString str = expr_to_string(e);
			gbString type_str = type_to_string(operand->type);
			error(e, "Cannot call a non-procedure: '%s' of type '%s'", str, type_str);
			gb_string_free(type_str);
			gb_string_free(str);

			operand->mode = Addressing_Invalid;
			operand->expr = call;

			return Expr_Stmt;
		}
	}

	CallArgumentData data = check_call_arguments(c, operand, proc_type, call, args);
	Type *result_type = data.result_type;
	gb_zero_item(operand);
	operand->expr = call;

	if (result_type == t_invalid) {
		operand->mode = Addressing_Invalid;
		operand->type = t_invalid;
		return Expr_Stmt;
	}

	Type *pt = base_type(proc_type);
	if (pt == t_invalid) {
		if (operand->expr != nullptr && operand->expr->kind == Ast_CallExpr) {
			pt = type_of_expr(operand->expr->CallExpr.proc);
		}
		if (pt == t_invalid && data.gen_entity) {
			pt = data.gen_entity->type;
		}
	}

	if (pt->kind == Type_Proc && pt->Proc.calling_convention == ProcCC_Odin) {
		if ((c->scope->flags & ScopeFlag_ContextDefined) == 0) {
			error(call, "'context' has not been defined within this scope, but is required for this procedure call");
		}
	}

	if (result_type == nullptr) {
		operand->mode = Addressing_NoValue;
	} else {
		GB_ASSERT(is_type_tuple(result_type));
		isize count = result_type->Tuple.variables.count;
		switch (count) {
		case 0:
			operand->mode = Addressing_NoValue;
			break;
		case 1:
			operand->mode = Addressing_Value;
			operand->type = result_type->Tuple.variables[0]->type;
			break;
		default:
			operand->mode = Addressing_Value;
			operand->type = result_type;
			break;
		}
	}

	switch (inlining) {
	case ProcInlining_inline:
		if (proc != nullptr) {
			Entity *e = entity_from_expr(proc);
			if (e != nullptr && e->kind == Entity_Procedure) {
				DeclInfo *decl = e->decl_info;
				if (decl->proc_lit) {
					ast_node(pl, ProcLit, decl->proc_lit);
					if (pl->inlining == ProcInlining_no_inline) {
						error(call, "'inline' cannot be applied to a procedure that has be marked as 'no_inline'");
					}
				}
			}
		}
		break;
	case ProcInlining_no_inline:
		break;
	}

	operand->expr = call;

	{
		if (proc_type == t_invalid) {
			// gb_printf_err("%s\n", expr_to_string(operand->expr));
		}
		Type *type = nullptr;
		if (operand->expr != nullptr && operand->expr->kind == Ast_CallExpr) {
			type = type_of_expr(operand->expr->CallExpr.proc);
		}
		if (type == nullptr) {
			type = pt;
		}
		type = base_type(type);
		if (type->kind == Type_Proc && type->Proc.optional_ok) {
			operand->mode = Addressing_OptionalOk;
			operand->type = type->Proc.results->Tuple.variables[0]->type;
			if (operand->expr != nullptr && operand->expr->kind == Ast_CallExpr) {
				operand->expr->CallExpr.optional_ok_one = true;
			}
		}
	}

	// add_type_and_value(c->info, operand->expr, operand->mode, operand->type, operand->value);

	return Expr_Expr;
}


void check_expr_with_type_hint(CheckerContext *c, Operand *o, Ast *e, Type *t) {
	check_expr_base(c, o, e, t);
	check_not_tuple(c, o);
	char const *err_str = nullptr;
	switch (o->mode) {
	case Addressing_NoValue:
		err_str = "used as a value";
		break;
	case Addressing_Type:
		if (t == nullptr || !is_type_typeid(t)) {
			err_str = "is not an expression but a type, in this context it is ambiguous";
		}
		break;
	case Addressing_Builtin:
		err_str = "must be called";
		break;
	}
	if (err_str != nullptr) {
		gbString str = expr_to_string(e);
		error(e, "'%s' %s", str, err_str);
		gb_string_free(str);
		o->mode = Addressing_Invalid;
	}
}

bool check_set_index_data(Operand *o, Type *t, bool indirection, i64 *max_count, Type *original_type) {
	switch (t->kind) {
	case Type_Basic:
		if (t->Basic.kind == Basic_string) {
			if (o->mode == Addressing_Constant) {
				*max_count = o->value.value_string.len;
			}
			if (o->mode != Addressing_Constant) {
				o->mode = Addressing_Value;
			}
			o->type = t_u8;
			return true;
		} else if (t->Basic.kind == Basic_UntypedString) {
			if (o->mode == Addressing_Constant) {
				*max_count = o->value.value_string.len;
				o->type = t_u8;
				return true;
			}
			return false;
		}
		break;

	case Type_MultiPointer:
		o->type = t->MultiPointer.elem;
		if (o->mode != Addressing_Constant) {
			o->mode = Addressing_Variable;
		}
		return true;

	case Type_Array:
		*max_count = t->Array.count;
		if (indirection) {
			o->mode = Addressing_Variable;
		} else if (o->mode != Addressing_Variable &&
		           o->mode != Addressing_Constant) {
			o->mode = Addressing_Value;
		}
		o->type = t->Array.elem;
		return true;

	case Type_EnumeratedArray:
		*max_count = t->EnumeratedArray.count;
		if (indirection) {
			o->mode = Addressing_Variable;
		} else if (o->mode != Addressing_Variable &&
		           o->mode != Addressing_Constant) {
			o->mode = Addressing_Value;
		}
		o->type = t->EnumeratedArray.elem;
		return true;
		
	case Type_Matrix:
		*max_count = t->Matrix.column_count;
		if (indirection) {
			o->mode = Addressing_Variable;
		} else if (o->mode != Addressing_Variable) {
			o->mode = Addressing_Value;
		}
		o->type = alloc_type_array(t->Matrix.elem, t->Matrix.row_count);
		return true;

	case Type_Slice:
		o->type = t->Slice.elem;
		if (o->mode != Addressing_Constant) {
			o->mode = Addressing_Variable;
		}
		return true;

	case Type_RelativeSlice:
		{
			Type *slice_type = base_type(t->RelativeSlice.slice_type);
			GB_ASSERT(slice_type->kind == Type_Slice);
			o->type = slice_type->Slice.elem;
			if (o->mode != Addressing_Constant) {
				o->mode = Addressing_Variable;
			}
		}
		return true;

	case Type_DynamicArray:
		o->type = t->DynamicArray.elem;
		if (o->mode != Addressing_Constant) {
			o->mode = Addressing_Variable;
		}
		return true;
	case Type_Struct:
		if (t->Struct.soa_kind != StructSoa_None) {
			if (t->Struct.soa_kind == StructSoa_Fixed) {
				*max_count = t->Struct.soa_count;
			}
			o->type = t->Struct.soa_elem;
			if (o->mode == Addressing_SoaVariable || o->mode == Addressing_Variable) {
				o->mode = Addressing_SoaVariable;
			} else {
				o->mode = Addressing_Value;
			}
			return true;
		}
		return false;
	}

	if (is_type_pointer(original_type) && indirection) {
		Type *ptr = base_type(original_type);
		if (ptr->kind == Type_Pointer && o->mode == Addressing_SoaVariable) {
			o->type = ptr->Pointer.elem;
			o->mode = Addressing_Value;
			return true;
		}
	}

	return false;
}

bool ternary_compare_types(Type *x, Type *y) {
	if (is_type_untyped_undef(x) && type_has_undef(y)) {
		return true;
	} else if (is_type_untyped_nil(x) && type_has_nil(y)) {
		return true;
	} else if (is_type_untyped_undef(y) && type_has_undef(x)) {
		return true;
	} else if (is_type_untyped_nil(y) && type_has_nil(x)) {
		return true;
	}
	return are_types_identical(x, y);
}


bool check_range(CheckerContext *c, Ast *node, Operand *x, Operand *y, ExactValue *inline_for_depth_, Type *type_hint=nullptr) {
	if (!is_ast_range(node)) {
		return false;
	}

	ast_node(ie, BinaryExpr, node);

	check_expr_with_type_hint(c, x, ie->left, type_hint);
	if (x->mode == Addressing_Invalid) {
		return false;
	}
	check_expr_with_type_hint(c, y, ie->right, type_hint);
	if (y->mode == Addressing_Invalid) {
		return false;
	}

	convert_to_typed(c, x, y->type);
	if (x->mode == Addressing_Invalid) {
		return false;
	}
	convert_to_typed(c, y, x->type);
	if (y->mode == Addressing_Invalid) {
		return false;
	}

	convert_to_typed(c, x, default_type(y->type));
	if (x->mode == Addressing_Invalid) {
		return false;
	}
	convert_to_typed(c, y, default_type(x->type));
	if (y->mode == Addressing_Invalid) {
		return false;
	}

	if (!are_types_identical(x->type, y->type)) {
		if (x->type != t_invalid &&
		    y->type != t_invalid) {
			gbString xt = type_to_string(x->type);
			gbString yt = type_to_string(y->type);
			gbString expr_str = expr_to_string(x->expr);
			error(ie->op, "Mismatched types in interval expression '%s' : '%s' vs '%s'", expr_str, xt, yt);
			gb_string_free(expr_str);
			gb_string_free(yt);
			gb_string_free(xt);
		}
		return false;
	}

	Type *type = x->type;
	if (!is_type_integer(type) && !is_type_float(type) && !is_type_pointer(type) && !is_type_enum(type)) {
		error(ie->op, "Only numerical and pointer types are allowed within interval expressions");
		return false;
	}

	if (x->mode == Addressing_Constant &&
	    y->mode == Addressing_Constant) {
		ExactValue a = x->value;
		ExactValue b = y->value;

		GB_ASSERT(are_types_identical(x->type, y->type));

		TokenKind op = Token_Lt;
		switch (ie->op.kind) {
		case Token_Ellipsis:  op = Token_LtEq; break; // ..
		case Token_RangeFull: op = Token_LtEq; break; // ..=
		case Token_RangeHalf: op = Token_Lt;   break; // ..<
		default: error(ie->op, "Invalid range operator"); break;
		}
		bool ok = compare_exact_values(op, a, b);
		if (!ok) {
			// TODO(bill): Better error message
			error(ie->op, "Invalid interval range");
			return false;
		}

		ExactValue inline_for_depth = exact_value_sub(b, a);
		if (ie->op.kind != Token_RangeHalf) {
			inline_for_depth = exact_value_increment_one(inline_for_depth);
		}

		if (inline_for_depth_) *inline_for_depth_ = inline_for_depth;
	} else if (inline_for_depth_ != nullptr) {
		error(ie->op, "Interval expressions must be constant");
		return false;
	}

	add_type_and_value(c->info, ie->left,  x->mode, x->type, x->value);
	add_type_and_value(c->info, ie->right, y->mode, y->type, y->value);

	return true;
}

bool check_is_operand_compound_lit_constant(CheckerContext *c, Operand *o) {
	if (is_operand_nil(*o)) {
		return true;
	}
	Ast *expr = unparen_expr(o->expr);
	if (expr != nullptr) {
		Entity *e = strip_entity_wrapping(entity_from_expr(expr));
		if (e != nullptr && e->kind == Entity_Procedure) {
			return true;
		}
		if (expr->kind == Ast_ProcLit) {
			add_type_and_value(c->info, expr, Addressing_Constant, type_of_expr(expr), exact_value_procedure(expr));
			return true;
		}
	}
	return o->mode == Addressing_Constant;
}


bool attempt_implicit_selector_expr(CheckerContext *c, Operand *o, AstImplicitSelectorExpr *ise, Type *th) {
	if (is_type_enum(th)) {
		Type *enum_type = base_type(th);
		GB_ASSERT(enum_type->kind == Type_Enum);

		String name = ise->selector->Ident.token.string;

		Entity *e = scope_lookup_current(enum_type->Enum.scope, name);
		if (e == nullptr) {
			return false;
		}
		GB_ASSERT(are_types_identical(base_type(e->type), enum_type));
		GB_ASSERT(e->kind == Entity_Constant);
		o->value = e->Constant.value;
		o->mode = Addressing_Constant;
		o->type = e->type;
		return true;
	}
	if (is_type_union(th)) {
		Type *union_type = base_type(th);
		auto operands = array_make<Operand>(temporary_allocator(), 0, union_type->Union.variants.count);

		for_array(i, union_type->Union.variants) {
			Type *vt = union_type->Union.variants[i];

			Operand x = {};
			if (attempt_implicit_selector_expr(c, &x, ise, vt)) {
				array_add(&operands, x);
			}
		}

		if (operands.count == 1) {
			*o = operands[0];
			return true;
		}
	}
	return false;
}

ExprKind check_implicit_selector_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ast_node(ise, ImplicitSelectorExpr, node);

	o->type = t_invalid;
	o->expr = node;
	o->mode = Addressing_Invalid;

	Type *th = type_hint;

	if (th == nullptr) {
		gbString str = expr_to_string(node);
		error(node, "Cannot determine type for implicit selector expression '%s'", str);
		gb_string_free(str);
		return Expr_Expr;
	}
	o->type = th;

	bool ok = attempt_implicit_selector_expr(c, o, ise, th);
	if (!ok) {
		String name = ise->selector->Ident.token.string;

		if (is_type_enum(th)) {
			Type *bt = base_type(th);
			GB_ASSERT(bt->kind == Type_Enum);

			gbString typ = type_to_string(th);
			defer (gb_string_free(typ));
			error(node, "Undeclared name '%.*s' for type '%s'", LIT(name), typ);

			check_did_you_mean_type(name, bt->Enum.fields);
		} else {
			gbString typ = type_to_string(th);
			gbString str = expr_to_string(node);
			error(node, "Invalid type '%s' for implicit selector expression '%s'", typ, str);
			gb_string_free(str);
			gb_string_free(typ);
		}
	}

	o->expr = node;
	return Expr_Expr;
}


void check_promote_optional_ok(CheckerContext *c, Operand *x, Type **val_type_, Type **ok_type_) {
	switch (x->mode) {
	case Addressing_MapIndex:
	case Addressing_OptionalOk:
	case Addressing_OptionalOkPtr:
		if (val_type_) *val_type_ = x->type;
		break;
	default:
		if (ok_type_) *ok_type_ = x->type;
		return;
	}

	Ast *expr = unparen_expr(x->expr);

	if (expr->kind == Ast_CallExpr) {
		Type *pt = base_type(type_of_expr(expr->CallExpr.proc));
		if (is_type_proc(pt)) {
			Type *tuple = pt->Proc.results;
			add_type_and_value(c->info, x->expr, x->mode, tuple, x->value);

			if (pt->Proc.result_count >= 2) {
				if (ok_type_) *ok_type_ = tuple->Tuple.variables[1]->type;
			}
			expr->CallExpr.optional_ok_one = false;
			x->type = tuple;
			return;
		}
	}

	Type *tuple = make_optional_ok_type(x->type);
	if (ok_type_) *ok_type_ = tuple->Tuple.variables[1]->type;
	add_type_and_value(c->info, x->expr, x->mode, tuple, x->value);
	x->type = tuple;
	GB_ASSERT(is_type_tuple(type_of_expr(x->expr)));
}


void check_matrix_index_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ast_node(ie, MatrixIndexExpr, node);
	
	check_expr(c, o, ie->expr);
	node->viral_state_flags |= ie->expr->viral_state_flags;
	if (o->mode == Addressing_Invalid) {
		o->expr = node;
		return;
	}
	
	Type *t = base_type(type_deref(o->type));
	bool is_ptr = is_type_pointer(o->type);
	bool is_const = o->mode == Addressing_Constant;
	
	if (t->kind != Type_Matrix) {
		gbString str = expr_to_string(o->expr);
		gbString type_str = type_to_string(o->type);
		defer (gb_string_free(str));
		defer (gb_string_free(type_str));
		if (is_const) {
			error(o->expr, "Cannot use matrix indexing on constant '%s' of type '%s'", str, type_str);
		} else {
			error(o->expr, "Cannot use matrix indexing on '%s' of type '%s'", str, type_str);
		}
		o->mode = Addressing_Invalid;
		o->expr = node;
		return;
	}
	o->type = t->Matrix.elem;
	if (is_ptr) {
		o->mode = Addressing_Variable;
	} else if (o->mode != Addressing_Variable) {
		o->mode = Addressing_Value;
	}
	
	if (ie->row_index == nullptr) {
		gbString str = expr_to_string(o->expr);
		error(o->expr, "Missing row index for '%s'", str);
		gb_string_free(str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return;
	}
	if (ie->column_index == nullptr) {
		gbString str = expr_to_string(o->expr);
		error(o->expr, "Missing column index for '%s'", str);
		gb_string_free(str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return;
	}
	
	i64 row_count = t->Matrix.row_count;
	i64 column_count = t->Matrix.column_count;
	
	i64 row_index = 0;
	i64 column_index = 0;
	bool row_ok = check_index_value(c, t, false, ie->row_index, row_count, &row_index, nullptr);
	bool column_ok = check_index_value(c, t, false, ie->column_index, column_count, &column_index, nullptr);
	
	
	gb_unused(row_ok);
	gb_unused(column_ok);
}


struct TypeAndToken {
	Type *type;
	Token token;
};

typedef PtrMap<uintptr, TypeAndToken> SeenMap;

void add_constant_switch_case(CheckerContext *ctx, SeenMap *seen, Operand operand, bool use_expr = true) {
	if (operand.mode != Addressing_Constant) {
		return;
	}
	if (operand.value.kind == ExactValue_Invalid) {
		return;
	}

	uintptr key = hash_exact_value(operand.value);
	TypeAndToken *found = map_get(seen, key);
	if (found != nullptr) {
		isize count = multi_map_count(seen, key);
		TypeAndToken *taps = gb_alloc_array(temporary_allocator(), TypeAndToken, count);

		multi_map_get_all(seen, key, taps);
		for (isize i = 0; i < count; i++) {
			TypeAndToken tap = taps[i];
			if (!are_types_identical(operand.type, tap.type)) {
				continue;
			}

			TokenPos pos = tap.token.pos;
			if (use_expr) {
				gbString expr_str = expr_to_string(operand.expr);
				error(operand.expr,
				      "Duplicate case '%s'\n"
				      "\tprevious case at %s",
				      expr_str,
				      token_pos_to_string(pos));
				gb_string_free(expr_str);
			} else {
				error(operand.expr, "Duplicate case found with previous case at %s", token_pos_to_string(pos));
			}
			return;
		}
	}

	TypeAndToken tap = {operand.type, ast_token(operand.expr)};
	multi_map_insert(seen, key, tap);
}


void add_to_seen_map(CheckerContext *ctx, SeenMap *seen, TokenKind upper_op, Operand const &x, Operand const &lhs, Operand const &rhs) {
	if (is_type_enum(x.type)) {
		// TODO(bill): Fix this logic so it's fast!!!

		i64 v0 = exact_value_to_i64(lhs.value);
		i64 v1 = exact_value_to_i64(rhs.value);
		Operand v = {};
		v.mode = Addressing_Constant;
		v.type = x.type;
		v.expr = x.expr;

		Type *bt = base_type(x.type);
		GB_ASSERT(bt->kind == Type_Enum);
		for (i64 vi = v0; vi <= v1; vi++) {
			if (upper_op != Token_LtEq && vi == v1) {
				break;
			}

			bool found = false;
			for_array(j, bt->Enum.fields) {
				Entity *f = bt->Enum.fields[j];
				GB_ASSERT(f->kind == Entity_Constant);

				i64 fv = exact_value_to_i64(f->Constant.value);
				if (fv == vi) {
					found = true;
					break;
				}
			}
			if (found) {
				v.value = exact_value_i64(vi);
				add_constant_switch_case(ctx, seen, v);
			}
		}
	} else {
		add_constant_switch_case(ctx, seen, lhs);
		if (upper_op == Token_LtEq) {
			add_constant_switch_case(ctx, seen, rhs);
		}
	}
}
void add_to_seen_map(CheckerContext *ctx, SeenMap *seen, Operand const &x) {
	add_constant_switch_case(ctx, seen, x);
}

ExprKind check_basic_directive_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ast_node(bd, BasicDirective, node);

	ExprKind kind = Expr_Expr;

	o->mode = Addressing_Constant;
	String name = bd->name.string;
	if (name == "file") {
		o->type = t_untyped_string;
		o->value = exact_value_string(get_file_path_string(bd->token.pos.file_id));
	} else if (name == "line") {
		o->type = t_untyped_integer;
		o->value = exact_value_i64(bd->token.pos.line);
	} else if (name == "procedure") {
		if (c->curr_proc_decl == nullptr) {
			error(node, "#procedure may only be used within procedures");
			o->type = t_untyped_string;
			o->value = exact_value_string(str_lit(""));
		} else {
			o->type = t_untyped_string;
			o->value = exact_value_string(c->proc_name);
		}
	} else if (name == "caller_location") {
		init_core_source_code_location(c->checker);
		error(node, "#caller_location may only be used as a default argument parameter");
		o->type = t_source_code_location;
		o->mode = Addressing_Value;
	} else {
		if (name == "location") {
			init_core_source_code_location(c->checker);
			error(node, "'#%.*s' must be used in a call expression", LIT(name));
			o->type = t_source_code_location;
			o->mode = Addressing_Value;
		} else if (
		    name == "assert" ||
		    name == "defined" ||
		    name == "config" ||
		    name == "load" ||
		    name == "load_hash" ||
		    name == "load_or"
		) {
			error(node, "'#%.*s' must be used as a call", LIT(name));
			o->type = t_invalid;
			o->mode = Addressing_Invalid;
		} else {
			error(node, "Unknown directive: #%.*s", LIT(name));
			o->type = t_invalid;
			o->mode = Addressing_Invalid;
		}

	}
	return kind;
}

ExprKind check_ternary_if_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = Expr_Expr;
	Operand cond = {Addressing_Invalid};
	ast_node(te, TernaryIfExpr, node);
	check_expr(c, &cond, te->cond);
	node->viral_state_flags |= te->cond->viral_state_flags;

	if (cond.mode != Addressing_Invalid && !is_type_boolean(cond.type)) {
		error(te->cond, "Non-boolean condition in ternary if expression");
	}

	Operand x = {Addressing_Invalid};
	Operand y = {Addressing_Invalid};
	check_expr_or_type(c, &x, te->x, type_hint);
	node->viral_state_flags |= te->x->viral_state_flags;

	if (te->y != nullptr) {
		check_expr_or_type(c, &y, te->y, type_hint);
		node->viral_state_flags |= te->y->viral_state_flags;
	} else {
		error(node, "A ternary expression must have an else clause");
		return kind;
	}

	if (x.type == nullptr || x.type == t_invalid ||
	    y.type == nullptr || y.type == t_invalid) {
		return kind;
	}

	convert_to_typed(c, &x, y.type);
	if (x.mode == Addressing_Invalid) {
		return kind;
	}
	convert_to_typed(c, &y, x.type);
	if (y.mode == Addressing_Invalid) {
		x.mode = Addressing_Invalid;
		return kind;
	}

	if (!ternary_compare_types(x.type, y.type)) {
		gbString its = type_to_string(x.type);
		gbString ets = type_to_string(y.type);
		error(node, "Mismatched types in ternary if expression, %s vs %s", its, ets);
		gb_string_free(ets);
		gb_string_free(its);
		return kind;
	}

	o->type = x.type;
	if (is_type_untyped_nil(o->type) || is_type_untyped_undef(o->type)) {
		o->type = y.type;
	}

	o->mode = Addressing_Value;
	o->expr = node;
	if (type_hint != nullptr && is_type_untyped(o->type)) {
		if (check_cast_internal(c, &x, type_hint) &&
		    check_cast_internal(c, &y, type_hint)) {
			convert_to_typed(c, o, type_hint);
			update_untyped_expr_type(c, node, type_hint, !is_type_untyped(type_hint));
		}
	}
	return kind;
}

ExprKind check_ternary_when_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = Expr_Expr;
	Operand cond = {};
	ast_node(te, TernaryWhenExpr, node);
	check_expr(c, &cond, te->cond);
	node->viral_state_flags |= te->cond->viral_state_flags;

	if (cond.mode != Addressing_Constant || !is_type_boolean(cond.type)) {
		error(te->cond, "Expected a constant boolean condition in ternary when expression");
		return kind;
	}

	if (cond.value.value_bool) {
		check_expr_or_type(c, o, te->x, type_hint);
		node->viral_state_flags |= te->x->viral_state_flags;
	} else {
		if (te->y != nullptr) {
			check_expr_or_type(c, o, te->y, type_hint);
			node->viral_state_flags |= te->y->viral_state_flags;
		} else {
			error(node, "A ternary when expression must have an else clause");
			return kind;
		}
	}
	return kind;
}

ExprKind check_or_else_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ast_node(oe, OrElseExpr, node);

	String name = oe->token.string;
	Ast *arg = oe->x;
	Ast *default_value = oe->y;

	Operand x = {};
	Operand y = {};
	check_multi_expr_with_type_hint(c, &x, arg, type_hint);
	if (x.mode == Addressing_Invalid) {
		o->mode = Addressing_Value;
		o->type = t_invalid;
		o->expr = node;
		return Expr_Expr;
	}

	check_multi_expr_with_type_hint(c, &y, default_value, x.type);
	error_operand_no_value(&y);
	if (y.mode == Addressing_Invalid) {
		o->mode = Addressing_Value;
		o->type = t_invalid;
		o->expr = node;
		return Expr_Expr;
	}

	Type *left_type = nullptr;
	Type *right_type = nullptr;
	check_or_else_split_types(c, &x, name, &left_type, &right_type);
	add_type_and_value(&c->checker->info, arg, x.mode, x.type, x.value);

	if (left_type != nullptr) {
		check_assignment(c, &y, left_type, name);
	} else {
		check_or_else_expr_no_value_error(c, name, x, type_hint);
	}

	if (left_type == nullptr) {
		left_type = t_invalid;
	}
	o->mode = Addressing_Value;
	o->type = left_type;
	o->expr = node;
	return Expr_Expr;
}

ExprKind check_or_return_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ast_node(re, OrReturnExpr, node);

	String name = re->token.string;
	Operand x = {};
	check_multi_expr_with_type_hint(c, &x, re->expr, type_hint);
	if (x.mode == Addressing_Invalid) {
		o->mode = Addressing_Value;
		o->type = t_invalid;
		o->expr = node;
		return Expr_Expr;
	}

	Type *left_type = nullptr;
	Type *right_type = nullptr;
	check_or_return_split_types(c, &x, name, &left_type, &right_type);
	add_type_and_value(&c->checker->info, re->expr, x.mode, x.type, x.value);

	if (right_type == nullptr) {
		check_or_else_expr_no_value_error(c, name, x, type_hint);
	} else {
		Type *proc_type = base_type(c->curr_proc_sig);
		GB_ASSERT(proc_type->kind == Type_Proc);
		Type *result_type = proc_type->Proc.results;
		if (result_type == nullptr) {
			error(node, "'%.*s' requires the current procedure to have at least one return value", LIT(name));
		} else {
			GB_ASSERT(result_type->kind == Type_Tuple);

			auto const &vars = result_type->Tuple.variables;
			Type *end_type = vars[vars.count-1]->type;

			if (vars.count > 1) {
				if (!proc_type->Proc.has_named_results) {
					error(node, "'%.*s' within a procedure with more than 1 return value requires that the return values are named, allowing for early return", LIT(name));
				}
			}

			Operand rhs = {};
			rhs.type = right_type;
			rhs.mode = Addressing_Value;

			// TODO(bill): better error message
			if (!check_is_assignable_to(c, &rhs, end_type)) {
				gbString a = type_to_string(right_type);
				gbString b = type_to_string(end_type);
				gbString ret_type = type_to_string(result_type);
				error(node, "Cannot assign end value of type '%s' to '%s' in '%.*s'", a, b, LIT(name));
				if (vars.count == 1) {
					error_line("\tProcedure return value type: %s\n", ret_type);
				} else {
					error_line("\tProcedure return value types: (%s)\n", ret_type);
				}
				gb_string_free(ret_type);
				gb_string_free(b);
				gb_string_free(a);
			}
		}
	}

	o->expr = node;
	o->type = left_type;
	if (left_type != nullptr) {
		o->mode = Addressing_Value;
	} else {
		o->mode = Addressing_NoValue;
	}

	if (c->curr_proc_sig == nullptr) {
		error(node, "'%.*s' can only be used within a procedure", LIT(name));
	}

	if (c->in_defer) {
		error(node, "'or_return' cannot be used within a defer statement");
	}

	return Expr_Expr;
}

ExprKind check_compound_literal(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = Expr_Expr;
	ast_node(cl, CompoundLit, node);

	Type *type = type_hint;
	if (type != nullptr && is_type_untyped(type)) {
		type = nullptr;
	}
	bool is_to_be_determined_array_count = false;
	bool is_constant = true;
	if (cl->type != nullptr) {
		type = nullptr;

		// [?]Type
		if (cl->type->kind == Ast_ArrayType && cl->type->ArrayType.count != nullptr) {
			Ast *count = cl->type->ArrayType.count;
			if (count->kind == Ast_UnaryExpr &&
			    count->UnaryExpr.op.kind == Token_Question) {
				type = alloc_type_array(check_type(c, cl->type->ArrayType.elem), -1);
				is_to_be_determined_array_count = true;
			}
			if (cl->elems.count > 0) {
				if (cl->type->ArrayType.tag != nullptr) {
					Ast *tag = cl->type->ArrayType.tag;
					GB_ASSERT(tag->kind == Ast_BasicDirective);
					String name = tag->BasicDirective.name.string;
					if (name == "soa") {
						error(node, "#soa arrays are not supported for compound literals");
						return kind;
					}
				}
			}
		}
		if (cl->type->kind == Ast_DynamicArrayType && cl->type->DynamicArrayType.tag != nullptr) {
			if (cl->elems.count > 0) {
				Ast *tag = cl->type->DynamicArrayType.tag;
				GB_ASSERT(tag->kind == Ast_BasicDirective);
				String name = tag->BasicDirective.name.string;
				if (name == "soa") {
					error(node, "#soa arrays are not supported for compound literals");
					return kind;
				}
			}
		}

		if (type == nullptr) {
			type = check_type(c, cl->type);
		}
	}

	if (type == nullptr) {
		error(node, "Missing type in compound literal");
		return kind;
	}


	Type *t = base_type(type);
	if (is_type_polymorphic(t)) {
		gbString str = type_to_string(type);
		error(node, "Cannot use a polymorphic type for a compound literal, got '%s'", str);
		o->expr = node;
		o->type = type;
		gb_string_free(str);
		return kind;
	}


	switch (t->kind) {
	case Type_Struct: {
		if (cl->elems.count == 0) {
			break; // NOTE(bill): No need to init
		}
		if (t->Struct.is_raw_union) {
			if (cl->elems.count > 0) {
				// NOTE: unions cannot be constant
				is_constant = false;

				if (cl->elems[0]->kind != Ast_FieldValue) {
					gbString type_str = type_to_string(type);
					error(node, "%s ('struct #raw_union') compound literals are only allowed to contain 'field = value' elements", type_str);
					gb_string_free(type_str);
				} else {
					if (cl->elems.count != 1) {
						gbString type_str = type_to_string(type);
						error(node, "%s ('struct #raw_union') compound literals are only allowed to contain up to 1 'field = value' element, got %td", type_str, cl->elems.count);
						gb_string_free(type_str);
					} else {
						Ast *elem = cl->elems[0];
						ast_node(fv, FieldValue, elem);
						if (fv->field->kind != Ast_Ident) {
							gbString expr_str = expr_to_string(fv->field);
							error(elem, "Invalid field name '%s' in structure literal", expr_str);
							gb_string_free(expr_str);
							break;
						}

						String name = fv->field->Ident.token.string;

						Selection sel = lookup_field(type, name, o->mode == Addressing_Type);
						bool is_unknown = sel.entity == nullptr;
						if (is_unknown) {
							error(elem, "Unknown field '%.*s' in structure literal", LIT(name));
							break;
						}

						if (sel.index.count > 1) {
							error(elem, "Cannot assign to an anonymous field '%.*s' in a structure literal (at the moment)", LIT(name));
							break;
						}

						Entity *field = t->Struct.fields[sel.index[0]];
						add_entity_use(c, fv->field, field);

						Operand o = {};
						check_expr_or_type(c, &o, fv->value, field->type);


						check_assignment(c, &o, field->type, str_lit("structure literal"));
					}

				}
			}
			break;
		}


		isize field_count = t->Struct.fields.count;
		isize min_field_count = t->Struct.fields.count;
		for (isize i = min_field_count-1; i >= 0; i--) {
			Entity *e = t->Struct.fields[i];
			GB_ASSERT(e->kind == Entity_Variable);
			if (e->Variable.param_value.kind != ParameterValue_Invalid) {
				min_field_count--;
			} else {
				break;
			}
		}

		if (cl->elems[0]->kind == Ast_FieldValue) {
			bool *fields_visited = gb_alloc_array(temporary_allocator(), bool, field_count);

			for_array(i, cl->elems) {
				Ast *elem = cl->elems[i];
				if (elem->kind != Ast_FieldValue) {
					error(elem, "Mixture of 'field = value' and value elements in a literal is not allowed");
					continue;
				}
				ast_node(fv, FieldValue, elem);
				if (fv->field->kind != Ast_Ident) {
					gbString expr_str = expr_to_string(fv->field);
					error(elem, "Invalid field name '%s' in structure literal", expr_str);
					gb_string_free(expr_str);
					continue;
				}
				String name = fv->field->Ident.token.string;

				Selection sel = lookup_field(type, name, o->mode == Addressing_Type);
				bool is_unknown = sel.entity == nullptr;
				if (is_unknown) {
					error(elem, "Unknown field '%.*s' in structure literal", LIT(name));
					continue;
				}

				if (sel.index.count > 1) {
					error(elem, "Cannot assign to an anonymous field '%.*s' in a structure literal (at the moment)", LIT(name));
					continue;
				}

				Entity *field = t->Struct.fields[sel.index[0]];
				add_entity_use(c, fv->field, field);

				if (fields_visited[sel.index[0]]) {
					error(elem, "Duplicate field '%.*s' in structure literal", LIT(name));
					continue;
				}

				fields_visited[sel.index[0]] = true;

				Operand o = {};
				check_expr_or_type(c, &o, fv->value, field->type);

				if (is_type_any(field->type) || is_type_union(field->type) || is_type_raw_union(field->type) || is_type_typeid(field->type)) {
					is_constant = false;
				}
				if (is_constant) {
					is_constant = check_is_operand_compound_lit_constant(c, &o);
				}

				check_assignment(c, &o, field->type, str_lit("structure literal"));
			}
		} else {
			bool seen_field_value = false;

			for_array(index, cl->elems) {
				Entity *field = nullptr;
				Ast *elem = cl->elems[index];
				if (elem->kind == Ast_FieldValue) {
					seen_field_value = true;
					error(elem, "Mixture of 'field = value' and value elements in a literal is not allowed");
					continue;
				} else if (seen_field_value) {
					error(elem, "Value elements cannot be used after a 'field = value'");
					continue;
				}
				if (index >= field_count) {
					error(elem, "Too many values in structure literal, expected %td, got %td", field_count, cl->elems.count);
					break;
				}

				if (field == nullptr) {
					field = t->Struct.fields[index];
				}

				Operand o = {};
				check_expr_or_type(c, &o, elem, field->type);

				if (is_type_any(field->type) || is_type_union(field->type) || is_type_raw_union(field->type) || is_type_typeid(field->type)) {
					is_constant = false;
				}
				if (is_constant) {
					is_constant = check_is_operand_compound_lit_constant(c, &o);
				}

				check_assignment(c, &o, field->type, str_lit("structure literal"));
			}
			if (cl->elems.count < field_count) {
				if (min_field_count < field_count) {
				    if (cl->elems.count < min_field_count) {
						error(cl->close, "Too few values in structure literal, expected at least %td, got %td", min_field_count, cl->elems.count);
				    }
				} else {
					error(cl->close, "Too few values in structure literal, expected %td, got %td", field_count, cl->elems.count);
				}
			}
		}

		break;
	}

	case Type_Slice:
	case Type_Array:
	case Type_DynamicArray:
	case Type_SimdVector:
	case Type_Matrix:
	{
		Type *elem_type = nullptr;
		String context_name = {};
		i64 max_type_count = -1;
		if (t->kind == Type_Slice) {
			elem_type = t->Slice.elem;
			context_name = str_lit("slice literal");
		} else if (t->kind == Type_Array) {
			elem_type = t->Array.elem;
			context_name = str_lit("array literal");
			if (!is_to_be_determined_array_count) {
				max_type_count = t->Array.count;
			}
		} else if (t->kind == Type_DynamicArray) {
			elem_type = t->DynamicArray.elem;
			context_name = str_lit("dynamic array literal");
			is_constant = false;

			if (!build_context.no_dynamic_literals) {
				add_package_dependency(c, "runtime", "__dynamic_array_reserve");
				add_package_dependency(c, "runtime", "__dynamic_array_append");
			}
		} else if (t->kind == Type_SimdVector) {
			elem_type = t->SimdVector.elem;
			context_name = str_lit("simd vector literal");
			max_type_count = t->SimdVector.count;
		} else if (t->kind == Type_Matrix) {
			elem_type = t->Matrix.elem;
			context_name = str_lit("matrix literal");
			max_type_count = t->Matrix.row_count*t->Matrix.column_count;
		} else {
			GB_PANIC("unreachable");
		}


		i64 max = 0;

		Type *bet = base_type(elem_type);
		if (!elem_type_can_be_constant(bet)) {
			is_constant = false;
		}

		if (bet == t_invalid) {
			break;
		}

		if (cl->elems.count > 0 && cl->elems[0]->kind == Ast_FieldValue) {
			if (is_type_simd_vector(t)) {
				error(cl->elems[0], "'field = value' is not allowed for SIMD vector literals");
			} else {
				RangeCache rc = range_cache_make(heap_allocator());
				defer (range_cache_destroy(&rc));

				for_array(i, cl->elems) {
					Ast *elem = cl->elems[i];
					if (elem->kind != Ast_FieldValue) {
						error(elem, "Mixture of 'field = value' and value elements in a literal is not allowed");
						continue;
					}
					ast_node(fv, FieldValue, elem);

					if (is_ast_range(fv->field)) {
						Token op = fv->field->BinaryExpr.op;

						Operand x = {};
						Operand y = {};
						bool ok = check_range(c, fv->field, &x, &y, nullptr);
						if (!ok) {
							continue;
						}
						if (x.mode != Addressing_Constant || !is_type_integer(core_type(x.type))) {
							error(x.expr, "Expected a constant integer as an array field");
							continue;
						}

						if (y.mode != Addressing_Constant || !is_type_integer(core_type(y.type))) {
							error(y.expr, "Expected a constant integer as an array field");
							continue;
						}

						i64 lo = exact_value_to_i64(x.value);
						i64 hi = exact_value_to_i64(y.value);
						i64 max_index = hi;
						if (op.kind == Token_RangeHalf) { // ..< (exclusive)
							hi -= 1;
						} else { // .. (inclusive)
							max_index += 1;
						}

						bool new_range = range_cache_add_range(&rc, lo, hi);
						if (!new_range) {
							error(elem, "Overlapping field range index %lld %.*s %lld for %.*s", lo, LIT(op.string), hi, LIT(context_name));
							continue;
						}


						if (max_type_count >= 0 && (lo < 0 || lo >= max_type_count)) {
							error(elem, "Index %lld is out of bounds (0..<%lld) for %.*s", lo, max_type_count, LIT(context_name));
							continue;
						}
						if (max_type_count >= 0 && (hi < 0 || hi >= max_type_count)) {
							error(elem, "Index %lld is out of bounds (0..<%lld) for %.*s", hi, max_type_count, LIT(context_name));
							continue;
						}

						if (max < hi) {
							max = max_index;
						}

						Operand operand = {};
						check_expr_with_type_hint(c, &operand, fv->value, elem_type);
						check_assignment(c, &operand, elem_type, context_name);

						is_constant = is_constant && operand.mode == Addressing_Constant;
					} else {
						Operand op_index = {};
						check_expr(c, &op_index, fv->field);

						if (op_index.mode != Addressing_Constant || !is_type_integer(core_type(op_index.type))) {
							error(elem, "Expected a constant integer as an array field");
							continue;
						}
						// add_type_and_value(c->info, op_index.expr, op_index.mode, op_index.type, op_index.value);

						i64 index = exact_value_to_i64(op_index.value);

						if (max_type_count >= 0 && (index < 0 || index >= max_type_count)) {
							error(elem, "Index %lld is out of bounds (0..<%lld) for %.*s", index, max_type_count, LIT(context_name));
							continue;
						}

						bool new_index = range_cache_add_index(&rc, index);
						if (!new_index) {
							error(elem, "Duplicate field index %lld for %.*s", index, LIT(context_name));
							continue;
						}

						if (max < index+1) {
							max = index+1;
						}

						Operand operand = {};
						check_expr_with_type_hint(c, &operand, fv->value, elem_type);
						check_assignment(c, &operand, elem_type, context_name);

						is_constant = is_constant && operand.mode == Addressing_Constant;
					}
				}

				cl->max_count = max;
			}

		} else {
			isize index = 0;
			for (; index < cl->elems.count; index++) {
				Ast *e = cl->elems[index];
				if (e == nullptr) {
					error(node, "Invalid literal element");
					continue;
				}

				if (e->kind == Ast_FieldValue) {
					error(e, "Mixture of 'field = value' and value elements in a literal is not allowed");
					continue;
				}

				if (0 <= max_type_count && max_type_count <= index) {
					error(e, "Index %lld is out of bounds (>= %lld) for %.*s", index, max_type_count, LIT(context_name));
				}

				Operand operand = {};
				check_expr_with_type_hint(c, &operand, e, elem_type);
				check_assignment(c, &operand, elem_type, context_name);

				is_constant = is_constant && operand.mode == Addressing_Constant;
			}

			if (max < index) {
				max = index;
			}
		}


		if (t->kind == Type_Array) {
			if (is_to_be_determined_array_count) {
				t->Array.count = max;
			} else if (cl->elems.count > 0 && cl->elems[0]->kind != Ast_FieldValue) {
				if (0 < max && max < t->Array.count) {
					error(node, "Expected %lld values for this array literal, got %lld", cast(long long)t->Array.count, cast(long long)max);
				}
			}
		}


		if (t->kind == Type_SimdVector) {
			if (!is_constant) {
				error(node, "Expected all constant elements for a simd vector");
			}
		}


		if (t->kind == Type_DynamicArray) {
			if (build_context.no_dynamic_literals && cl->elems.count) {
				error(node, "Compound literals of dynamic types have been disabled");
			}
		}

		if (t->kind == Type_Matrix) {
			if (cl->elems.count > 0 && cl->elems[0]->kind != Ast_FieldValue) {
				if (0 < max && max < max_type_count) {
					error(node, "Expected %lld values for this matrix literal, got %lld", cast(long long)max_type_count, cast(long long)max);
				}
			}
		}

		break;
	}

	case Type_EnumeratedArray:
	{
		Type *elem_type = t->EnumeratedArray.elem;
		Type *index_type = t->EnumeratedArray.index;
		String context_name = str_lit("enumerated array literal");
		i64 max_type_count = t->EnumeratedArray.count;

		gbString index_type_str = type_to_string(index_type);
		defer (gb_string_free(index_type_str));

		i64 total_lo = exact_value_to_i64(*t->EnumeratedArray.min_value);
		i64 total_hi = exact_value_to_i64(*t->EnumeratedArray.max_value);

		String total_lo_string = {};
		String total_hi_string = {};
		GB_ASSERT(is_type_enum(index_type));
		{
			Type *bt = base_type(index_type);
			GB_ASSERT(bt->kind == Type_Enum);
			for_array(i, bt->Enum.fields) {
				Entity *f = bt->Enum.fields[i];
				if (f->kind != Entity_Constant) {
					continue;
				}
				if (total_lo_string.len == 0 && compare_exact_values(Token_CmpEq, f->Constant.value, *t->EnumeratedArray.min_value)) {
					total_lo_string = f->token.string;
				}
				if (total_hi_string.len == 0 && compare_exact_values(Token_CmpEq, f->Constant.value, *t->EnumeratedArray.max_value)) {
					total_hi_string = f->token.string;
				}
				if (total_lo_string.len != 0 && total_hi_string.len != 0) {
					break;
				}
			}
		}

		i64 max = 0;

		Type *bet = base_type(elem_type);
		if (!elem_type_can_be_constant(bet)) {
			is_constant = false;
		}

		if (bet == t_invalid) {
			break;
		}
		bool is_partial = cl->tag && (cl->tag->BasicDirective.name.string == "partial");

		SeenMap seen = {}; // NOTE(bill): Multimap, Key: ExactValue
		map_init(&seen, heap_allocator());
		defer (map_destroy(&seen));

		if (cl->elems.count > 0 && cl->elems[0]->kind == Ast_FieldValue) {
			RangeCache rc = range_cache_make(heap_allocator());
			defer (range_cache_destroy(&rc));

			for_array(i, cl->elems) {
				Ast *elem = cl->elems[i];
				if (elem->kind != Ast_FieldValue) {
					error(elem, "Mixture of 'field = value' and value elements in a literal is not allowed");
					continue;
				}
				ast_node(fv, FieldValue, elem);

				if (is_ast_range(fv->field)) {
					Token op = fv->field->BinaryExpr.op;

					Operand x = {};
					Operand y = {};
					bool ok = check_range(c, fv->field, &x, &y, nullptr, index_type);
					if (!ok) {
						continue;
					}
					if (x.mode != Addressing_Constant || !are_types_identical(x.type, index_type)) {
						error(x.expr, "Expected a constant enum of type '%s' as an array field", index_type_str);
						continue;
					}

					if (y.mode != Addressing_Constant || !are_types_identical(x.type, index_type)) {
						error(y.expr, "Expected a constant enum of type '%s' as an array field", index_type_str);
						continue;
					}

					i64 lo = exact_value_to_i64(x.value);
					i64 hi = exact_value_to_i64(y.value);
					i64 max_index = hi;
					if (op.kind == Token_RangeHalf) {
						hi -= 1;
					}

					bool new_range = range_cache_add_range(&rc, lo, hi);
					if (!new_range) {
						gbString lo_str = expr_to_string(x.expr);
						gbString hi_str = expr_to_string(y.expr);
						error(elem, "Overlapping field range index %s %.*s %s for %.*s", lo_str, LIT(op.string), hi_str, LIT(context_name));
						gb_string_free(hi_str);
						gb_string_free(lo_str);
						continue;
					}


					// NOTE(bill): These are sanity checks for invalid enum values
					if (max_type_count >= 0 && (lo < total_lo || lo > total_hi)) {
						gbString lo_str = expr_to_string(x.expr);
						error(elem, "Index %s is out of bounds (%.*s .. %.*s) for %.*s", lo_str, LIT(total_lo_string), LIT(total_hi_string), LIT(context_name));
						gb_string_free(lo_str);
						continue;
					}
					if (max_type_count >= 0 && (hi < 0 || hi > total_hi)) {
						gbString hi_str = expr_to_string(y.expr);
						error(elem, "Index %s is out of bounds (%.*s .. %.*s) for %.*s", hi_str, LIT(total_lo_string), LIT(total_hi_string), LIT(context_name));
						gb_string_free(hi_str);
						continue;
					}

					if (max < hi) {
						max = max_index;
					}

					Operand operand = {};
					check_expr_with_type_hint(c, &operand, fv->value, elem_type);
					check_assignment(c, &operand, elem_type, context_name);

					is_constant = is_constant && operand.mode == Addressing_Constant;

					TokenKind upper_op = Token_LtEq;
					if (op.kind == Token_RangeHalf) {
						upper_op = Token_Lt;
					}
					add_to_seen_map(c, &seen, upper_op, x, x, y);
				} else {
					Operand op_index = {};
					check_expr_with_type_hint(c, &op_index, fv->field, index_type);

					if (op_index.mode != Addressing_Constant || !are_types_identical(op_index.type, index_type)) {
						error(op_index.expr, "Expected a constant enum of type '%s' as an array field", index_type_str);
						continue;
					}

					i64 index = exact_value_to_i64(op_index.value);

					if (max_type_count >= 0 && (index < total_lo || index > total_hi)) {
						gbString idx_str = expr_to_string(op_index.expr);
						error(elem, "Index %s is out of bounds (%.*s .. %.*s) for %.*s", idx_str, LIT(total_lo_string), LIT(total_hi_string), LIT(context_name));
						gb_string_free(idx_str);
						continue;
					}

					bool new_index = range_cache_add_index(&rc, index);
					if (!new_index) {
						gbString idx_str = expr_to_string(op_index.expr);
						error(elem, "Duplicate field index %s for %.*s", idx_str, LIT(context_name));
						gb_string_free(idx_str);
						continue;
					}

					if (max < index+1) {
						max = index+1;
					}

					Operand operand = {};
					check_expr_with_type_hint(c, &operand, fv->value, elem_type);
					check_assignment(c, &operand, elem_type, context_name);

					is_constant = is_constant && operand.mode == Addressing_Constant;

					add_to_seen_map(c, &seen, op_index);
				}
			}

			cl->max_count = max;

		} else {
			isize index = 0;
			for (; index < cl->elems.count; index++) {
				Ast *e = cl->elems[index];
				if (e == nullptr) {
					error(node, "Invalid literal element");
					continue;
				}

				if (e->kind == Ast_FieldValue) {
					error(e, "Mixture of 'field = value' and value elements in a literal is not allowed");
					continue;
				}

				if (0 <= max_type_count && max_type_count <= index) {
					error(e, "Index %lld is out of bounds (>= %lld) for %.*s", index, max_type_count, LIT(context_name));
				}

				Operand operand = {};
				check_expr_with_type_hint(c, &operand, e, elem_type);
				check_assignment(c, &operand, elem_type, context_name);

				is_constant = is_constant && operand.mode == Addressing_Constant;
			}

			if (max < index) {
				max = index;
			}
		}

		bool was_error = false;
		if (cl->elems.count > 0 && cl->elems[0]->kind != Ast_FieldValue) {
			if (0 < max && max < t->EnumeratedArray.count) {
				error(node, "Expected %lld values for this enumerated array literal, got %lld", cast(long long)t->EnumeratedArray.count, cast(long long)max);
				was_error = true;
			} else {
				error(node, "Enumerated array literals must only have 'field = value' elements, bare elements are not allowed");
				was_error = true;
			}
		}

		// NOTE(bill): Check for missing cases when `#partial literal` is not present
		if (cl->elems.count > 0 && !was_error && !is_partial) {
			Type *et = base_type(index_type);
			GB_ASSERT(et->kind == Type_Enum);
			auto fields = et->Enum.fields;

			auto unhandled = array_make<Entity *>(temporary_allocator(), 0, fields.count);

			for_array(i, fields) {
				Entity *f = fields[i];
				if (f->kind != Entity_Constant) {
					continue;
				}
				ExactValue v = f->Constant.value;
				auto found = map_get(&seen, hash_exact_value(v));
				if (!found) {
					array_add(&unhandled, f);
				}
			}

			if (unhandled.count > 0) {
				begin_error_block();
				defer (end_error_block());

				if (unhandled.count == 1) {
					error_no_newline(node, "Unhandled enumerated array case: %.*s", LIT(unhandled[0]->token.string));
				} else {
					error(node, "Unhandled enumerated array cases:");
					for_array(i, unhandled) {
						Entity *f = unhandled[i];
						error_line("\t%.*s\n", LIT(f->token.string));
					}
				}
				error_line("\n");

				error_line("\tSuggestion: Was '#partial %s{...}' wanted?\n", type_to_string(type));
			}
		}

		break;
	}

	case Type_Basic: {
		if (!is_type_any(t)) {
			if (cl->elems.count != 0) {
				error(node, "Illegal compound literal");
			}
			break;
		}
		if (cl->elems.count == 0) {
			break; // NOTE(bill): No need to init
		}
		{ // Checker values
			Type *field_types[2] = {t_rawptr, t_typeid};
			isize field_count = 2;
			if (cl->elems[0]->kind == Ast_FieldValue) {
				bool fields_visited[2] = {};

				for_array(i, cl->elems) {
					Ast *elem = cl->elems[i];
					if (elem->kind != Ast_FieldValue) {
						error(elem, "Mixture of 'field = value' and value elements in a 'any' literal is not allowed");
						continue;
					}
					ast_node(fv, FieldValue, elem);
					if (fv->field->kind != Ast_Ident) {
						gbString expr_str = expr_to_string(fv->field);
						error(elem, "Invalid field name '%s' in 'any' literal", expr_str);
						gb_string_free(expr_str);
						continue;
					}
					String name = fv->field->Ident.token.string;

					Selection sel = lookup_field(type, name, o->mode == Addressing_Type);
					if (sel.entity == nullptr) {
						error(elem, "Unknown field '%.*s' in 'any' literal", LIT(name));
						continue;
					}

					isize index = sel.index[0];

					if (fields_visited[index]) {
						error(elem, "Duplicate field '%.*s' in 'any' literal", LIT(name));
						continue;
					}

					fields_visited[index] = true;
					check_expr(c, o, fv->value);

					// NOTE(bill): 'any' literals can never be constant
					is_constant = false;

					check_assignment(c, o, field_types[index], str_lit("'any' literal"));
				}
			} else {
				for_array(index, cl->elems) {
					Ast *elem = cl->elems[index];
					if (elem->kind == Ast_FieldValue) {
						error(elem, "Mixture of 'field = value' and value elements in a 'any' literal is not allowed");
						continue;
					}


					check_expr(c, o, elem);
					if (index >= field_count) {
						error(o->expr, "Too many values in 'any' literal, expected %td", field_count);
						break;
					}

					// NOTE(bill): 'any' literals can never be constant
					is_constant = false;

					check_assignment(c, o, field_types[index], str_lit("'any' literal"));
				}
				if (cl->elems.count < field_count) {
					error(cl->close, "Too few values in 'any' literal, expected %td, got %td", field_count, cl->elems.count);
				}
			}
		}

		break;
	}

	case Type_Map: {
		if (cl->elems.count == 0) {
			break;
		}
		is_constant = false;
		{ // Checker values
			bool key_is_typeid = is_type_typeid(t->Map.key);
			bool value_is_typeid = is_type_typeid(t->Map.value);

			for_array(i, cl->elems) {
				Ast *elem = cl->elems[i];
				if (elem->kind != Ast_FieldValue) {
					error(elem, "Only 'field = value' elements are allowed in a map literal");
					continue;
				}
				ast_node(fv, FieldValue, elem);

				if (key_is_typeid) {
					check_expr_or_type(c, o, fv->field, t->Map.key);
				} else {
					check_expr_with_type_hint(c, o, fv->field, t->Map.key);
				}
				check_assignment(c, o, t->Map.key, str_lit("map literal"));
				if (o->mode == Addressing_Invalid) {
					continue;
				}

				if (value_is_typeid) {
					check_expr_or_type(c, o, fv->value, t->Map.value);
				} else {
					check_expr_with_type_hint(c, o, fv->value, t->Map.value);
				}
				check_assignment(c, o, t->Map.value, str_lit("map literal"));
			}
		}

		if (build_context.no_dynamic_literals && cl->elems.count) {
			error(node, "Compound literals of dynamic types have been disabled");
		} else {
			add_package_dependency(c, "runtime", "__dynamic_map_reserve");
			add_package_dependency(c, "runtime", "__dynamic_map_set");
		}
		break;
	}

	case Type_BitSet: {
		if (cl->elems.count == 0) {
			break; // NOTE(bill): No need to init
		}
		Type *et = base_type(t->BitSet.elem);
		isize field_count = 0;
		if (et->kind == Type_Enum) {
			field_count = et->Enum.fields.count;
		}

		if (cl->elems[0]->kind == Ast_FieldValue) {
			error(cl->elems[0], "'field = value' in a bit_set a literal is not allowed");
			is_constant = false;
		} else {
			for_array(index, cl->elems) {
				Ast *elem = cl->elems[index];
				if (elem->kind == Ast_FieldValue) {
					error(elem, "'field = value' in a bit_set a literal is not allowed");
					continue;
				}

				check_expr_with_type_hint(c, o, elem, et);

				if (is_constant) {
					is_constant = o->mode == Addressing_Constant;
				}

				check_assignment(c, o, t->BitSet.elem, str_lit("bit_set literal"));
				if (o->mode == Addressing_Constant) {
					i64 lower = t->BitSet.lower;
					i64 upper = t->BitSet.upper;
					i64 v = exact_value_to_i64(o->value);
					if (lower <= v && v <= upper) {
						// okay
					} else {
						error(elem, "Bit field value out of bounds, %lld not in the range %lld .. %lld", v, lower, upper);
						continue;
					}
				}
			}
		}
		break;
	}

	default: {
		if (cl->elems.count == 0) {
			break; // NOTE(bill): No need to init
		}

		gbString str = type_to_string(type);
		error(node, "Invalid compound literal type '%s'", str);
		gb_string_free(str);
		return kind;
	}
	}

	if (is_constant) {
		o->mode = Addressing_Constant;

		if (is_type_bit_set(type)) {
			// NOTE(bill): Encode as an integer

			i64 lower = base_type(type)->BitSet.lower;

			u64 bits = 0;
			for_array(index, cl->elems) {
				Ast *elem = cl->elems[index];
				GB_ASSERT(elem->kind != Ast_FieldValue);
				TypeAndValue tav = elem->tav;
				ExactValue i = exact_value_to_integer(tav.value);
				if (i.kind != ExactValue_Integer) {
					continue;
				}
				i64 val = big_int_to_i64(&i.value_integer);
				val -= lower;
				u64 bit = u64(1ll<<val);
				bits |= bit;
			}
			o->value = exact_value_u64(bits);
		} else if (is_type_constant_type(type) && cl->elems.count == 0) {
			ExactValue value = exact_value_compound(node);
			Type *bt = core_type(type);
			if (bt->kind == Type_Basic) {
				if (bt->Basic.flags & BasicFlag_Boolean) {
					value = exact_value_bool(false);
				} else if (bt->Basic.flags & BasicFlag_Integer) {
					value = exact_value_i64(0);
				} else if (bt->Basic.flags & BasicFlag_Unsigned) {
					value = exact_value_i64(0);
				} else if (bt->Basic.flags & BasicFlag_Float) {
					value = exact_value_float(0);
				} else if (bt->Basic.flags & BasicFlag_Complex) {
					value = exact_value_complex(0, 0);
				} else if (bt->Basic.flags & BasicFlag_Quaternion) {
					value = exact_value_quaternion(0, 0, 0, 0);
				} else if (bt->Basic.flags & BasicFlag_Pointer) {
					value = exact_value_pointer(0);
				} else if (bt->Basic.flags & BasicFlag_String) {
					String empty_string = {};
					value = exact_value_string(empty_string);
				} else if (bt->Basic.flags & BasicFlag_Rune) {
					value = exact_value_i64(0);
				}
			}

			o->value = value;
		} else {
			o->value = exact_value_compound(node);
		}
	} else {
		o->mode = Addressing_Value;
	}
	o->type = type;
	return kind;
}

ExprKind check_type_assertion(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = Expr_Expr;
	ast_node(ta, TypeAssertion, node);
	check_expr(c, o, ta->expr);
	node->viral_state_flags |= ta->expr->viral_state_flags;

	if (o->mode == Addressing_Invalid) {
		o->expr = node;
		return kind;
	}
	if (o->mode == Addressing_Constant) {
		gbString expr_str = expr_to_string(o->expr);
		error(o->expr, "A type assertion cannot be applied to a constant expression: '%s'", expr_str);
		gb_string_free(expr_str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return kind;
	}

	if (is_type_untyped(o->type)) {
		gbString expr_str = expr_to_string(o->expr);
		error(o->expr, "A type assertion cannot be applied to an untyped expression: '%s'", expr_str);
		gb_string_free(expr_str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return kind;
	}

	Type *src = type_deref(o->type);
	Type *bsrc = base_type(src);


	if (ta->type != nullptr && ta->type->kind == Ast_UnaryExpr && ta->type->UnaryExpr.op.kind == Token_Question) {
		if (!is_type_union(src)) {
			gbString str = type_to_string(o->type);
			error(o->expr, "Type assertions with .? can only operate on unions, got %s", str);
			gb_string_free(str);
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		}

		if (bsrc->Union.variants.count != 1 && type_hint != nullptr) {
			bool allowed = false;
			for_array(i, bsrc->Union.variants) {
				Type *vt = bsrc->Union.variants[i];
				if (are_types_identical(vt, type_hint)) {
					allowed = true;
					add_type_info_type(c, vt);
					break;
				}
			}
			if (allowed) {
				add_type_info_type(c, o->type);
				o->type = type_hint;
				o->mode = Addressing_OptionalOk;
				return kind;
			}
		}

		if (bsrc->Union.variants.count != 1) {
			error(o->expr, "Type assertions with .? can only operate on unions with 1 variant, got %lld", cast(long long)bsrc->Union.variants.count);
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		}

		add_type_info_type(c, o->type);
		add_type_info_type(c, bsrc->Union.variants[0]);

		o->type = bsrc->Union.variants[0];
		o->mode = Addressing_OptionalOk;
	} else {
		Type *t = check_type(c, ta->type);
		Type *dst = t;

		if (is_type_union(src)) {
			bool ok = false;
			for_array(i, bsrc->Union.variants) {
				Type *vt = bsrc->Union.variants[i];
				if (are_types_identical(vt, dst)) {
					ok = true;
					break;
				}
			}

			if (!ok) {
				gbString expr_str = expr_to_string(o->expr);
				gbString dst_type_str = type_to_string(t);
				defer (gb_string_free(expr_str));
				defer (gb_string_free(dst_type_str));
				if (bsrc->Union.variants.count == 0) {
					error(o->expr, "Cannot type assert '%s' to '%s' as this is an empty union", expr_str, dst_type_str);
				} else {
					error(o->expr, "Cannot type assert '%s' to '%s' as it is not a variant of that union", expr_str, dst_type_str);
				}
				o->mode = Addressing_Invalid;
				o->expr = node;
				return kind;
			}

			add_type_info_type(c, o->type);
			add_type_info_type(c, t);

			o->type = t;
			o->mode = Addressing_OptionalOk;
		} else if (is_type_any(src)) {
			o->type = t;
			o->mode = Addressing_OptionalOk;

			add_type_info_type(c, o->type);
			add_type_info_type(c, t);
		} else {
			gbString str = type_to_string(o->type);
			error(o->expr, "Type assertions can only operate on unions and 'any', got %s", str);
			gb_string_free(str);
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		}
	}

	if ((c->state_flags & StateFlag_no_type_assert) == 0) {
		add_package_dependency(c, "runtime", "type_assertion_check");
		add_package_dependency(c, "runtime", "type_assertion_check2");
	}
	return kind;
}

ExprKind check_selector_call_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ast_node(se, SelectorCallExpr, node);
	// IMPORTANT NOTE(bill, 2020-05-22): This is a complete hack to get a shorthand which is extremely useful for vtables
	// COM APIs is a great example of where this kind of thing is extremely useful
	// General idea:
	//
	//     x->y(123)  ==  x.y(x, 123)
	//
	// How this has been implemented at the moment is quite hacky but it's done so to reduce need for huge backend changes
	// Just regenerating a new AST aids things
	//
	// TODO(bill): Is this a good hack or not?
	//
	// NOTE(bill, 2020-05-22): I'm going to regret this decision, ain't I?


	if (se->modified_call) {
		// Prevent double evaluation
		o->expr  = node;
		o->type  = node->tav.type;
		o->value = node->tav.value;
		o->mode  = node->tav.mode;
		return Expr_Expr;
	}

	bool allow_arrow_right_selector_expr;
	allow_arrow_right_selector_expr = c->allow_arrow_right_selector_expr;
	c->allow_arrow_right_selector_expr = true;
	Operand x = {};
	ExprKind kind = check_expr_base(c, &x, se->expr, nullptr);
	c->allow_arrow_right_selector_expr = allow_arrow_right_selector_expr;

	if (x.mode == Addressing_Invalid || x.type == t_invalid) {
		o->mode = Addressing_Invalid;
		o->type = t_invalid;
		o->expr = node;
		return kind;
	}
	if (!is_type_proc(x.type)) {
		gbString type_str = type_to_string(x.type);
		error(se->call, "Selector call expressions expect a procedure type for the call, got '%s'", type_str);
		gb_string_free(type_str);

		o->mode = Addressing_Invalid;
		o->type = t_invalid;
		o->expr = node;
		return Expr_Stmt;
	}

	ast_node(ce, CallExpr, se->call);

	GB_ASSERT(x.expr->kind == Ast_SelectorExpr);

	Ast *first_arg = x.expr->SelectorExpr.expr;
	GB_ASSERT(first_arg != nullptr);

	Type *pt = base_type(x.type);
	GB_ASSERT(pt->kind == Type_Proc);
	Type *first_type = nullptr;
	String first_arg_name = {};
	if (pt->Proc.param_count > 0) {
		Entity *f = pt->Proc.params->Tuple.variables[0];
		first_type = f->type;
		first_arg_name = f->token.string;
	}
	if (first_arg_name.len == 0) {
		first_arg_name = str_lit("_");
	}

	if (first_type == nullptr) {
		error(se->call, "Selector call expressions expect a procedure type for the call with at least 1 parameter");
		o->mode = Addressing_Invalid;
		o->type = t_invalid;
		o->expr = node;
		return Expr_Stmt;
	}

	Operand y = {};
	y.mode = first_arg->tav.mode;
	y.type = first_arg->tav.type;
	y.value = first_arg->tav.value;
	if (check_is_assignable_to(c, &y, first_type)) {
		// Do nothing, it's valid
	} else {
		Operand z = y;
		z.type = type_deref(y.type);
		if (check_is_assignable_to(c, &z, first_type)) {
			// NOTE(bill): AST GENERATION HACK!
			Token op = {Token_Pointer};
			first_arg = ast_deref_expr(first_arg->file(), first_arg, op);
		} else if (y.mode == Addressing_Variable) {
			Operand w = y;
			w.type = alloc_type_pointer(y.type);
			if (check_is_assignable_to(c, &w, first_type)) {
				// NOTE(bill): AST GENERATION HACK!
				Token op = {Token_And};
				first_arg = ast_unary_expr(first_arg->file(), op, first_arg);
			}
		}
	}

	if (ce->args.count > 0) {
		bool fail = false;
		bool first_is_field_value = (ce->args[0]->kind == Ast_FieldValue);
		for_array(i, ce->args) {
			Ast *arg = ce->args[i];
			bool mix = false;
			if (first_is_field_value) {
				mix = arg->kind != Ast_FieldValue;
			} else {
				mix = arg->kind == Ast_FieldValue;
			}
			if (mix) {
				fail = true;
				break;
			}
		}
		if (!fail && first_is_field_value) {
			Token op = {Token_Eq};
			AstFile *f = first_arg->file();
			first_arg = ast_field_value(f, ast_ident(f, make_token_ident(first_arg_name)), first_arg, op);
		}
	}



	auto modified_args = slice_make<Ast *>(heap_allocator(), ce->args.count+1);
	modified_args[0] = first_arg;
	slice_copy(&modified_args, ce->args, 1);
	ce->args = modified_args;
	se->modified_call = true;

	allow_arrow_right_selector_expr = c->allow_arrow_right_selector_expr;
	c->allow_arrow_right_selector_expr = true;
	check_expr_base(c, o, se->call, type_hint);
	c->allow_arrow_right_selector_expr = allow_arrow_right_selector_expr;

	o->expr = node;
	return Expr_Expr;
}


ExprKind check_index_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = Expr_Expr;
	ast_node(ie, IndexExpr, node);
	check_expr(c, o, ie->expr);
	node->viral_state_flags |= ie->expr->viral_state_flags;
	if (o->mode == Addressing_Invalid) {
		o->expr = node;
		return kind;
	}

	Type *t = base_type(type_deref(o->type));
	bool is_ptr = is_type_pointer(o->type);
	bool is_const = o->mode == Addressing_Constant;

	if (is_type_map(t)) {
		Operand key = {};
		if (is_type_typeid(t->Map.key)) {
			check_expr_or_type(c, &key, ie->index, t->Map.key);
		} else {
			check_expr_with_type_hint(c, &key, ie->index, t->Map.key);
		}
		check_assignment(c, &key, t->Map.key, str_lit("map index"));
		if (key.mode == Addressing_Invalid) {
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		}
		o->mode = Addressing_MapIndex;
		o->type = t->Map.value;
		o->expr = node;

		add_package_dependency(c, "runtime", "__dynamic_map_get");
		add_package_dependency(c, "runtime", "__dynamic_map_set");
		return Expr_Expr;
	}

	i64 max_count = -1;
	bool valid = check_set_index_data(o, t, is_ptr, &max_count, o->type);

	if (is_const) {
		if (is_type_array(t)) {
			// OKay
		} else if (is_type_slice(t)) {
			// Okay
		} else if (is_type_enumerated_array(t)) {
			// Okay
		} else if (is_type_string(t)) {
			// Okay
		} else if (is_type_relative_slice(t)) {
			// Okay
		} else if (is_type_matrix(t)) {
			// Okay
		} else {
			valid = false;
		}
	}

	if (!valid) {
		gbString str = expr_to_string(o->expr);
		gbString type_str = type_to_string(o->type);
		defer (gb_string_free(str));
		defer (gb_string_free(type_str));
		if (is_const) {
			error(o->expr, "Cannot index constant '%s' of type '%s'", str, type_str);
		} else {
			error(o->expr, "Cannot index '%s' of type '%s'", str, type_str);
		}
		o->mode = Addressing_Invalid;
		o->expr = node;
		return kind;
	}

	if (ie->index == nullptr) {
		gbString str = expr_to_string(o->expr);
		error(o->expr, "Missing index for '%s'", str);
		gb_string_free(str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return kind;
	}

	Type *index_type_hint = nullptr;
	if (is_type_enumerated_array(t)) {
		Type *bt = base_type(t);
		GB_ASSERT(bt->kind == Type_EnumeratedArray);
		index_type_hint = bt->EnumeratedArray.index;
	}

	i64 index = 0;
	bool ok = check_index_value(c, t, false, ie->index, max_count, &index, index_type_hint);
	if (is_const) {
		if (index < 0) {
			gbString str = expr_to_string(o->expr);
			error(o->expr, "Cannot index a constant '%s'", str);
			error_line("\tSuggestion: store the constant into a variable in order to index it with a variable index\n");
			gb_string_free(str);
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		} else if (ok) {
			ExactValue value = type_and_value_of_expr(ie->expr).value;
			o->mode = Addressing_Constant;
			bool success = false;
			bool finish = false;
			o->value = get_constant_field_single(c, value, cast(i32)index, &success, &finish);
			if (!success) {
				gbString str = expr_to_string(o->expr);
				error(o->expr, "Cannot index a constant '%s' with index %lld", str, cast(long long)index);
				error_line("\tSuggestion: store the constant into a variable in order to index it with a variable index\n");
				gb_string_free(str);
				o->mode = Addressing_Invalid;
				o->expr = node;
				return kind;
			}
		}
	}

	if (type_hint != nullptr && is_type_matrix(t)) {
		// TODO(bill): allow matrix columns to be assignable to other types which are the same internally
		// if a type hint exists
	}
	return kind;
}

ExprKind check_slice_expr(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = Expr_Stmt;
	ast_node(se, SliceExpr, node);
	check_expr(c, o, se->expr);
	node->viral_state_flags |= se->expr->viral_state_flags;

	if (o->mode == Addressing_Invalid) {
		o->mode = Addressing_Invalid;
		o->expr = node;
		return kind;
	}

	bool valid = false;
	i64 max_count = -1;
	Type *t = base_type(type_deref(o->type));
	switch (t->kind) {
	case Type_Basic:
		if (t->Basic.kind == Basic_string || t->Basic.kind == Basic_UntypedString) {
			valid = true;
			if (o->mode == Addressing_Constant) {
				max_count = o->value.value_string.len;
			}
			o->type = type_deref(o->type);
		}
		break;

	case Type_Array:
		valid = true;
		max_count = t->Array.count;
		if (o->mode != Addressing_Variable && !is_type_pointer(o->type)) {
			gbString str = expr_to_string(node);
			error(node, "Cannot slice array '%s', value is not addressable", str);
			gb_string_free(str);
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		}
		o->type = alloc_type_slice(t->Array.elem);
		break;

	case Type_MultiPointer:
		valid = true;
		o->type = type_deref(o->type);
		break;

	case Type_Slice:
		valid = true;
		o->type = type_deref(o->type);
		break;

	case Type_DynamicArray:
		valid = true;
		o->type = alloc_type_slice(t->DynamicArray.elem);
		break;

	case Type_Struct:
		if (is_type_soa_struct(t)) {
			valid = true;
			o->type = make_soa_struct_slice(c, nullptr, nullptr, t->Struct.soa_elem);
		}
		break;

	case Type_RelativeSlice:
		valid = true;
		o->type = t->RelativeSlice.slice_type;
		if (o->mode != Addressing_Variable) {
			gbString str = expr_to_string(node);
			error(node, "Cannot relative slice '%s', value is not addressable", str);
			gb_string_free(str);
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		}
		break;
	}

	if (!valid) {
		gbString str = expr_to_string(o->expr);
		gbString type_str = type_to_string(o->type);
		error(o->expr, "Cannot slice '%s' of type '%s'", str, type_str);
		gb_string_free(type_str);
		gb_string_free(str);
		o->mode = Addressing_Invalid;
		o->expr = node;
		return kind;
	}

	if (se->low == nullptr && se->high != nullptr) {
		// It is okay to continue as it will assume the 1st index is zero
	}

	i64 indices[2] = {};
	Ast *nodes[2] = {se->low, se->high};
	for (isize i = 0; i < gb_count_of(nodes); i++) {
		i64 index = max_count;
		if (nodes[i] != nullptr) {
			i64 capacity = -1;
			if (max_count >= 0) {
				capacity = max_count;
			}
			i64 j = 0;
			if (check_index_value(c, t, true, nodes[i], capacity, &j)) {
				index = j;
			}

			node->viral_state_flags |= nodes[i]->viral_state_flags;
		} else if (i == 0) {
			index = 0;
		}
		indices[i] = index;
	}

	for (isize i = 0; i < gb_count_of(indices); i++) {
		i64 a = indices[i];
		for (isize j = i+1; j < gb_count_of(indices); j++) {
			i64 b = indices[j];
			if (a > b && b >= 0) {
				error(se->close, "Invalid slice indices: [%td > %td]", a, b);
			}
		}
	}

	if (max_count < 0)  {
		if (o->mode == Addressing_Constant) {
			gbString s = expr_to_string(se->expr);
			error(se->expr, "Cannot slice constant value '%s'", s);
			gb_string_free(s);
		}
	}

	if (t->kind == Type_MultiPointer && se->high != nullptr) {
		/*
			x[:]   -> [^]T
			x[i:]  -> [^]T
			x[:n]  -> []T
			x[i:n] -> []T
		*/
		o->type = alloc_type_slice(t->MultiPointer.elem);
	}

	o->mode = Addressing_Value;

	if (is_type_string(t) && max_count >= 0) {
		bool all_constant = true;
		for (isize i = 0; i < gb_count_of(nodes); i++) {
			if (nodes[i] != nullptr) {
				TypeAndValue tav = type_and_value_of_expr(nodes[i]);
				if (tav.mode != Addressing_Constant) {
					all_constant = false;
					break;
				}
			}
		}
		if (!all_constant) {
			gbString str = expr_to_string(o->expr);
			error(o->expr, "Cannot slice '%s' with non-constant indices", str);
			error_line("\tSuggestion: store the constant into a variable in order to index it with a variable index\n");
			gb_string_free(str);
			o->mode = Addressing_Value; // NOTE(bill): Keep subsequent values going without erring
			o->expr = node;
			return kind;
		}

		String s = {};
		if (o->value.kind == ExactValue_String) {
			s = o->value.value_string;
		}

		o->mode = Addressing_Constant;
		o->type = t;
		o->value = exact_value_string(substring(s, cast(isize)indices[0], cast(isize)indices[1]));
	}
	return kind;
}

ExprKind check_expr_base_internal(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	u32 prev_state_flags = c->state_flags;
	defer (c->state_flags = prev_state_flags);
	if (node->state_flags != 0) {
		u32 in = node->state_flags;
		u32 out = c->state_flags;

		if (in & StateFlag_no_bounds_check) {
			out |= StateFlag_no_bounds_check;
			out &= ~StateFlag_bounds_check;
		} else if (in & StateFlag_bounds_check) {
			out |= StateFlag_bounds_check;
			out &= ~StateFlag_no_bounds_check;
		}

		if (in & StateFlag_no_type_assert) {
			out |= StateFlag_no_type_assert;
			out &= ~StateFlag_type_assert;
		} else if (in & StateFlag_type_assert) {
			out |= StateFlag_type_assert;
			out &= ~StateFlag_no_type_assert;
		}

		c->state_flags = out;
	}

	ExprKind kind = Expr_Stmt;

	o->mode = Addressing_Invalid;
	o->type = t_invalid;

	switch (node->kind) {
	default:
		return kind;

	case_ast_node(be, BadExpr, node)
		return kind;
	case_end;

	case_ast_node(i, Implicit, node)
		switch (i->kind) {
		case Token_context:
			{
				if (c->proc_name.len == 0 && c->curr_proc_sig == nullptr) {
					error(node, "'context' is only allowed within procedures %p", c->curr_proc_decl);
					return kind;
				}
				if (unparen_expr(c->assignment_lhs_hint) == node) {
					c->scope->flags |= ScopeFlag_ContextDefined;
				}

				if ((c->scope->flags & ScopeFlag_ContextDefined) == 0) {
					error(node, "'context' has not been defined within this scope");
					// Continue with value
				}

				init_core_context(c->checker);
				o->mode = Addressing_Context;
				o->type = t_context;
			}
			break;

		default:
			error(node, "Illegal implicit name '%.*s'", LIT(i->string));
			return kind;
		}
	case_end;

	case_ast_node(i, Ident, node);
		check_ident(c, o, node, nullptr, type_hint, false);
	case_end;

	case_ast_node(u, Undef, node);
		o->mode = Addressing_Value;
		o->type = t_untyped_undef;
	case_end;


	case_ast_node(bl, BasicLit, node);
		Type *t = t_invalid;
		switch (node->tav.value.kind) {
		case ExactValue_String:     t = t_untyped_string;     break;
		case ExactValue_Float:      t = t_untyped_float;      break;
		case ExactValue_Complex:    t = t_untyped_complex;    break;
		case ExactValue_Quaternion: t = t_untyped_quaternion; break;
		case ExactValue_Integer:
			t = t_untyped_integer;
			if (bl->token.kind == Token_Rune) {
				t = t_untyped_rune;
			}
			break;
		default:
			GB_PANIC("Unhandled value type for basic literal");
			break;
		}

		o->mode  = Addressing_Constant;
		o->type  = t;
		o->value = node->tav.value;
	case_end;

	case_ast_node(bd, BasicDirective, node);
		kind = check_basic_directive_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(pg, ProcGroup, node);
		error(node, "Illegal use of a procedure group");
		o->mode = Addressing_Invalid;
	case_end;

	case_ast_node(pl, ProcLit, node);
		CheckerContext ctx = *c;

		DeclInfo *decl = nullptr;
		Type *type = alloc_type(Type_Proc);
		check_open_scope(&ctx, pl->type);
		{
			decl = make_decl_info(ctx.scope, ctx.decl);
			decl->proc_lit  = node;
			ctx.decl = decl;
			defer (ctx.decl = ctx.decl->parent);

			if (pl->tags != 0) {
				error(node, "A procedure literal cannot have tags");
				pl->tags = 0; // TODO(bill): Should I zero this?!
			}

			check_procedure_type(&ctx, type, pl->type);
			if (!is_type_proc(type)) {
				gbString str = expr_to_string(node);
				error(node, "Invalid procedure literal '%s'", str);
				gb_string_free(str);
				check_close_scope(&ctx);
				return kind;
			}

			if (pl->body == nullptr) {
				error(node, "A procedure literal must have a body");
				return kind;
			}

			pl->decl = decl;
			check_procedure_later(&ctx, ctx.file, empty_token, decl, type, pl->body, pl->tags);
		}
		check_close_scope(&ctx);

		o->mode = Addressing_Value;
		o->type = type;
	case_end;

	case_ast_node(te, TernaryIfExpr, node);
		kind = check_ternary_if_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(te, TernaryWhenExpr, node);
		kind = check_ternary_when_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(oe, OrElseExpr, node);
		return check_or_else_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(re, OrReturnExpr, node);
		return check_or_return_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(cl, CompoundLit, node);
		kind = check_compound_literal(c, o, node, type_hint);
	case_end;

	case_ast_node(pe, ParenExpr, node);
		kind = check_expr_base(c, o, pe->expr, type_hint);
		node->viral_state_flags |= pe->expr->viral_state_flags;
		o->expr = node;
	case_end;

	case_ast_node(te, TagExpr, node);
		String name = te->name.string;
		error(node, "Unknown tag expression, #%.*s", LIT(name));
		if (te->expr) {
			kind = check_expr_base(c, o, te->expr, type_hint);
			node->viral_state_flags |= te->expr->viral_state_flags;
		}
		o->expr = node;
	case_end;

	case_ast_node(ta, TypeAssertion, node);
		kind = check_type_assertion(c, o, node, type_hint);
	case_end;

	case_ast_node(tc, TypeCast, node);
		check_expr_or_type(c, o, tc->type);
		if (o->mode != Addressing_Type) {
			gbString str = expr_to_string(tc->type);
			error(tc->type, "Expected a type, got %s", str);
			gb_string_free(str);
			o->mode = Addressing_Invalid;
		}
		if (o->mode == Addressing_Invalid) {
			o->expr = node;
			return kind;
		}
		Type *type = o->type;
		check_expr_base(c, o, tc->expr, type);
		node->viral_state_flags |= tc->expr->viral_state_flags;

		if (o->mode != Addressing_Invalid) {
			switch (tc->token.kind) {
			case Token_transmute:
				check_transmute(c, node, o, type);
				break;
			case Token_cast:
				check_cast(c, o, type);
				break;
			default:
				error(node, "Invalid AST: Invalid casting expression");
				o->mode = Addressing_Invalid;
				break;
			}
		}
		return Expr_Expr;
	case_end;

	case_ast_node(ac, AutoCast, node);
		check_expr_base(c, o, ac->expr, type_hint);
		node->viral_state_flags |= ac->expr->viral_state_flags;

		if (o->mode == Addressing_Invalid) {
			o->expr = node;
			return kind;
		}
		if (type_hint) {
			Type *type = type_of_expr(ac->expr);
			check_cast(c, o, type_hint);
			if (is_type_typed(type) && are_types_identical(type, type_hint)) {
				if (build_context.vet_extra) {
					error(node, "Redundant 'auto_cast' applied to expression");
				}
			}

		}
		o->expr = node;
		return Expr_Expr;
	case_end;

	case_ast_node(ue, UnaryExpr, node);
		Type *th = type_hint;
		if (ue->op.kind == Token_And) {
			th = type_deref(th);
		}
		check_expr_base(c, o, ue->expr, th);
		node->viral_state_flags |= ue->expr->viral_state_flags;

		if (o->mode != Addressing_Invalid) {
			check_unary_expr(c, o, ue->op, node);
		}
		o->expr = node;
		return kind;
	case_end;


	case_ast_node(be, BinaryExpr, node);
		check_binary_expr(c, o, node, type_hint, true);
		if (o->mode == Addressing_Invalid) {
			o->expr = node;
			return kind;
		}
	case_end;

	case_ast_node(se, SelectorExpr, node);
		check_selector(c, o, node, type_hint);
		node->viral_state_flags |= se->expr->viral_state_flags;
	case_end;

	case_ast_node(se, SelectorCallExpr, node);
		return check_selector_call_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(ise, ImplicitSelectorExpr, node);
		return check_implicit_selector_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(ie, IndexExpr, node);
		kind = check_index_expr(c, o, node, type_hint);
	case_end;

	case_ast_node(se, SliceExpr, node);
		kind = check_slice_expr(c, o, node, type_hint);
	case_end;
	
	case_ast_node(mie, MatrixIndexExpr, node);
		check_matrix_index_expr(c, o, node, type_hint);
		o->expr = node;
		return Expr_Expr;
	case_end;

	case_ast_node(ce, CallExpr, node);
		return check_call_expr(c, o, node, ce->proc, ce->args, ce->inlining, type_hint);
	case_end;

	case_ast_node(de, DerefExpr, node);
		check_expr_or_type(c, o, de->expr);
		node->viral_state_flags |= de->expr->viral_state_flags;

		if (o->mode == Addressing_Invalid) {
			o->mode = Addressing_Invalid;
			o->expr = node;
			return kind;
		} else {
			Type *t = base_type(o->type);
			if (t->kind == Type_Pointer && !is_type_empty_union(t->Pointer.elem)) {
				o->mode = Addressing_Variable;
				o->type = t->Pointer.elem;
 			} else if (t->kind == Type_RelativePointer) {
 				if (o->mode != Addressing_Variable) {
 					gbString str = expr_to_string(o->expr);
 					gbString typ = type_to_string(o->type);
 					error(o->expr, "Cannot dereference relative pointer '%s' of type '%s' as it does not have a variable addressing mode", str, typ);
 					gb_string_free(typ);
 					gb_string_free(str);
 				}

 				// NOTE(bill): This is required because when dereferencing, the original type has been lost
				add_type_info_type(c, o->type);

 				Type *ptr_type = base_type(t->RelativePointer.pointer_type);
 				GB_ASSERT(ptr_type->kind == Type_Pointer);
				o->mode = Addressing_Variable;
				o->type = ptr_type->Pointer.elem;
 			} else {
 				gbString str = expr_to_string(o->expr);
 				gbString typ = type_to_string(o->type);
 				error(o->expr, "Cannot dereference '%s' of type '%s'", str, typ);
 				gb_string_free(typ);
 				gb_string_free(str);
 				o->mode = Addressing_Invalid;
 				o->expr = node;
 				return kind;
 			}
		}
	case_end;

	case_ast_node(ia, InlineAsmExpr, node);
		if (c->curr_proc_decl == nullptr) {
			error(node, "Inline asm expressions are only allowed within a procedure body");
		}

		auto param_types = array_make<Type *>(heap_allocator(), ia->param_types.count);
		Type *return_type = nullptr;
		for_array(i, ia->param_types) {
			param_types[i] = check_type(c, ia->param_types[i]);
		}
		if (ia->return_type != nullptr) {
			return_type = check_type(c, ia->return_type);
		}
		Operand x = {};
		check_expr(c, &x, ia->asm_string);
		if (x.mode != Addressing_Constant || !is_type_string(x.type)) {
			error(x.expr, "Expected a constant string for the inline asm main parameter");
		}
		check_expr(c, &x, ia->constraints_string);
		if (x.mode != Addressing_Constant || !is_type_string(x.type)) {
			error(x.expr, "Expected a constant string for the inline asm constraints parameter");
		}

		Scope *scope = create_scope(c->info, c->scope);
		scope->flags |= ScopeFlag_Proc;

		Type *params = alloc_type_tuple();
		Type *results = alloc_type_tuple();
		if (param_types.count != 0) {
			slice_init(&params->Tuple.variables, heap_allocator(), param_types.count);
			for_array(i, param_types) {
				params->Tuple.variables[i] = alloc_entity_param(scope, blank_token, param_types[i], false, true);
			}
		}
		if (return_type != nullptr) {
			slice_init(&results->Tuple.variables, heap_allocator(), 1);
			results->Tuple.variables[0] = alloc_entity_param(scope, blank_token, return_type, false, true);
		}


		Type *pt = alloc_type_proc(scope, params, param_types.count, results, return_type != nullptr ? 1 : 0, false, ProcCC_InlineAsm);
		o->type = pt;
		o->mode = Addressing_Value;
		o->expr = node;
		return Expr_Expr;
	case_end;

	case Ast_TypeidType:
	case Ast_PolyType:
	case Ast_ProcType:
	case Ast_PointerType:
	case Ast_MultiPointerType:
	case Ast_ArrayType:
	case Ast_DynamicArrayType:
	case Ast_StructType:
	case Ast_UnionType:
	case Ast_EnumType:
	case Ast_MapType:
	case Ast_BitSetType:
	case Ast_MatrixType:
		o->mode = Addressing_Type;
		o->type = check_type(c, node);
		break;
	}

	kind = Expr_Expr;
	o->expr = node;
	return kind;
}



ExprKind check_expr_base(CheckerContext *c, Operand *o, Ast *node, Type *type_hint) {
	ExprKind kind = check_expr_base_internal(c, o, node, type_hint);
	if (o->type != nullptr && core_type(o->type) == nullptr) {
		o->type = t_invalid;
		gbString xs = expr_to_string(o->expr);
		if (o->mode == Addressing_Type) {
			error(o->expr, "Invalid type usage '%s'", xs);
		} else {
			error(o->expr, "Invalid expression '%s'", xs);
		}
		gb_string_free(xs);
	}
	if (o->type != nullptr && is_type_untyped(o->type)) {
		add_untyped(c, node, o->mode, o->type, o->value);
	}
	add_type_and_value(c->info, node, o->mode, o->type, o->value);
	return kind;
}


void check_multi_expr_or_type(CheckerContext *c, Operand *o, Ast *e) {
	check_expr_base(c, o, e, nullptr);
	switch (o->mode) {
	default:
		return; // NOTE(bill): Valid
	case Addressing_NoValue:
		error_operand_no_value(o);
		break;
	}
	o->mode = Addressing_Invalid;
}

void check_multi_expr(CheckerContext *c, Operand *o, Ast *e) {
	check_expr_base(c, o, e, nullptr);
	switch (o->mode) {
	default:
		return; // NOTE(bill): Valid
	case Addressing_NoValue:
		error_operand_no_value(o);
		break;
	case Addressing_Type:
		error_operand_not_expression(o);
		break;
	}
	o->mode = Addressing_Invalid;
}

void check_multi_expr_with_type_hint(CheckerContext *c, Operand *o, Ast *e, Type *type_hint) {
	check_expr_base(c, o, e, type_hint);
	switch (o->mode) {
	default:
		return; // NOTE(bill): Valid
	case Addressing_NoValue:
		error_operand_no_value(o);
		break;
	case Addressing_Type:
		error_operand_not_expression(o);
		break;
	}
	o->mode = Addressing_Invalid;
}

void check_not_tuple(CheckerContext *c, Operand *o) {
	if (o->mode == Addressing_Value) {
		// NOTE(bill): Tuples are not first class thus never named
		if (o->type->kind == Type_Tuple) {
			isize count = o->type->Tuple.variables.count;
			error(o->expr,
			      "%td-valued expression found where single value expected", count);
			o->mode = Addressing_Invalid;
			GB_ASSERT(count != 1);
		}
	}
}

void check_expr(CheckerContext *c, Operand *o, Ast *e) {
	check_multi_expr(c, o, e);
	check_not_tuple(c, o);
}


void check_expr_or_type(CheckerContext *c, Operand *o, Ast *e, Type *type_hint) {
	check_expr_base(c, o, e, type_hint);
	check_not_tuple(c, o);
	error_operand_no_value(o);
}



bool is_exact_value_zero(ExactValue const &v) {
	switch (v.kind) {
	case ExactValue_Invalid:
		return true;
	case ExactValue_Bool:
		return !v.value_bool;
	case ExactValue_String:
		return v.value_string.len == 0;
	case ExactValue_Integer:
		return big_int_is_zero(&v.value_integer);
	case ExactValue_Float:
		return v.value_float == 0.0;
	case ExactValue_Complex:
		if (v.value_complex) {
			return v.value_complex->real == 0.0 && v.value_complex->imag == 0.0;
		}
		return true;
	case ExactValue_Quaternion:
		if (v.value_quaternion) {
			return v.value_quaternion->real == 0.0 &&
			       v.value_quaternion->imag == 0.0 &&
			       v.value_quaternion->jmag == 0.0 &&
			       v.value_quaternion->kmag == 0.0;
		}
		return true;
	case ExactValue_Pointer:
		return v.value_pointer == 0;
	case ExactValue_Compound:
		if (v.value_compound == nullptr) {
			return true;
		} else {
			ast_node(cl, CompoundLit, v.value_compound);
			if (cl->elems.count == 0) {
				return true;
			} else {
				for_array(i, cl->elems) {
					Ast *elem = cl->elems[i];
					if (elem->tav.mode != Addressing_Constant) {
						// if (elem->tav.value.kind != ExactValue_Invalid) {
						return false;
						// }
					}
					if (!is_exact_value_zero(elem->tav.value)) {
						return false;
					}
				}
				return true;
			}
		}
	case ExactValue_Procedure:
		return v.value_procedure == nullptr;
	case ExactValue_Typeid:
		return v.value_typeid == nullptr;
	}
	return true;

}







gbString write_expr_to_string(gbString str, Ast *node, bool shorthand);

gbString write_struct_fields_to_string(gbString str, Slice<Ast *> const &params) {
	for_array(i, params) {
		if (i > 0) {
			str = gb_string_appendc(str, ", ");
		}
		str = write_expr_to_string(str, params[i], false);
	}
	return str;
}

gbString string_append_string(gbString str, String string) {
	if (string.len > 0) {
		return gb_string_append_length(str, &string[0], string.len);
	}
	return str;
}


gbString string_append_token(gbString str, Token token) {
	str = string_append_string(str, token.string);
	return str;
}


gbString write_expr_to_string(gbString str, Ast *node, bool shorthand) {
	if (node == nullptr)
		return str;

	if (is_ast_stmt(node)) {
		GB_ASSERT("stmt passed to write_expr_to_string");
	}

	switch (node->kind) {
	default:
		str = gb_string_appendc(str, "(BadExpr)");
		break;

	case_ast_node(i, Ident, node);
		str = string_append_token(str, i->token);
	case_end;

	case_ast_node(i, Implicit, node);
		str = string_append_token(str, *i);
	case_end;

	case_ast_node(bl, BasicLit, node);
		str = string_append_token(str, bl->token);
	case_end;

	case_ast_node(bd, BasicDirective, node);
		str = gb_string_append_rune(str, '#');
		str = string_append_string(str, bd->name.string);
	case_end;

	case_ast_node(ud, Undef, node);
		str = gb_string_appendc(str, "---");
	case_end;

	case_ast_node(pg, ProcGroup, node);
		str = gb_string_appendc(str, "proc{");
		for_array(i, pg->args) {
			if (i > 0) str = gb_string_appendc(str, ", ");
			str = write_expr_to_string(str, pg->args[i], shorthand);
		}
		str = gb_string_append_rune(str, '}');
	case_end;

	case_ast_node(pl, ProcLit, node);
		str = write_expr_to_string(str, pl->type, shorthand);
		if (pl->body) {
			str = gb_string_appendc(str, " {...}");
		} else {
			str = gb_string_appendc(str, " ---");
		}
	case_end;

	case_ast_node(cl, CompoundLit, node);
		str = write_expr_to_string(str, cl->type, shorthand);
		str = gb_string_append_rune(str, '{');
		if (shorthand) {
			str = gb_string_appendc(str, "...");
		} else {
			for_array(i, cl->elems) {
				if (i > 0) str = gb_string_appendc(str, ", ");
				str = write_expr_to_string(str, cl->elems[i], shorthand);
			}
		}
		str = gb_string_append_rune(str, '}');
	case_end;


	case_ast_node(te, TagExpr, node);
		str = gb_string_append_rune(str, '#');
		str = string_append_token(str, te->name);
		str = write_expr_to_string(str, te->expr, shorthand);
	case_end;

	case_ast_node(ue, UnaryExpr, node);
		str = string_append_token(str, ue->op);
		str = write_expr_to_string(str, ue->expr, shorthand);
	case_end;

	case_ast_node(de, DerefExpr, node);
		str = write_expr_to_string(str, de->expr, shorthand);
		str = gb_string_append_rune(str, '^');
	case_end;

	case_ast_node(be, BinaryExpr, node);
		str = write_expr_to_string(str, be->left, shorthand);
		str = gb_string_append_rune(str, ' ');
		str = string_append_token(str, be->op);
		str = gb_string_append_rune(str, ' ');
		str = write_expr_to_string(str, be->right, shorthand);
	case_end;

	case_ast_node(te, TernaryIfExpr, node);
		TokenPos x = ast_token(te->x).pos;
		TokenPos cond = ast_token(te->cond).pos;
		if (x < cond) {
			str = write_expr_to_string(str, te->x, shorthand);
			str = gb_string_appendc(str, " if ");
			str = write_expr_to_string(str, te->cond, shorthand);
			str = gb_string_appendc(str, " else ");
			str = write_expr_to_string(str, te->y, shorthand);
		} else {
			str = write_expr_to_string(str, te->cond, shorthand);
			str = gb_string_appendc(str, " ? ");
			str = write_expr_to_string(str, te->x, shorthand);
			str = gb_string_appendc(str, " : ");
			str = write_expr_to_string(str, te->y, shorthand);
		}
	case_end;

	case_ast_node(te, TernaryWhenExpr, node);
		str = write_expr_to_string(str, te->x, shorthand);
		str = gb_string_appendc(str, " when ");
		str = write_expr_to_string(str, te->cond, shorthand);
		str = gb_string_appendc(str, " else ");
		str = write_expr_to_string(str, te->y, shorthand);
	case_end;

	case_ast_node(oe, OrElseExpr, node);
		str = write_expr_to_string(str, oe->x, shorthand);
		str = gb_string_appendc(str, " or_else ");
		str = write_expr_to_string(str, oe->y, shorthand);
	case_end;

	case_ast_node(oe, OrReturnExpr, node);
		str = write_expr_to_string(str, oe->expr, shorthand);
		str = gb_string_appendc(str, " or_return");
	case_end;

	case_ast_node(pe, ParenExpr, node);
		str = gb_string_append_rune(str, '(');
		str = write_expr_to_string(str, pe->expr, shorthand);
		str = gb_string_append_rune(str, ')');
	case_end;

	case_ast_node(se, SelectorExpr, node);
		str = write_expr_to_string(str, se->expr, shorthand);
		str = string_append_token(str, se->token);
		str = write_expr_to_string(str, se->selector, shorthand);
	case_end;

	case_ast_node(se, ImplicitSelectorExpr, node);
		str = gb_string_append_rune(str, '.');
		str = write_expr_to_string(str, se->selector, shorthand);
	case_end;

	case_ast_node(se, SelectorCallExpr, node);
		str = write_expr_to_string(str, se->expr, shorthand);
		str = gb_string_appendc(str, "(");
		ast_node(ce, CallExpr, se->call);
		isize start = se->modified_call ? 1 : 0;
		for (isize i = start; i < ce->args.count; i++) {
			Ast *arg = ce->args[i];
			if (i > start) {
				str = gb_string_appendc(str, ", ");
			}
			str = write_expr_to_string(str, arg, shorthand);
		}
		str = gb_string_appendc(str, ")");
	case_end;

	case_ast_node(ta, TypeAssertion, node);
		str = write_expr_to_string(str, ta->expr, shorthand);
		if (ta->type != nullptr &&
		    ta->type->kind == Ast_UnaryExpr &&
		    ta->type->UnaryExpr.op.kind == Token_Question) {
			str = gb_string_appendc(str, ".?");
		} else {
			str = gb_string_appendc(str, ".(");
			str = write_expr_to_string(str, ta->type, shorthand);
			str = gb_string_append_rune(str, ')');
		}
	case_end;

	case_ast_node(tc, TypeCast, node);
		str = string_append_token(str, tc->token);
		str = gb_string_append_rune(str, '(');
		str = write_expr_to_string(str, tc->type, shorthand);
		str = gb_string_append_rune(str, ')');
		str = write_expr_to_string(str, tc->expr, shorthand);
	case_end;

	case_ast_node(ac, AutoCast, node);
		str = string_append_token(str, ac->token);
		str = gb_string_append_rune(str, ' ');
		str = write_expr_to_string(str, ac->expr, shorthand);
	case_end;

	case_ast_node(ie, IndexExpr, node);
		str = write_expr_to_string(str, ie->expr, shorthand);
		str = gb_string_append_rune(str, '[');
		str = write_expr_to_string(str, ie->index, shorthand);
		str = gb_string_append_rune(str, ']');
	case_end;

	case_ast_node(se, SliceExpr, node);
		str = write_expr_to_string(str, se->expr, shorthand);
		str = gb_string_append_rune(str, '[');
		str = write_expr_to_string(str, se->low, shorthand);
		str = string_append_token(str, se->interval);
		str = write_expr_to_string(str, se->high, shorthand);
		str = gb_string_append_rune(str, ']');
	case_end;

	case_ast_node(mie, MatrixIndexExpr, node);
		str = write_expr_to_string(str, mie->expr, shorthand);
		str = gb_string_append_rune(str, '[');
		str = write_expr_to_string(str, mie->row_index, shorthand);
		str = gb_string_appendc(str, ", ");
		str = write_expr_to_string(str, mie->column_index, shorthand);
		str = gb_string_append_rune(str, ']');
	case_end;
	
	case_ast_node(e, Ellipsis, node);
		str = gb_string_appendc(str, "..");
		str = write_expr_to_string(str, e->expr, shorthand);
	case_end;

	case_ast_node(fv, FieldValue, node);
		str = write_expr_to_string(str, fv->field, shorthand);
		str = gb_string_appendc(str, " = ");
		str = write_expr_to_string(str, fv->value, shorthand);
	case_end;
	case_ast_node(fv, EnumFieldValue, node);
		str = write_expr_to_string(str, fv->name, shorthand);
		if (fv->value) {
			str = gb_string_appendc(str, " = ");
			str = write_expr_to_string(str, fv->value, shorthand);
		}
	case_end;

	case_ast_node(ht, HelperType, node);
		str = gb_string_appendc(str, "#type ");
		str = write_expr_to_string(str, ht->type, shorthand);
	case_end;

	case_ast_node(ht, DistinctType, node);
		str = gb_string_appendc(str, "distinct ");
		str = write_expr_to_string(str, ht->type, shorthand);
	case_end;

	case_ast_node(pt, PolyType, node);
		str = gb_string_append_rune(str, '$');
		str = write_expr_to_string(str, pt->type, shorthand);
		if (pt->specialization != nullptr) {
			str = gb_string_append_rune(str, '/');
			str = write_expr_to_string(str, pt->specialization, shorthand);
		}
	case_end;

	case_ast_node(pt, PointerType, node);
		str = gb_string_append_rune(str, '^');
		str = write_expr_to_string(str, pt->type, shorthand);
	case_end;

	case_ast_node(pt, MultiPointerType, node);
		str = gb_string_appendc(str, "[^]");
		str = write_expr_to_string(str, pt->type, shorthand);
	case_end;

	case_ast_node(at, ArrayType, node);
		str = gb_string_append_rune(str, '[');
		if (at->count != nullptr &&
		    at->count->kind == Ast_UnaryExpr &&
		    at->count->UnaryExpr.op.kind == Token_Question) {
			str = gb_string_appendc(str, "?");
		} else {
			str = write_expr_to_string(str, at->count, shorthand);
		}
		str = gb_string_append_rune(str, ']');
		str = write_expr_to_string(str, at->elem, shorthand);
	case_end;

	case_ast_node(at, DynamicArrayType, node);
		str = gb_string_appendc(str, "[dynamic]");
		str = write_expr_to_string(str, at->elem, shorthand);
	case_end;

	case_ast_node(bs, BitSetType, node);
		str = gb_string_appendc(str, "bit_set[");
		str = write_expr_to_string(str, bs->elem, shorthand);
		str = gb_string_appendc(str, "]");
	case_end;


	case_ast_node(mt, MapType, node);
		str = gb_string_appendc(str, "map[");
		str = write_expr_to_string(str, mt->key, shorthand);
		str = gb_string_append_rune(str, ']');
		str = write_expr_to_string(str, mt->value, shorthand);
	case_end;
	
	case_ast_node(mt, MatrixType, node);
		str = gb_string_appendc(str, "matrix[");
		str = write_expr_to_string(str, mt->row_count, shorthand);
		str = gb_string_appendc(str, ", ");
		str = write_expr_to_string(str, mt->column_count, shorthand);
		str = gb_string_append_rune(str, ']');
		str = write_expr_to_string(str, mt->elem, shorthand);
	case_end;


	case_ast_node(f, Field, node);
		if (f->flags&FieldFlag_using) {
			str = gb_string_appendc(str, "using ");
		}
		if (f->flags&FieldFlag_no_alias) {
			str = gb_string_appendc(str, "#no_alias ");
		}
		if (f->flags&FieldFlag_c_vararg) {
			str = gb_string_appendc(str, "#c_vararg ");
		}
		if (f->flags&FieldFlag_auto_cast) {
			str = gb_string_appendc(str, "auto_cast ");
		}
		if (f->flags&FieldFlag_any_int) {
			str = gb_string_appendc(str, "#any_int ");
		}
		if (f->flags&FieldFlag_const) {
			str = gb_string_appendc(str, "#const ");
		}

		for_array(i, f->names) {
			Ast *name = f->names[i];
			if (i > 0) str = gb_string_appendc(str, ", ");
			str = write_expr_to_string(str, name, shorthand);
		}
		if (f->names.count > 0) {
			if (f->type == nullptr && f->default_value != nullptr) {
				str = gb_string_append_rune(str, ' ');
			}
			str = gb_string_appendc(str, ":");
		}
		if (f->type != nullptr) {
			str = gb_string_append_rune(str, ' ');
			str = write_expr_to_string(str, f->type, shorthand);
		}
		if (f->default_value != nullptr) {
			if (f->type != nullptr) {
				str = gb_string_append_rune(str, ' ');
			}
			str = gb_string_appendc(str, "= ");
			str = write_expr_to_string(str, f->default_value, shorthand);
		}

	case_end;

	case_ast_node(f, FieldList, node);
		bool has_name = false;
		for_array(i, f->list) {
			ast_node(field, Field, f->list[i]);
			if (field->names.count > 1) {
				has_name = true;
				break;
			}

			if (field->names.count == 0) {
				continue;
			}
			if (!is_blank_ident(field->names[0])) {
				has_name = true;
				break;
			}
		}

		for_array(i, f->list) {
			if (i > 0) str = gb_string_appendc(str, ", ");
			if (has_name) {
				str = write_expr_to_string(str, f->list[i], shorthand);
			} else {
				ast_node(field, Field, f->list[i]);

				if (field->flags&FieldFlag_using) {
					str = gb_string_appendc(str, "using ");
				}
				if (field->flags&FieldFlag_no_alias) {
					str = gb_string_appendc(str, "#no_alias ");
				}
				if (field->flags&FieldFlag_c_vararg) {
					str = gb_string_appendc(str, "#c_vararg ");
				}

				str = write_expr_to_string(str, field->type, shorthand);
			}
		}
	case_end;

	case_ast_node(ce, CallExpr, node);
		switch (ce->inlining) {
		case ProcInlining_inline:
			str = gb_string_appendc(str, "#force_inline ");
			break;
		case ProcInlining_no_inline:
			str = gb_string_appendc(str, "#force_no_inline ");
			break;
		}

		str = write_expr_to_string(str, ce->proc, shorthand);
		str = gb_string_appendc(str, "(");

		for_array(i, ce->args) {
			Ast *arg = ce->args[i];
			if (i > 0) {
				str = gb_string_appendc(str, ", ");
			}
			str = write_expr_to_string(str, arg, shorthand);
		}
		str = gb_string_appendc(str, ")");
	case_end;

	case_ast_node(tt, TypeidType, node);
		str = gb_string_appendc(str, "typeid");
		if (tt->specialization) {
			str = gb_string_appendc(str, "/");
			str = write_expr_to_string(str, tt->specialization, shorthand);
		}
	case_end;

	case_ast_node(pt, ProcType, node);
		str = gb_string_appendc(str, "proc(");
		str = write_expr_to_string(str, pt->params, shorthand);
		str = gb_string_appendc(str, ")");
		if (pt->results != nullptr) {
			str = gb_string_appendc(str, " -> ");

			bool parens_needed = false;
			if (pt->results && pt->results->kind == Ast_FieldList) {
				for_array(i, pt->results->FieldList.list) {
					Ast *field = pt->results->FieldList.list[i];
					ast_node(f, Field, field);
					if (f->names.count != 0) {
						parens_needed = true;
						break;
					}
				}
			}

			if (parens_needed) {
				str = gb_string_append_rune(str, '(');
			}
			str = write_expr_to_string(str, pt->results, shorthand);
			if (parens_needed) {
				str = gb_string_append_rune(str, ')');
			}
		}

	case_end;

	case_ast_node(st, StructType, node);
		str = gb_string_appendc(str, "struct ");
		if (st->polymorphic_params) {
			str = gb_string_append_rune(str, '(');
			str = write_expr_to_string(str, st->polymorphic_params, shorthand);
			str = gb_string_appendc(str, ") ");
		}
		if (st->is_packed)    str = gb_string_appendc(str, "#packed ");
		if (st->is_raw_union) str = gb_string_appendc(str, "#raw_union ");
		if (st->align) {
			str = gb_string_appendc(str, "#align ");
			str = write_expr_to_string(str, st->align, shorthand);
			str = gb_string_append_rune(str, ' ');
		}
		str = gb_string_append_rune(str, '{');
		if (shorthand) {
			str = gb_string_appendc(str, "...");
		} else {
			str = write_struct_fields_to_string(str, st->fields);
		}
		str = gb_string_append_rune(str, '}');
	case_end;


	case_ast_node(st, UnionType, node);
		str = gb_string_appendc(str, "union ");
		if (st->polymorphic_params) {
			str = gb_string_append_rune(str, '(');
			str = write_expr_to_string(str, st->polymorphic_params, shorthand);
			str = gb_string_appendc(str, ") ");
		}
		if (st->no_nil) str = gb_string_appendc(str, "#no_nil ");
		if (st->maybe)  str = gb_string_appendc(str, "#maybe ");
		if (st->align) {
			str = gb_string_appendc(str, "#align ");
			str = write_expr_to_string(str, st->align, shorthand);
			str = gb_string_append_rune(str, ' ');
		}
		str = gb_string_append_rune(str, '{');
		if (shorthand) {
			str = gb_string_appendc(str, "...");
		} else {
			str = write_struct_fields_to_string(str, st->variants);
		}
		str = gb_string_append_rune(str, '}');
	case_end;

	case_ast_node(et, EnumType, node);
		str = gb_string_appendc(str, "enum ");
		if (et->base_type != nullptr) {
			str = write_expr_to_string(str, et->base_type, shorthand);
			str = gb_string_append_rune(str, ' ');
		}
		str = gb_string_append_rune(str, '{');
		if (shorthand) {
			str = gb_string_appendc(str, "...");
		} else {
			for_array(i, et->fields) {
				if (i > 0) {
					str = gb_string_appendc(str, ", ");
				}
				str = write_expr_to_string(str, et->fields[i], shorthand);
			}
		}
		str = gb_string_append_rune(str, '}');
	case_end;

	case_ast_node(rt, RelativeType, node);
		str = write_expr_to_string(str, rt->tag, shorthand);
		str = gb_string_appendc(str, "" );
		str = write_expr_to_string(str, rt->type, shorthand);
	case_end;


	case_ast_node(ia, InlineAsmExpr, node);
		str = gb_string_appendc(str, "asm(");
		for_array(i, ia->param_types) {
			if (i > 0) {
				str = gb_string_appendc(str, ", ");
			}
			str = write_expr_to_string(str, ia->param_types[i], shorthand);
		}
		str = gb_string_appendc(str, ")");
		if (ia->return_type != nullptr) {
			str = gb_string_appendc(str, " -> ");
			str = write_expr_to_string(str, ia->return_type, shorthand);
		}
		if (ia->has_side_effects) {
			str = gb_string_appendc(str, " #side_effects");
		}
		if (ia->is_align_stack) {
			str = gb_string_appendc(str, " #stack_align");
		}
		if (ia->dialect) {
			str = gb_string_appendc(str, " #");
			str = gb_string_appendc(str, inline_asm_dialect_strings[ia->dialect]);
		}
		str = gb_string_appendc(str, " {");
		if (shorthand) {
			str = gb_string_appendc(str, "...");
		} else {
			str = write_expr_to_string(str, ia->asm_string, shorthand);
			str = gb_string_appendc(str, ", ");
			str = write_expr_to_string(str, ia->constraints_string, shorthand);
		}
		str = gb_string_appendc(str, "}");
	case_end;
	}

	return str;
}

gbString expr_to_string(Ast *expression) {
	return write_expr_to_string(gb_string_make(heap_allocator(), ""), expression, false);
}
gbString expr_to_string_shorthand(Ast *expression) {
	return write_expr_to_string(gb_string_make(heap_allocator(), ""), expression, true);
}
