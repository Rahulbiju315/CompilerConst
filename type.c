#include "type.h"
#include "ast_def.h"
#include "symbolTable.h"
#include "typeCheck.h"
#include <stdlib.h>
#include <stdio.h>
#include "semError.h"
#include "astPrint.h"
const TypeExpression typeError = (TypeExpression){BTYPE_ERROR, NULL, NULL};
TypeExpression typeVoid(){
    TypeExpression tvoid = (TypeExpression){BTYPE_VOID, NULL, NULL};
    tvoid.expList = malloc(sizeof(ExpressionList));
    return tvoid;
}

const int INPUT_LIST = 0;
const int OUTPUT_LIST = 1;

ExpressionList *createExpressionList()
{
    ExpressionList *expList = malloc(sizeof(ExpressionList));
    expList->typeExpressionList = malloc(sizeof(TypeExpression));
    expList->size = 0;
    expList->cap = 1;
    return expList;
}

void growIfFull(ExpressionList *list)
{
    if (list->size == list->cap)
    {
        list->cap = (int)(list->cap * 1.4 + 1);
        list->typeExpressionList = realloc(list->typeExpressionList, list->cap * sizeof(TypeExpression));
    }
}

void insertToExpList(ExpressionList *list, TypeExpression t)
{
    growIfFull(list);
    list->typeExpressionList[list->size] = t;
    list->size++;
}

int validate(TypeExpression t, BasicType basicType)
{
    if (t.basicType != basicType)
    {
        maybePrintf("Attempting to use type %s as type %s\n", basicTypeToString(t.basicType), basicTypeToString(basicType));
        return 0;
    }

    return 1;
}

void addToUnion(TypeExpression unionType, TypeExpression t)
{
    if (!validate(unionType, BTYPE_UNION))
        return;
    insertToExpList(unionType.expList, t);
}

void addToFuncInput(TypeExpression functionType, TypeExpression t)
{
    if (!validate(functionType, BTYPE_FUNCTION))
        return;
    TypeExpression inputType = getInputType(functionType);
    insertToExpList(inputType.expList, t);
}

void addToFuncOutput(TypeExpression functionType, TypeExpression t)
{
    if (!validate(functionType, BTYPE_FUNCTION))
        return;
    TypeExpression outputType = getOutputType(functionType);
    insertToExpList(outputType.expList, t);
}

void addToRecord(TypeExpression recordType, TypeExpression t)
{
    if (!validate(recordType, BTYPE_RECORD))
        return;
    insertToExpList(recordType.expList, t);
}

TypeExpression getOutputType(TypeExpression functionType)
{
    if (!validate(functionType, BTYPE_FUNCTION))
        return typeError;
    return functionType.expList->typeExpressionList[OUTPUT_LIST];
}

TypeExpression getInputType(TypeExpression functionType)
{
    if (!validate(functionType, BTYPE_FUNCTION))
        return typeError;
    return functionType.expList->typeExpressionList[INPUT_LIST];
}

TypeExpression typeExpression(BasicType type)
{
    TypeExpression te = (TypeExpression){type, NULL, NULL};
    return te;
}

TypeExpression functionTypeExpression()
{
    TypeExpression funcTE = typeExpression(BTYPE_FUNCTION);
    funcTE.expList = createExpressionList();

    TypeExpression inpTE = typeExpression(BTYPE_PARAM_LIST);
    TypeExpression outTE = typeExpression(BTYPE_PARAM_LIST);

    insertToExpList(funcTE.expList, inpTE);
    insertToExpList(funcTE.expList, outTE);

    return funcTE;
}

TypeExpression unionTypeExpression()
{
    TypeExpression unionTE = typeExpression(BTYPE_UNION);
    unionTE.expList = createExpressionList();

    return unionTE;
}

TypeExpression rnumTypeExpression()
{
    TypeExpression rnumTE = typeExpression(BTYPE_RNUM);
    rnumTE.expList = createExpressionList();

    return rnumTE;
}

TypeExpression numTypeExpression()
{
    TypeExpression numTE = typeExpression(BTYPE_NUM);
    numTE.expList = createExpressionList();

    return numTE;
}

TypeExpression recordTypeExpression(char *name)
{
    TypeExpression recordTE = typeExpression(BTYPE_RECORD);
    recordTE.name = name;
    recordTE.expList = createExpressionList();

    return recordTE;
}

