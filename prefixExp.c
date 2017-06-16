/* prefixExp.c, Gerard Renardel, 29 January 2014
 *
 * In this file functions are defined for the construction of expression trees
 * from prefix expressions generated by the following BNF grammar:
 *
 * <prefexp>   ::= <number> | <identifier> | '+' <prefexp> <prefexp>
 *             | '-' <prefexp> <prefexp> | '*' <prefexp> <prefexp> | '/' <prefexp> <prefexp>
 *
 * <number>      ::= <digit> { <digit> }
 *
 * <identifier> ::= <letter> { <letter> | <digit> }
 *
 * Starting pount is the token list obtained from the scanner (in scanner.c).
 */

#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc, free */
#include <assert.h> /* assert */
#include<string.h>
#include "scanner.h"
#include "recognizeExp.h"
#include "evalExp.h"
#include "prefixExp.h"
/* The function newExpTreeNode creates a new node for an expression tree.
 */

int main(){
	infixExpTrees();
	return 0;
}

ExpTree newExpTreeNode(TokenType tt, Token t, ExpTree tL, ExpTree tR) {
  ExpTree new = malloc(sizeof(ExpTreeNode));
  assert (new!=NULL);
  new->tt = tt;
  new->t = t;
  new->left = tL;
  new->right = tR;
  return new;
}

/* The function valueIdentifier recognizes an identifier in a token list and
 * makes the second parameter point to it.
 */

int valueIdentifier(List *lp, char **sp) {
  if (*lp != NULL && (*lp)->tt == Identifier ) {
    *sp = ((*lp)->t).identifier;
    *lp = (*lp)->next;
    return 1;
  }
  return 0;
}

/* The function valueOperator recognizes an arithmetic operator in a token list
 * and makes the second parameter point to it.
 * Here the auxiliary function isOperator is used.
 */

int isOperator(char c) {
  return ( c == '+' || c == '-' || c == '*' || c == '/');
}

int valueOperator(List *lp, char *cp) {
  if (*lp != NULL && (*lp)->tt == Symbol && isOperator(((*lp)->t).symbol) ) {
    *cp = ((*lp)->t).symbol;
    *lp = (*lp)->next;
    return 1;
  }else
  return 0;
}

int valueOperatorForMultDiv(List *lp, char *cp) {
  if (*lp != NULL && (*lp)->tt == Symbol && (((*lp)->t).symbol == '*' || ((*lp)->t).symbol == '/') ) {
    *cp = ((*lp)->t).symbol;
	*lp = (*lp)->next;
    return 1;
  }else
  return 0;
}

int valueOperatorForPlusMinus(List *lp, char *cp) {
  if (*lp != NULL && (*lp)->tt == Symbol && (((*lp)->t).symbol == '+' || ((*lp)->t).symbol == '-') ) {
    *cp = ((*lp)->t).symbol;
	*lp = (*lp)->next;
    return 1;
  }else
  return 0;
}

/* De functie freeExpTree frees the memory of the nodes in the expression tree.
 * Observe that here, unlike in freeList, the strings in indentifier nodes
 * are not freed. The reason is that the function newExpTree does not allocate
 * memory for strings in nodes, but only a pointer to a string in a node
 * in the token list.
 */

void freeExpTree(ExpTree tr) {
  if (tr==NULL) {
    return;
  }
  freeExpTree(tr->left);
  freeExpTree(tr->right);
  free(tr);
}

/*----------------------------------------------------*/


int treeTerm(List *lp, ExpTree *tp) {
	char c;
	ExpTree tR;
	Token t;

	if (!treeFactor(lp, tp))
	{
		return 0;
	}
	while (valueOperatorForMultDiv(lp,&c))
	{
		if (!treeFactor(lp, &tR))
		{
			return 0;
		}
		t.symbol= c;
		*tp = newExpTreeNode(Symbol,t, *tp, tR);
	}
	return 1;
}

