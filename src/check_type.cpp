ParameterValue handle_parameter_value(CheckerContext *ctx, Type *in_type, Type **out_type_, Ast *expr, bool allow_caller_location);

void populate_using_array_index(CheckerContext *ctx, Ast *node, AstField *field, Type *t, String name, i32 idx) {
	t = base_type(t);
	GB_ASSERT(t->kind == Type_Array);
	Entity *e = scope_lookup_current(ctx->scope, name);
	if (e != nullptr) {
		gbString str = nullptr;
		defer (gb_string_free(str));
		if (node != nullptr) {
			str = expr_to_string(node);
		}
		if (str != nullptr) {
			error(e->token, "'%.*s' is already declared in '%s'", LIT(name), str);
		} else {
			error(e->token, "'%.*s' is already declared", LIT(name));
		}
	} else {
		Token tok = make_token_ident(name);
		if (field->names.count > 0) {
			tok.pos = ast_token(field->names[0]).pos;
		} else {
			tok.pos = ast_token(field->type).pos;
		}
		Entity *f = alloc_entity_array_elem(nullptr, tok, t->Array.elem, idx);
		add_entity(ctx, ctx->scope, nullptr, f);
	}
}

void populate_using_entity_scope(CheckerContext *ctx, Ast *node, AstField *field, Type *t) {
	if (t == nullptr) {
		return;
	}
	t = base_type(type_deref(t));
	gbString str = nullptr;
	defer (gb_string_free(str));
	if (node != nullptr) {
		str = expr_to_string(node);
	}

	if (t->kind == Type_Struct) {
		for (Entity *f : t->Struct.fields) {
			GB_ASSERT(f->kind == Entity_Variable);
			String name = f->token.string;
			Entity *e = scope_lookup_current(ctx->scope, name);
			if (e != nullptr && name != "_") {
				// TODO(bill): Better type error
				if (str != nullptr) {
					error(e->token, "'%.*s' is already declared in '%s'", LIT(name), str);
				} else {
					error(e->token, "'%.*s' is already declared", LIT(name));
				}
			} else {
				add_entity(ctx, ctx->scope, nullptr, f);
				if (f->flags & EntityFlag_Using) {
					populate_using_entity_scope(ctx, node, field, f->type);
				}
			}
		}
	} else if (t->kind == Type_Array && t->Array.count <= 4) {
		switch (t->Array.count) {
		case 4:
			populate_using_array_index(ctx, node, field, t, str_lit("w"), 3);
			populate_using_array_index(ctx, node, field, t, str_lit("a"), 3);
			/*fallthrough*/
		case 3:
			populate_using_array_index(ctx, node, field, t, str_lit("z"), 2);
			populate_using_array_index(ctx, node, field, t, str_lit("b"), 2);
			/*fallthrough*/
		case 2:
			populate_using_array_index(ctx, node, field, t, str_lit("y"), 1);
			populate_using_array_index(ctx, node, field, t, str_lit("g"), 1);
			/*fallthrough*/
		case 1:
			populate_using_array_index(ctx, node, field, t, str_lit("x"), 0);
			populate_using_array_index(ctx, node, field, t, str_lit("r"), 0);
			/*fallthrough*/
		default:
			break;
		}
	}
}

bool does_field_type_allow_using(Type *t) {
	t = base_type(t);
	if (is_type_struct(t)) {
		return true;
	} else if (is_type_array(t)) {
		return t->Array.count <= 4;
	}
	return false;
}

void check_struct_fields(CheckerContext *ctx, Ast *node, Slice<Entity *> *fields, String **tags, Slice<Ast *> const &params,
                         isize init_field_capacity, Type *struct_type, String context) {
	auto fields_array = array_make<Entity *>(heap_allocator(), 0, init_field_capacity);
	auto tags_array = array_make<String>(heap_allocator(), 0, init_field_capacity);

	GB_ASSERT(node->kind == Ast_StructType);
	GB_ASSERT(struct_type->kind == Type_Struct);

	isize variable_count = 0;
	for_array(i, params) {
		Ast *field = params[i];
		if (ast_node_expect(field, Ast_Field)) {
			ast_node(f, Field, field);
			variable_count += gb_max(f->names.count, 1);
		}
	}

	i32 field_src_index = 0;
	i32 field_group_index = -1;
	for_array(i, params) {
		Ast *param = params[i];
		if (param->kind != Ast_Field) {
			continue;
		}
		field_group_index += 1;

		ast_node(p, Field, param);
		Ast *type_expr = p->type;
		Type *type = nullptr;
		CommentGroup *docs = p->docs;
		CommentGroup *comment = p->comment;

		if (type_expr != nullptr) {
			type = check_type_expr(ctx, type_expr, nullptr);
			if (is_type_polymorphic(type)) {
				struct_type->Struct.is_polymorphic = true;
				type = nullptr;
			}
		}
		if (type == nullptr) {
			error(params[i], "Invalid parameter type");
			type = t_invalid;
		}
		if (is_type_untyped(type)) {
			if (is_type_untyped_undef(type)) {
				error(params[i], "Cannot determine parameter type from ---");
			} else {
				error(params[i], "Cannot determine parameter type from a nil");
			}
			type = t_invalid;
		}

		bool is_using = (p->flags&FieldFlag_using) != 0;

		for_array(j, p->names) {
			Ast *name = p->names[j];
			if (!ast_node_expect2(name, Ast_Ident, Ast_PolyType)) {
				continue;
			}
			if (name->kind == Ast_PolyType) {
				name = name->PolyType.type;
			}
			Token name_token = name->Ident.token;

			Entity *field = alloc_entity_field(ctx->scope, name_token, type, is_using, field_src_index);
			add_entity(ctx, ctx->scope, name, field);
			field->Variable.field_group_index = field_group_index;

			if (j == 0) {
				field->Variable.docs = docs;
			}
			if (j+1 == p->names.count) {
				field->Variable.comment = comment;
			}

			array_add(&fields_array, field);
			String tag = p->tag.string;
			if (tag.len != 0 && !unquote_string(permanent_allocator(), &tag, 0, tag.text[0] == '`')) {
				error(p->tag, "Invalid string literal");
				tag = {};
			}
			array_add(&tags_array, tag);

			field_src_index += 1;
		}


		if (is_using && p->names.count > 0) {
			Type *first_type = fields_array[fields_array.count-1]->type;
			Type *t = base_type(type_deref(first_type));

			if (!does_field_type_allow_using(t) &&
			    p->names.count >= 1 &&
			    p->names[0]->kind == Ast_Ident) {
				Token name_token = p->names[0]->Ident.token;
				gbString type_str = type_to_string(first_type);
				error(name_token, "'using' cannot be applied to the field '%.*s' of type '%s'", LIT(name_token.string), type_str);
				gb_string_free(type_str);
				continue;
			}

			populate_using_entity_scope(ctx, node, p, type);
		}
	}
	
	*fields = slice_from_array(fields_array);
	*tags = tags_array.data;
}


bool check_custom_align(CheckerContext *ctx, Ast *node, i64 *align_) {
	GB_ASSERT(align_ != nullptr);
	Operand o = {};
	check_expr(ctx, &o, node);
	if (o.mode != Addressing_Constant) {
		if (o.mode != Addressing_Invalid) {
			error(node, "#align must be a constant");
		}
		return false;
	}

	Type *type = base_type(o.type);
	if (is_type_untyped(type) || is_type_integer(type)) {
		if (o.value.kind == ExactValue_Integer) {
			BigInt v = o.value.value_integer;
			if (v.used > 1) {
				gbAllocator a = heap_allocator();
				String str = big_int_to_string(a, &v);
				error(node, "#align too large, %.*s", LIT(str));
				gb_free(a, str.text);
				return false;
			}
			i64 align = big_int_to_i64(&v);
			if (align < 1 || !gb_is_power_of_two(cast(isize)align)) {
				error(node, "#align must be a power of 2, got %lld", align);
				return false;
			}
			*align_ = align;
			return true;
		}
	}

	error(node, "#align must be an integer");
	return false;
}


Entity *find_polymorphic_record_entity(CheckerContext *ctx, Type *original_type, isize param_count, Array<Operand> const &ordered_operands, bool *failure) {
	mutex_lock(&ctx->info->gen_types_mutex);
	defer (mutex_unlock(&ctx->info->gen_types_mutex));

	auto *found_gen_types = map_get(&ctx->info->gen_types, original_type);
	if (found_gen_types != nullptr) {
		// GB_ASSERT_MSG(ordered_operands.count >= param_count, "%td >= %td", ordered_operands.count, param_count);

		for_array(i, *found_gen_types) {
			Entity *e = (*found_gen_types)[i];
			Type *t = base_type(e->type);
			TypeTuple *tuple = get_record_polymorphic_params(t);
			GB_ASSERT(param_count == tuple->variables.count);

			bool skip = false;

			for (isize j = 0; j < param_count; j++) {
				Entity *p = tuple->variables[j];
				Operand o = {};
				if (j < ordered_operands.count) {
					o = ordered_operands[j];
				}
				if (o.expr == nullptr) {
					continue;
				}
				Entity *oe = entity_of_node(o.expr);
				if (p == oe) {
					// NOTE(bill): This is the same type, make sure that it will be be same thing and use that
					// Saves on a lot of checking too below
					continue;
				}

				if (p->kind == Entity_TypeName) {
					if (is_type_polymorphic(o.type)) {
						// NOTE(bill): Do not add polymorphic version to the gen_types
						skip = true;
						break;
					}
					if (!are_types_identical(o.type, p->type)) {
						skip = true;
						break;
					}
				} else if (p->kind == Entity_Constant) {
					if (!compare_exact_values(Token_CmpEq, o.value, p->Constant.value)) {
						skip = true;
						break;
					}
					if (!are_types_identical(o.type, p->type)) {
						skip = true;
						break;
					}
				} else {
					GB_PANIC("Unknown entity kind");
				}
			}
			if (!skip) {
				return e;
			}
		}
	}
	return nullptr;
}


void add_polymorphic_record_entity(CheckerContext *ctx, Ast *node, Type *named_type, Type *original_type) {
	GB_ASSERT(is_type_named(named_type));
	gbAllocator a = heap_allocator();
	Scope *s = ctx->scope->parent;

	Entity *e = nullptr;
	{
		Token token = ast_token(node);
		token.kind = Token_String;
		token.string = named_type->Named.name;

		Ast *node = ast_ident(nullptr, token);

		e = alloc_entity_type_name(s, token, named_type);
		e->state = EntityState_Resolved;
		e->file = ctx->file;
		e->pkg = ctx->pkg;
		add_entity_use(ctx, node, e);
	}

	named_type->Named.type_name = e;
	GB_ASSERT(original_type->kind == Type_Named);
	e->TypeName.objc_class_name = original_type->Named.type_name->TypeName.objc_class_name;
	// TODO(bill): Is this even correct? Or should the metadata be copied?
	e->TypeName.objc_metadata = original_type->Named.type_name->TypeName.objc_metadata;

	mutex_lock(&ctx->info->gen_types_mutex);
	auto *found_gen_types = map_get(&ctx->info->gen_types, original_type);
	if (found_gen_types) {
		array_add(found_gen_types, e);
	} else {
		auto array = array_make<Entity *>(heap_allocator());
		array_add(&array, e);
		map_set(&ctx->info->gen_types, original_type, array);
	}
	mutex_unlock(&ctx->info->gen_types_mutex);
}

