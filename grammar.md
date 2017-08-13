The current grammar of Spero, given in a basic PEG format

TODO: Consider how errors in sub-expressions should be handled by "parent" expressions
TODO: Need to consider a way to grab "chunks" of syntactic errors
  Right now I think "P:">.;]" will provide about 7 errors (way too many)
	I could have a buffer of contiguous errors where every "failure" gets appended to the buffer
	  TODO: Figure out how to use this buffer correctly
		  I think this would mostly be used at the top level (ie. a gstmt/etc)
			  Could just specify that when one of the other directions succeeds, flush the buffer as well
			  Sub-levels are fairly well specified in what is/isn't allowed, but there's still this issue
	Late failures of syntax need to "swallow" their failing nodes
	  This means that 'gstmt' will always succeed, errors are reported immediately
		This might have some problems with contextualizing the errors
	Could also have the last statement push on an "error" ast node
	  Reporting syntactical errors then becomes a specific analysis phase
		Restore points would simply be nodes that stop/constrain the contextualization
# NOTE: A "restore" point should be any high-level construct that allows for contextualizing error reports

	program = gstmt*
	  # Could I use "unexpected character" here? FIRST set type lookahead
	gstmt = mod_dec | impl | stmt
	mod_dec = "mod" varname (";" | eolf)
	  # error if eolf or ";" not used
	  # restore point?
	mod_path = (":" not_at[typ_ch | ":"] var)*
	  # error if var not used
	_mod_path = (var ":")*
	  # fail if eolf, error otherwise if not ":"
	impl = "impl" single_type for_type? scope
	  # error if no scope, invalid typing
		# TODO: Enforce for_type ???
	  # restore point?
	for_type = "for" single_type
	  # fail if scope, error if wrong keyword or invalid typing
	stmt = annotation* (ganot | mod_alias | expr) ";"?
	  # restore point?
	annotation = "@" var tuple?
	  # error if non-var name used
	ganot = "@!" var tuple?
	  # error if non-var name used
	mod_alias = "use" _mod_path (mul_imp | imp_alias)
	  # TODO: Convert over to using 'mod_path' (I might be removing 'mod_path' though)
	  # error if no path given
	mul_imp = "{" binding ("," binding)* "}"
	  # error if not "," used or binding not used
	imp_alias = binding alias?
	alias = array? "as" binding array?
	  # error if binding not used or invalid keyword
	binding = var | typ
	expr = assign | ("do"? valexpr)
	mvexpr = valexpr | in_assign
	  # error if unexpected keyword/etc. used
	mvdexpr = "do"? mvexpr
	assign = vcontext (type_assign | var_assign)
	  # error if wrong keyword used
	type_assign = typname generic? "=" (adt_dec | arg_tuple)? scope
	  # error if "=" not used or no scope used
	var_assign = assign_pat generic? (interface | asgn_val)
	interface = type_inf asgn_val?
	asgn_val = "=" valexpr ("in" mvexpr)?
	  # error if "=" not used
	in_assign = vcontext assign_pat generic? type_inf? "=" valexpr "in" mvexpr
	  # error if "=" not used or "in" not used
	scope = "{" stmt* "}"
	  # error if no closing "}"
	arg_tuple = "(" (arg ("," arg)*)? ")"
	  # error if no closing ")"
	arg = var type_inf?
	type_inf = "::" type
	adt_dec = adt ("|" adt)*
	adt = typ tuple_type?
	  # error if typ not used
	valexpr = "mut"? (control | binexpr) type_inf?
	control = match | forl | whilel | branch | jump | loop
	loop = "loop" mvdexpr
	match = "match" mvexpr "{" case+ "}"
	  # error if no closing "}" or no case statements
	case = pattern if_core? "=>" mvexpr ";"?
	  # error if "=>" not used or ";}\n" not end character
	forl = "for" pattern "in" mvexpr mvdexpr
	  # error if "in" not used
	whilel = "while" mvexpr mvdexpr
	branch = if_core mvdexpr eslif_case* else_case?
	if_core = "if" mvexpr
	elsif_case = "elsif" mvexpr mvdexpr
	  # error if "elseif" used
	else_case = "else" mvdexpr
	jump = jump_key mvexpr?
	binexpr = unexpr (op unexpr?)?
	unexpr = un_op? index
	index = fncall "." (dot_ctrl | index_cont)
	index_cont = fncall ("." fncall)*
	dot_ctrl = dotloop | dotwhile | dotfor | dotbranch | dotmatch | jump_key
	dotloop = "loop"
	dotwhile = "while" mvexpr
	dotfor = "for" pattern "in" mvexpr
	  # error if "in" not used
	dotbranch = if_core elsif_case* else_case?
	dotmatch = "match" "{" case+ "}"
	  # error if no closing "}" or no case statements
	fncall = (named | valcall) actcall*
  named = varname type_const?
	type_const = typ actcall? anon_type?
	valcall = atom | op
	actcall = (array tuple?) | tuple
	anon_type = "::" scope
	typ_ch = [A-Z]
	var = [a-z][a-zA-Z_]
	typ = typ_ch [a-zA-Z_]
	varname = var mod_path
	typname = varname ":" typ
	  # error if typ not used
	atom = fn_tuple | array | literal | plambda | scope
	literal = binary | hex | decimal | char | string | "false" | "true"
	tuple = "(" (mvexpr ("," mvexpr)* )? ")"
	  # error if no closing ")"
	array = "[" (mvexpr ("," mvexpr)* )? "]"
	  # error if no closing "]"
	binary = "0b" [01]+
	hex = "0x" [0-9a-fA-F]+
	decimal = [0-9]+ ("." [0-9]+)?
	char = "'" "\"? . "'"
	  # error if no closing '''
	string = """ ("\"? .)* """
	  # error if no closing '"'
	plambda = "_"
	fn_tuple = (tuple lambda?) | dot_fn
	lambda = "->" mvexpr
	dot_fn = "." (dot_ctrl | index_cont)
	  # error if not dot_ctrl or index_cont used
	generic = "[" gen_part ("," gen_part)* "]"
	  # error if no closing "]"
	gen_part = type_gen | val_gen
	  # error if not type_gen or val_gen
	type_gen = typ variance? relation?
	val_gen = var relation?
	relation = ("::" | "!:") type
	pattern = "_" | literal | pat_adt | capture
	pat_adt = typname pat_tuple?
	pat_tuple = "(" (pattern ("," pattern)* )? ")"
	  # error if no closing ")"
	capture = capture_desc (pat_tuple | var)
	  # error if pat_tuple or var not used
	capture_desc = "mut"?  "&"?
	assign_pat = var | op | "_" | assign_tuple
	assign_tuple = "(" assign_pat ("," assign_pat)* ")"
	  # error if no closing ")"
	type = or_type
	  # error if no or_type
	or_type = and_type ("|" and_type)*
	and_type = ntype ("&" ntype)*
	ntype = mut_type | ("{" type "}")
	  # error if no closing "}"
	mut_type = "mut"? (tuple_fn_type | ref_type)
	tuple_fn_type = tuple_type fn_type?
	fn_type = "->" type
	  # error if "->," not used
	ref_type = single_type typ_ptr
	single_type = typname array?
	
