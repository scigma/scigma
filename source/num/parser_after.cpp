/* Called by yyparse on error. */

  void
    yyerror (char const *s)
  {
    if (s!=NULL)
      parserError=s;
    for (unsigned int i =0;i<newlyDefinedFunctions.size();++i)
      undefine_function(newlyDefinedFunctions[i],*currentFunctionSet);
  }
  
#include <stdio.h>
  
  int
    yylex (void)
  {
    char c=' ';
    /* Ignore white space, get first nonwhite character. */
    while (c == ' ' || c == '\t')
      parserStream.get(c);
    if (c == EOF)
      return 0;
    
    /* Char starts a number => parse the number.         */
    if (isdigit (c)||c=='.')
      {
        parserStream.unget();
        parserStream>>yylval.value;
	if(parserStream.fail())
	  return 0;
        return NUM;
      }
    /* Char starts an identifier => read the name.      */
    if (isalpha (c))
      {
        int i;
        /* Initially make the buffer long enough
	   for a 40-character symbol name. */
        if (length == 0)
	  length = 40, symbuf = (char *)malloc (length + 1);
        i = 0;
        do
	  {
	    /* If buffer is full, make it bigger.        */
            if (i == length)
	      {
                length *= 2;
                symbuf = (char *) realloc (symbuf, length + 1);
	      }
            /* Add this character to the buffer.         */
            symbuf[i++] = c;
            /* Get another character.                    */
            parserStream.get(c);
	  }
        while (isalnum (c)||c=='_');
	
        parserStream.unget();
        symbuf[i] = '\0';
	
        Operator op;
        if(Operator::get_operator(symbuf,op))
	  {
            yylval.op = new_op_ptr(op);
            return OP;
	  }
	
	FunctionSet::iterator fInfoIterator(get_function_from_set(symbuf,*currentFunctionSet));
        if(fInfoIterator!=currentFunctionSet->end())
	  {
            yylval.function = new_f_ptr(fInfoIterator->f);
	    return FUNC;
	  }
        define_function(symbuf,*currentFunctionSet);
        yylval.function = new_f_ptr(get_function_from_set(symbuf,*currentFunctionSet)->f);
	return FUNC;
      }
    /* Any other character is a token by itself.        */
    return (int)c;
  }

#pragma clang diagnostic pop

} /* end namespace num */
} /* end namespace scigma */