Type *check_record_polymorphic_params(CheckerContext *ctx, Ast *polymorphic_params,
                                      bool *is_polymorphic_,
                                      Ast *node, Array<Operand> *poly_operands,
                                      Type *named_type, Type *original_type_for_poly) {
	Type *polymorphic_params_type = nullptr;
	bool can_check_fields = true;
	GB_ASSERT(is_polymorphic_ != nullptr);

	if (polymorphic_params == nullptr) {
		if (!*is_polymorphic_) {
			*is_polymorphic_ = polymorphic_params != nullptr && poly_operands == nullptr;
		}
		return polymorphic_params_type;
	}

	ast_node(field_list, FieldList, polymorphic_params);
	Slice<Ast *> params = field_list->list;
	if (params.count != 0) {
		isize variable_count = 0;
		for_array(i, params) {
			Ast *field = params[i];
			if (ast_node_expect(field, Ast_Field)) {
				ast_node(f, Field, field);
				variable_count += gb_max(f->names.count, 1);
			}
		}

		auto entities = array_make<Entity *>(permanent_allocator(), 0, variable_count);

		for_array(i, params) {
			Ast *param = params[i];
			if (param->kind != Ast_Field) {
				continue;
			}
			ast_node(p, Field, param);
			Ast *type_expr = p->type;
			Ast *default_value = unparen_expr(p->default_value);
			Type *type = nullptr;
			bool is_type_param = false;
			bool is_type_polymorphic_type = false;
			if (type_expr == nullptr && default_value == nullptr) {
				error(param, "Expected a type for this parameter");
				continue;
			}

			if (type_expr != nullptr) {
				if (type_expr->kind == Ast_Ellipsis) {
					type_expr = type_expr->Ellipsis.expr;
					error(param, "A polymorphic parameter cannot be variadic");
				}
				if (type_expr->kind == Ast_TypeidType) {
					is_type_param = true;
					Type *specialization = nullptr;
					if (type_expr->TypeidType.specialization != nullptr) {
						Ast *s = type_expr->TypeidType.specialization;
						specialization = check_type(ctx, s);
					}
					type = alloc_type_generic(ctx->scope, 0, str_lit(""), specialization);
				} else {
					type = check_type(ctx, type_expr);
					if (is_type_polymorphic(type)) {
						is_type_polymorphic_type = true;
					}
				}
			}

			ParameterValue param_value = {};
			if (default_value != nullptr)  {
				Type *out_type = nullptr;
				param_value = handle_parameter_value(ctx, type, &out_type, default_value, false);
				if (type == nullptr && out_type != nullptr) {
					type = out_type;
				}
				if (param_value.kind != ParameterValue_Constant && param_value.kind != ParameterValue_Nil) {
					error(default_value, "Invalid parameter value");
					param_value = {};
				}
			}


			if (type == nullptr) {
				error(params[i], "Invalid parameter type");
				type = t_invalid;
			}
			if (is_type_untyped(type)) {
				if (is_type_untyped_undef(type)) {
					error(params[i], "Cannot determine parameter type from ---");
				} else {
					error(params[i], "Cannot determine parameter type from a nil");
				}
				type = t_invalid;
			}

			if (is_type_polymorphic_type) {
				gbString str = type_to_string(type);
				error(params[i], "Parameter types cannot be polymorphic, got %s", str);
				gb_string_free(str);
				type = t_invalid;
			}

			if (!is_type_param && !is_type_constant_type(type)) {
				gbString str = type_to_string(type);
				error(params[i], "A parameter must be a valid constant type, got %s", str);
				gb_string_free(str);
			}

			Scope *scope = ctx->scope;
			for_array(j, p->names) {
				Ast *name = p->names[j];
				if (!ast_node_expect2(name, Ast_Ident, Ast_PolyType)) {
					continue;
				}
				if (name->kind == Ast_PolyType) {
					name = name->PolyType.type;
				}
				Entity *e = nullptr;

				Token token = name->Ident.token;

				if (poly_operands != nullptr) {
					Operand operand = {};
					operand.type = t_invalid;
					if (entities.count < poly_operands->count) {
						operand = (*poly_operands)[entities.count];
					} else if (param_value.kind != ParameterValue_Invalid) {
						operand.mode = Addressing_Constant;
						operand.value = param_value.value;
					}
					if (is_type_param) {
						if (is_type_polymorphic(base_type(operand.type))) {
							*is_polymorphic_ = true;
							can_check_fields = false;
						}
						e = alloc_entity_type_name(scope, token, operand.type);
						e->TypeName.is_type_alias = true;
						e->flags |= EntityFlag_PolyConst;
					} else {
						if (is_type_polymorphic(base_type(operand.type))) {
							*is_polymorphic_ = true;
							can_check_fields = false;
						}
						if (e == nullptr) {
							e = alloc_entity_constant(scope, token, operand.type, operand.value);
							e->Constant.param_value = param_value;
						}
					}
				} else {
					if (is_type_param) {
						e = alloc_entity_type_name(scope, token, type);
						e->TypeName.is_type_alias = true;
						e->flags |= EntityFlag_PolyConst;
					} else {
						e = alloc_entity_constant(scope, token, type, param_value.value);
						e->Constant.param_value = param_value;
					}
				}

				e->state = EntityState_Resolved;
				add_entity(ctx, scope, name, e);
				array_add(&entities, e);
			}
		}

		if (entities.count > 0) {
			Type *tuple = alloc_type_tuple();
			tuple->Tuple.variables = slice_from_array(entities);
			polymorphic_params_type = tuple;
		}
	}

	if (original_type_for_poly != nullptr) {
		GB_ASSERT(named_type != nullptr);
		add_polymorphic_record_entity(ctx, node, named_type, original_type_for_poly);
	}

	if (!*is_polymorphic_) {
		*is_polymorphic_ = polymorphic_params != nullptr && poly_operands == nullptr;
	}

	return polymorphic_params_type;
}

bool check_record_poly_operand_specialization(CheckerContext *ctx, Type *record_type, Array<Operand> *poly_operands, bool *is_polymorphic_) {
	if (poly_operands == nullptr) {
		return false;
	}
	for (isize i = 0; i < poly_operands->count; i++) {
		Operand o = (*poly_operands)[i];
		if (is_type_polymorphic(o.type)) {
			return false;
		}
		if (record_type == o.type) {
			// NOTE(bill): Cycle
			return false;
		}
		if (o.mode == Addressing_Type) {
			// NOTE(bill): ANNOYING EDGE CASE FOR `where` clauses
			// TODO(bill, 2021-03-27): Is this even a valid HACK?!
			Entity *entity = entity_of_node(o.expr);
			if (entity != nullptr &&
			    entity->kind == Entity_TypeName &&
			    entity->type == t_typeid) {
			    	*is_polymorphic_ = true;
				return false;
			}
		}
	}
	return true;
}


void check_struct_type(CheckerContext *ctx, Type *struct_type, Ast *node, Array<Operand> *poly_operands, Type *named_type, Type *original_type_for_poly) {
	GB_ASSERT(is_type_struct(struct_type));
	ast_node(st, StructType, node);

	String context = str_lit("struct");

	isize min_field_count = 0;
	for_array(field_index, st->fields) {
	Ast *field = st->fields[field_index];
		switch (field->kind) {
		case_ast_node(f, ValueDecl, field);
			min_field_count += f->names.count;
		case_end;
		case_ast_node(f, Field, field);
			min_field_count += f->names.count;
		case_end;
		}
	}
	
	scope_reserve(ctx->scope, min_field_count);

	if (st->is_raw_union && min_field_count > 1) {
		struct_type->Struct.is_raw_union = true;
		context = str_lit("struct #raw_union");
	}

	struct_type->Struct.scope     = ctx->scope;
	struct_type->Struct.is_packed = st->is_packed;
	struct_type->Struct.polymorphic_params = check_record_polymorphic_params(
		ctx, st->polymorphic_params,
		&struct_type->Struct.is_polymorphic,
		node, poly_operands,
		named_type, original_type_for_poly
	);;
	struct_type->Struct.is_poly_specialized = check_record_poly_operand_specialization(ctx, struct_type, poly_operands, &struct_type->Struct.is_polymorphic);

	if (!struct_type->Struct.is_polymorphic) {
		if (st->where_clauses.count > 0 && st->polymorphic_params == nullptr) {
			error(st->where_clauses[0], "'where' clauses can only be used on structures with polymorphic parameters");
		} else {
			bool where_clause_ok = evaluate_where_clauses(ctx, node, ctx->scope, &st->where_clauses, true);
			gb_unused(where_clause_ok);
		}
		check_struct_fields(ctx, node, &struct_type->Struct.fields, &struct_type->Struct.tags, st->fields, min_field_count, struct_type, context);
	}

	if (st->align != nullptr) {
		if (st->is_packed) {
			syntax_error(st->align, "'#align' cannot be applied with '#packed'");
			return;
		}
		i64 custom_align = 1;
		if (check_custom_align(ctx, st->align, &custom_align)) {
			struct_type->Struct.custom_align = custom_align;
		}
	}
}
void check_union_type(CheckerContext *ctx, Type *union_type, Ast *node, Array<Operand> *poly_operands, Type *named_type, Type *original_type_for_poly) {
	GB_ASSERT(is_type_union(union_type));
	ast_node(ut, UnionType, node);


	union_type->Union.scope = ctx->scope;
	union_type->Union.polymorphic_params = check_record_polymorphic_params(
		ctx, ut->polymorphic_params,
		&union_type->Union.is_polymorphic,
		node, poly_operands,
		named_type, original_type_for_poly
	);
	union_type->Union.is_poly_specialized = check_record_poly_operand_specialization(ctx, union_type, poly_operands, &union_type->Union.is_polymorphic);

	if (!union_type->Union.is_polymorphic) {
		if (ut->where_clauses.count > 0 && ut->polymorphic_params == nullptr) {
			error(ut->where_clauses[0], "'where' clauses can only be used on unions with polymorphic parameters");
		} else {
			bool where_clause_ok = evaluate_where_clauses(ctx, node, ctx->scope, &ut->where_clauses, true);
			gb_unused(where_clause_ok);
		}
	}

	auto variants = array_make<Type *>(permanent_allocator(), 0, ut->variants.count);

	for_array(i, ut->variants) {
		Ast *node = ut->variants[i];
		Type *t = check_type_expr(ctx, node, nullptr);
		if (t != nullptr && t != t_invalid) {
			bool ok = true;
			t = default_type(t);
			if (is_type_untyped(t) || is_type_empty_union(t)) {
				ok = false;
				gbString str = type_to_string(t);
				error(node, "Invalid variant type in union '%s'", str);
				gb_string_free(str);
			} else {
				for_array(j, variants) {
					if (are_types_identical(t, variants[j])) {
						ok = false;
						gbString str = type_to_string(t);
						error(node, "Duplicate variant type '%s'", str);
						gb_string_free(str);
						break;
					}
				}
			}
			if (ok) {
				array_add(&variants, t);
			}
		}
	}

	union_type->Union.variants = slice_from_array(variants);
	union_type->Union.no_nil = ut->no_nil;
	union_type->Union.maybe = ut->maybe;
	if (union_type->Union.no_nil) {
		if (variants.count < 2) {
			error(ut->align, "A union with #no_nil must have at least 2 variants");
		}
	}
	if (union_type->Union.maybe) {
		if (variants.count != 1) {
			error(ut->align, "A union with #maybe must have at 1 variant, got %lld", cast(long long)variants.count);
		}
	}

	if (ut->align != nullptr) {
		i64 custom_align = 1;
		if (check_custom_align(ctx, ut->align, &custom_align)) {
			if (variants.count == 0) {
				error(ut->align, "An empty union cannot have a custom alignment");
			} else {
				union_type->Union.custom_align = custom_align;
			}
		}
	}
}