int treeInfixExpression(List *lp, ExpTree *tp) {
    char c;
    Token t;
    ExpTree tR;
    if (!treeTerm(lp, tp)){
        return 0;
    }
    while (valueOperatorForPlusMinus(lp,&c)){

        if (!treeTerm(lp,&tR)){
            return 0;
        }
        t.symbol= c;

        *tp = newExpTreeNode(Symbol,t, *tp, tR);
    }
    if (*lp==NULL){
        return 1;
    }
    return 1;
}

int treeFactor(List *lp, ExpTree *tp) {
  double w;
  char *s;
  Token t;

  if ( valueNumber(lp,&w) ) {
    t.number = (int)w;
    *tp = newExpTreeNode(Number, t, NULL, NULL);
    return 1;
  }
  if ( valueIdentifier(lp,&s) ) {
    t.identifier = s;
    *tp = newExpTreeNode(Identifier, t, NULL, NULL);
    return 1;
  }
  if (acceptCharacter(lp,'(') && treeInfixExpression(lp, tp) && acceptCharacter(lp, ')'))
  {
    return 1;
  }
  return 0;
}
/*-----------------------------------*/

/* The function printExpTreeInfix does what its name suggests.
 */

void printExpTreeInfix(ExpTree tr) {
  if (tr == NULL) {
    return;
  }
  switch (tr->tt) {
  case Number:
    printf("%d",(tr->t).number);
   break;
  case Identifier:
    printf("%s",(tr->t).identifier);
    break;
  case Symbol:
    printf("(");
    printExpTreeInfix(tr->left);
    printf(" %c ",(tr->t).symbol);
    printExpTreeInfix(tr->right);
    printf(")");
    break;
  }
}

/* The function isNumerical checks for an expression tree whether it represents
 * a numerical expression, i.e. without identifiers.
 */

int isNumerical(ExpTree tr) {
  assert(tr!=NULL);
  if (tr->tt==Number) {
    return 1;
  }
  if (tr->tt==Identifier) {
    return 0;
  }
  return (isNumerical(tr->left) && isNumerical(tr->right));
}

/* The function valueExpTree computes the value of an expression tree that represents a
 * numerical expression.
 */

double valueExpTree(ExpTree tr) {  /* precondition: isNumerical(tr)) */
  double lval, rval;
  assert(tr!=NULL);
  if (tr->tt==Number) {
    return (tr->t).number;
  }
  lval = valueExpTree(tr->left);
  rval = valueExpTree(tr->right);
  switch ((tr->t).symbol) {
  case '+':
    return (lval + rval);
  case '-':
    return (lval - rval);
  case '*':
    return (lval * rval);
  case '/':
    assert( rval!=0 );
    return (lval / rval);
  default:
    abort();
  }
}

/*-----------------simplify-------------------*/

ExpTree simplify(ExpTree tr){

    if((tr->left) == NULL && (tr->right)==NULL){
        return tr;
    }
    (tr->left)=simplify((tr)->left);
    (tr->right)=simplify((tr)->right);

    switch ((tr->t).symbol) {
        case '+':
            if((tr->left->tt)==Number && (tr->left->t).number==0){
                return tr->right;
            }
            else if( (tr->right->tt)==Number && (tr->right->t).number==0){
                return tr->left;
            }
            break;
        case '-':
            if( (tr->right->tt)==Number && (tr->right->t).number==0){
				return tr->left;
            }
            break;
        case '*':
            if( (tr->left->tt)==Number && (tr->left->t).number==0){
                return tr->left;
            }
            else if( (tr->right->tt)==Number && (tr->right->t).number==0){
                return tr->right;
            }
            else if( (tr->left->tt)==Number && (tr->left->t).number == 1){
                return tr->right;
            }
            else if( (tr->right->tt)==Number && (tr->right->t).number == 1){
				return tr->left;
            }
            break;
        case '/':
            if( (tr->left->tt)==Number && (tr->left->t).number==0){
                return tr->left;
            }
            else if( (tr->right->tt)==Number && (tr->right->t).number==1){
                return tr->left;
            }
            break;
        default:
        exit(EXIT_FAILURE);

    }
    return tr;
}
/*-----------------End simplify-------------------*/


