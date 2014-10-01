%{
#include "parser_before.cpp"
%}

%union {
  double  value;      		/* For returning numbers. */
  Operator*  op;		/* For operators */
  Function* function;		/* For returning functions */
  ValueSequence  sequence;	/* For parameter lists */  
  FunctionSequence fsequence;	/* For parameter lists containing functions */
}

%token <value> NUM		/* Simple double precision number. */
%token <function> FUNC		/* Functions */
%token <op>	OP		/* Operators */
%type <value> exp		/* expressions containing numbers only */
%type <function> func		/* expressions containing FUNCS */
%type <sequence> seq		/* parameter lists */
%type <fsequence> fseq		/* function lists */

%right '='
%left ','
%left '-' '+'
%left '*' '/'
%left NEG			/* negation--unary minus */
%right '^'			/* exponentiation */
%nonassoc '$'			/* operator to get current value from functions  */
%nonassoc '\''			/* used to denote time derivatives */
%nonassoc '!'			/* used to delete functions */
%%				/* The grammar follows. */

input: 
       | input line
;
line:
        '\n' {YYACCEPT;}
	| exp '\n' {std::stringstream ss;ss<<$1;ss>>parserResult;requiredFunctions.clear();YYACCEPT;}	/* returns the value of an expression */
	| FUNC '=' exp '\n' {redefine_function(*$1,Function($3),currentExpression,*currentFunctionSet);YYACCEPT;}	/* a = 1 */
	| FUNC '=' func '\n' {try{redefine_function(*$1,*$3,currentExpression,*currentFunctionSet);}catch(std::string error){yyerror(error.c_str());YYABORT;};YYACCEPT;} /* y = x + 1 */
//	| FUNC '(' fseq ')'  '=' exp '\n' {try{redefine_function_with_arguments(*$1,Function($6),currentExpression,$3,*currentFunctionSet);}catch(std::string error){yyerror(error.c_str());YYABORT;}YYACCEPT;} /* y(x) =  1 (although it does not make particularly much sense)*/    
	| FUNC '(' fseq ')'  '=' func '\n' {try{redefine_function_with_arguments(*$1,*$6, currentExpression, $3,*currentFunctionSet);}catch(std::string error){yyerror(error.c_str());YYABORT;}YYACCEPT;} /* y(x) = x+1 */
  	| FUNC '\'' '=' exp '\n'{set_time_derivative(*$1,Function($4),currentExpression,*currentFunctionSet);YYACCEPT;}	/* y' = 1 */      
	| FUNC '\'' '=' func '\n'{set_time_derivative(*$1,*$4,currentExpression,*currentFunctionSet);YYACCEPT;}/* y' = x+1 */
       	| '!' FUNC '\n'{try{undefine_function(*$2,*currentFunctionSet);}catch(std::string error){yyerror(error.c_str());YYABORT;}YYACCEPT;}    /* !x */
       	| '!' FUNC '\'' '\n'{unset_time_derivative(*$2,*currentFunctionSet);YYACCEPT;}  /* !x' */
	| error '\n' {YYABORT;}
;
exp:  NUM               {$$ = $1;}
       | exp '+' exp       {$$ = $1 + $3;}
       | exp '-' exp       { $$ = $1 - $3;}
       | exp '*' exp       { $$ = $1 * $3;} 
       | exp '/' exp       { $$ = $1 / $3;}
       | '-' exp %prec NEG {$$ = -$2;}
       | exp '^' exp       {$$ = pow ($1, $3);}
       | '(' exp ')'       { $$ = $2;}
       | '$' func	     {$$=$2->evaluate();extract_required_functions(*$2,*currentFunctionSet);}	/* $y uses current numerical value of y */
       | OP'('seq')'     {if($3.length==$1->get_number_of_arguments()){$$=(*$1)($3.values);}else{yyerror(std::string("wrong numer of arguments for "+$1->get_name()+"\n").c_str());YYABORT;}}
;
func: FUNC	{$$=$1;}	
       | func '+' func  {$$ = new_f_ptr(*$1+*$3);}  		/* x+y */
       | func '-' func   {$$ = new_f_ptr(*$1-*$3);}
       | func '*' func   {$$ = new_f_ptr(*$1*(*$3));}
       | func '/' func    {$$ = new_f_ptr(*$1/(*$3));}
       | '-' func %prec NEG {$$ = new_f_ptr(-(*$2));}
       | func '^' func  {Function f[2]; f[0]=*$1;f[1]=*$3; Operator pow; Operator::get_operator("^",pow);$$=new_f_ptr(pow(f));}
       | '(' func ')'  {$$=$2;}
       | exp '+' func  {$$ = new_f_ptr($1 + *$3);}  		/* 1+x */
       | exp '-' func   {$$ = new_f_ptr($1 - *$3);}    
       | exp '*' func   {$$ = new_f_ptr($1 * *$3);}    
       | exp '/' func    {$$ =new_f_ptr($1 / *$3);}
       | exp '^' func  {Function f[2]; f[0]=$1;f[1]=*$3; Operator pow; Operator::get_operator("^",pow);$$=new_f_ptr(pow(f));}
       | func '+' exp  {$$ = new_f_ptr(*$1 + $3);}  		/* x+1 */
       | func '-' exp   {$$ = new_f_ptr(*$1 - $3);}    
       | func '*' exp   {$$ = new_f_ptr(*$1 * $3);}    
       | func '/' exp    {$$ =new_f_ptr(*$1 / $3);}
       | func '^' exp  {Function f[2]; f[0]=*$1;f[1]=$3; Operator pow; Operator::get_operator("^",pow);$$=new_f_ptr(pow(f));}
       | OP'('fseq')'  {if($3.length==$1->get_number_of_arguments()){Function* f = new Function[$3.length];for(unsigned int i = 0;i<$3.length;i++){f[i]=*($3.functions[i]);}$$=new_f_ptr((*$1)(f));delete[] f;}else{yyerror(std::string("wrong number of arguments for "+$1->get_name()+"\n").c_str());YYABORT;}}	/* atan2(y,x) */
       | FUNC'('fseq')' {try{$$ = new_f_ptr(insert_arguments_into_function(*$1,$3,*currentFunctionSet));}catch(std::string error){yyerror(error.c_str());YYABORT;}}; /* f(x,3) */
       | FUNC'('seq')' {try{$$ = new_f_ptr(insert_arguments_into_function(*$1,$3,*currentFunctionSet));}catch(std::string error){yyerror(error.c_str());YYABORT;}}; /* f(1,2) */
;


seq: exp	{ValueSequence vs; vs.length=1;vs.values[0]=$1;$$=vs;}
       | seq ',' exp  {$1.values[$1.length++]=$3;$$=$1;}
;

fseq: func {FunctionSequence fs; fs.length=1;fs.functions[0]=$1;$$=fs;}
       | fseq ',' exp {$1.functions[$1.length++]=new_f_ptr($3);$$=$1;}
       | fseq ',' func {$1.functions[$1.length++]=new_f_ptr(*$3);$$=$1;}
       | seq ',' func {FunctionSequence fs; fs.length=$1.length;for(size_t i = 0;i<$1.length;i++){fs.functions[i]=new_f_ptr($1.values[i]);}fs.functions[fs.length++]=new_f_ptr(*$3);$$=fs;}
;

/* End of grammar. */
%%

#include "parser_after.cpp"