void check_enum_type(CheckerContext *ctx, Type *enum_type, Type *named_type, Ast *node) {
	ast_node(et, EnumType, node);
	GB_ASSERT(is_type_enum(enum_type));

	Type *base_type = t_int;
	if (et->base_type != nullptr) {
		base_type = check_type(ctx, et->base_type);
	}

	if (base_type == nullptr || !is_type_integer(base_type)) {
		error(node, "Base type for enumeration must be an integer");
		return;
	}
	if (is_type_enum(base_type)) {
		error(node, "Base type for enumeration cannot be another enumeration");
		return;
	}

	if (is_type_integer_128bit(base_type)) {
		error(node, "Base type for enumeration cannot be a 128-bit integer");
		return;
	}

	// NOTE(bill): Must be up here for the 'check_init_constant' system
	enum_type->Enum.base_type = base_type;
	enum_type->Enum.scope = ctx->scope;

	auto fields = array_make<Entity *>(permanent_allocator(), 0, et->fields.count);

	Type *constant_type = enum_type;
	if (named_type != nullptr) {
		constant_type = named_type;
	}

	ExactValue iota = exact_value_i64(-1);
	ExactValue min_value = exact_value_i64(0);
	ExactValue max_value = exact_value_i64(0);
	isize min_value_index = 0;
	isize max_value_index = 0;
	bool min_value_set = false;
	bool max_value_set = false;

	scope_reserve(ctx->scope, et->fields.count);

	for_array(i, et->fields) {
		Ast *field = et->fields[i];
		Ast *ident = nullptr;
		Ast *init = nullptr;
		u32 entity_flags = 0;
		if (field->kind != Ast_EnumFieldValue) {
			error(field, "An enum field's name must be an identifier");
			continue;
		}
		ident = field->EnumFieldValue.name;
		init = field->EnumFieldValue.value;
		if (ident == nullptr || ident->kind != Ast_Ident) {
			error(field, "An enum field's name must be an identifier");
			continue;
		}
		CommentGroup *docs    = field->EnumFieldValue.docs;
		CommentGroup *comment = field->EnumFieldValue.comment;

		String name = ident->Ident.token.string;

		if (init != nullptr) {
			Operand o = {};
			check_expr(ctx, &o, init);
			if (o.mode != Addressing_Constant) {
				error(init, "Enumeration value must be a constant");
				o.mode = Addressing_Invalid;
			}
			if (o.mode != Addressing_Invalid) {
				check_assignment(ctx, &o, constant_type, str_lit("enumeration"));
			}
			if (o.mode != Addressing_Invalid) {
				iota = o.value;
			} else {
				iota = exact_binary_operator_value(Token_Add, iota, exact_value_i64(1));
			}
		} else {
			iota = exact_binary_operator_value(Token_Add, iota, exact_value_i64(1));
			entity_flags |= EntityConstantFlag_ImplicitEnumValue;
		}


		// NOTE(bill): Skip blank identifiers
		if (is_blank_ident(name)) {
			continue;
		} else if (name == "names") {
			error(field, "'names' is a reserved identifier for enumerations");
			continue;
		}

		if (min_value_set) {
			if (compare_exact_values(Token_Gt, min_value, iota)) {
				min_value_index = i;
				min_value = iota;
			}
		} else {
			min_value_index = i;
			min_value = iota;
			min_value_set = true;
		}
		if (max_value_set) {
			if (compare_exact_values(Token_Lt, max_value, iota)) {
				max_value_index = i;
				max_value = iota;
			}
		} else {
			max_value_index = i;
			max_value = iota;
			max_value_set = true;
		}

		Entity *e = alloc_entity_constant(ctx->scope, ident->Ident.token, constant_type, iota);
		e->identifier = ident;
		e->flags |= EntityFlag_Visited;
		e->state = EntityState_Resolved;
		e->Constant.flags |= entity_flags;
		e->Constant.docs = docs;
		e->Constant.comment = comment;

		if (scope_lookup_current(ctx->scope, name) != nullptr) {
			error(ident, "'%.*s' is already declared in this enumeration", LIT(name));
		} else {
			add_entity(ctx, ctx->scope, nullptr, e);
			array_add(&fields, e);
			// TODO(bill): Should I add a use for the enum value?
			add_entity_use(ctx, field, e);
		}
	}
	GB_ASSERT(fields.count <= et->fields.count);


	enum_type->Enum.fields = fields;
	*enum_type->Enum.min_value = min_value;
	*enum_type->Enum.max_value = max_value;

	enum_type->Enum.min_value_index = min_value_index;
	enum_type->Enum.max_value_index = max_value_index;
}

bool is_type_valid_bit_set_range(Type *t) {
	if (is_type_integer(t)) {
		return true;
	}
	if (is_type_rune(t)) {
		return true;
	}
	return false;
}

void check_bit_set_type(CheckerContext *c, Type *type, Type *named_type, Ast *node) {
	ast_node(bs, BitSetType, node);
	GB_ASSERT(type->kind == Type_BitSet);
	type->BitSet.node = node;

	/* i64 const DEFAULT_BITS = cast(i64)(8*build_context.word_size); */
	i64 const MAX_BITS = 128;

	Ast *base = unparen_expr(bs->elem);
	if (is_ast_range(base)) {
		ast_node(be, BinaryExpr, base);
		Operand lhs = {};
		Operand rhs = {};
		check_expr(c, &lhs, be->left);
		check_expr(c, &rhs, be->right);
		if (lhs.mode == Addressing_Invalid || rhs.mode == Addressing_Invalid) {
			return;
		}
		convert_to_typed(c, &lhs, rhs.type);
		if (lhs.mode == Addressing_Invalid) {
			return;
		}
		convert_to_typed(c, &rhs, lhs.type);
		if (rhs.mode == Addressing_Invalid) {
			return;
		}
		if (!are_types_identical(lhs.type, rhs.type)) {
			if (lhs.type != t_invalid &&
			    rhs.type != t_invalid) {
				gbString xt = type_to_string(lhs.type);
				gbString yt = type_to_string(rhs.type);
				gbString expr_str = expr_to_string(bs->elem);
				error(bs->elem, "Mismatched types in range '%s' : '%s' vs '%s'", expr_str, xt, yt);
				gb_string_free(expr_str);
				gb_string_free(yt);
				gb_string_free(xt);
			}
			return;
		}

		if (!is_type_valid_bit_set_range(lhs.type)) {
			gbString str = type_to_string(lhs.type);
			error(bs->elem, "'%s' is invalid for an interval expression, expected an integer or rune", str);
			gb_string_free(str);
			return;
		}

		if (lhs.mode != Addressing_Constant || rhs.mode != Addressing_Constant) {
			error(bs->elem, "Intervals must be constant values");
			return;
		}

		ExactValue iv = exact_value_to_integer(lhs.value);
		ExactValue jv = exact_value_to_integer(rhs.value);
		GB_ASSERT(iv.kind == ExactValue_Integer);
		GB_ASSERT(jv.kind == ExactValue_Integer);

		BigInt i = iv.value_integer;
		BigInt j = jv.value_integer;
		if (big_int_cmp(&i, &j) > 0) {
			gbAllocator a = heap_allocator();
			String si = big_int_to_string(a, &i);
			String sj = big_int_to_string(a, &j);
			error(bs->elem, "Lower interval bound larger than upper bound, %.*s .. %.*s", LIT(si), LIT(sj));
			gb_free(a, si.text);
			gb_free(a, sj.text);
			return;
		}

		Type *t = default_type(lhs.type);
		if (bs->underlying != nullptr) {
			Type *u = check_type(c, bs->underlying);
			if (!is_type_integer(u)) {
				gbString ts = type_to_string(u);
				error(bs->underlying, "Expected an underlying integer for the bit set, got %s", ts);
				gb_string_free(ts);
				return;
			}
			type->BitSet.underlying = u;
		}

		if (!check_representable_as_constant(c, iv, t, nullptr)) {
			gbAllocator a = heap_allocator();
			String s = big_int_to_string(a, &i);
			gbString ts = type_to_string(t);
			error(bs->elem, "%.*s is not representable by %s", LIT(s), ts);
			gb_string_free(ts);
			gb_free(a, s.text);
			return;
		}
		if (!check_representable_as_constant(c, iv, t, nullptr)) {
			gbAllocator a = heap_allocator();
			String s = big_int_to_string(a, &j);
			gbString ts = type_to_string(t);
			error(bs->elem, "%.*s is not representable by %s", LIT(s), ts);
			gb_string_free(ts);
			gb_free(a, s.text);
			return;
		}
		i64 lower = big_int_to_i64(&i);
		i64 upper = big_int_to_i64(&j);
		
		i64 actual_lower = lower;
		i64 bits = MAX_BITS;
		if (type->BitSet.underlying != nullptr) {
			bits = 8*type_size_of(type->BitSet.underlying);
			
			if (lower > 0) {
				actual_lower = 0;
			} else if (lower < 0) {
				error(bs->elem, "bit_set does not allow a negative lower bound (%lld) when an underlying type is set", lower);
			}
		}

		i64 bits_required = upper-actual_lower;
		switch (be->op.kind) {
		case Token_Ellipsis:
		case Token_RangeFull:
			bits_required += 1;
			break;
		}
		bool is_valid = true;

		switch (be->op.kind) {
		case Token_Ellipsis:
		case Token_RangeFull:
			if (upper - lower >= bits) {
				is_valid = false;
			}
			break;
		case Token_RangeHalf:
			if (upper - lower > bits) {
				is_valid = false;
			}
			upper -= 1;
			break;
		}
		if (!is_valid) {
			if (actual_lower != lower) {
				error(bs->elem, "bit_set range is greater than %lld bits, %lld bits are required (internal the lower changed was changed 0 as an underlying type was set)", bits, bits_required);
			} else {
				error(bs->elem, "bit_set range is greater than %lld bits, %lld bits are required", bits, bits_required);
			}
		}
		
		type->BitSet.elem  = t;
		type->BitSet.lower = lower;
		type->BitSet.upper = upper;
	} else {
		Type *elem = check_type_expr(c, bs->elem, nullptr);

		#if 0
		if (named_type != nullptr && named_type->kind == Type_Named &&
		    elem->kind == Type_Enum) {
			// NOTE(bill): Anonymous enumeration

			String prefix = named_type->Named.name;
			String enum_name = concatenate_strings(heap_allocator(), prefix, str_lit(".enum"));

			Token token = make_token_ident(enum_name);

			Entity *e = alloc_entity_type_name(nullptr, token, nullptr, EntityState_Resolved);
			Type *named = alloc_type_named(enum_name, elem, e);
			e->type = named;
			e->TypeName.is_type_alias = true;
			elem = named;
		}
		#endif

		type->BitSet.elem = elem;
		if (!is_type_valid_bit_set_elem(elem)) {
			error(bs->elem, "Expected an enum type for a bit_set");
		} else {
			Type *et = base_type(elem);
			if (et->kind == Type_Enum) {
				if (!is_type_integer(et->Enum.base_type)) {
					error(bs->elem, "Enum type for bit_set must be an integer");
					return;
				}
				i64 lower = I64_MAX;
				i64 upper = I64_MIN;

				for_array(i, et->Enum.fields) {
					Entity *e = et->Enum.fields[i];
					if (e->kind != Entity_Constant) {
						continue;
					}
					ExactValue value = exact_value_to_integer(e->Constant.value);
					GB_ASSERT(value.kind == ExactValue_Integer);
					// NOTE(bill): enum types should be able to store i64 values
					i64 x = big_int_to_i64(&value.value_integer);
					lower = gb_min(lower, x);
					upper = gb_max(upper, x);
				}
				if (et->Enum.fields.count == 0) {
					lower = 0;
					upper = 0;
				}

				GB_ASSERT(lower <= upper);
				
				bool lower_changed = false;
				i64 bits = MAX_BITS
;				if (bs->underlying != nullptr) {
					Type *u = check_type(c, bs->underlying);
					if (!is_type_integer(u)) {
						gbString ts = type_to_string(u);
						error(bs->underlying, "Expected an underlying integer for the bit set, got %s", ts);
						gb_string_free(ts);
						return;
					}
					type->BitSet.underlying = u;
					bits = 8*type_size_of(u);
					
					if (lower > 0) {
						lower = 0;
						lower_changed = true;
					} else if (lower < 0) {
						gbString s = type_to_string(elem);
						error(bs->elem, "bit_set does not allow a negative lower bound (%lld) of the element type '%s' when an underlying type is set", lower, s);
						gb_string_free(s);
					}
				}

				if (upper - lower >= bits) {
					i64 bits_required = upper-lower+1;
					if (lower_changed) {
						error(bs->elem, "bit_set range is greater than %lld bits, %lld bits are required (internal the lower changed was changed 0 as an underlying type was set)", bits, bits_required);
					} else {
						error(bs->elem, "bit_set range is greater than %lld bits, %lld bits are required", bits, bits_required);
					}
				}

				type->BitSet.lower = lower;
				type->BitSet.upper = upper;
			}
		}
	}	
}