TypeExpression paramListTypeExpression()
{
    TypeExpression paramListTE = typeExpression(BTYPE_PARAM_LIST);
    paramListTE.expList = createExpressionList();

    return paramListTE;
}

// I am basically writing function for each and every ast element separately.


void handleTypeExpressionProgram(Ast_Program *astElement)
{
    // changing the current symbol table.
    //localSymbolTableList.current = localSymbolTableList.size - 1;
    handleTypeExpressionOtherFunctions(astElement->otherFunctions);
    handleTypeExpressionMain(astElement->mainFunction);
    if ((astElement->mainFunction->typeExpr.basicType == BTYPE_ERROR) || (astElement->otherFunctions->typeExpr.basicType == BTYPE_ERROR))
    {
        astElement->typeExpr = typeExpression(BTYPE_ERROR);
    }
    else
    {
        astElement->typeExpr = typeExpression(BTYPE_VOID);
    }
}

void handleTypeExpressionMain(Ast_Main* astElement){
    //change the symbol table.
    loadSymbolTable("_main");
    handleTypeExpressionStmts(astElement->stmts);
    if(astElement->stmts->typeExpr.basicType == BTYPE_ERROR){
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }else{
        astElement->typeExpr.basicType = BTYPE_VOID;
        astElement->typeExpr.expList = createExpressionList();
    }
}

void handleTypeExpressionFunction(Ast_Function *astElement)
{
    // if(findFunc(astElement->funId) != NULL)
    //     error("Redefinition of function");
    //pushSymbolTable(astElement->funId);
    // insertFunc(astElement->funId, topSymbolTable());
    loadSymbolTable(astElement->funId);
    
    handleTypeExpressionParameterList(astElement->input_par);
    handleTypeExpressionParameterList(astElement->output_par);
    handleTypeExpressionStmts(astElement->stmts);
    if ((astElement->input_par->typeExpr.basicType == BTYPE_ERROR) || (astElement->output_par->typeExpr.basicType == BTYPE_ERROR) || (astElement->stmts->typeExpr.basicType == BTYPE_ERROR))
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr = functionTypeExpression();
        astElement->typeExpr.expList->typeExpressionList[0] = astElement->input_par->typeExpr;
        astElement->typeExpr.expList->typeExpressionList[1] = astElement->output_par->typeExpr;
    }
    SymbolVal *variable = findFunc(astElement->funId);
    variable->typeExpr = astElement->typeExpr;
}

void handleTypeExpressionOtherFunctions(Ast_OtherFunctions *astElement)
{
    int isError = 0;
    for (int i = 0; i < astElement->functionList->size; i++)
    {
        Ast_Function *f = astElement->functionList->nodes[i]->node.function;
        //localSymbolTableList.current = i;
        handleTypeExpressionFunction(f);
        if (f->typeExpr.basicType == BTYPE_ERROR)
        {
            isError = 1;
            astElement->typeExpr.basicType = BTYPE_ERROR;
        }
    }
    if (isError == 0)
    {
        astElement->typeExpr.basicType = BTYPE_VOID;
        astElement->typeExpr.expList = createExpressionList();
    }
}

void handleTypeExpressionParameterList(Ast_ParameterList *astElement)
{
    int isError = 0;
    for (int i = 0; i < astElement->parameterList->size; i++)
    {
        Ast_ParameterDeclaration *paradec = astElement->parameterList->nodes[i]->node.parameterDeclaration;
        handleTypeExpressionParameterDeclaration(paradec);
        if (paradec->typeExpr.basicType == BTYPE_ERROR)
        {
            isError = 1;
        }
    }

    if (isError == 0)
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr.basicType = BTYPE_PARAM_LIST;
        astElement->typeExpr.expList = createExpressionList();
        for (int i = 0; i < astElement->parameterList->size; i++)
        {
            if (astElement->typeExpr.expList->size == astElement->typeExpr.expList->cap)
            {
                growIfFull(astElement->typeExpr.expList);
            }
            Ast_ParameterDeclaration *paradec = astElement->parameterList->nodes[i]->node.parameterDeclaration;
            insertToExpList(astElement->typeExpr.expList, paradec->typeExpr);
        }
    }
}

