The current grammar of Spero, given in a basic PEG format

	//
	// Ignored / Special Characters
	//
	eolf           = "\r\n" / "\n"
	comment        = "#" (!"\n" .)* eolf
	multiline      = "##" (!"##" .)* "##"
	whitespace     = [" \n\r\t"]+
	ig             = multiline / comment / whitespace
	oparen         = "(" ig*
	cparen         = ")" ig*
	cbrack         = "]" ig*
	obrack         = "[" ig*
	obrace         = "{" ig*
	cbrace         = "}" ig*
	placeholder    = "_" ig*
	id_other       = ["a-zA-Z0-9_"]

	//
	// Language Keywords
	//
	KEY(str)       = str !id_other ig*
	k_let          = KEY("let")
	k_def          = KEY("def")
	k_static       = KEY("static")
	k_mut          = KEY("mut")
	k_do           = KEY("do")
	k_mod          = KEY("use")
	k_match        = KEY("match")
	k_if           = KEY("if")
	k_elsif        = KEY("elsif")
	k_else         = KEY("else")
	k_while        = KEY("while")
	k_for          = KEY("for")
	k_loop         = KEY("loop")
	k_break        = KEY("break")
	k_continue     = KEY("continue")
	k_yield        = KEY("yield")
	k_ret          = KEY("return")
	k_wait         = KEY("wait")
	k_impl         = KEY("impls")
	k_in           = KEY("in")
	b_false        = KEY("false")
	b_true         = KEY("true")
	vcontext       = k_let / k_def / k_static
	jump_key       = k_break / k_continue / k_ret / k_yield / k_wait
	keyword        = vcontext / jump_key / placeholder / k_mut / k_mod / k_use / k_match / k_if
					/ k_elsif / k_else / k_while / k_for / k_loop / k_impl / k_in / b_false / b_true

	//
	// Language Bindings
	//
	var            = !keyword ["a-z_"] id_other* "!" / "?"
	typ            = ["A-Z"] id_other*
	op_char        = ["!$%^&*?<>|`/\\-=+~"]
	op             = (["&~!?"] / "->")? op_char+ ig*
	variable       = var ig*
	name_path      = (var / typ ":")*
	typ_ptr        = ["&*"]
	type           = name_path typ ig* array? typ_ptr? ig*
	mut_type       = k_mut? type
	binding        = (name_path typ / var) / op ig*

	//
	// Language Literals
	//
	hex            = "0x" ["0-9a-fA-F"]+
	bin            = "0b" ["0-1"]+
	float          = ["0-9"]+ "." ["0-9"]+
	num            = ["0-9"]+
	string         = "\"" (!"\"" "\\"? .)* "\""
	character      = "'" "\\"? . "'"
	Seq(o,e,s,c)   = o (e (s ig* e)*)? c
	tuple          = Seq(oparen, valexpr, ",", cparen)
	array          = Seq(obrack, valexpr, ",", cbrack)
	fn_rettype     = mut_type ig* scope
	op_forward     = op valexpr?
	fn_forward     = "." dot_ctrl / op_forward / valexpr
	fn_def         = "->" ig* fn_rettype / fn_forward / valexpr
	fn_or_tuple    = fn_forward / (tuple fn_def?)
	literal        = (hex / bin / num / string / b_false / b_true / array / fn_or_tuple) ig*

	//
	// Language Atoms
	//
	named_arg      = var ig* inf?
	con_tuple      = Seq(oparen, named_arg, ",", cparen)
	anon_type      = ":::" ig* con_tuple? scope
	scope          = Seq(obrace, expr, ",", cbrace)
	wait_stmt      = k_wait valexpr
	atom           = scope / wait_stmt / lit / binding ig*
	fnseq          = (array anon_type? tuple?) / (anon_type tuple?) / tuple
	fncall         = atom fnseq*

	//
	// Language Decorators
	//
	unary          = ["&!-~"]
	glbl_annot     = "@" var "!" tuple?
	annotation     = "@" var tuple?
	mod_dec        = k_mod var (":" var)* ig+
	inf_type       = mut_type / inf_fn_type
	type_tuple     = Seq(oparen, inf_type, ",", cparen)
	inf_fn_type    = type_tuple (ig* "->" ig* inf_type)?
	inf            = "::" ig* inf_type
	gen_type       = typ ["+-"]? ig* (("::"/"!:"/"<"/">") ig* type)?
	gen_val        = variable ("::" ig* type)? ("=" ig* valexpr)?
	gen_part       = gen_type / gen_val
	gen            = Seq(obrack, gen_part, ",", cbrack)
	use_rebind     = (var ig* ("as" ig* var ig*)?) / (typ ig* ("as" ig* typ ig*))
	use_many       = Seq(obrace, use_rebind, ",", cbrace)
	use_final      = (use_many (":" placeholder)) / placeholder / var

	//
	// Patterns
	//
	var_tuple      = Seq(oparen, var_pattern, ",", cparen)
	var_pattern    = var / op / var_tuple
	var_type       = typ
	tuple_pat      = Seq(oparen, pattern, ",", cparen)
	pattern_adt    = typ tuple_pat?
	pattern_tuple  = k_mut? tuple_pat
	pattern_var    = k_mut var
	pattern_val    = hex / bin / num / str / character / false / true / array
	pattern        = placeholder / pattern_var / pattern_tuple / pattern_adt / pattern_val

	//
	// Assignment
	//
	adt_con        = typ type_tuple? ig*
	constructors   = (adt_con "|" ig*)* (con_tuple / adt_con)?
	assign_val     = "=" ig* valexpr
	var_assgin     = var_pattern ig* generic? (inf assign_val?) / assign_val
	lhs_inher      = inf "=" ig*
	rhs_inf        = typ ig* "::" ig*
	rhs_inher      = "=" ig* rhs_inf?
	type_assign    = typ !oparen ig* generic? lhs_inher / rhs_inher cons scope
	assign         = vcontext type_assign / var_assgin

	//
	// Control Flow
	//
	if_core        = k_if valexpr
	elses          = (k_elsif valexpr k_do? valexpr)* k_else valexpr
	branch         = if_core k_do? valexpr elses
	loop           = k_loop valexpr
	while_core     = k_while valexpr
	while_l        = while_core k_do? valexpr
	for_core       = k_for pattern "in" ig* valexpr
	for_l          = for_core k_do? valexpr
	jump           = jump_key valexpr?
	case_pat       = pattern ig*
	case_stmt      = Seq(eps, case_pat, ",", eps) ig* if_core? "->" ig* valexpr
	match_expr     = k_match fncall obrace case_stmt+ cbrace
	dot_match      = k_match obrace case_stmt+ cbrace
	dot_ctrl       = if_core / k_loop / while_core / jump_key / dot_match / for_core / dot_if
	control        = branch / loop / while_l / for_l / match_expr / jump

	//
	// Operator Precedence
	//
	PREC(ch)       = [ch] op_char*
	BINARY(op, s)  = s (op ig* s)*
	binary_prec_1  = BINARY(range, PREC("$?`\\"))
	binary_prec_2  = BINARY(binary_prec_1, PREC("/%*"))
	op_prec_3      = ("->" op_char+) / PREC("+-")
	binary_prec_3  = BINARY(binary_prec_2, op_prec_3)
	op_prec_4      = ("!" op_char+) / PREC("=")
	binary_prec_4  = BINARY(binary_prec_3, op_prec_4)
	binary_prec_5  = BINARY(binary_prec_4, PREC("&<>"))
	binary_prec_6  = BINARY(binary_prec_5, PREC("^"))
	binary_prec_7  = BINARY(binary_prec_6, PREC("|"))

	//
	// Expressions
	//
	_index_        = !".." "." ig* fncall
	index          = unary* fncall _index_* ("." dot_ctrl) / inf / eps
	range          = index (".." index?)?
	valexpr        = k_mut? (control / range / binary) ig* ";"? ig*
	mod_use        = k_use (var / placeholder ":")* use_final
	impl_expr      = k_impl name_path typ ig*
	expr           = (mod_use / impl_expr / assign / (k_do? valexpr)) ig* ";"? ig*
	stmt           = (annotation ig*)* glbl_annot / mod_dec / expr
	program        = ig* stmt* eolf