bool check_type_specialization_to(CheckerContext *ctx, Type *specialization, Type *type, bool compound, bool modify_type) {
	if (type == nullptr ||
	    type == t_invalid) {
		return true;
	}

	Type *t = base_type(type);
	Type *s = base_type(specialization);
	if (t->kind != s->kind) {
		if (t->kind == Type_EnumeratedArray && s->kind == Type_Array) {
			// Might be okay, check later
		} else {
			return false;
		}
	}

	if (is_type_untyped(t)) {
		Operand o = {Addressing_Value};
		o.type = default_type(type);
		bool can_convert = check_cast_internal(ctx, &o, specialization);
		return can_convert;
	} else if (t->kind == Type_Struct) {
		if (t->Struct.polymorphic_parent == specialization) {
			return true;
		}

		if (t->Struct.polymorphic_parent == s->Struct.polymorphic_parent &&
		    s->Struct.polymorphic_params != nullptr &&
		    t->Struct.polymorphic_params != nullptr) {

			TypeTuple *s_tuple = &s->Struct.polymorphic_params->Tuple;
			TypeTuple *t_tuple = &t->Struct.polymorphic_params->Tuple;
			GB_ASSERT(t_tuple->variables.count == s_tuple->variables.count);
			for_array(i, s_tuple->variables) {
				Entity *s_e = s_tuple->variables[i];
				Entity *t_e = t_tuple->variables[i];
				Type *st = s_e->type;
				Type *tt = t_e->type;

				// NOTE(bill, 2018-12-14): This is needed to override polymorphic named constants in types
				if (st->kind == Type_Generic && t_e->kind == Entity_Constant) {
					Entity *e = scope_lookup(st->Generic.scope, st->Generic.name);
					GB_ASSERT(e != nullptr);
					if (modify_type) {
						e->kind = Entity_Constant;
						e->Constant.value = t_e->Constant.value;
						e->type = t_e->type;
					}
				} else {
					if (st->kind == Type_Basic && tt->kind == Type_Basic &&
						s_e->kind == Entity_Constant && t_e->kind == Entity_Constant) {
						if (!compare_exact_values(Token_CmpEq, s_e->Constant.value, t_e->Constant.value))
							return false;
					} else {
						bool ok = is_polymorphic_type_assignable(ctx, st, tt, true, modify_type);
						if (!ok) {
							// TODO(bill, 2021-08-19): is this logic correct?
							return false;
						}
					}
				}
			}

			if (modify_type) {
				// NOTE(bill): This is needed in order to change the actual type but still have the types defined within it
				gb_memmove(specialization, type, gb_size_of(Type));
			}

			return true;
		}
	} else if (t->kind == Type_Union) {
		if (t->Union.polymorphic_parent == specialization) {
			return true;
		}

		if (t->Union.polymorphic_parent == s->Union.polymorphic_parent &&
		    s->Union.polymorphic_params != nullptr &&
		    t->Union.polymorphic_params != nullptr) {

			TypeTuple *s_tuple = &s->Union.polymorphic_params->Tuple;
			TypeTuple *t_tuple = &t->Union.polymorphic_params->Tuple;
			GB_ASSERT(t_tuple->variables.count == s_tuple->variables.count);
			for_array(i, s_tuple->variables) {
				Entity *s_e = s_tuple->variables[i];
				Entity *t_e = t_tuple->variables[i];
				Type *st = s_e->type;
				Type *tt = t_e->type;

				// NOTE(bill, 2018-12-14): This is needed to override polymorphic named constants in types
				if (st->kind == Type_Generic && t_e->kind == Entity_Constant) {
					Entity *e = scope_lookup(st->Generic.scope, st->Generic.name);
					GB_ASSERT(e != nullptr);
					if (modify_type) {
						e->kind = Entity_Constant;
						e->Constant.value = t_e->Constant.value;
						e->type = t_e->type;
					}
				} else {
					bool ok = is_polymorphic_type_assignable(ctx, st, tt, true, modify_type);
					if (!ok) {
						// TODO(bill, 2021-08-19): is this logic correct?
						return false;
					}
				}
			}

			if (modify_type) {
				// NOTE(bill): This is needed in order to change the actual type but still have the types defined within it
				gb_memmove(specialization, type, gb_size_of(Type));
			}

			return true;
		}
	}

	if (specialization->kind == Type_Named &&
	    type->kind != Type_Named) {
		return false;
	}
	if (is_polymorphic_type_assignable(ctx, base_type(specialization), base_type(type), compound, modify_type)) {
		return true;
	}

	return false;
}


Type *determine_type_from_polymorphic(CheckerContext *ctx, Type *poly_type, Operand operand) {
	bool modify_type = !ctx->no_polymorphic_errors;
	bool show_error = modify_type && !ctx->hide_polymorphic_errors;
	if (!is_operand_value(operand)) {
		if (show_error) {
			gbString pts = type_to_string(poly_type);
			gbString ots = type_to_string(operand.type);
			defer (gb_string_free(pts));
			defer (gb_string_free(ots));
			error(operand.expr, "Cannot determine polymorphic type from parameter: '%s' to '%s'", ots, pts);
		}
		return t_invalid;
	}

	if (is_polymorphic_type_assignable(ctx, poly_type, operand.type, false, modify_type)) {
		return poly_type;
	}
	if (show_error) {
		gbString pts = type_to_string(poly_type);
		gbString ots = type_to_string(operand.type);
		defer (gb_string_free(pts));
		defer (gb_string_free(ots));
		error(operand.expr, "Cannot determine polymorphic type from parameter: '%s' to '%s'", ots, pts);
	}
	return t_invalid;
}

bool is_expr_from_a_parameter(CheckerContext *ctx, Ast *expr) {
	if (expr == nullptr) {
		return false;
	}
	expr = unparen_expr(expr);
	if (expr->kind == Ast_SelectorExpr) {
		Ast *lhs = expr->SelectorExpr.expr;
		return is_expr_from_a_parameter(ctx, lhs);
	} else if (expr->kind == Ast_Ident) {
		Operand x= {};
		Entity *e = check_ident(ctx, &x, expr, nullptr, nullptr, false);
		if (e->flags & EntityFlag_Param) {
			return true;
		}
	}
	return false;
}


ParameterValue handle_parameter_value(CheckerContext *ctx, Type *in_type, Type **out_type_, Ast *expr, bool allow_caller_location) {
	ParameterValue param_value = {};
	param_value.original_ast_expr = expr;
	if (expr == nullptr) {
		return param_value;
	}
	Operand o = {};

	if (allow_caller_location &&
	    expr->kind == Ast_BasicDirective &&
	    expr->BasicDirective.name.string == "caller_location") {
		init_core_source_code_location(ctx->checker);
		param_value.kind = ParameterValue_Location;
		o.type = t_source_code_location;

		if (in_type) {
			check_assignment(ctx, &o, in_type, str_lit("parameter value"));
		}

	} else {
		if (in_type) {
			check_expr_with_type_hint(ctx, &o, expr, in_type);
		} else {
			check_expr(ctx, &o, expr);
		}

		if (in_type) {
			check_assignment(ctx, &o, in_type, str_lit("parameter value"));
		}


		if (is_operand_nil(o)) {
			param_value.kind = ParameterValue_Nil;
		} else if (o.mode != Addressing_Constant) {
			if (expr->kind == Ast_ProcLit) {
				param_value.kind = ParameterValue_Constant;
				param_value.value = exact_value_procedure(expr);
			} else {
				Entity *e = entity_from_expr(o.expr);

				if (e != nullptr) {
					if (e->kind == Entity_Procedure) {
						param_value.kind = ParameterValue_Constant;
						param_value.value = exact_value_procedure(e->identifier);
						add_entity_use(ctx, e->identifier, e);
					} else {
						if (e->flags & EntityFlag_Param) {
							error(expr, "Default parameter cannot be another parameter");
						} else {
							if (is_expr_from_a_parameter(ctx, expr)) {
								error(expr, "Default parameter cannot be another parameter");
							} else {
								param_value.kind = ParameterValue_Value;
								param_value.ast_value = expr;
								add_entity_use(ctx, e->identifier, e);
							}
						}
					}
				} else if (allow_caller_location && o.mode == Addressing_Context) {
					param_value.kind = ParameterValue_Value;
					param_value.ast_value = expr;
				} else if (o.value.kind != ExactValue_Invalid) {
					param_value.kind = ParameterValue_Constant;
					param_value.value = o.value;
				} else {
					error(expr, "Default parameter must be a constant, %d", o.mode);
				}
			}
		} else {
			if (o.value.kind != ExactValue_Invalid) {
				param_value.kind = ParameterValue_Constant;
				param_value.value = o.value;
			} else {
				gbString s = expr_to_string(o.expr);
				error(o.expr, "Invalid constant parameter, got '%s'", s);
				// error(o.expr, "Invalid constant parameter, got '%s' %d %d", s, o.mode, o.value.kind);
				gb_string_free(s);
			}
		}
	}

	if (out_type_) {
		if (in_type != nullptr) {
			*out_type_ = in_type;
		} else {
			*out_type_ = default_type(o.type);
		}
	}

	return param_value;
}