void handleTypeExpressionStmts(Ast_Stmts *astElement)
{
    handleTypeExpressionTypeDefinitions(astElement->typeDefinitions);
    handleTypeExpressionDeclarations(astElement->declarations);
    handleTypeExpressionOtherStmts(astElement->otherStmts);
    handleTypeExpressionIdlist(astElement->returnIds);
    //should not we the return ids with output ids of the fucntion?
    SymbolVal *variable = findFunc(currentSymbolTable()->name);
    if(!checkifEqual(variable->typeExpr.expList->typeExpressionList[1],astElement->returnIds->typeExpr)){
        printf("Return Ids of the function %s are not matching with output parameters\n",currentSymbolTable()->name);
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = NULL;
        return;
    }
    if ((astElement->declarations->typeExpr.basicType == BTYPE_ERROR) || (astElement->typeDefinitions->typeExpr.basicType == BTYPE_ERROR) || (astElement->otherStmts->typeExpr.basicType == BTYPE_ERROR) || (astElement->returnIds->typeExpr.basicType == BTYPE_ERROR))
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr.basicType = BTYPE_VOID;
        astElement->typeExpr.expList = createExpressionList();
    }
}

void handleTypeExpressionParameterDeclaration(Ast_ParameterDeclaration *astElement)
{
    handleTypeExpressionDatatype(astElement->datatype);

    if (astElement->datatype->typeExpr.basicType == BTYPE_ERROR)
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr = astElement->datatype->typeExpr;
    }
}

void handleTypeExpressionDatatype(Ast_Datatype *astElement)
{
    if (astElement->datatype == DT_NUM)
    {
        astElement->typeExpr.basicType = BTYPE_NUM;
        astElement->typeExpr.expList = createExpressionList();
    }
    else if (astElement->datatype == DT_RECORD)
    {
        astElement->typeExpr = recordTypeExpression("");
        astElement->typeExpr.name = astElement->name;
    }
    else if (astElement->datatype == DT_RNUM)
    {
        astElement->typeExpr.basicType = BTYPE_RNUM;
        astElement->typeExpr.expList = createExpressionList();
    }
    else if (astElement->datatype == DT_UNION)
    {
        astElement->typeExpr = unionTypeExpression();
        astElement->typeExpr.name = astElement->name;
    }
    else
    {
        maybePrintf("Error: Datatype does not exist(Line Number %d)\n", astElement->lineNo);
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
        return;
    }
}

void handleTypeExpressionTypeDefinitions(Ast_TypeDefinitions *astElement)
{
    int isError = 0;
    for (int i = 0; i < astElement->typeDefintionList->size; i++)
    {
        Ast_TypeDefinition *typedeff = astElement->typeDefintionList->nodes[i]->node.typeDefinition;
        handleTypeExpressionTypeDefinition(typedeff);
        if (typedeff->typeExpr.basicType == BTYPE_ERROR)
        {
            isError = 1;
        }
    }
    if (isError)
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr.basicType = BTYPE_VOID;
        astElement->typeExpr.expList = createExpressionList();
    }
}

void handleTypeExpressionDeclarations(Ast_Declarations *astElement)
{
    int isError = 0;
    for (int i = 0; i < astElement->declarationList->size; i++)
    {
        Ast_Declaration *astdec = astElement->declarationList->nodes[i]->node.declaration;
        handleTypeExpressionDeclaration(astdec);
        if (astdec->typeExpr.basicType == BTYPE_ERROR)
        {
            isError = 1;
        }
    }
    if (isError)
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr.basicType = BTYPE_VOID;
        astElement->typeExpr.expList = createExpressionList();
    }
}

void handleTypeExpressionOtherStmts(Ast_OtherStmts *astElement)
{
    int isError = 0;
    for (int i = 0; i < astElement->otherStmtList->size; i++)
    {
        Ast_Stmt *statement = astElement->otherStmtList->nodes[i]->node.stmt;
        handleTypeExpressionStmt(statement);
        if (statement->type == STMT_ASSIGN)
        {
            if (statement->stmtUnion.assignStmt->typeExpr.basicType == BTYPE_ERROR)
            {
                maybePrintf("Left side and right side of assignment are different(Line number %d)\n", statement->stmtUnion.assignStmt->lineNo);
            }
        }
        else if (statement->type == STMT_COND)
        {
            if (statement->stmtUnion.condStmt->typeExpr.basicType == BTYPE_ERROR)
            {
                isError = 1;
            }
        }
        else if (statement->type == STMT_ITER)
        {
            if (statement->stmtUnion.iterStmt->typeExpr.basicType == BTYPE_ERROR)
            {
                isError = 1;
            }
        }
        else if (statement->type == STMT_FUN_CALL)
        {
            if (statement->stmtUnion.funCallStmt->typeExpr.basicType == BTYPE_ERROR)
            {
                isError = 1;
            }
        }
        else if (statement->type == STMT_IO)
        {
            if (statement->stmtUnion.ioStmt->typeExpr.basicType == BTYPE_ERROR)
            {
                isError = 1;
            }
        }
    }
    if (isError)
    {
        astElement->typeExpr.basicType = BTYPE_ERROR;
        astElement->typeExpr.expList = createExpressionList();
    }
    else
    {
        astElement->typeExpr.basicType = BTYPE_VOID;
        astElement->typeExpr.expList = createExpressionList();
    }
}