/*-----------------Differentiate-------------------*/
/* the function differentiate creates a new tree, tr2,
* that contains the derivative of the ecuation saved in tr.
 */
ExpTree differentiate(ExpTree tr, ExpTree tr2){
    Token tempToken;           /* Temporary token.*/
    ExpTree tR,tL,tL1,tL2;      /* Temporary trees used to create tr2.*/
    char *x = "x";
    if((tr->left) == NULL && (tr->right)==NULL){
     if(((tr->tt)==Identifier) && (strcmp((tr->t).identifier, x)==0)){
        tempToken.number =1;     /*If it's the variable x, the derivative of it is 1. */
        tr2=newExpTreeNode(Number, tempToken, NULL, NULL);
	}else{
        tempToken.number=0;    /*If it's a constant or another variable, the derivative is 0. */
        tr2=newExpTreeNode(Number,tempToken , NULL, NULL);
	}
    }
	 if ((tr->tt)==Symbol && ((tr->t).symbol=='+' || (tr->t).symbol=='-')){
	    tL=differentiate(tr->left,tr->left);
	    tR=differentiate(tr->right,tr->right);
		tr2=newExpTreeNode(Symbol,tr->t,tL,tR);/* We send the token tr->t because the symbol remains the same.*/
    }else if ((tr->tt)==Symbol && (tr->t).symbol=='*'){
        tempToken.symbol='*';
        tL=newExpTreeNode(Symbol,tempToken,differentiate(tr->left,tr->left),tr->right);
        tR=newExpTreeNode(Symbol,tempToken,tr->left,differentiate(tr->right,tr->right));
        tempToken.symbol='+';
        tr2=newExpTreeNode(Symbol,tempToken,tL,tR);
   }else if ((tr->tt)==Symbol && (tr->t).symbol=='/'){
        tempToken.symbol='*';
        tR=newExpTreeNode(Symbol,tempToken,tr->right,tr->right);
        tL1=newExpTreeNode(Symbol,tempToken,differentiate(tr->left,tr->left),tr->right);
        tL2=newExpTreeNode(Symbol,tempToken,tr->left,differentiate(tr->right,tr->right));
        tempToken.symbol='-';
        tL=newExpTreeNode(Symbol,tempToken,tL1,tL2);
        tempToken.symbol='/';
        tr2=newExpTreeNode(Symbol,tempToken,tL,tR);
    }
    return tr2;
}
/*-----------------End Differentiate-------------------*/


/* the function prefExpressionExpTrees performs a dialogue with the user and tries
 * to recognize the input as an infix expression. When it is a numerical infix
 * expression, its value is computed and printed, else we print its derivative to x.
 */
void infixExpTrees() {
  char *ar;
  List tl, tl1;
  ExpTree t = NULL, t2=NULL;
  printf("give an expression: ");
  ar = readInput();
  while (ar[0] != '!') {
    tl = tokenList(ar);
    printList(tl);
    tl1 = tl;
    if ( treeInfixExpression(&tl1,&t) && tl1 == NULL ) {
         /* there should be no tokens left */
      printf("in infix notation: ");
      printExpTreeInfix(t);
      printf("\n");
      if ( isNumerical(t) ) {
       printf("the value is %g\n",valueExpTree(t));
      } else {
        printf("this is not a numerical expression\n");
      }
      printf("simplified: ");
      t=simplify(t);
      printExpTreeInfix(t);
      t2=differentiate(t,t2);

      printf("\nderivative to x: ");
      t2=simplify(t2);
      printExpTreeInfix(t2);
	  printf("\n");
    } else {
      printf("this is not an expression\n");
    }
    freeExpTree(t);
    t = NULL;
    t2= NULL;
    freeTokenList(tl);
    free(ar);
    printf("\ngive an expression: ");
    ar = readInput();
  }
  free(ar);
  printf("good bye\n");
}