Type *check_get_params(CheckerContext *ctx, Scope *scope, Ast *_params, bool *is_variadic_, isize *variadic_index_, bool *success_, isize *specialization_count_, Array<Operand> *operands) {
	if (_params == nullptr) {
		return nullptr;
	}

	bool success = true;
	ast_node(field_list, FieldList, _params);
	Slice<Ast *> params = field_list->list;

	if (params.count == 0) {
		if (success_) *success_ = success;
		return nullptr;
	}

	isize variable_count = 0;
	for_array(i, params) {
		Ast *field = params[i];
		if (ast_node_expect(field, Ast_Field)) {
			ast_node(f, Field, field);
			variable_count += gb_max(f->names.count, 1);
		}
	}
	isize min_variable_count = variable_count;
	for (isize i = params.count-1; i >= 0; i--) {
		Ast *field = params[i];
		if (field->kind == Ast_Field) {
			ast_node(f, Field, field);
			if (f->default_value == nullptr)  {
				break;
			}
			min_variable_count--;
		}
	}


	bool is_variadic = false;
	isize variadic_index = -1;
	bool is_c_vararg = false;
	auto variables = array_make<Entity *>(permanent_allocator(), 0, variable_count);
	i32 field_group_index = -1;
	for_array(i, params) {
		Ast *param = params[i];
		if (param->kind != Ast_Field) {
			continue;
		}
		field_group_index += 1;
		ast_node(p, Field, param);
		Ast *type_expr = unparen_expr(p->type);
		Type *type = nullptr;
		Ast *default_value = unparen_expr(p->default_value);
		ParameterValue param_value = {};

		bool is_type_param = false;
		bool is_type_polymorphic_type = false;
		bool detemine_type_from_operand = false;
		Type *specialization = nullptr;

		bool is_using = (p->flags&FieldFlag_using) != 0;

		if (type_expr == nullptr) {
			param_value = handle_parameter_value(ctx, nullptr, &type, default_value, true);
		} else {
			if (type_expr->kind == Ast_Ellipsis) {
				type_expr = type_expr->Ellipsis.expr;
				is_variadic = true;
				variadic_index = variables.count;
				if (p->names.count != 1) {
					error(param, "Invalid AST: Invalid variadic parameter with multiple names");
					success = false;
				}
			}
			if (type_expr->kind == Ast_TypeidType)  {
				ast_node(tt, TypeidType, type_expr);
				if (tt->specialization) {
					specialization = check_type(ctx, tt->specialization);
					if (specialization == t_invalid){
						specialization = nullptr;
					}

					if (operands != nullptr) {
						detemine_type_from_operand = true;
						type = t_invalid;
					} else {
						type = alloc_type_generic(ctx->scope, 0, str_lit(""), specialization);
					}
				} else {
					type = t_typeid;
				}
			} else {
				bool prev = ctx->allow_polymorphic_types;
				if (operands != nullptr) {
					ctx->allow_polymorphic_types = true;
				}
				type = check_type(ctx, type_expr);

				ctx->allow_polymorphic_types = prev;

				if (is_type_polymorphic(type)) {
					is_type_polymorphic_type = true;
				}
			}

			if (default_value != nullptr) {
				if (type_expr != nullptr && type_expr->kind == Ast_TypeidType) {
					error(type_expr, "A type parameter may not have a default value");
				} else {
					param_value = handle_parameter_value(ctx, type, nullptr, default_value, true);
				}
			}
		}



		if (type == nullptr) {
			error(param, "Invalid parameter type");
			type = t_invalid;
		}
		if (is_type_untyped(type)) {
			if (is_type_untyped_undef(type)) {
				error(param, "Cannot determine parameter type from ---");
			} else {
				error(param, "Cannot determine parameter type from a nil");
			}
			type = t_invalid;
		}
		if (is_type_empty_union(type)) {
			gbString str = type_to_string(type);
			error(param, "Invalid use of an empty union '%s'", str);
			gb_string_free(str);
			type = t_invalid;
		}

		if (is_type_polymorphic(type)) {
			switch (param_value.kind) {
			case ParameterValue_Invalid:
			case ParameterValue_Constant:
			case ParameterValue_Nil:
				break;
			case ParameterValue_Location:
			case ParameterValue_Value:
				gbString str = type_to_string(type);
				error(params[i], "A default value for a parameter must not be a polymorphic constant type, got %s", str);
				gb_string_free(str);
				break;
			}
		}


		if (p->flags&FieldFlag_c_vararg) {
			if (p->type == nullptr ||
			    p->type->kind != Ast_Ellipsis) {
				error(param, "'#c_vararg' can only be applied to variadic type fields");
				p->flags &= ~FieldFlag_c_vararg; // Remove the flag
			} else {
				is_c_vararg = true;
			}
		}

		for_array(j, p->names) {
			Ast *name = p->names[j];

			bool is_poly_name = false;

			switch (name->kind) {
			case Ast_Ident:
				break;
			case Ast_PolyType:
				GB_ASSERT(name->PolyType.specialization == nullptr);
				is_poly_name = true;
				name = name->PolyType.type;
				break;
			}
			if (!ast_node_expect(name, Ast_Ident)) {
				continue;
			}

			if (is_poly_name) {
				if (type_expr != nullptr && type_expr->kind == Ast_TypeidType) {
					is_type_param = true;
				} else {
					if (param_value.kind != ParameterValue_Invalid)  {
						error(default_value, "Constant parameters cannot have a default value");
						param_value.kind = ParameterValue_Invalid;
					}
				}
			}

			Entity *param = nullptr;
			if (is_type_param) {
				if (operands != nullptr) {
					Operand o = (*operands)[variables.count];
					if (o.mode == Addressing_Type) {
						type = o.type;
					} else {
						if (!ctx->no_polymorphic_errors) {
							error(o.expr, "Expected a type to assign to the type parameter");
						}
						success = false;
						type = t_invalid;
					}
					if (is_type_polymorphic(type)) {
						gbString str = type_to_string(type);
						error(o.expr, "Cannot pass polymorphic type as a parameter, got '%s'", str);
						gb_string_free(str);
						success = false;
						type = t_invalid;
					}
					if (is_type_untyped(default_type(type))) {
						gbString str = type_to_string(type);
						error(o.expr, "Cannot determine type from the parameter, got '%s'", str);
						gb_string_free(str);
						success = false;
						type = t_invalid;
					}
					bool modify_type = !ctx->no_polymorphic_errors;

					if (specialization != nullptr && !check_type_specialization_to(ctx, specialization, type, false, modify_type)) {
						if (!ctx->no_polymorphic_errors) {
							gbString t = type_to_string(type);
							gbString s = type_to_string(specialization);
							error(o.expr, "Cannot convert type '%s' to the specialization '%s'", t, s);
							gb_string_free(s);
							gb_string_free(t);
						}
						success = false;
						type = t_invalid;
					}
				}

				if (p->flags&FieldFlag_auto_cast) {
					error(name, "'auto_cast' can only be applied to variable fields");
					p->flags &= ~FieldFlag_auto_cast;
				}
				if (p->flags&FieldFlag_const) {
					error(name, "'#const' can only be applied to variable fields");
					p->flags &= ~FieldFlag_const;
				}
				if (p->flags&FieldFlag_any_int) {
					error(name, "'#any_int' can only be applied to variable fields");
					p->flags &= ~FieldFlag_any_int;
				}

				param = alloc_entity_type_name(scope, name->Ident.token, type, EntityState_Resolved);
				param->TypeName.is_type_alias = true;
			} else {
				ExactValue poly_const = {};

				if (operands != nullptr && variables.count < operands->count) {

					Operand op = (*operands)[variables.count];
					if (op.expr == nullptr) {
						// NOTE(bill): 2019-03-30
						// This is just to add the error message to determine_type_from_polymorphic which
						// depends on valid position information
						op.expr = _params;
					}
					if (is_type_polymorphic_type) {
						type = determine_type_from_polymorphic(ctx, type, op);
						if (type == t_invalid) {
							success = false;
						} else if (!ctx->no_polymorphic_errors) {
							// NOTE(bill): The type should be determined now and thus, no need to determine the type any more
							is_type_polymorphic_type = false;
						}
					}
					if (is_poly_name) {
						bool valid = false;
						if (is_type_proc(op.type)) {
							Entity *proc_entity = entity_from_expr(op.expr);
							valid = proc_entity != nullptr;
							poly_const = exact_value_procedure(proc_entity->identifier.load() ? proc_entity->identifier.load() : op.expr);
						}
						if (!valid) {
							if (op.mode == Addressing_Constant) {
								poly_const = op.value;
							} else {
								error(op.expr, "Expected a constant value for this polymorphic name parameter");
								success = false;
							}
						}
					}
					if (type != t_invalid && !check_is_assignable_to(ctx, &op, type)) {
						bool ok = true;
						if (p->flags&FieldFlag_auto_cast) {
							if (!check_is_castable_to(ctx, &op, type)) {
								ok = false;
							}
						} else if (p->flags&FieldFlag_any_int) {
							if ((!is_type_integer(op.type) && !is_type_enum(op.type)) || (!is_type_integer(type) && !is_type_enum(type))) {
								ok = false;
							} else if (!check_is_castable_to(ctx, &op, type)) {
								ok = false;
							}
						}
						if (!ok) {
							success = false;
							#if 0
								gbString got = type_to_string(op.type);
								gbString expected = type_to_string(type);
								error(op.expr, "Cannot assigned type to parameter, got type '%s', expected '%s'", got, expected);
								gb_string_free(expected);
								gb_string_free(got);
							#endif
						}
					}

					if (is_type_untyped(default_type(type))) {
						gbString str = type_to_string(type);
						error(op.expr, "Cannot determine type from the parameter, got '%s'", str);
						gb_string_free(str);
						success = false;
						type = t_invalid;
					}
				}

				if (p->flags&FieldFlag_no_alias) {
					if (!is_type_pointer(type)) {
						error(name, "'#no_alias' can only be applied to fields of pointer type");
						p->flags &= ~FieldFlag_no_alias; // Remove the flag
					}
				}
				if (is_poly_name) {
					if (p->flags&FieldFlag_no_alias) {
						error(name, "'#no_alias' can only be applied to non constant values");
						p->flags &= ~FieldFlag_no_alias; // Remove the flag
					}
					if (p->flags&FieldFlag_auto_cast) {
						error(name, "'auto_cast' can only be applied to variable fields");
						p->flags &= ~FieldFlag_auto_cast;
					}
					if (p->flags&FieldFlag_any_int) {
						error(name, "'#any_int' can only be applied to variable fields");
						p->flags &= ~FieldFlag_any_int;
					}
					if (p->flags&FieldFlag_const) {
						error(name, "'#const' can only be applied to variable fields");
						p->flags &= ~FieldFlag_const;
					}

					if (!is_type_constant_type(type) && !is_type_polymorphic(type)) {
						gbString str = type_to_string(type);
						error(params[i], "A parameter must be a valid constant type, got %s", str);
						gb_string_free(str);
					}

					param = alloc_entity_const_param(scope, name->Ident.token, type, poly_const, is_type_polymorphic(type));
					param->Constant.field_group_index = field_group_index;
				} else {
					param = alloc_entity_param(scope, name->Ident.token, type, is_using, true);
					param->Variable.param_value = param_value;
					param->Variable.field_group_index = field_group_index;
				}
			}
			if (p->flags&FieldFlag_no_alias) {
				param->flags |= EntityFlag_NoAlias;
			}
			if (p->flags&FieldFlag_auto_cast) {
				param->flags |= EntityFlag_AutoCast;
			}
			if (p->flags&FieldFlag_any_int) {
				if (!is_type_integer(param->type) && !is_type_enum(param->type)) {
					gbString str = type_to_string(param->type);
					error(name, "A parameter with '#any_int' must be an integer, got %s", str);
					gb_string_free(str);
				}
				param->flags |= EntityFlag_AnyInt;
			}
			if (p->flags&FieldFlag_const) {
				param->flags |= EntityFlag_ConstInput;
			}

			param->state = EntityState_Resolved; // NOTE(bill): This should have be resolved whilst determining it
			add_entity(ctx, scope, name, param);
			if (is_using) {
				add_entity_use(ctx, name, param);
			}
			array_add(&variables, param);
		}
	}


	if (is_variadic) {
		GB_ASSERT(variadic_index >= 0);
	}

	if (is_variadic) {
		GB_ASSERT(params.count > 0);
		// NOTE(bill): Change last variadic parameter to be a slice
		// Custom Calling convention for variadic parameters
		Entity *end = variables[variadic_index];
		end->type = alloc_type_slice(end->type);
		end->flags |= EntityFlag_Ellipsis;
		if (is_c_vararg) {
			end->flags |= EntityFlag_CVarArg;
		}
	}

	isize specialization_count = 0;
	if (scope != nullptr) {
		for_array(i, scope->elements.entries) {
			Entity *e = scope->elements.entries[i].value;
			if (e->kind == Entity_TypeName) {
				Type *t = e->type;
				if (t->kind == Type_Generic &&
				    t->Generic.specialized != nullptr) {
					specialization_count += 1;
				}
			}
		}
	}

	Type *tuple = alloc_type_tuple();
	tuple->Tuple.variables = slice_from_array(variables);

	if (success_) *success_ = success;
	if (specialization_count_) *specialization_count_ = specialization_count;
	if (is_variadic_) *is_variadic_ = is_variadic;
	if (variadic_index_) *variadic_index_ = variadic_index;

	return tuple;
}

Type *check_get_results(CheckerContext *ctx, Scope *scope, Ast *_results) {
	if (_results == nullptr) {
		return nullptr;
	}
	ast_node(field_list, FieldList, _results);
	Slice<Ast *> results = field_list->list;

	if (results.count == 0) {
		return nullptr;
	}
	Type *tuple = alloc_type_tuple();

	isize variable_count = 0;
	for_array(i, results) {
		Ast *field = results[i];
		if (ast_node_expect(field, Ast_Field)) {
			ast_node(f, Field, field);
			variable_count += gb_max(f->names.count, 1);
		}
	}

	auto variables = array_make<Entity *>(permanent_allocator(), 0, variable_count);
	i32 field_group_index = -1;
	for_array(i, results) {
		field_group_index += 1;

		ast_node(field, Field, results[i]);
		Ast *default_value = unparen_expr(field->default_value);
		ParameterValue param_value = {};

		Type *type = nullptr;
		if (field->type == nullptr) {
			param_value = handle_parameter_value(ctx, nullptr, &type, default_value, false);
		} else {
			type = check_type(ctx, field->type);

			if (default_value != nullptr) {
				param_value = handle_parameter_value(ctx, type, nullptr, default_value, false);
			}
		}

		if (type == nullptr) {
			error(results[i], "Invalid parameter type");
			type = t_invalid;
		}
		if (is_type_untyped(type)) {
			error(results[i], "Cannot determine parameter type from a nil");
			type = t_invalid;
		}


		if (field->names.count == 0) {
			Token token = ast_token(field->type);
			token.string = str_lit("");
			Entity *param = alloc_entity_param(scope, token, type, false, false);
			param->Variable.param_value = param_value;
			param->Variable.field_group_index = -1;
			array_add(&variables, param);
		} else {
			for_array(j, field->names) {
				Token token = ast_token(results[i]);
				if (field->type != nullptr) {
					token = ast_token(field->type);
				}
				token.string = str_lit("");

				Ast *name = field->names[j];
				if (name->kind != Ast_Ident) {
					error(name, "Expected an identifer for as the field name");
				} else {
					token = name->Ident.token;
				}

				if (is_blank_ident(token))  {
					error(name, "Result value cannot be a blank identifer `_`");
				}

				Entity *param = alloc_entity_param(scope, token, type, false, false);
				param->flags |= EntityFlag_Result;
				param->Variable.param_value = param_value;
				param->Variable.field_group_index = field_group_index;
				array_add(&variables, param);
				add_entity(ctx, scope, name, param);
				// NOTE(bill): Removes `declared but not used` when using -vet
				add_entity_use(ctx, name, param);
			}
		}
	}

	for_array(i, variables) {
		String x = variables[i]->token.string;
		if (x.len == 0 || is_blank_ident(x)) {
			continue;
		}
		for (isize j = i+1; j < variables.count; j++) {
			String y = variables[j]->token.string;
			if (y.len == 0 || is_blank_ident(y)) {
				continue;
			}
			if (x == y) {
				error(variables[j]->token, "Duplicate return value name '%.*s'", LIT(y));
			}
		}
	}

	tuple->Tuple.variables = slice_from_array(variables);

	return tuple;
}