void handleTypeExpressionIdlist(Ast_IdList *astElement)
{
    int isError = 0;
    // Basically get the types of the ids in the corresponding symbol table.
    for (int i = 0; i < astElement->idList->size; i++)
    {
        Ast_Id *id = astElement->idList->nodes[i]->node.id;
        SymbolVal *entry = findVar(id->id);
        // what if the variable is not in the symbol table.
        // what will it return and we need to return array.
        if (entry == NULL)
        {
            isError = 1;
            maybePrintf("Variable %s is used without declaring(Line Number %d)\n", id->id, astElement->lineNo);
        }

    }
    if (isError)
    {
        astElement->typeExpr = typeError;
    }
    else
    {
        astElement->typeExpr = paramListTypeExpression();

        for (int i = 0; i < astElement->idList->size; i++)
        {
            Ast_Id *id = astElement->idList->nodes[i]->node.id;
            SymbolVal *entry = findVar(id->id);
            // How do I initialise to typeexpr structure???
            // Insert all of them to this.
            insertToExpList(astElement->typeExpr.expList,entry->typeExpr);
        }
    }
}

void handleTypeExpressionTypeDefinition(Ast_TypeDefinition *astElement)
{
    // These are already insterted into symbol table.
    // what should I do more?
    // If I do not find it in symbol table then i think i need to report error.
}

void handleTypeExpressionDeclaration(Ast_Declaration *astElement)
{
    // push the corresponding thing into symbol table.
    // check if the id is alredy present in global symbol table.
    // SymbolVal *dec = find(&globVarSymbolTable, astElement->id);
    // if (dec != NULL)
    // {
    //     maybePrintf("%s is a global variable and it is declared as a local variabe also.\n", astElement->id);
    //     astElement->typeExpr.basicType = BTYPE_ERROR;
    //     astElement->typeExpr.expList = createExpressionList();
    // }
    // else
    // {
    //     // Inserting into the symbol table.
    //     insertVar(astElement->id, astElement->datatype->datatype, astElement->datatype->name);
    //     // what the hell is typename??
    // }
}

