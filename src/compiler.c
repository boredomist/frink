#include "compiler.h"

static Instruction pushConstant(AttoBlock* b, TValue t) {
  int index = -1;
  int i;
  Vector *v = b->k;
  for(i = 0; i < v->size && index == -1; ++i) {
    TValue tv = getIndex(v, i);

    if(tv.type == t.type) {
      switch(t.type) {
      case TYPE_NUMBER:
	if(TV2NUM(t) == TV2NUM(tv)) {
	  index = i;
	}
	break;
      case TYPE_STRING:
	if(strcmp(TV2STR(tv), TV2STR(t)) == 0) {
	  index = i;
	}
	break;
      case TYPE_NULL:
        break;
      default:
	fprintf(stderr, "Unknown type: %d\n", t.type);
	return -1;
      }
    }
  }
  
  if(index != -1) {
    return index;
  }

  return append(b->k, t) - 1;
}

/* Macros for laziness */
#define IS(x, y) (strcmp(x, y) == 0)
#define PUSH(op) AttoBlock_push_inst(b, op)
#define IF(op)   if(IS(word, op))
#define EIF(op)  else IF(op)
// return number of tokens to skip, or -1 for error. messy
static int compileWord(FrinkProgram* fp, int tokIndex, AttoBlock *b, char* word) {
  IF("pop") {
    PUSH(OP_POP);
  } EIF("dup") {
    PUSH(OP_DUP);  
  } EIF("swap") {
    PUSH(OP_SWAP);
  } EIF("+") {
    PUSH(OP_ADD);
  } EIF("-") {
    PUSH(OP_SUB);
  } EIF("*") {
    PUSH(OP_MUL);
  } EIF("/") {
    PUSH(OP_DIV);
  } EIF("%") {
    PUSH(OP_MOD);
  } EIF("^") {
    PUSH(OP_POW);
  } EIF (".") {
    PUSH(OP_PRINT);
  } EIF("var"){
    if(tokIndex < fp->len - 1) {
      Token nextToken = fp->tokens[++tokIndex];
      if(FrinkProgram_add_var(fp, nextToken.content)) {
        fprintf(stderr, "Var already defined: %s\n", nextToken.content);
        return -1;
      }
    } else {
      fprintf(stderr, "Expected variable name, but got EOF\n");
      return -1;
    }
    return 1;
  } else {
    // check if word is a var
    int index = FrinkProgram_find_var(fp, word);
    if(index != -1) {
      int t = 0;
      if(tokIndex < fp->len - 1) {
        Token nextToken = fp->tokens[++tokIndex];
        if(!strcmp(nextToken.content, "set")) {
          t = 1;
          PUSH(OP_SETVAR);
          PUSH(index);
        } else if (!strcmp(nextToken.content, "value")) {
          t = 1;
          PUSH(OP_PUSHVAR);
          PUSH(index);
        }
      }
      if(!t) {
        fprintf(stderr, "Cannot deref var (%s). Must use '%s value' or '%s set'\n", word, word, word);
        return -1;        
      }
      return 1;
    }

    fprintf(stderr, "Unknown word or var: %s\n", word);
    return -1;
  }
  return 0;
}
#undef PUSH
#undef IS
#undef IF
#undef EIF

// TODO: when more than 3 constants, first opcode's value gets wrecked 
// pushing NOP as first inst seems to fix this, but still needs to be fixed

AttoBlock* compileFrink(FrinkProgram* fp) {
  AttoBlock* b = AttoBlockNew();

  AttoBlock_push_inst(b, OP_NOP);

  int i;
  for(i = 0; i < fp->len; ++i) {
      Token t = fp->tokens[i];

      switch(t.type) {
      case TOKEN_WORD: {
	int v = compileWord(fp, i, b, t.content);
        if(v == 0) {
        } else if( v != -1) {
          i += v;
        } else {
          fprintf(stderr, "Error during compile. Abort\n");
          return NULL;
        }
	break;
      }
      case TOKEN_STRING: {
	TValue tv;
	AttoString str;
	Value v;
	str.len = strlen(t.content);
	str.ptr = t.content;
	v.string = str;
	tv.type = TYPE_STRING;
	tv.value = v;

	Instruction in = pushConstant(b, tv);
	AttoBlock_push_inst(b, (int)OP_PUSHCONST);
	AttoBlock_push_inst(b, in);

	break;
      }
      case TOKEN_NUMBER: {
	AttoNumber d = strtod(t.content, NULL);

	Instruction in = pushConstant(b, createNumber(d));
	AttoBlock_push_inst(b, (int)OP_PUSHCONST);
	AttoBlock_push_inst(b, in);

	break;
      }
		
      case TOKEN_UNKNOWN:
      default:
        if(strcmp(t.content, "NULL")) {
          fprintf(stderr, "Unknown token: `%s'\n", t.content);
        }
    }
  }

  AttoBlock_push_inst(b, OP_NOP);

  b->sizev = fp->numvars;

  return b;

}