// NOTE(bill): 'operands' is for generating non generic procedure type
bool check_procedure_type(CheckerContext *ctx, Type *type, Ast *proc_type_node, Array<Operand> *operands) {
	ast_node(pt, ProcType, proc_type_node);

	if (ctx->polymorphic_scope == nullptr && ctx->allow_polymorphic_types) {
		ctx->polymorphic_scope = ctx->scope;
	}

	CheckerContext c_ = *ctx;
	CheckerContext *c = &c_;

	c->curr_proc_sig = type;
	c->in_proc_sig = true;


	ProcCallingConvention cc = pt->calling_convention;
	if (cc == ProcCC_ForeignBlockDefault) {
		cc = ProcCC_CDecl;
		if (c->foreign_context.default_cc > 0) {
			cc = c->foreign_context.default_cc;
		}
	}
	GB_ASSERT(cc > 0);
	if (cc == ProcCC_Odin) {
		c->scope->flags |= ScopeFlag_ContextDefined;
	} else {
		c->scope->flags &= ~ScopeFlag_ContextDefined;
	}

	TargetArchKind arch = build_context.metrics.arch;
	switch (cc) {
	case ProcCC_StdCall:
	case ProcCC_FastCall:
		if (arch != TargetArch_i386 && arch != TargetArch_amd64) {
			error(proc_type_node, "Invalid procedure calling convention \"%s\" for target architecture, expected either i386 or amd64, got %.*s",
			      proc_calling_convention_strings[cc], LIT(target_arch_names[arch]));
		}
		break;
	case ProcCC_Win64:
	case ProcCC_SysV:
		if (arch != TargetArch_amd64) {
			error(proc_type_node, "Invalid procedure calling convention \"%s\" for target architecture, expected amd64, got %.*s",
			      proc_calling_convention_strings[cc], LIT(target_arch_names[arch]));
		}
		break;
	}


	bool variadic = false;
	isize variadic_index = -1;
	bool success = true;
	isize specialization_count = 0;
	Type *params  = check_get_params(c, c->scope, pt->params, &variadic, &variadic_index, &success, &specialization_count, operands);
	Type *results = check_get_results(c, c->scope, pt->results);


	isize param_count = 0;
	isize result_count = 0;
	if (params)  param_count  = params ->Tuple.variables.count;
	if (results) result_count = results->Tuple.variables.count;

	if (param_count > 0) {
		for_array(i, params->Tuple.variables) {
			Entity *param = params->Tuple.variables[i];
			if (param->kind == Entity_Variable) {
				ParameterValue pv = param->Variable.param_value;
				if (pv.kind == ParameterValue_Constant &&
				    pv.value.kind == ExactValue_Procedure) {
					type->Proc.has_proc_default_values = true;
					break;
				}
			}
		}
	}

	if (result_count > 0) {
		Entity *first = results->Tuple.variables[0];
		type->Proc.has_named_results = first->token.string != "";
	}

	bool optional_ok = (pt->tags & ProcTag_optional_ok) != 0;
	if (optional_ok) {
		if (result_count != 2) {
			error(proc_type_node, "A procedure type with the #optional_ok tag requires 2 return values, got %td", result_count);
		} else {
			Entity *second = results->Tuple.variables[1];
			if (is_type_polymorphic(second->type)) {
				// ignore
			} else if (is_type_boolean(second->type)) {
				// GOOD
			} else {
				error(second->token, "Second return value of an #optional_ok procedure must be a boolean, got %s", type_to_string(second->type));
			}
		}
	}
	if (pt->tags & ProcTag_optional_second) {
		if (optional_ok) {
			error(proc_type_node, "A procedure type cannot have both an #optional_ok tag and #optional_second");
		}
		optional_ok = true;
		if (result_count != 2) {
			error(proc_type_node, "A procedure type with the #optional_second tag requires 2 return values, got %td", result_count);
		} else {
			bool ok = false;
			AstFile *file = proc_type_node->file();
			if (file && file->pkg) {
				ok = file->pkg->scope == ctx->info->runtime_package->scope;
			}

			if (!ok) {
				error(proc_type_node, "A procedure type with the #optional_second may only be allowed within 'package runtime'");
			}
		}
	}

	type->Proc.node                 = proc_type_node;
	type->Proc.scope                = c->scope;
	type->Proc.params               = params;
	type->Proc.param_count          = cast(i32)param_count;
	type->Proc.results              = results;
	type->Proc.result_count         = cast(i32)result_count;
	type->Proc.variadic             = variadic;
	type->Proc.variadic_index       = cast(i32)variadic_index;
	type->Proc.calling_convention   = cc;
	type->Proc.is_polymorphic       = pt->generic;
	type->Proc.specialization_count = specialization_count;
	type->Proc.diverging            = pt->diverging;
	type->Proc.optional_ok          = optional_ok;

	if (param_count > 0) {
		Entity *end = params->Tuple.variables[param_count-1];
		if (end->flags&EntityFlag_CVarArg) {
			if (cc == ProcCC_StdCall || cc == ProcCC_CDecl) {
				type->Proc.c_vararg = true;
			} else {
				error(end->token, "Calling convention does not support #c_vararg");
			}
		}
	}


	bool is_polymorphic = false;
	for (isize i = 0; i < param_count; i++) {
		Entity *e = params->Tuple.variables[i];
		if (e->kind != Entity_Variable) {
			is_polymorphic = true;
			break;
		} else if (is_type_polymorphic(e->type)) {
			is_polymorphic = true;
			break;
		}
	}
	for (isize i = 0; i < result_count; i++) {
		Entity *e = results->Tuple.variables[i];
		if (e->kind != Entity_Variable) {
			is_polymorphic = true;
			break;
		} else if (is_type_polymorphic(e->type)) {
			is_polymorphic = true;
			break;
		}
	}
	type->Proc.is_polymorphic = is_polymorphic;

	return success;
}


i64 check_array_count(CheckerContext *ctx, Operand *o, Ast *e) {
	if (e == nullptr) {
		return 0;
	}
	if (e->kind == Ast_UnaryExpr &&
	    e->UnaryExpr.op.kind == Token_Question) {
		return -1;
	}

	check_expr_or_type(ctx, o, e);
	if (o->mode == Addressing_Type) {
		Type *ot = base_type(o->type);

		if (ot->kind == Type_Generic) {
			if (ctx->allow_polymorphic_types) {
				if (ot->Generic.specialized) {
					ot->Generic.specialized = nullptr;
					error(o->expr, "Polymorphic array length cannot have a specialization");
				}
				return 0;
			}
		}
		if (is_type_enum(ot)) {
			return -1;
		}
	}

	if (o->mode != Addressing_Constant) {
		if (o->mode != Addressing_Invalid) {
			Entity *entity = entity_of_node(o->expr);
			bool is_poly_type = false;
			if (entity != nullptr) {
				is_poly_type = \
					entity->kind == Entity_TypeName &&
					entity->type == t_typeid &&
					entity->flags&EntityFlag_PolyConst;
			}

			// NOTE(bill, 2021-03-27): Improve error message for parametric polymorphic parameters which want to generate
			// and enumerated array but cannot determine what it ought to be yet
			if (ctx->allow_polymorphic_types && is_poly_type) {
				return 0;
			}

			gbString s = expr_to_string(o->expr);
			error(e, "Array count must be a constant integer, got %s", s);
			gb_string_free(s);

			if (is_poly_type) {
				error_line("\tSuggestion: 'where' clause may be required to restrict the enumerated array index type to an enum\n");
				error_line("\t            'where intrinsics.type_is_enum(%.*s)'\n", LIT(entity->token.string));
			}

			o->mode = Addressing_Invalid;
			o->type = t_invalid;
		}
		return 0;
	}
	Type *type = core_type(o->type);
	if (is_type_untyped(type) || is_type_integer(type)) {
		if (o->value.kind == ExactValue_Integer) {
			BigInt count = o->value.value_integer;
			if (big_int_is_neg(&o->value.value_integer)) {
				gbAllocator a = heap_allocator();
				String str = big_int_to_string(a, &count);
				error(e, "Invalid negative array count, %.*s", LIT(str));
				gb_free(a, str.text);
				return 0;
			}
			switch (count.used) {
			case 0: return 0;
			case 1: return big_int_to_u64(&count);
			}
			gbAllocator a = heap_allocator();
			String str = big_int_to_string(a, &count);
			error(e, "Array count too large, %.*s", LIT(str));
			gb_free(a, str.text);
			return 0;
		}
	}

	error(e, "Array count must be a constant integer");
	return 0;
}

Type *make_optional_ok_type(Type *value, bool typed) {
	gbAllocator a = permanent_allocator();
	Type *t = alloc_type_tuple();
	slice_init(&t->Tuple.variables, a, 2);
	t->Tuple.variables[0] = alloc_entity_field(nullptr, blank_token, value,  false, 0);
	t->Tuple.variables[1] = alloc_entity_field(nullptr, blank_token, typed ? t_bool : t_untyped_bool, false, 1);
	return t;
}

void init_map_entry_type(Type *type) {
	GB_ASSERT(type->kind == Type_Map);
	if (type->Map.entry_type != nullptr) return;

	// NOTE(bill): The preload types may have not been set yet
	GB_ASSERT(t_map_hash != nullptr);

	/*
	struct {
		hash:  runtime.Map_Hash,
		next:  int,
		key:   Key,
		value: Value,
	}
	*/
	Scope *s = create_scope(nullptr, builtin_pkg->scope);

	auto fields = slice_make<Entity *>(permanent_allocator(), 4);
	fields[0] = alloc_entity_field(s, make_token_ident(str_lit("hash")),  t_uintptr,       false, 0, EntityState_Resolved);
	fields[1] = alloc_entity_field(s, make_token_ident(str_lit("next")),  t_int,           false, 1, EntityState_Resolved);
	fields[2] = alloc_entity_field(s, make_token_ident(str_lit("key")),   type->Map.key,   false, 2, EntityState_Resolved);
	fields[3] = alloc_entity_field(s, make_token_ident(str_lit("value")), type->Map.value, false, 3, EntityState_Resolved);

	Type *entry_type = alloc_type_struct();
	entry_type->Struct.fields  = fields;
	entry_type->Struct.tags    = gb_alloc_array(permanent_allocator(), String, fields.count);
	
	type_set_offsets(entry_type);
	type->Map.entry_type = entry_type;
}

void init_map_internal_types(Type *type) {
	GB_ASSERT(type->kind == Type_Map);
	init_map_entry_type(type);
	if (type->Map.internal_type != nullptr) return;
	if (type->Map.generated_struct_type != nullptr) return;

	Type *key   = type->Map.key;
	Type *value = type->Map.value;
	GB_ASSERT(key != nullptr);
	GB_ASSERT(value != nullptr);

	Type *generated_struct_type = alloc_type_struct();

	/*
	struct {
		hashes:  []int;
		entries: [dynamic]EntryType;
	}
	*/
	Scope *s = create_scope(nullptr, builtin_pkg->scope);

	Type *hashes_type  = alloc_type_slice(t_int);
	Type *entries_type = alloc_type_dynamic_array(type->Map.entry_type);


	auto fields = slice_make<Entity *>(permanent_allocator(), 2);
	fields[0] = alloc_entity_field(s, make_token_ident(str_lit("hashes")),  hashes_type,  false, 0, EntityState_Resolved);
	fields[1] = alloc_entity_field(s, make_token_ident(str_lit("entries")), entries_type, false, 1, EntityState_Resolved);

	generated_struct_type->Struct.fields = fields;
	type_set_offsets(generated_struct_type);
	
	type->Map.generated_struct_type = generated_struct_type;
	type->Map.internal_type         = generated_struct_type;
	type->Map.lookup_result_type    = make_optional_ok_type(value);
}

void add_map_key_type_dependencies(CheckerContext *ctx, Type *key) {
	key = core_type(key);

	if (is_type_cstring(key)) {
		add_package_dependency(ctx, "runtime", "default_hasher_cstring");
	} else if (is_type_string(key)) {
		add_package_dependency(ctx, "runtime", "default_hasher_string");
	} else if (!is_type_polymorphic(key)) {
		if (!is_type_comparable(key)) {
			return;
		}

		if (is_type_simple_compare(key)) {
			i64 sz = type_size_of(key);
			if (1 <= sz && sz <= 16) {
				char buf[20] = {};
				gb_snprintf(buf, 20, "default_hasher%d", cast(i32)sz);
				add_package_dependency(ctx, "runtime", buf);
				return;
			} else {
				add_package_dependency(ctx, "runtime", "default_hasher_n");
				return;
			}
		}

		if (key->kind == Type_Struct) {
			add_package_dependency(ctx, "runtime", "default_hasher_n");
			for_array(i, key->Struct.fields) {
				Entity *field = key->Struct.fields[i];
				add_map_key_type_dependencies(ctx, field->type);
			}
		} else if (key->kind == Type_Union) {
			add_package_dependency(ctx, "runtime", "default_hasher_n");
			for_array(i, key->Union.variants) {
				Type *v = key->Union.variants[i];
				add_map_key_type_dependencies(ctx, v);
			}
		} else if (key->kind == Type_EnumeratedArray) {
			add_package_dependency(ctx, "runtime", "default_hasher_n");
			add_map_key_type_dependencies(ctx, key->EnumeratedArray.elem);
		} else if (key->kind == Type_Array) {
			add_package_dependency(ctx, "runtime", "default_hasher_n");
			add_map_key_type_dependencies(ctx, key->Array.elem);
		}
	}
}

void check_map_type(CheckerContext *ctx, Type *type, Ast *node) {
	GB_ASSERT(type->kind == Type_Map);
	ast_node(mt, MapType, node);

	Type *key   = check_type(ctx, mt->key);
	Type *value = check_type(ctx, mt->value);

	if (!is_type_valid_for_keys(key)) {
		if (is_type_boolean(key)) {
			error(node, "A boolean cannot be used as a key for a map, use an array instead for this case");
		} else {
			gbString str = type_to_string(key);
			error(node, "Invalid type of a key for a map, got '%s'", str);
			gb_string_free(str);
		}
	}
	if (type_size_of(key) == 0) {
		gbString str = type_to_string(key);
		error(node, "Invalid type of a key for a map of size 0, got '%s'", str);
		gb_string_free(str);
	}

	type->Map.key   = key;
	type->Map.value = value;

	add_map_key_type_dependencies(ctx, key);

	init_core_map_type(ctx->checker);
	init_map_internal_types(type);

	// error(node, "'map' types are not yet implemented");
}