int checkifEqual(TypeExpression exp1, TypeExpression exp2)
{
    if (exp1.basicType == exp2.basicType)
    {
        if ((exp1.expList == NULL) && (exp2.expList == NULL))
        {
            return 1;
        }
        else if ((exp1.expList != NULL) && (exp2.expList != NULL))
        {
            if (exp1.expList->size == exp2.expList->size)
            {
                for (int i = 0; i < exp1.expList->size; i++)
                {
                    // but what if there are in different order.
                    if (!checkifEqual(exp1.expList->typeExpressionList[i], exp2.expList->typeExpressionList[i]))
                    {
                        return 0;
                    }
                }
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void handleTypeExpressionStmt(Ast_Stmt *astElement)
{
    if (astElement->type == STMT_ASSIGN)
    {
        Ast_AssignmentStmt *assignmentStatement = astElement->stmtUnion.assignStmt;
        // need to check if the type of left hand side is equal to right hand side.
        handleTypeExpressionArithmeticExpression(assignmentStatement->expression);
        handleTypeExpressionSingleOrRecId(assignmentStatement->singleOrRecId);
        if (checkifEqual(assignmentStatement->expression->typeExpr, assignmentStatement->singleOrRecId->typeExpr))
        {
            if (assignmentStatement->expression->typeExpr.basicType == BTYPE_ERROR)
            {
                assignmentStatement->typeExpr.basicType = BTYPE_ERROR;
                assignmentStatement->typeExpr.expList = createExpressionList();
            }
            else
            {
                assignmentStatement->typeExpr.basicType = BTYPE_VOID;
                assignmentStatement->typeExpr.expList = createExpressionList();
            }
        }
        else
        {
            assignmentStatement->typeExpr.basicType = BTYPE_ERROR;
            assignmentStatement->typeExpr.expList = createExpressionList();
        }
    }
    else if (astElement->type == STMT_COND)
    {
        Ast_ConditionalStmt *conditionalStatement = astElement->stmtUnion.condStmt;
        handleTypeExpressionOtherStmts(conditionalStatement->body);
        handleTypeExpressionOtherStmts(conditionalStatement->elsePart);
        handleTypeExpressionBooleanExpression(conditionalStatement->condition);
        TypeExpression *bodytype = &conditionalStatement->body->typeExpr;
        TypeExpression *elseParttype = &conditionalStatement->elsePart->typeExpr;
        TypeExpression *conditiontype;
        if(conditionalStatement->condition->bexpType == BEXP_BOOL_OP){
              conditiontype = &conditionalStatement->condition->bexp.boolOp->typeExpr;    
        }
        else if(conditionalStatement->condition->bexpType == BEXP_VAR_COMP){
              conditiontype = &conditionalStatement->condition->bexp.boolOp->typeExpr;  
        }

        if((bodytype->basicType == BTYPE_ERROR) || (elseParttype->basicType == BTYPE_ERROR) || (conditiontype->basicType == BTYPE_ERROR)){
            conditionalStatement->typeExpr.basicType = BTYPE_ERROR;
            conditionalStatement->typeExpr.expList = createExpressionList();
        }else{
            conditionalStatement->typeExpr.basicType = BTYPE_VOID;
            conditionalStatement->typeExpr.expList = createExpressionList();
        }

    }
    else if (astElement->type == STMT_ITER)
    {
        int isError = 0;
        Ast_IterativeStmt *iterativeStatement = astElement->stmtUnion.iterStmt;
        handleTypeExpressionBooleanExpression(iterativeStatement->condition);
        handleTypeExpressionOtherStmts(iterativeStatement->body);

        if(iterativeStatement->condition->bexpType == BEXP_BOOL_OP){
              if(iterativeStatement->condition->bexp.boolOp->typeExpr.basicType == BTYPE_ERROR){
                        isError = 1; 
              }     
        }
        else if(iterativeStatement->condition->bexpType == BEXP_VAR_COMP){
            if(iterativeStatement->condition->bexp.varComp->typeExpr.basicType == BTYPE_ERROR){
                isError = 1;
            }

        }

        if(iterativeStatement->body->typeExpr.basicType == BTYPE_ERROR){
            isError = 1;
        }

        if(isError){
            iterativeStatement->typeExpr.basicType = BTYPE_ERROR;
            iterativeStatement->typeExpr.expList = createExpressionList();
        }else{
            iterativeStatement->typeExpr.basicType = BTYPE_VOID;
            iterativeStatement->typeExpr.expList = createExpressionList();
        }



    }
    else if (astElement->type == STMT_FUN_CALL)
    {
        Ast_FunCallStmt *functionCallStatement = astElement->stmtUnion.funCallStmt;
        handleTypeExpressionIdlist(functionCallStatement->inputIdList);
        handleTypeExpressionIdlist(functionCallStatement->outputIdList);
        //extract function input and output par from symbol table and match them.
        
        SymbolVal *variable = findFunc(functionCallStatement->funId);
        if(strcmp(functionCallStatement->funId,currentSymbolTable()->name) == 0){
            printf("Recursion is not allowed\n");
            functionCallStatement->typeExpr.basicType = BTYPE_ERROR;
             functionCallStatement->typeExpr.expList = NULL; 
        }
        if(checkifEqual(variable->typeExpr.expList->typeExpressionList[0],functionCallStatement->inputIdList->typeExpr) && (checkifEqual(variable->typeExpr.expList->typeExpressionList[1],functionCallStatement->outputIdList->typeExpr))){
             functionCallStatement->typeExpr.basicType = BTYPE_VOID;
             functionCallStatement->typeExpr.expList = createExpressionList();

        }else{
             maybePrintf("function call data types not matched\n");
             functionCallStatement->typeExpr.basicType = BTYPE_ERROR;
             functionCallStatement->typeExpr.expList = createExpressionList(); 
        }


    }
    else if (astElement->type == STMT_IO)
    {
        Ast_IoStmt *ioStatement = astElement->stmtUnion.ioStmt;
        handleExpressionVar(ioStatement->var);
        if(ioStatement->var->typeExpr.basicType == BTYPE_ERROR){
            ioStatement->typeExpr.basicType = BTYPE_ERROR;
            ioStatement->typeExpr.expList = createExpressionList();
        }else{
            ioStatement->typeExpr.basicType = BTYPE_VOID;
            ioStatement->typeExpr.expList = createExpressionList();
        }
    }
}

int handleTypeExpressionArithmeticExpression(Ast_ArithmeticExpression *astElement)
{   

    
    if(astElement->aexpType == AEXP_EXP){
    
        TypeExpression *leftTypex;
        TypeExpression *rightTypex;
        ArithmeticExpression *arithexp = astElement->aexpUnion.exp;


        if(!handleTypeExpressionArithmeticExpression(arithexp->left))
            return 0;
        leftTypex = &arithexp->left->typeExpr;
        if(leftTypex->basicType == BTYPE_ERROR){
            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
        }
        
        if(!handleTypeExpressionArithmeticExpression(arithexp->right))
            return 0;
        rightTypex = &arithexp->right->typeExpr;
        if(rightTypex->basicType == BTYPE_ERROR){
            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
        }

        

        if ((leftTypex->basicType != BTYPE_NUM) && 
        (leftTypex->basicType != BTYPE_RNUM) && 
        (leftTypex->basicType != BTYPE_RECORD) && 
        (leftTypex->basicType != BTYPE_ERROR) &&
        (leftTypex->basicType != BTYPE_TAGGED_UNION))
        {
            maybePrintf("Arithmetic operations are only supported for record,integers and real numbers(Line Number %d)\n", astElement->lineNo);
            maybePrintf("Left :");
            printArithmeticExpression(arithexp->left, 0);
            maybePrintf("\tType :");
            printTypeExpr(arithexp->left->typeExpr);
            maybePrintf("\n");
            maybePrintf("Right :");
            printArithmeticExpression(arithexp->right, 0);
            maybePrintf("\tType :");
            printTypeExpr(arithexp->right->typeExpr);
            maybePrintf("\n");


            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
            return 0;
        }

        if ((rightTypex->basicType != BTYPE_NUM) && 
        (rightTypex->basicType != BTYPE_RNUM) && 
        (rightTypex->basicType != BTYPE_RECORD) && 
        (rightTypex->basicType != BTYPE_ERROR) &&
        (rightTypex->basicType != BTYPE_TAGGED_UNION))
        {
            maybePrintf("Arithmetic operations are only supported for record,integers and real numbers(Line Number %d)\n", astElement->lineNo);
            maybePrintf("Left : %s, Right : %s\n", basicTypeToString(leftTypex->basicType), basicTypeToString(leftTypex->basicType));

            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
            return 0;
        }

        if (arithexp->op == AOP_PLUS)
        {

            // both left and right types must be equal
            // otherwise throw error.
            if (checkifEqual(*leftTypex, *rightTypex))
            {
                astElement->typeExpr = *leftTypex;
            }
            else
            {
                maybePrintf("Right hand side and left hand side are of different types.(Line Number %d)", astElement->lineNo);
                maybePrintf("\n\tLeft : ");
                printArithmeticExpression(expAexp(astElement)->left, 0);
                maybePrintf("\n\tRight : ");
                printArithmeticExpression(expAexp(astElement)->right, 0);
                maybePrintf("\n");

                astElement->typeExpr.basicType = BTYPE_ERROR;
                astElement->typeExpr.expList = createExpressionList();
                return 0;
            }
        }
        else if (arithexp->op == AOP_MINUS)
        {

            // both left and right types must be equal
            // otherwise throw error.
            if (checkifEqual(*leftTypex, *rightTypex))
            {
                astElement->typeExpr = *leftTypex;
            }
            else
            {
                maybePrintf("Right hand side and left hand side are of different types.(Line Number %d)", astElement->lineNo);
                maybePrintf("\n\tLeft : ");
                printArithmeticExpression(expAexp(astElement)->left, 0);
                maybePrintf("\n\tRight : ");
                printArithmeticExpression(expAexp(astElement)->right, 0);
                maybePrintf("\n");
                astElement->typeExpr.basicType = BTYPE_ERROR;
                astElement->typeExpr.expList = createExpressionList();
                return 0;
            }
        }
        else if (arithexp->op == AOP_DIV)
        {

            // right must be of the type num or rnum.
            // left can be num or rnum or record type.

            if ((leftTypex->basicType == BTYPE_NUM) && (rightTypex->basicType == BTYPE_NUM))
            {
                astElement->typeExpr = *leftTypex;
            }
            else if ((leftTypex->basicType == BTYPE_RNUM) && (rightTypex->basicType == BTYPE_RNUM))
            {
                astElement->typeExpr = *leftTypex;
            }
            // else if ((leftTypex->basicType == BTYPE_RECORD) && ((rightTypex->basicType == BTYPE_NUM) || (rightTypex->basicType == BTYPE_RNUM)))
            // {
            //     astElement->typeExpr = *leftTypex;
            // }
            else
            {
                astElement->typeExpr.basicType = BTYPE_ERROR;
                astElement->typeExpr.expList = createExpressionList();
                maybePrintf("Division of records / unions is not supported(Line Number %d)\n", astElement->lineNo);
                return 0;
            }
        }
        else if (arithexp->op == AOP_MUL)
        {
            if ((leftTypex->basicType == BTYPE_NUM) && (rightTypex->basicType == BTYPE_NUM))
            {
                astElement->typeExpr = *leftTypex;
            }
            else if ((leftTypex->basicType == BTYPE_RNUM) && (rightTypex->basicType == BTYPE_RNUM))
            {
                astElement->typeExpr = *leftTypex;
            }
            else if ((leftTypex->basicType == BTYPE_RECORD) && ((rightTypex->basicType == BTYPE_NUM) || (rightTypex->basicType == BTYPE_RNUM)))
            {
                astElement->typeExpr = *leftTypex;
            }
            else
            {
                astElement->typeExpr.basicType = BTYPE_ERROR;
                astElement->typeExpr.expList = createExpressionList();
                maybePrintf("Multiplication of record or union with record or union is not supported(Line Number %d)\n", astElement->lineNo);
                return 0;
            }
        }
    
    }
    else if(astElement->aexpType == AEXP_VAR){
        handleExpressionVar(astElement->aexpUnion.var);
        if(astElement->aexpUnion.var->typeExpr.basicType == BTYPE_ERROR){
            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
        }else{
            astElement->typeExpr = astElement->aexpUnion.var->typeExpr;
            // maybePrintf("Assigning type\n", astElement->typeExpr.basicType);
        }
    }

    return 1;
}

void handleTypeExpressionSingleOrRecId(Ast_SingleOrRecId *astElement)
{
    if (astElement->fieldNameList->size == 0)
    {
        
        SymbolVal *variable = find(&globVarSymbolTable, astElement->id);
        if (variable == NULL)
        {
            // maybePrintf("YOOO THE ID %s is not present in sym table of %s\n", astElement->id, currentSymbolTable()->name);
            variable = findVar(astElement->id);
            if (variable == NULL)
            {
                maybePrintf("Variable %s is not declared but used (Line Number %d)\n", astElement->id, astElement->lineNo);
                astElement->typeExpr.basicType = BTYPE_ERROR;
                astElement->typeExpr.expList = createExpressionList();
                return;
            }
            // extract type from symbol table and then equate it to typeexpr.
            // if(astElement->typeExpr.basicType == BTYPE_VOID){
            //     maybePrintf("HERER OFR %S\n",  astElement->id);
            // }
            
            astElement->typeExpr = variable->typeExpr;
        
        }
        else
        {
            
            
            // extract type from symbol table and then equate it to typeexpr.
            astElement->typeExpr = variable->typeExpr;
            maybePrintf("%s : ", astElement->id);
            printTypeExpr(astElement->typeExpr);
            maybePrintf("\n");
        }
    }
    else
    {

        // check if the id is in the typedefinition symbol table.
        // if yes then check if the field definitons order match the record.

        // SymbolVal *variable = find(&typeDefSymbolTable, astElement->id);
        // if (variable == NULL)
        // {
        //     maybePrintf("%s record is not declared but used\n", astElement->id);
        //     astElement->typeExpr.basicType = BTYPE_ERROR;
        //     astElement->typeExpr.expList = createExpressionList();
        // }
        // else
        // {
        //     //incomplete.
        // }

        SymbolVal *variable = findType(astElement);
        if(variable == NULL){
            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
        }else{
            astElement->typeExpr =  variable->typeExpr;
        }
    }
}

void handleExpressionVar(Ast_Var *astElement)
{
    if(astElement->varType ==  VAR_ID){
        // maybePrintf("<< %s >>", astElement->varUnion.singleOrRecId->id);
        handleTypeExpressionSingleOrRecId(astElement->varUnion.singleOrRecId);
        if(astElement->varUnion.singleOrRecId->typeExpr.basicType == BTYPE_ERROR){
            astElement->typeExpr.basicType = BTYPE_ERROR;
            astElement->typeExpr.expList = createExpressionList();
        } else{
            astElement->typeExpr = astElement->varUnion.singleOrRecId->typeExpr;
        }
    }
    else if(astElement->varType == VAR_NUM){
        astElement->typeExpr = numTypeExpression();
    }
    else if(astElement->varType == VAR_RNUM){
        astElement->typeExpr = rnumTypeExpression();
    }

}

void handleTypeExpressionBooleanExpression(Ast_BooleanExpression *astElement){
     
     //what will be the type of boolean expression?
     if(astElement->bexpType == BEXP_BOOL_OP){

          BoolOperation *bool =  astElement->bexp.boolOp;
          handleTypeExpressionBooleanExpression(bool->left);
          handleTypeExpressionBooleanExpression(bool->right);

          TypeExpression *lefttypex;
          TypeExpression *righttypex;

          if(bool->left->bexpType == BEXP_BOOL_OP){
              lefttypex = &bool->left->bexp.boolOp->typeExpr; 
          }else{
              lefttypex = &bool->left->bexp.varComp->typeExpr;
          }
          if(bool->right->bexpType == BEXP_BOOL_OP){
               righttypex = &bool->right->bexp.boolOp->typeExpr;  
          }else{
               righttypex = &bool->right->bexp.varComp->typeExpr;
          }


          if((lefttypex->basicType == BTYPE_ERROR) || (righttypex->basicType == BTYPE_ERROR)){
              bool->typeExpr.basicType = BTYPE_ERROR;
              bool->typeExpr.expList = createExpressionList();
          }else{
              bool->typeExpr.basicType = BTYPE_VOID;
              bool->typeExpr.expList = createExpressionList();
          }
          
     }
     else if(astElement->bexpType == BEXP_VAR_COMP){
          VarComparison *varcomp = astElement->bexp.varComp;
          handleExpressionVar(varcomp->left);
          handleExpressionVar(varcomp->right);
          if((varcomp->left->typeExpr.basicType != BTYPE_ERROR) || (varcomp->right->typeExpr.basicType != BTYPE_ERROR)){
               varcomp->typeExpr.basicType = BTYPE_ERROR;
               varcomp->typeExpr.expList = createExpressionList();
               return;     
          }

          if(((varcomp->left->typeExpr.basicType != BTYPE_NUM) && (varcomp->left->typeExpr.basicType != BTYPE_RNUM))  || ((varcomp->right->typeExpr.basicType != BTYPE_RNUM) && (varcomp->right->typeExpr.basicType != BTYPE_NUM))){
              maybePrintf("Error : var comparison is not compatiable with this type(Line Number %d)\n", astElement->lineNo);
              varcomp->typeExpr.basicType = BTYPE_ERROR;
              varcomp->typeExpr.expList = createExpressionList();
              return;
          }

          if((varcomp->left->typeExpr.basicType != varcomp->right->typeExpr.basicType)){
              maybePrintf("Error: data types should be same for the comparison(Line Number %d)\n", astElement->lineNo);
              varcomp->typeExpr.basicType = BTYPE_ERROR;
              varcomp->typeExpr.expList = createExpressionList();
              return;
          }

          varcomp->typeExpr.basicType = BTYPE_VOID;
          varcomp->typeExpr.expList = createExpressionList();

     }
}

void printTypeExpr(TypeExpression t){
    maybePrintf("%s<", basicTypeToString(t.basicType));
    for(int i = 0; i < t.expList->size; i++){
        printTypeExpr(t.expList->typeExpressionList[i]);
        maybePrintf(", ");
    }
    maybePrintf(">");
}