void check_matrix_type(CheckerContext *ctx, Type **type, Ast *node) {
	ast_node(mt, MatrixType, node);
	
	Operand row = {};
	Operand column = {};
	
	i64 row_count = check_array_count(ctx, &row, mt->row_count);
	i64 column_count = check_array_count(ctx, &column, mt->column_count);
	
	Type *elem = check_type_expr(ctx, mt->elem, nullptr);
	
	Type *generic_row = nullptr;
	Type *generic_column = nullptr;
	
	if (row.mode == Addressing_Type && row.type->kind == Type_Generic) {
		generic_row = row.type;
	}
	
	if (column.mode == Addressing_Type && column.type->kind == Type_Generic) {
		generic_column = column.type;
	}
	
	if (row_count < MATRIX_ELEMENT_COUNT_MIN && generic_row == nullptr) {
		gbString s = expr_to_string(row.expr);
		error(row.expr, "Invalid matrix row count, expected %d+ rows, got %s", MATRIX_ELEMENT_COUNT_MIN, s);
		gb_string_free(s);
	}
	
	if (column_count < MATRIX_ELEMENT_COUNT_MIN && generic_column == nullptr) {
		gbString s = expr_to_string(column.expr);
		error(column.expr, "Invalid matrix column count, expected %d+ rows, got %s", MATRIX_ELEMENT_COUNT_MIN, s);
		gb_string_free(s);
	}
	
	if (row_count*column_count > MATRIX_ELEMENT_COUNT_MAX) {
		i64 element_count = row_count*column_count;
		error(column.expr, "Matrix types are limited to a maximum of %d elements, got %lld", MATRIX_ELEMENT_COUNT_MAX, cast(long long)element_count);
	}
	
	if (!is_type_valid_for_matrix_elems(elem)) {
		if (elem == t_typeid) {
			Entity *e = entity_of_node(mt->elem);
			if (e && e->kind == Entity_TypeName && e->TypeName.is_type_alias) {
				// HACK TODO(bill): This is to allow polymorphic parameters for matrix elements
				// proc($T: typeid) -> matrix[2, 2]T
				//
				// THIS IS NEEDS TO BE FIXED AND NOT USE THIS HACK
				goto type_assign;
			}
		}
		gbString s = type_to_string(elem);
		error(column.expr, "Matrix elements types are limited to integers, floats, and complex, got %s", s);
		gb_string_free(s);
	}
type_assign:;
	
	*type = alloc_type_matrix(elem, row_count, column_count, generic_row, generic_column);
	
	return;
}



Type *make_soa_struct_internal(CheckerContext *ctx, Ast *array_typ_expr, Ast *elem_expr, Type *elem, i64 count, Type *generic_type, StructSoaKind soa_kind) {
	Type *bt_elem = base_type(elem);

	bool is_polymorphic = is_type_polymorphic(elem);

	if ((!is_polymorphic || soa_kind == StructSoa_Fixed) && !is_type_struct(elem) && !is_type_raw_union(elem) && !(is_type_array(elem) && bt_elem->Array.count <= 4)) {
		gbString str = type_to_string(elem);
		error(elem_expr, "Invalid type for an #soa array, expected a struct or array of length 4 or below, got '%s'", str);
		gb_string_free(str);
		return alloc_type_array(elem, count, generic_type);
	}

	Type *soa_struct = nullptr;
	Scope *scope = nullptr;

	isize field_count = 0;
	i32 extra_field_count = 0;
	switch (soa_kind) {
	case StructSoa_Fixed:	extra_field_count = 0; break;
	case StructSoa_Slice:	extra_field_count = 1; break;
	case StructSoa_Dynamic:	extra_field_count = 3; break;
	}
	if (is_polymorphic && soa_kind != StructSoa_Fixed) {
		field_count = 0;

		soa_struct = alloc_type_struct();
		soa_struct->Struct.fields = slice_make<Entity *>(permanent_allocator(), field_count+extra_field_count);
		soa_struct->Struct.tags = gb_alloc_array(permanent_allocator(), String, field_count+extra_field_count);
		soa_struct->Struct.node = array_typ_expr;
		soa_struct->Struct.soa_kind = soa_kind;
		soa_struct->Struct.soa_elem = elem;
		soa_struct->Struct.soa_count = 0;
		soa_struct->Struct.is_polymorphic = true;

		scope = create_scope(ctx->info, ctx->scope);
		soa_struct->Struct.scope = scope;
	} else if (is_type_array(elem)) {
		Type *old_array = base_type(elem);
		field_count = cast(isize)old_array->Array.count;

		soa_struct = alloc_type_struct();
		soa_struct->Struct.fields = slice_make<Entity *>(permanent_allocator(), field_count+extra_field_count);
		soa_struct->Struct.tags = gb_alloc_array(permanent_allocator(), String, field_count+extra_field_count);
		soa_struct->Struct.node = array_typ_expr;
		soa_struct->Struct.soa_kind = soa_kind;
		soa_struct->Struct.soa_elem = elem;
		if (count > I32_MAX) {
			count = I32_MAX;
			error(array_typ_expr, "Array count too large for an #soa struct, got %lld", cast(long long)count);
		}
		soa_struct->Struct.soa_count = cast(i32)count;

		scope = create_scope(ctx->info, ctx->scope, 8);
		soa_struct->Struct.scope = scope;

		String params_xyzw[4] = {
			str_lit("x"),
			str_lit("y"),
			str_lit("z"),
			str_lit("w")
		};

		for (isize i = 0; i < cast(isize)old_array->Array.count; i++) {
			Type *field_type = nullptr;
			if (soa_kind == StructSoa_Fixed) {
				GB_ASSERT(count >= 0);
				field_type = alloc_type_array(old_array->Array.elem, count);
			} else {
				field_type = alloc_type_pointer(old_array->Array.elem);
			}
			Token token = {};
			token.string = params_xyzw[i];

			Entity *new_field = alloc_entity_field(scope, token, field_type, false, cast(i32)i);
			soa_struct->Struct.fields[i] = new_field;
			add_entity(ctx, scope, nullptr, new_field);
			add_entity_use(ctx, nullptr, new_field);
		}

	} else {
		GB_ASSERT(is_type_struct(elem));

		Type *old_struct = base_type(elem);
		field_count = old_struct->Struct.fields.count;

		soa_struct = alloc_type_struct();
		soa_struct->Struct.fields = slice_make<Entity *>(permanent_allocator(), field_count+extra_field_count);
		soa_struct->Struct.tags = gb_alloc_array(permanent_allocator(), String, field_count+extra_field_count);
		soa_struct->Struct.node = array_typ_expr;
		soa_struct->Struct.soa_kind = soa_kind;
		soa_struct->Struct.soa_elem = elem;
		if (count > I32_MAX) {
			count = I32_MAX;
			error(array_typ_expr, "Array count too large for an #soa struct, got %lld", cast(long long)count);
		}
		soa_struct->Struct.soa_count = cast(i32)count;

		scope = create_scope(ctx->info, old_struct->Struct.scope->parent);
		soa_struct->Struct.scope = scope;

		for_array(i, old_struct->Struct.fields) {
			Entity *old_field = old_struct->Struct.fields[i];
			if (old_field->kind == Entity_Variable) {
				Type *field_type = nullptr;
				if (soa_kind == StructSoa_Fixed) {
					GB_ASSERT(count >= 0);
					field_type = alloc_type_array(old_field->type, count);
				} else {
					field_type = alloc_type_pointer(old_field->type);
				}
				Entity *new_field = alloc_entity_field(scope, old_field->token, field_type, false, old_field->Variable.field_index);
				soa_struct->Struct.fields[i] = new_field;
				add_entity(ctx, scope, nullptr, new_field);
				add_entity_use(ctx, nullptr, new_field);
			} else {
				soa_struct->Struct.fields[i] = old_field;
			}

			soa_struct->Struct.tags[i] = old_struct->Struct.tags[i];
		}
	}

	if (soa_kind != StructSoa_Fixed) {
		Entity *len_field = alloc_entity_field(scope, empty_token, t_int, false, cast(i32)field_count+0);
		soa_struct->Struct.fields[field_count+0] = len_field;
		add_entity(ctx, scope, nullptr, len_field);
		add_entity_use(ctx, nullptr, len_field);

		if (soa_kind == StructSoa_Dynamic) {
			Entity *cap_field = alloc_entity_field(scope, empty_token, t_int, false, cast(i32)field_count+1);
			soa_struct->Struct.fields[field_count+1] = cap_field;
			add_entity(ctx, scope, nullptr, cap_field);
			add_entity_use(ctx, nullptr, cap_field);

			Token token = {};
			token.string = str_lit("allocator");
			init_mem_allocator(ctx->checker);
			Entity *allocator_field = alloc_entity_field(scope, token, t_allocator, false, cast(i32)field_count+2);
			soa_struct->Struct.fields[field_count+2] = allocator_field;
			add_entity(ctx, scope, nullptr, allocator_field);
			add_entity_use(ctx, nullptr, allocator_field);
		}
	}

	Token token = {};
	token.string = str_lit("Base_Type");
	Entity *base_type_entity = alloc_entity_type_name(scope, token, elem, EntityState_Resolved);
	add_entity(ctx, scope, nullptr, base_type_entity);

	add_type_info_type(ctx, soa_struct);

	return soa_struct;
}


Type *make_soa_struct_fixed(CheckerContext *ctx, Ast *array_typ_expr, Ast *elem_expr, Type *elem, i64 count, Type *generic_type) {
	return make_soa_struct_internal(ctx, array_typ_expr, elem_expr, elem, count, generic_type, StructSoa_Fixed);
}

Type *make_soa_struct_slice(CheckerContext *ctx, Ast *array_typ_expr, Ast *elem_expr, Type *elem) {
	return make_soa_struct_internal(ctx, array_typ_expr, elem_expr, elem, -1, nullptr, StructSoa_Slice);
}


Type *make_soa_struct_dynamic_array(CheckerContext *ctx, Ast *array_typ_expr, Ast *elem_expr, Type *elem) {
	return make_soa_struct_internal(ctx, array_typ_expr, elem_expr, elem, -1, nullptr, StructSoa_Dynamic);
}

bool check_type_internal(CheckerContext *ctx, Ast *e, Type **type, Type *named_type) {
	GB_ASSERT_NOT_NULL(type);
	if (e == nullptr) {
		*type = t_invalid;
		return true;
	}

	switch (e->kind) {
	case_ast_node(i, Ident, e);
		Operand o = {};
		Entity *entity = check_ident(ctx, &o, e, named_type, nullptr, false);
		gb_unused(entity);

		gbString err_str = nullptr;
		defer (gb_string_free(err_str));

		switch (o.mode) {
		case Addressing_Invalid:
			break;
		case Addressing_Type: {
			*type = o.type;
			if (!ctx->in_polymorphic_specialization) {
				Type *t = base_type(o.type);
				if (t != nullptr && is_type_polymorphic_record_unspecialized(t)) {
					err_str = expr_to_string(e);
					error(e, "Invalid use of a non-specialized polymorphic type '%s'", err_str);
					return true;
				}
			}
			return true;
		}

		case Addressing_NoValue:
			err_str = expr_to_string(e);
			error(e, "'%s' used as a type", err_str);
			break;

		default:
			err_str = expr_to_string(e);
			error(e, "'%s' used as a type when not a type", err_str);
			break;
		}
	case_end;

	case_ast_node(ht, HelperType, e);
		return check_type_internal(ctx, ht->type, type, named_type);
	case_end;

	case_ast_node(dt, DistinctType, e);
		error(e, "Invalid use of a distinct type");
		// NOTE(bill): Treat it as a HelperType to remove errors
		return check_type_internal(ctx, dt->type, type, named_type);
	case_end;

	case_ast_node(tt, TypeidType, e);
		e->tav.mode = Addressing_Type;
		e->tav.type = t_typeid;
		*type = t_typeid;
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(pt, PolyType, e);
		Ast *ident = pt->type;
		if (ident->kind != Ast_Ident) {
			error(ident, "Expected an identifier after the $");
			*type = t_invalid;
			return false;
		}

		Token token = ident->Ident.token;
		Type *specific = nullptr;
		if (pt->specialization != nullptr) {
			CheckerContext c = *ctx;
			c.in_polymorphic_specialization = true;

			Ast *s = pt->specialization;
			specific = check_type(&c, s);
		}
		Type *t = alloc_type_generic(ctx->scope, 0, token.string, specific);
		if (ctx->allow_polymorphic_types) {
			Scope *ps = ctx->polymorphic_scope;
			Scope *s = ctx->scope;
			Scope *entity_scope = s;
			if (ps != nullptr && ps != s) {
				// TODO(bill): Is this check needed?
				// GB_ASSERT_MSG(is_scope_an_ancestor(ps, s) >= 0);
				entity_scope = ps;
			}
			Entity *e = alloc_entity_type_name(entity_scope, token, t);
			t->Generic.entity = e;
			e->TypeName.is_type_alias = true;
			e->state = EntityState_Resolved;
			add_entity(ctx, ps, ident, e);
			add_entity(ctx, s, ident, e);
		} else {
			error(ident, "Invalid use of a polymorphic parameter '$%.*s'", LIT(token.string));
			*type = t_invalid;
			return false;
		}
		*type = t;
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(se, SelectorExpr, e);
		Operand o = {};
		check_selector(ctx, &o, e, nullptr);

		gbString err_str;
		switch (o.mode) {
		case Addressing_Invalid:
			break;
		case Addressing_Type:
			GB_ASSERT(o.type != nullptr);
			*type = o.type;
			return true;
		case Addressing_NoValue:
			err_str = expr_to_string(e);
			error(e, "'%s' used as a type", err_str);
			gb_string_free(err_str);
			break;
		default:
			err_str = expr_to_string(e);
			error(e, "'%s' is not a type", err_str);
			gb_string_free(err_str);
			break;
		}
	case_end;

	case_ast_node(pe, ParenExpr, e);
		*type = check_type_expr(ctx, pe->expr, named_type);
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(ue, UnaryExpr, e);
		switch (ue->op.kind) {
		case Token_Pointer:
			*type = alloc_type_pointer(check_type(ctx, ue->expr));
			set_base_type(named_type, *type);
			return true;
		}
	case_end;

	case_ast_node(pt, PointerType, e);
		*type = alloc_type_pointer(check_type(ctx, pt->type));
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(pt, MultiPointerType, e);
		*type = alloc_type_multi_pointer(check_type(ctx, pt->type));
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(rt, RelativeType, e);
		GB_ASSERT(rt->tag->kind == Ast_CallExpr);
		ast_node(ce, CallExpr, rt->tag);

		Type *base_integer = nullptr;

		if (ce->args.count != 1) {
			error(rt->type, "#relative expected 1 type argument, got %td", ce->args.count);
		} else {
			base_integer = check_type(ctx, ce->args[0]);
			if (!is_type_integer(base_integer)) {
				error(rt->type, "#relative base types must be an integer");
				base_integer = nullptr;
			} else if (type_size_of(base_integer) > 64) {
				error(rt->type, "#relative base integer types be less than or equal to 64-bits");
				base_integer = nullptr;
			}
		}

		Type *relative_type = nullptr;
		Type *base_type = check_type(ctx, rt->type);
		if (!is_type_pointer(base_type) && !is_type_slice(base_type)) {
			error(rt->type, "#relative types can only be a pointer or slice");
			relative_type = base_type;
		} else if (base_integer == nullptr) {
			relative_type = base_type;
		} else {
			if (is_type_pointer(base_type)) {
				relative_type = alloc_type_relative_pointer(base_type, base_integer);
			} else if (is_type_slice(base_type)) {
				relative_type = alloc_type_relative_slice(base_type, base_integer);
			}
		}
		GB_ASSERT(relative_type != nullptr);

		*type = relative_type;
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(at, ArrayType, e);
		if (at->count != nullptr) {
			Operand o = {};
			i64 count = check_array_count(ctx, &o, at->count);
			Type *generic_type = nullptr;

			Type *elem = check_type_expr(ctx, at->elem, nullptr);

			if (o.mode == Addressing_Type && o.type->kind == Type_Generic) {
				generic_type = o.type;
			} else if (o.mode == Addressing_Type && is_type_enum(o.type)) {
				Type *index = o.type;
				Type *bt = base_type(index);
				GB_ASSERT(bt->kind == Type_Enum);

				Type *t = alloc_type_enumerated_array(elem, index, bt->Enum.min_value, bt->Enum.max_value, Token_Invalid);

				bool is_sparse = false;
				if (at->tag != nullptr) {
					GB_ASSERT(at->tag->kind == Ast_BasicDirective);
					String name = at->tag->BasicDirective.name.string;
					if (name == "sparse") {
						is_sparse = true;
					} else {
						error(at->tag, "Invalid tag applied to an enumerated array, got #%.*s", LIT(name));
					}
				}

				if (!is_sparse && t->EnumeratedArray.count > bt->Enum.fields.count) {
					error(e, "Non-contiguous enumeration used as an index in an enumerated array");
					long long ea_count   = cast(long long)t->EnumeratedArray.count;
					long long enum_count = cast(long long)bt->Enum.fields.count;
					error_line("\tenumerated array length: %lld\n", ea_count);
					error_line("\tenum field count: %lld\n", enum_count);
					error_line("\tSuggestion: prepend #sparse to the enumerated array to allow for non-contiguous elements\n");
					if (2*enum_count < ea_count) {
						error_line("\tWarning: the number of named elements is much smaller than the length of the array, are you sure this is what you want?\n");
						error_line("\t         this warning will be removed if #sparse is applied\n");
					}
				}
				t->EnumeratedArray.is_sparse = is_sparse;

				*type = t;

				goto array_end;
			}

			if (count < 0) {
				error(at->count, "? can only be used in conjuction with compound literals");
				count = 0;
			}


			if (at->tag != nullptr) {
				GB_ASSERT(at->tag->kind == Ast_BasicDirective);
				String name = at->tag->BasicDirective.name.string;
				if (name == "soa") {
					*type = make_soa_struct_fixed(ctx, e, at->elem, elem, count, generic_type);
				} else if (name == "simd") {
					if (!is_type_valid_vector_elem(elem)) {
						gbString str = type_to_string(elem);
						error(at->elem, "Invalid element type for 'intrinsics.simd_vector', expected an integer or float with no specific endianness, got '%s'", str);
						gb_string_free(str);
						*type = alloc_type_array(elem, count, generic_type);
						goto array_end;
					}

					*type = alloc_type_simd_vector(count, elem);
				} else {
					error(at->tag, "Invalid tag applied to array, got #%.*s", LIT(name));
					*type = alloc_type_array(elem, count, generic_type);
				}
			} else {
				*type = alloc_type_array(elem, count, generic_type);
			}
		} else {
			Type *elem = check_type(ctx, at->elem);

			if (at->tag != nullptr) {
				GB_ASSERT(at->tag->kind == Ast_BasicDirective);
				String name = at->tag->BasicDirective.name.string;
				if (name == "soa") {
					*type = make_soa_struct_slice(ctx, e, at->elem, elem);
				} else {
					error(at->tag, "Invalid tag applied to array, got #%.*s", LIT(name));
					*type = alloc_type_slice(elem);
				}
			} else {
				*type = alloc_type_slice(elem);
			}
		}
	array_end:
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(dat, DynamicArrayType, e);
		Type *elem = check_type(ctx, dat->elem);
		if (dat->tag != nullptr) {
			GB_ASSERT(dat->tag->kind == Ast_BasicDirective);
			String name = dat->tag->BasicDirective.name.string;
			if (name == "soa") {
				*type = make_soa_struct_dynamic_array(ctx, e, dat->elem, elem);
			} else {
				error(dat->tag, "Invalid tag applied to dynamic array, got #%.*s", LIT(name));
				*type = alloc_type_dynamic_array(elem);
			}
		} else {
			*type = alloc_type_dynamic_array(elem);
		}
		set_base_type(named_type, *type);
		return true;
	case_end;

	case_ast_node(st, StructType, e);
		CheckerContext c = *ctx;
		c.in_polymorphic_specialization = false;
		c.type_level += 1;

		*type = alloc_type_struct();
		set_base_type(named_type, *type);
		check_open_scope(&c, e);
		check_struct_type(&c, *type, e, nullptr, named_type);
		check_close_scope(&c);
		(*type)->Struct.node = e;
		return true;
	case_end;

	case_ast_node(ut, UnionType, e);
		CheckerContext c = *ctx;
		c.in_polymorphic_specialization = false;
		c.type_level += 1;

		*type = alloc_type_union();
		set_base_type(named_type, *type);
		check_open_scope(&c, e);
		check_union_type(&c, *type, e, nullptr, named_type);
		check_close_scope(&c);
		(*type)->Union.node = e;
		return true;
	case_end;

	case_ast_node(et, EnumType, e);
		bool ips = ctx->in_polymorphic_specialization;
		defer (ctx->in_polymorphic_specialization = ips);
		ctx->in_polymorphic_specialization = false;
		ctx->in_enum_type = true;

		*type = alloc_type_enum();
		set_base_type(named_type, *type);
		check_open_scope(ctx, e);
		check_enum_type(ctx, *type, named_type, e);
		check_close_scope(ctx);
		(*type)->Enum.node = e;

		ctx->in_enum_type = false;
		return true;
	case_end;

	case_ast_node(bs, BitSetType, e);
		*type = alloc_type_bit_set();
		set_base_type(named_type, *type);
		check_bit_set_type(ctx, *type, named_type, e);
		return true;
	case_end;


	case_ast_node(pt, ProcType, e);
		bool ips = ctx->in_polymorphic_specialization;
		defer (ctx->in_polymorphic_specialization = ips);
		ctx->in_polymorphic_specialization = false;

		*type = alloc_type(Type_Proc);
		set_base_type(named_type, *type);
		check_open_scope(ctx, e);
		check_procedure_type(ctx, *type, e);
		check_close_scope(ctx);
		return true;
	case_end;

	case_ast_node(mt, MapType, e);
		bool ips = ctx->in_polymorphic_specialization;
		defer (ctx->in_polymorphic_specialization = ips);
		ctx->in_polymorphic_specialization = false;

		*type = alloc_type(Type_Map);
		set_base_type(named_type, *type);
		check_map_type(ctx, *type, e);
		return true;
	case_end;

	case_ast_node(ce, CallExpr, e);
		Operand o = {};
		check_expr_or_type(ctx, &o, e);
		if (o.mode == Addressing_Type) {
			*type = o.type;
			set_base_type(named_type, *type);
			return true;
		}
	case_end;

	case_ast_node(te, TernaryIfExpr, e);
		Operand o = {};
		check_expr_or_type(ctx, &o, e);
		if (o.mode == Addressing_Type) {
			*type = o.type;
			set_base_type(named_type, *type);
			return true;
		}
	case_end;

	case_ast_node(te, TernaryWhenExpr, e);
		Operand o = {};
		check_expr_or_type(ctx, &o, e);
		if (o.mode == Addressing_Type) {
			*type = o.type;
			set_base_type(named_type, *type);
			return true;
		}
	case_end;
	
	
	case_ast_node(mt, MatrixType, e);
		check_matrix_type(ctx, type, e);
		set_base_type(named_type, *type);
		return true;
	case_end;
	}

	*type = t_invalid;
	return false;
}

Type *check_type(CheckerContext *ctx, Ast *e) {
	CheckerContext c = *ctx;
	c.type_path = new_checker_type_path();
	defer (destroy_checker_type_path(c.type_path));

	return check_type_expr(&c, e, nullptr);
}

Type *check_type_expr(CheckerContext *ctx, Ast *e, Type *named_type) {
	Type *type = nullptr;
	bool ok = check_type_internal(ctx, e, &type, named_type);

	if (!ok) {
		gbString err_str = expr_to_string(e);
		error(e, "'%s' is not a type", err_str);
		gb_string_free(err_str);
		type = t_invalid;
	}

	if (type == nullptr) {
		type = t_invalid;
	}

	if (type->kind == Type_Named &&
	    type->Named.base == nullptr) {
		// IMPORTANT TODO(bill): Is this a serious error?!
		#if 0
		error(e, "Invalid type definition of '%.*s'", LIT(type->Named.name));
		#endif
		type->Named.base = t_invalid;
	}

	if (is_type_polymorphic(type)) {
		type->flags |= TypeFlag_Polymorphic;
	} else if (is_type_polymorphic(type, true)) {
		type->flags |= TypeFlag_PolySpecialized;
	}

	#if 0
	if (!ctx->allow_polymorphic_types && is_type_polymorphic(type)) {
		gbString str = type_to_string(type);
		error(e, "Invalid use of a polymorphic type '%s'", str);
		gb_string_free(str);
		type = t_invalid;
	}
	#endif

	if (is_type_typed(type)) {
		add_type_and_value(ctx->info, e, Addressing_Type, type, empty_exact_value);
	} else {
		gbString name = type_to_string(type);
		error(e, "Invalid type definition of %s", name);
		gb_string_free(name);
		type = t_invalid;
	}
	set_base_type(named_type, type);

	return type;
}
