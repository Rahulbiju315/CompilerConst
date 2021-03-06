#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "token.h"
#include "symbolTable.h"
#include <stdio.h>

int AST_SIZE = 0;
int MAYBE = 0;
char* msg = NULL;
AstNode *makeEmptyAstNode()
{
    AstNode *astNode = malloc(sizeof(AstNode));
    memset(&astNode->node, 0, sizeof(AstNodeUnion));
    memset(&astNode->type, 0, sizeof(AstNodeType));
    return astNode;
}

float strToReal(char* str)
{
    return strtof(str, NULL);
}

int strToInt(char* str)
{   
    return strtol(str, NULL, 10);
}

int LINE = 1;
void updateLine(ParseTreeElement* ptElement){
    // for(int i )
}
void handleParseTreeElement(ParseTreeElement *ptElement)
{
    // if(ptElement == NULL){
    //     maybePrintf("PTELEMENT IS NULL!!");
    //     return;
    // }
    // maybePrintf("Handling ParseTreeElement of type %s - %s (Rule no = %d)\n", 
        // ptElement->elem.isTerminal ? "TERM" : "NON TERM", nonTermToStr(ptElement->elem.symbol), ptElement->ruleNo);
    if (!ptElement->elem.isTerminal)
    {
        switch (ptElement->elem.symbol)
        {
            case PROGRAM:
            {
                ParseTreeElement *prog = ptElement;
                ParseTreeElement *othFun = &ptElement->children[0];
                ParseTreeElement *mainFun = &ptElement->children[1];

                declareAstNode(nodeOtherFunctions, AST_OTHERFUNCTIONS, Ast_OtherFunctions, otherFunctions);
                nodeToAst(nodeOtherFunctions, otherFunctions)->functionList = createAstList();
                othFun->node_inh = nodeOtherFunctions;

                handleParseTreeElement(othFun);
                handleParseTreeElement(mainFun);

                declareAstNode(nodeProgram, AST_PROGRAM, Ast_Program, program);
                nodeToAst(nodeProgram, program)->mainFunction =
                    nodeToAst(mainFun->node_syn, mainFunction);

                nodeToAst(nodeProgram, program)->otherFunctions =
                    nodeToAst(othFun->node_syn, otherFunctions);

                ptElement->node_syn = nodeProgram;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeProgram,program)->lineNo = prog->lineNo;
                break;
            }

            case MAIN_FUNCTION:
            {
                ParseTreeElement *mainFunc = ptElement;
                ParseTreeElement *statements = &ptElement->children[1];

                pushSymbolTable("_main");
                loadSymbolTable("_main");

                handleParseTreeElement(statements);
                
                
                declareAstNode(nodeMainFunction, AST_MAIN, Ast_Main, mainFunction);
                nodeToAst(nodeMainFunction, mainFunction)->stmts =
                    nodeToAst(statements->node_syn, stmts);

                mainFunc->node_syn = nodeMainFunction;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeMainFunction,mainFunction)->lineNo = mainFunc->lineNo;
                break;
            }

            case OTHER_FUNCTIONS:
            {
                ParseTreeElement *otherFunc = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *func = &ptElement->children[0];
                    ParseTreeElement *otherFunc1 = &ptElement->children[1];

                    func->node_inh = ptElement->node_inh;
                    handleParseTreeElement(func);

                    otherFunc1->node_inh = func->node_syn;
                    handleParseTreeElement(otherFunc1);

                    otherFunc->node_syn = otherFunc1->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }

                else if (ptElement->ruleNo == 1)
                {
                    otherFunc->node_syn = ptElement->node_inh;
                }

                break;
            }

            case FUNCTION:
            {
                ParseTreeElement *func = ptElement;
                TokenInfo tkFunId = func->children[0].tinfo;
                ParseTreeElement *input_par = &ptElement->children[1];
                ParseTreeElement *output_par = &ptElement->children[2];
                ParseTreeElement *statements = &ptElement->children[4];
               
                pushSymbolTable(tkFunId.lexeme);
                loadSymbolTable(tkFunId.lexeme);

                handleParseTreeElement(input_par);
                handleParseTreeElement(output_par);
                handleParseTreeElement(statements);
                declareAstNode(nodeFunction, AST_FUNCTION, Ast_Function, function);

                nodeToAst(nodeFunction, function)->funId = tkFunId.lexeme;
                nodeToAst(nodeFunction, function)->input_par = nodeToAst(input_par->node_syn, parameterList);
                nodeToAst(nodeFunction, function)->output_par = nodeToAst(output_par->node_syn, parameterList);
                nodeToAst(nodeFunction, function)->stmts = nodeToAst(statements->node_syn, stmts);
    
                insertTo(nodeToAst(func->node_inh, otherFunctions)->functionList, nodeFunction);
                func->node_syn = func->node_inh;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeFunction, function)->lineNo = func->lineNo;
                break;
            }

            case INPUT_PAR:
            {
                ParseTreeElement *input_par = ptElement;
                ParseTreeElement *parameter_list = &ptElement->children[4];
                declareAstNode(nodeParameterList, AST_PARAMETERLIST, Ast_ParameterList, parameterList);
                nodeToAst(nodeParameterList, parameterList)->parameterList = createAstList();
                parameter_list->node_inh = nodeParameterList;
                handleParseTreeElement(parameter_list);
                
                AstList* list = nodeToAst(nodeParameterList, parameterList)->parameterList;
                for(int i = 0; i < list->size; i++){
                    Ast_ParameterDeclaration* param = nodeToAst(list->nodes[i], parameterDeclaration);
                    if(findVar(param->id) != NULL){
                        maybePrintf("Redeclaration of var %s in function %s\n", param->id, currentSymbolTable()->name);
                    }
                    insertVar(param->id, IN_PAR, param->datatype->datatype, param->datatype->name);
                }
                
                input_par->node_syn = parameter_list->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case OUTPUT_PAR:
            {
                ParseTreeElement *output_par = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *parameter_list = &ptElement->children[4];
                    declareAstNode(nodeParameterList, AST_PARAMETERLIST, Ast_ParameterList, parameterList);
                    nodeToAst(nodeParameterList, parameterList)->parameterList = createAstList();
                    parameter_list->node_inh = nodeParameterList;
                    handleParseTreeElement(parameter_list);
                    AstList* list = nodeToAst(nodeParameterList, parameterList)->parameterList;
                    for(int i = 0; i < list->size; i++){
                        Ast_ParameterDeclaration* param = nodeToAst(list->nodes[i], parameterDeclaration);
                        insertVar(param->id, OUT_PAR, param->datatype->datatype, param->datatype->name);
                    }
                    output_par->node_syn = parameter_list->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeParameterList, AST_PARAMETERLIST, Ast_ParameterList, parameterList);
                    nodeToAst(nodeParameterList, parameterList)->parameterList = createAstList();
                    output_par->node_syn = nodeParameterList;
                }
                break;
            }

            case PARAMETER_LIST:
            {
                ParseTreeElement *parameter_list = ptElement;
                ParseTreeElement *datatype = &ptElement->children[0];
                TokenInfo tkId = ptElement->children[1].tinfo;
                ParseTreeElement *remaining_list = &ptElement->children[2];
                handleParseTreeElement(datatype);
                // ADDTYPE TK_ID

                declareAstNode(nodeParameterDecl, AST_PARAMETERDECL, Ast_ParameterDeclaration, parameterDeclaration);

                nodeToAst(nodeParameterDecl, parameterDeclaration)->datatype = 
                    nodeToAst(datatype->node_syn, datatype);
                nodeToAst(nodeParameterDecl, parameterDeclaration)->id = 
                    tkId.lexeme;

                insertTo(nodeToAst(parameter_list->node_inh, parameterList)->parameterList, nodeParameterDecl);
                remaining_list->node_inh = parameter_list->node_inh;
                handleParseTreeElement(remaining_list);
                parameter_list->node_syn = remaining_list->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                //nodeToAst(nodeParameterDecl,parameterDeclaration)->lineNo = parameter_list->lineNo; 

                break;
            }

            case DATATYPE:
            {
                ParseTreeElement *datatype = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *primitive_datatype = &ptElement->children[0];
                    handleParseTreeElement(primitive_datatype);
                    datatype->node_syn = primitive_datatype->node_syn;
                }
                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *constructed_datatype = &ptElement->children[0];
                    handleParseTreeElement(constructed_datatype);
                    datatype->node_syn = constructed_datatype->node_syn;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case PRIMITIVE_DATATYPE:
            {
                ParseTreeElement *primitive_datatype = ptElement;
                TokenInfo tkNum = ptElement->children[0].tinfo;
                declareAstNode(nodeDatatype, AST_DATATYPE, Ast_Datatype, datatype);
                if (ptElement->ruleNo == 0)
                {
                    //nodeToAst(nodeDatatype, datatype)->typeExpr = numTypeExpression();
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_NUM;
                    nodeToAst(nodeDatatype, datatype)->name = NULL;
                    primitive_datatype->node_syn = nodeDatatype;
                }
                else if (ptElement->ruleNo == 1)
                {
                    //nodeToAst(nodeDatatype, datatype)->typeExpr = rnumTypeExpression();
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_RNUM;
                    nodeToAst(nodeDatatype, datatype)->name = NULL;
                    primitive_datatype->node_syn = nodeDatatype;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeDatatype,datatype)->lineNo = primitive_datatype->lineNo;
                break;
            }

            case CONSTRUCTED_DATATYPE:
            {
                ParseTreeElement *constructed_datatype = ptElement;
                TokenInfo tkRuid;
                declareAstNode(nodeDatatype, AST_DATATYPE, Ast_Datatype, datatype);
                if (ptElement->ruleNo == 0)
                {
                    tkRuid = ptElement->children[1].tinfo;
                    //nodeToAst(nodeDatatype, datatype)->name = tkRuid.lexeme;
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_RECORD;
                    //nodeToAst(nodeDatatype, datatype)->typeExpr = recordTypeExpression();
                }
                else if (ptElement->ruleNo == 1)
                {
                    tkRuid = ptElement->children[1].tinfo;
                    //nodeToAst(nodeDatatype, datatype)->name = tkRuid.lexeme;
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_UNION;
                    //nodeToAst(nodeDatatype, datatype)->typeExpr = unionTypeExpression();
                }
                else if (ptElement->ruleNo == 2)
                {
                    tkRuid = ptElement->children[0].tinfo;
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_REC_OR_UNION;
                }

                nodeToAst(nodeDatatype, datatype)->name = tkRuid.lexeme;
                constructed_datatype->node_syn = nodeDatatype;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeDatatype,datatype)->lineNo = constructed_datatype->lineNo;
                break;
            }

            case REMAINING_LIST:
            {
                ParseTreeElement *remaining_list = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *parameter_list = &ptElement->children[1];
                    parameter_list->node_inh = remaining_list->node_inh;
                    handleParseTreeElement(parameter_list);
                    remaining_list->node_syn = parameter_list->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    remaining_list->node_syn = remaining_list->node_inh;
                }
                break;
            }

            case STMTS:
            {
                ParseTreeElement *stmts = ptElement;
                ParseTreeElement *type_definitions = &ptElement->children[0];
                ParseTreeElement *declarations = &ptElement->children[1];
                ParseTreeElement *other_stmts = &ptElement->children[2];
                ParseTreeElement *return_stmt = &ptElement->children[3];

                declareAstNode(nodeTypeDefinitions, AST_TYPEDEFINITIONS, Ast_TypeDefinitions, typeDefinitions);
                nodeToAst(nodeTypeDefinitions, typeDefinitions)->typeDefintionList = createAstList();
                type_definitions->node_inh = nodeTypeDefinitions;
                handleParseTreeElement(type_definitions);
        
                declareAstNode(nodeDecls, AST_DECLARATIONS, Ast_Declarations, declarations);
                nodeToAst(nodeDecls, declarations)->declarationList = createAstList();
                declarations->node_inh = nodeDecls;
                handleParseTreeElement(declarations);

                declareAstNode(nodeOtherStmts, AST_OTHERSTMTS, Ast_OtherStmts, otherStmts);
                nodeToAst(nodeOtherStmts, otherStmts)->otherStmtList = createAstList();
                other_stmts->node_inh = nodeOtherStmts;
                handleParseTreeElement(other_stmts);

                handleParseTreeElement(return_stmt);

                declareAstNode(nodeStmts, AST_STMTS, Ast_Stmts, stmts);
                nodeToAst(nodeStmts, stmts)->typeDefinitions = nodeToAst(type_definitions->node_syn, typeDefinitions);
                nodeToAst(nodeStmts, stmts)->declarations = nodeToAst(declarations->node_syn, declarations);
                nodeToAst(nodeStmts, stmts)->otherStmts = nodeToAst(other_stmts->node_syn, otherStmts);
                nodeToAst(nodeStmts, stmts)->returnIds = nodeToAst(return_stmt->node_syn, idList); 
                stmts->node_syn = nodeStmts;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeStmts,stmts)->lineNo = stmts->lineNo;
                break;
            }

            case TYPE_DEFINITIONS:
            {
                ParseTreeElement *type_def = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *actual_or_redef = &ptElement->children[0];
                    ParseTreeElement *type_def1 = &ptElement->children[1];
                    actual_or_redef->node_inh = type_def->node_inh;
                    handleParseTreeElement(actual_or_redef);
                    type_def1->node_inh = actual_or_redef->node_syn;
                    handleParseTreeElement(type_def1);
                    type_def->node_syn = type_def1->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    type_def->node_syn = type_def->node_inh;
                }
                else{
                    maybePrintf("Error Rule doesnt match!!!\n");
                }
                break;
            }

            case TYPE_DEFINITION:
            {
                ParseTreeElement *type_def = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *field_def = &ptElement->children[2];
                    TokenInfo tkRuid = type_def->children[1].tinfo;

                    handleParseTreeElement(field_def);

                    declareAstNode(nodeTypeDefinition, AST_TYPEDEFINITION, Ast_TypeDefinition, typeDefinition);
                    nodeToAst(nodeTypeDefinition, typeDefinition)->type = TDEF;
                    
                    Ast_TypeDefinition* typeDef = nodeToAst(nodeTypeDefinition, typeDefinition);
                    tdef(typeDef) = malloc(sizeof(TypeDef));
                    tdef(typeDef)->fieldDefinitions = 
                        nodeToAst(field_def->node_syn, fieldDefinitions);
                    tdef(nodeToAst(nodeTypeDefinition, typeDefinition))->id = tkRuid.lexeme;
                    tdef(nodeToAst(nodeTypeDefinition, typeDefinition))->datatype = DT_RECORD;

                    // insertTo(nodeToAst(type_def->node_inh, typeDefinitions)->typeDefintionList,
                    //         nodeTypeDefinition);

                    type_def->node_syn = nodeTypeDefinition;

                    insertTypeDef(tkRuid.lexeme, DT_RECORD, nodeToAst(field_def->node_syn, fieldDefinitions));
                   // ptElement->lineNo = ptElement->children[0].lineNo;
                    nodeToAst(nodeTypeDefinition,typeDefinition)->lineNo = type_def->lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *field_def = &ptElement->children[2];
                    TokenInfo tkRuid = type_def->children[1].tinfo;

                    handleParseTreeElement(field_def);

                    declareAstNode(nodeTypeDefinition, AST_TYPEDEFINITION, Ast_TypeDefinition, typeDefinition);
                    nodeToAst(nodeTypeDefinition, typeDefinition)->type = TDEF;
                    Ast_TypeDefinition* typeDef = nodeToAst(nodeTypeDefinition, typeDefinition);
                    tdef(typeDef) = malloc(sizeof(TypeDef));
                    tdef(nodeToAst(nodeTypeDefinition, typeDefinition))->fieldDefinitions = nodeToAst(field_def->node_syn, fieldDefinitions);
                    tdef(nodeToAst(nodeTypeDefinition, typeDefinition))->id = tkRuid.lexeme;
                    tdef(nodeToAst(nodeTypeDefinition, typeDefinition))->datatype = DT_UNION;

                    // insertTo(nodeToAst(type_def->node_inh, typeDefinitions)->typeDefintionList,
                    //         nodeTypeDefinition);

                    type_def->node_syn = nodeTypeDefinition;
                    insertTypeDef(tkRuid.lexeme, DT_UNION, nodeToAst(field_def->node_syn, fieldDefinitions));
                   // ptElement->lineNo = ptElement->children[0].lineNo;
                    nodeToAst(nodeTypeDefinition,typeDefinition)->lineNo = type_def->lineNo;
                }
                break;
            }

            case FIELD_DEFINITIONS:
            {
                ParseTreeElement *field_defs = ptElement;
                ParseTreeElement *field_def1 = &ptElement->children[0];
                ParseTreeElement *field_def2 = &ptElement->children[1];
                ParseTreeElement *more_fields = &ptElement->children[2];
                
                declareAstNode(nodeFieldDefinitions, AST_FIELDDEFINITIONS, Ast_FieldDefinitions, fieldDefinitions);
                nodeToAst(nodeFieldDefinitions, fieldDefinitions)->fieldDefinitionList = createAstList();

                field_def1->node_inh = nodeFieldDefinitions;
                handleParseTreeElement(field_def1);

                field_def2->node_inh = field_def1->node_syn;
                handleParseTreeElement(field_def2);

                // insertTo(nodeToAst(nodeFieldDefinitions, fieldDefinitions)->fieldDefinitionList,
                //         field_def1->node_syn);
                // insertTo(nodeToAst(nodeFieldDefinitions, fieldDefinitions)->fieldDefinitionList,
                //         field_def2->node_syn);

                more_fields->node_inh = field_def2->node_syn;
                handleParseTreeElement(more_fields);
                field_defs->node_syn = more_fields->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeFieldDefinitions,fieldDefinitions)->lineNo = field_defs->lineNo;
                break;
            }

            case FIELD_DEFINITION:
            {
                ParseTreeElement *field_def = ptElement;
                ParseTreeElement *field_type = &ptElement->children[1];
                handleParseTreeElement(field_type);
                TokenInfo tkFieldid = ptElement->children[3].tinfo;

                declareAstNode(nodeFieldDefinition, AST_FIELDDEFINITION, Ast_FieldDefinition, fieldDefinition);
                nodeToAst(nodeFieldDefinition, fieldDefinition)->fieldType = nodeToAst(field_type->node_syn, datatype);
                maybePrintf("%s is of type %s\n", tkFieldid.lexeme, dtypeToStr(nodeToAst(field_type->node_syn, datatype)->datatype));
                nodeToAst(nodeFieldDefinition, fieldDefinition)->id = tkFieldid.lexeme;
            
                insertTo(nodeToAst(field_def->node_inh, fieldDefinitions)->fieldDefinitionList, nodeFieldDefinition);
                field_def->node_syn = field_def->node_inh;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeFieldDefinition,fieldDefinition)->lineNo = field_def->lineNo;
                break;
            }
            case MORE_FIELDS:
            {
                ParseTreeElement *more_fields = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *field_def = &ptElement->children[0];
                    ParseTreeElement *more_fields1 = &ptElement->children[1];

                    field_def->node_inh = ptElement->node_inh;
                    handleParseTreeElement(field_def);

                    more_fields1->node_inh = field_def->node_syn;
                    handleParseTreeElement(more_fields1);

                    more_fields->node_syn = more_fields1->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    more_fields->node_syn = ptElement->node_inh;
                }
                break;
            }
            case DECLARATIONS:
            {
                ParseTreeElement *declarations = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *declaration = &ptElement->children[0];
                    ParseTreeElement *declarations1 = &ptElement->children[1];

                    declaration->node_inh = ptElement->node_inh;
                    handleParseTreeElement(declaration);

                    declarations1->node_inh = declaration->node_syn;
                    handleParseTreeElement(declarations1);

                    declarations->node_syn = declarations1->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    declarations->node_syn = ptElement->node_inh;
                }
                break;
            }
            case DECLARATION:
            {
                ParseTreeElement *declaration = ptElement;
                ParseTreeElement *datatype = &ptElement->children[1];
                ParseTreeElement *global_or_not = &ptElement->children[4];
                TokenInfo tkId = ptElement->children[3].tinfo;
                //maybePrintf("Declaration id is %s \n", tkId.lexeme);
                handleParseTreeElement(datatype);
                
                handleParseTreeElement(global_or_not);
                declareAstNode(nodeDeclaration, AST_DECLARATION, Ast_Declaration, declaration);
                // DATATYPE: nodeToAst(nodeDeclaration,declaration)->type = nodeToAst(type->node_syn,datatype);

                nodeToAst(nodeDeclaration, declaration)->id = tkId.lexeme;

                Ast_Datatype* dtype = nodeToAst(datatype->node_syn, datatype);
                nodeToAst(nodeDeclaration, declaration)->datatype = dtype;
                
                int isGlobal = nodeToAst(global_or_not->node_syn, isGlobalOrNot)->isGlobal;
                nodeToAst(nodeDeclaration, declaration)->isGlobal = isGlobal;

                insertTo(nodeToAst(declaration->node_inh, declarations)->declarationList, nodeDeclaration);
                declaration->node_syn = declaration->node_inh;

                if(isGlobal){
                    if(findGlobalVar(tkId.lexeme) != NULL){
                        maybePrintf("Error! Redeclaration of global varaible %s\n", tkId.lexeme);
                    }
                    else
                        insertGlobVar(tkId.lexeme, dtype->datatype, dtype->name);
                }
                else{
                    if(findVar(tkId.lexeme) != NULL){
                        maybePrintf("Error! Redeclaration of local varaible %s in %s\n", tkId.lexeme, currentSymbolTable()->name);
                    }
                    insertVar(tkId.lexeme, NOT_PAR, dtype->datatype, dtype->name);
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeDeclaration,declaration)->lineNo = declaration->lineNo;
                break;
            }
            case GLOBAL_OR_NOT:
            {

                ParseTreeElement *global_or_not = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    declareAstNode(nodeIsG, AST_ISGLOBALORNOT, Ast_IsGlobalOrNot,
                        isGlobalOrNot);
                    nodeToAst(nodeIsG, isGlobalOrNot)->isGlobal = 1;

                    global_or_not->node_syn = nodeIsG;
                   // ptElement->lineNo = ptElement->children[0].lineNo;
                    nodeToAst(nodeIsG,isGlobalOrNot)->lineNo = global_or_not->lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeIsG, AST_ISGLOBALORNOT, Ast_IsGlobalOrNot,
                        isGlobalOrNot);
                    nodeToAst(nodeIsG, isGlobalOrNot)->isGlobal = 0;

                    global_or_not->node_syn = nodeIsG;
                    nodeToAst(nodeIsG,isGlobalOrNot)->lineNo = global_or_not->lineNo;
                }
                break;
            }
            case OTHER_STMTS:
            {
                ParseTreeElement *other_stmts = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *stmt = &ptElement->children[0];
                    ParseTreeElement *other_stmts1 = &ptElement->children[1];

                    stmt->node_inh = ptElement->node_inh;
                    handleParseTreeElement(stmt);

                    other_stmts1->node_inh = stmt->node_syn;
                    handleParseTreeElement(other_stmts1);

                    other_stmts->node_syn = other_stmts1->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    other_stmts->node_syn = ptElement->node_inh;
                }
                break;
            }
            case STMT:
            {
                ParseTreeElement *stmt = ptElement;
                declareAstNode(nodeStmt, AST_STMT, Ast_Stmt, stmt);
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *assignment_stmt = &ptElement->children[0];
                    handleParseTreeElement(assignment_stmt);

                    
                    nodeToAst(nodeStmt, stmt)->type = STMT_ASSIGN;
                    assignStmt(nodeToAst(nodeStmt, stmt)) = nodeToAst(assignment_stmt->node_syn, assignmentStmt);
                    
                    insertTo(nodeToAst(stmt->node_inh, otherStmts)->otherStmtList, nodeStmt);
                    stmt->node_syn = stmt->node_inh;
                }
                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *iterative_stmt = &ptElement->children[0];
                    handleParseTreeElement(iterative_stmt);

                    
                    nodeToAst(nodeStmt, stmt)->type = STMT_ITER;
                    iterStmt(nodeToAst(nodeStmt, stmt)) = nodeToAst(iterative_stmt->node_syn, iterativeStmt);

                    insertTo(nodeToAst(stmt->node_inh, otherStmts)->otherStmtList, nodeStmt);
                    stmt->node_syn = stmt->node_inh;
                }
                else if (ptElement->ruleNo == 2)
                {
                    ParseTreeElement *conditional_stmt = &ptElement->children[0];
                    handleParseTreeElement(conditional_stmt);
                    
                    nodeToAst(nodeStmt, stmt)->type = STMT_COND;
                    condStmt(nodeToAst(nodeStmt, stmt)) = nodeToAst(conditional_stmt->node_syn, conditionalStmt);

                    insertTo(nodeToAst(stmt->node_inh, otherStmts)->otherStmtList, nodeStmt);
                    stmt->node_syn = stmt->node_inh;
                }

                else if (ptElement->ruleNo == 3)
                {
                    ParseTreeElement *io_stmt = &ptElement->children[0];
                    handleParseTreeElement(io_stmt);
                    
                    nodeToAst(nodeStmt, stmt)->type = STMT_IO;
                    ipopStmt(nodeToAst(nodeStmt, stmt)) = nodeToAst(io_stmt->node_syn, ioStmt);

                    insertTo(nodeToAst(stmt->node_inh, otherStmts)->otherStmtList, nodeStmt);
                    stmt->node_syn = stmt->node_inh;
                }
                else if (ptElement->ruleNo == 4)
                {
                    ParseTreeElement *fun_call_stmt = &ptElement->children[0];
                    handleParseTreeElement(fun_call_stmt);
                    
                    nodeToAst(nodeStmt, stmt)->type = STMT_FUN_CALL;
                    fCallStmt(nodeToAst(nodeStmt, stmt)) = nodeToAst(fun_call_stmt->node_syn, funCallStmt);

                    insertTo(nodeToAst(stmt->node_inh, otherStmts)->otherStmtList, nodeStmt);
                    stmt->node_syn = stmt->node_inh;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeStmt,stmt)->lineNo = stmt->lineNo;

                break;
            }
            case ASSIGNMENT_STMT:
            {
                ParseTreeElement *assignment_stmt = ptElement;
                ParseTreeElement *single_or_rec_id = &ptElement->children[0];
                ParseTreeElement *arithmetic_expression = &ptElement->children[2];
                handleParseTreeElement(single_or_rec_id);
                handleParseTreeElement(arithmetic_expression);
                declareAstNode(nodeAssignmentStmt, AST_ASSIGNMENTSTMT, Ast_AssignmentStmt, assignmentStmt);

                nodeToAst(nodeAssignmentStmt, assignmentStmt)->singleOrRecId = 
                    nodeToAst(single_or_rec_id->node_syn, singleOrRecId);
                nodeToAst(nodeAssignmentStmt, assignmentStmt)->expression = 
                    nodeToAst(arithmetic_expression->node_syn, arithmeticExpression);

                assignment_stmt->node_syn = nodeAssignmentStmt;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeAssignmentStmt,assignmentStmt)->lineNo = assignment_stmt->lineNo;

                break;
            }
            case SINGLE_OR_REC_ID:
            {
                ParseTreeElement *single_or_rec_id = ptElement;
                ParseTreeElement *option_single_constructed = &ptElement->children[1];
                TokenInfo tkId = ptElement->children[0].tinfo;

                handleParseTreeElement(option_single_constructed);
                // TK_ID
                declareAstNode(nodeSingleOrRecId, AST_SINGLEORRECID, Ast_SingleOrRecId, singleOrRecId);
                nodeToAst(nodeSingleOrRecId, singleOrRecId)->id = tkId.lexeme;
                nodeToAst(nodeSingleOrRecId, singleOrRecId)->fieldNameList = nodeToAst(option_single_constructed->node_syn, 
                    moreExp)->oneExpansionList;

                single_or_rec_id->node_syn = nodeSingleOrRecId;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeSingleOrRecId,singleOrRecId)->lineNo = single_or_rec_id->lineNo;
                break;
            }
            case FUN_CALL_STMT:
            {
                ParseTreeElement *fun_call_stmt = ptElement;
                ParseTreeElement *output_parameters = &ptElement->children[0];
                ParseTreeElement *input_parameters = &ptElement->children[5];
                handleParseTreeElement(output_parameters);
                handleParseTreeElement(input_parameters);

                TokenInfo tkFunid = ptElement->children[2].tinfo;
                declareAstNode(nodeFunCallStmt, AST_FUNCALLSTMT, Ast_FunCallStmt, funCallStmt);
                nodeToAst(nodeFunCallStmt, funCallStmt)->outputIdList = nodeToAst(output_parameters->node_syn, idList);
                nodeToAst(nodeFunCallStmt, funCallStmt)->inputIdList = nodeToAst(input_parameters->node_syn, idList);
                nodeToAst(nodeFunCallStmt, funCallStmt)->funId = tkFunid.lexeme;
                fun_call_stmt->node_syn = nodeFunCallStmt;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeFunCallStmt,funCallStmt)->lineNo = fun_call_stmt->lineNo;
                break;
            }
            case OUTPUT_PARAMETERS:
            {
                ParseTreeElement *output_parameters = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *id_list = &ptElement->children[1];

                    declareAstNode(nodeIdList, AST_IDLIST, Ast_IdList, idList);
                    nodeToAst(nodeIdList, idList)->idList = createAstList();
                    id_list->node_inh = nodeIdList;

                    handleParseTreeElement(id_list);

                    id_list->node_inh = nodeIdList;
                    output_parameters->node_syn = id_list->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;
                    

                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeIdList, AST_IDLIST, Ast_IdList, idList);
                    nodeToAst(nodeIdList, idList)->idList = createAstList();
                    output_parameters->node_syn = nodeIdList;
                }
                break;
            }

            case INPUT_PARAMETERS:
            {
                ParseTreeElement *input_parameters = ptElement;
                ParseTreeElement *id_list = &ptElement->children[1];

                declareAstNode(nodeIdList, AST_IDLIST, Ast_IdList, idList);
                nodeToAst(nodeIdList, idList)->idList = createAstList();
                id_list->node_inh = nodeIdList;
                handleParseTreeElement(id_list);
                input_parameters->node_syn = id_list->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                
                break;
            }
            case ITERATIVE_STMT:
            {
                ParseTreeElement *iterative_stmt = ptElement;
                ParseTreeElement *boolean_expression = &ptElement->children[2];
                ParseTreeElement *stmt = &ptElement->children[4];
                ParseTreeElement *other_stmts = &ptElement->children[5];
                handleParseTreeElement(boolean_expression);
                declareAstNode(nodeOtherStmts, AST_OTHERSTMTS, Ast_OtherStmts, otherStmts);
                nodeToAst(nodeOtherStmts, otherStmts)->otherStmtList = createAstList();
                stmt->node_inh = nodeOtherStmts;
                handleParseTreeElement(stmt);
                other_stmts->node_inh = stmt->node_syn;
                handleParseTreeElement(other_stmts);

                declareAstNode(nodeIterativeStmt, AST_ITERATIVESTMT, Ast_IterativeStmt, iterativeStmt);

                nodeToAst(nodeIterativeStmt, iterativeStmt)->condition = 
                    nodeToAst(boolean_expression->node_syn, booleanExpression);

                nodeToAst(nodeIterativeStmt, iterativeStmt)->body = nodeToAst(nodeOtherStmts, otherStmts);

                iterative_stmt->node_syn = nodeIterativeStmt;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeIterativeStmt,iterativeStmt)->lineNo = iterative_stmt->lineNo;

                break;
            }

            case CONDITIONAL_STMT:
            {
                ParseTreeElement *condStmt = ptElement;
                ParseTreeElement *boolExp = &ptElement->children[2];
                ParseTreeElement *stmt = &ptElement->children[4];
                ParseTreeElement *other_stmts = &ptElement->children[5];
                ParseTreeElement *else_part = &ptElement->children[6];
                handleParseTreeElement(boolExp);
                declareAstNode(nodeOtherStmts, AST_OTHERSTMTS, Ast_OtherStmts, otherStmts);
                nodeToAst(nodeOtherStmts, otherStmts)->otherStmtList = createAstList();
                stmt->node_inh = nodeOtherStmts;
                handleParseTreeElement(stmt);
                other_stmts->node_inh = stmt->node_syn;
                handleParseTreeElement(other_stmts);
                handleParseTreeElement(else_part);
                declareAstNode(nodeConditionalStmt, AST_CONDITIONALSTMT,
                            Ast_ConditionalStmt, conditionalStmt);

                nodeToAst(nodeConditionalStmt, conditionalStmt)->body = nodeToAst(nodeOtherStmts, otherStmts);
                nodeToAst(nodeConditionalStmt, conditionalStmt)->condition = nodeToAst(boolExp->node_syn, booleanExpression);

                nodeToAst(nodeConditionalStmt, conditionalStmt)->elsePart = nodeToAst(else_part->node_syn, elsePart)->otherStmts;

                condStmt->node_syn = nodeConditionalStmt;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeConditionalStmt,conditionalStmt)->lineNo = condStmt->lineNo;
                break;
            }
            
            case IO_STMT:
            {
                
                ParseTreeElement* ioStmt = ptElement;
                ParseTreeElement* var = &ptElement->children[2];
                handleParseTreeElement(var);
                declareAstNode(nodeIo, AST_IOSTMT, Ast_IoStmt,
                        ioStmt);

                if(ioStmt->ruleNo == 0){
                    
                    nodeToAst(nodeIo, ioStmt)->ioType = IO_READ;
                    nodeToAst(nodeIo, ioStmt)->var = nodeToAst(var->node_syn, var);
                    ioStmt->node_syn = nodeIo;
                }
                else if(ioStmt->ruleNo == 1){
                    nodeToAst(nodeIo, ioStmt)->ioType = IO_WRITE;
                    nodeToAst(nodeIo, ioStmt)->var = nodeToAst(var->node_syn, var);
                    ioStmt->node_syn = nodeIo;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeIo,ioStmt)->lineNo = ioStmt->lineNo;

                break;
            }
            case BOOLEAN_EXPRESSION:
            {
                ParseTreeElement *boolean_expression = ptElement;
                declareAstNode(nodeBooleanExpression, AST_BOOLEXP, Ast_BooleanExpression, booleanExpression);
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *tk_op1 = &ptElement->children[0];
                    ParseTreeElement *boolean_expression1 = &ptElement->children[1];
                    ParseTreeElement *tk_cl1 = &ptElement->children[2];
                    ParseTreeElement *logical_op = &ptElement->children[3];
                    ParseTreeElement *tk_op2 = &ptElement->children[4];
                    ParseTreeElement *boolean_expression2 = &ptElement->children[5];
                    ParseTreeElement *tk_cl2 = &ptElement->children[6];
                    handleParseTreeElement(boolean_expression1);
                    handleParseTreeElement(boolean_expression2);
                    handleParseTreeElement(logical_op);
                    

                    nodeBooleanExpression->node.booleanExpression->bexpType = BEXP_BOOL_OP;
                    nodeBooleanExpression->node.booleanExpression->bexp.boolOp = (BoolOperation *)malloc(sizeof(BoolOperation));

                    boolOpBexp(nodeToAst(nodeBooleanExpression, booleanExpression))->op =
                        nodeToAst(logical_op->node_syn, logicalOp)->op;

                    Ast_BooleanExpression* left = nodeToAst(boolean_expression1->node_syn, booleanExpression);
                    Ast_BooleanExpression* right = nodeToAst(boolean_expression2->node_syn, booleanExpression);

                    boolOpBexp(nodeToAst(nodeBooleanExpression, booleanExpression))->left = 
                            left;
                    boolOpBexp(nodeToAst(nodeBooleanExpression, booleanExpression))->right = 
                            right;
                    
                    // if(left->bexpType == BEXP_BOOL_OP){
                        
                    // }
                    // else if(left->bexpType == BEXP_VAR_COMP){
                    //     boolOpBexp(nodeToAst(nodeBooleanExpression, booleanExpression))->left =
                    //         left;
                    // }

                    // if(right->bexpType == BEXP_BOOL_OP){
                        
                    // }
                    // else if(right->bexpType == BEXP_VAR_COMP){
                    //     boolOpBexp(nodeToAst(nodeBooleanExpression, booleanExpression))->right =
                    //         varCompBexp(right);
                    // }
                    
                    ptElement->node_syn = nodeBooleanExpression;
                    
                }
                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *var1 = &ptElement->children[0];
                    ParseTreeElement *relop = &ptElement->children[1];
                    ParseTreeElement *var2 = &ptElement->children[2];
                    handleParseTreeElement(var1);
                    handleParseTreeElement(var2);
                    handleParseTreeElement(relop);
                    

                    nodeBooleanExpression->node.booleanExpression->bexpType = BEXP_VAR_COMP;
                    nodeBooleanExpression->node.booleanExpression->bexp.varComp = (VarComparison *)malloc(sizeof(VarComparison));
                    varCompBexp(nodeToAst(nodeBooleanExpression, booleanExpression))->op =
                        nodeToAst(relop->node_syn, relationalOp)->op;
                    nodeBooleanExpression->node.booleanExpression->bexp.varComp->left = nodeToAst(var1->node_syn, var);
                    nodeBooleanExpression->node.booleanExpression->bexp.varComp->right = nodeToAst(var2->node_syn, var);

                    ptElement->node_syn = nodeBooleanExpression;
                }
                else if (ptElement->ruleNo == 2)
                {
                    ParseTreeElement *tk_not = &ptElement->children[0];
                    ParseTreeElement *boolean = &ptElement->children[2];
                    handleParseTreeElement(boolean);
                    handleParseTreeElement(tk_not);
                    

                    nodeBooleanExpression->node.booleanExpression->bexpType = BEXP_BOOL_OP;
                    nodeBooleanExpression->node.booleanExpression->bexp.boolOp = (BoolOperation *)malloc(sizeof(BoolOperation));
                    nodeBooleanExpression->node.booleanExpression->bexp.boolOp->op = LOP_NOT;
                    nodeBooleanExpression->node.booleanExpression->bexp.boolOp->left = nodeToAst(boolean->node_syn, booleanExpression);
                    nodeBooleanExpression->node.booleanExpression->bexp.boolOp->right = NULL;

                    ptElement->node_syn = nodeBooleanExpression;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeBooleanExpression,booleanExpression)->lineNo = boolean_expression->lineNo;

                break;
            }

            case VAR:
            {
                ParseTreeElement *var = ptElement;
                declareAstNode(nodeVar, AST_VAR, Ast_Var, var);
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *singleorrecId = &ptElement->children[0];
                    handleParseTreeElement(singleorrecId);

                    
                    nodeToAst(nodeVar, var)->varType = VAR_ID;
                    varVar(nodeToAst(nodeVar, var)) = 
                        nodeToAst(singleorrecId->node_syn, singleOrRecId);  

                    var->node_syn = nodeVar;
                }
                else if (ptElement->ruleNo == 1)
                {
                    TokenInfo tkNum = ptElement->children[0].tinfo;
                    declareAstNode(nodeNum, AST_NUM, Ast_Num, num);
                    //nodeToAst(nodeNum, num)->typeExpr = numTypeExpression();
                    nodeToAst(nodeNum, num)->val = strToInt(tkNum.lexeme);

                    
                    nodeToAst(nodeVar, var)->varType = VAR_NUM;
                    numVar(nodeToAst(nodeVar, var)) = nodeToAst(nodeNum, num);
                    
                    var->node_syn = nodeVar;
                }
                else if (ptElement->ruleNo == 2)
                {
                    TokenInfo tkRnum = ptElement->children[0].tinfo;
                    declareAstNode(nodeRnum, AST_RNUM, Ast_Rnum, rnum);
                    //nodeToAst(nodeRnum, rnum)->typeExpr = rnumTypeExpression();
                    nodeToAst(nodeRnum, rnum)->val = strToReal(tkRnum.lexeme);

                    
                    nodeToAst(nodeVar, var)->varType = VAR_RNUM;
                    rnumVar(nodeToAst(nodeVar, var)) = nodeToAst(nodeRnum, rnum);

                    var->node_syn = nodeVar;
                }
                else{
                    maybePrintf("BLahh");
                    exit(0);
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeVar,var)->lineNo = var->lineNo;
                break;
            }

            case LOGICAL_OP:
            {
                ParseTreeElement *localOp = ptElement;
                declareAstNode(nodeLogOp, AST_LOGICALOP, Ast_LogicalOperator, logicalOp);
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *tkand = &ptElement->children[0];

                    
                    nodeToAst(nodeLogOp, logicalOp)->op = LOP_AND;

                    localOp->node_syn = nodeLogOp;
                }
                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *tkor = &ptElement->children[0];

                    
                    nodeToAst(nodeLogOp, logicalOp)->op = LOP_OR;

                    localOp->node_syn = nodeLogOp;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeLogOp,logicalOp)->lineNo = localOp->lineNo;
                break;
            }

            case RELATIONAL_OP:
            {
                ParseTreeElement *relationalOP = ptElement;
                declareAstNode(nodeROp, AST_RELATIONALOP, Ast_RelationalOperator, relationalOp);
                if (ptElement->ruleNo == 0)
                {
                    TokenInfo tkLt = ptElement->children[0].tinfo;
                    
                    nodeToAst(nodeROp, relationalOp)->op = ROP_Lt;
                    relationalOP->node_syn = nodeROp;
                }
                else if (ptElement->ruleNo == 1)
                {
                    TokenInfo tkLe = ptElement->children[0].tinfo;
                    
                    nodeToAst(nodeROp, relationalOp)->op = ROP_Le;
                    relationalOP->node_syn = nodeROp;
                }
                else if (ptElement->ruleNo == 2)
                {
                    TokenInfo tkEq = ptElement->children[0].tinfo;
                    
                    nodeToAst(nodeROp, relationalOp)->op = ROP_Eq;
                    relationalOP->node_syn = nodeROp;
                }
                else if (ptElement->ruleNo == 3)
                {
                    TokenInfo tkGt = ptElement->children[0].tinfo;
                    
                    nodeToAst(nodeROp, relationalOp)->op = ROP_Gt;
                    relationalOP->node_syn = nodeROp;
                }
                else if (ptElement->ruleNo == 4)
                {
                    TokenInfo tkGe = ptElement->children[0].tinfo;
                    
                    nodeToAst(nodeROp, relationalOp)->op = ROP_Ge;
                    relationalOP->node_syn = nodeROp;
                }
                else if (ptElement->ruleNo == 5)
                {
                    TokenInfo tkNe = ptElement->children[0].tinfo;
                    
                    nodeToAst(nodeROp, relationalOp)->op = ROP_Ne;
                    relationalOP->node_syn = nodeROp;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeROp,relationalOp)->lineNo = relationalOP->lineNo;
                break;
            }

            case RETURN_STMT:
            {
                ParseTreeElement *returnStmt = ptElement;
                ParseTreeElement *optioanlReturn = &ptElement->children[1];
                handleParseTreeElement(optioanlReturn);
                returnStmt->node_syn = optioanlReturn->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case OPTIONAL_RETURN:
            {
                ParseTreeElement *optionalReturn = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *idList = &ptElement->children[1];
                    declareAstNode(nodeidList, AST_IDLIST, Ast_IdList, idList);
                    nodeToAst(nodeidList, idList)->idList = createAstList();
                    idList->node_inh = nodeidList;
                    handleParseTreeElement(idList);
                    optionalReturn->node_syn = idList->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeidList, AST_IDLIST, Ast_IdList, idList);
                    nodeToAst(nodeidList, idList)->idList = createAstList();
                    optionalReturn->node_syn = nodeidList;
                }
                break;
            }

            case DEFINE_TYPE_STATEMENT:
            {
                ParseTreeElement* dtypeStmt = ptElement;
                ParseTreeElement* a = &ptElement->children[1];

                TokenInfo tkruid = dtypeStmt->children[2].tinfo;
                TokenInfo tkruid1 = dtypeStmt->children[4].tinfo;

                handleParseTreeElement(a);
                declareAstNode(nodeTrdef, AST_TYPEDEFINITION, Ast_TypeDefinition, typeDefinition);
                nodeToAst(nodeTrdef, typeDefinition)->type = TRDEF;
                trdef(nodeToAst(nodeTrdef, typeDefinition)) = malloc(sizeof(TypeRdef));
                trdef(nodeToAst(nodeTrdef, typeDefinition))->from = tkruid1.lexeme;
                trdef(nodeToAst(nodeTrdef, typeDefinition))->to = tkruid.lexeme;
                
                dtypeStmt->node_syn = nodeTrdef;
                
                insertTypeRedef(tkruid1.lexeme, tkruid.lexeme);
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeTrdef,typeDefinition)->lineNo = dtypeStmt->lineNo;
                break;
            }
            case ID_LIST:
            {
                ParseTreeElement *idList = ptElement;
                TokenInfo tkId = ptElement->children[0].tinfo;
                ParseTreeElement *moreIds = &ptElement->children[1];

                declareAstNode(nodeId, AST_ID, Ast_Id, id);
                nodeToAst(nodeId, id)->id = tkId.lexeme;
                insertTo(nodeToAst(idList->node_inh, idList)->idList, nodeId);
                moreIds->node_inh = idList->node_inh;
                handleParseTreeElement(moreIds);
                idList->node_syn = moreIds->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;
                nodeToAst(nodeId,id)->lineNo = idList->lineNo;
                break;
            }

            case MORE_IDS:
            {
                ParseTreeElement *moreIds = ptElement;

                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *idList = &ptElement->children[1];
                    idList->node_inh = moreIds->node_inh;
                    handleParseTreeElement(idList);
                    moreIds->node_syn = idList->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }

                else if (ptElement->ruleNo == 1)
                {
                    moreIds->node_syn = moreIds->node_inh;
                }
                break;
            }

            case ACTUAL_OR_REDEFINED:
            {
                ParseTreeElement *actualOrredefined = ptElement;

                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *typeDefinition = &ptElement->children[0];
                    typeDefinition->node_inh = actualOrredefined->node_inh;
                    handleParseTreeElement(typeDefinition);
                    insertTo(nodeToAst(actualOrredefined->node_inh, typeDefinitions)->typeDefintionList, typeDefinition->node_syn);
                    ptElement->node_syn = ptElement->node_inh;
                }

                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *defineTypeStatement = &ptElement->children[0];
                    handleParseTreeElement(defineTypeStatement);
                    insertTo(nodeToAst(actualOrredefined->node_inh, typeDefinitions)->typeDefintionList, defineTypeStatement->node_syn);
                    ptElement->node_syn = ptElement->node_inh;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case FIELD_TYPE:
            {
                ParseTreeElement *fieldType = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *primitiveDatatype = &ptElement->children[0];
                    handleParseTreeElement(primitiveDatatype);
                    fieldType->node_syn = primitiveDatatype->node_syn;
                }
                else if (ptElement->ruleNo == 1)
                {
                    TokenInfo tkRuId = ptElement->children[0].tinfo;
                    declareAstNode(nodeRuid, AST_DATATYPE, Ast_Datatype, datatype);
                    nodeToAst(nodeRuid, datatype)->datatype = DT_REC_OR_UNION;
                    nodeToAst(nodeRuid, datatype)->name = tkRuId.lexeme;
                    fieldType->node_syn = nodeRuid;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case OPTION_SINGLE_CONSTRUCTED:
            {
                ParseTreeElement *optionSingleConstructed = ptElement;

                if(optionSingleConstructed->ruleNo == 0){
                    ParseTreeElement* oneExp = &ptElement->children[0];
                    ParseTreeElement* moreExp = &ptElement->children[1];
                    declareAstNode(nodeMoreExp, AST_MOREEXPANSION, Ast_MoreExpansion,
                        moreExp);
                    nodeToAst(nodeMoreExp, moreExp)->oneExpansionList = createAstList();
                    oneExp->node_inh = nodeMoreExp;

                    handleParseTreeElement(oneExp);

                    moreExp->node_inh = oneExp->node_syn;
                    handleParseTreeElement(moreExp);

                    optionSingleConstructed->node_syn = moreExp->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if(optionSingleConstructed->ruleNo == 1){
                    declareAstNode(nodeMoreExp, AST_MOREEXPANSION, Ast_MoreExpansion,
                        moreExp);
                    nodeToAst(nodeMoreExp, moreExp)->oneExpansionList = createAstList();
                    optionSingleConstructed->node_syn = nodeMoreExp;
                }
                break;
            }

            case ONE_EXPANSION:
            {
                ParseTreeElement *oneExpansion = ptElement;
                // ONE_EXPANSION INSERTO
                TokenInfo tkFieldId = ptElement->children[1].tinfo;
                declareAstNode(nodeId, AST_ID, Ast_Id, id);
                nodeToAst(nodeId, id)->id = tkFieldId.lexeme;
                insertTo(nodeToAst(oneExpansion->node_inh, moreExp)->oneExpansionList, nodeId);
                oneExpansion->node_syn = oneExpansion->node_inh;
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case MORE_EXPANSIONS:
            {
                ParseTreeElement *moreExpansions = ptElement;

                if (moreExpansions->ruleNo == 0)
                {
                    ParseTreeElement *oneExpansion = &ptElement->children[0];
                    ParseTreeElement *moreExpansion1 = &ptElement->children[1];
                    oneExpansion->node_inh = moreExpansions->node_inh;
                    handleParseTreeElement(oneExpansion);
                    moreExpansion1->node_inh = oneExpansion->node_syn;
                    handleParseTreeElement(moreExpansion1);
                    moreExpansions->node_syn = moreExpansion1->node_syn;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }

                else if (moreExpansions->ruleNo == 1)
                {
                    moreExpansions->node_syn = moreExpansions->node_inh;
                }
                break;
            }

            case EXP_PRIME:
            {
                ParseTreeElement *expPrime = ptElement;
                if(expPrime->ruleNo == 0){
                    ParseTreeElement *lowPrOp = &expPrime->children[0];
                    ParseTreeElement *term = &expPrime->children[1];
                    ParseTreeElement* expPrime1 = &ptElement->children[2];

                    declareAstNode(nodeArithExpr, AST_ARITHMETICEXPR, Ast_ArithmeticExpression,
                                arithmeticExpression);
                    nodeToAst(nodeArithExpr, arithmeticExpression)->aexpType = AEXP_EXP;
                    handleParseTreeElement(lowPrOp);
                    expAexp(nodeToAst(nodeArithExpr, arithmeticExpression)) = malloc(sizeof(ArithmeticExpression));
                    expAexp(nodeToAst(nodeArithExpr, arithmeticExpression))->op =
                        nodeToAst(lowPrOp->node_syn, arithmeticOp)->op;

                    handleParseTreeElement(term);

                    Ast_ArithmeticExpression* left = nodeToAst(expPrime->node_inh, arithmeticExpression);
                    Ast_ArithmeticExpression* right = nodeToAst(term->node_syn, arithmeticExpression);
                    Ast_ArithmeticExpression* aexp = nodeToAst(nodeArithExpr, arithmeticExpression);

                    expAexp(aexp)->left = left;
                    expAexp(aexp)->right = right;

                    expPrime1->node_inh = nodeArithExpr;
                    handleParseTreeElement(expPrime1);
                    expPrime->node_syn = expPrime1->node_syn;
                    nodeToAst(nodeArithExpr, arithmeticExpression)->lineNo = expPrime->lineNo;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }

                else if(expPrime->ruleNo == 1){
                    expPrime->node_syn = expPrime->node_inh;
                }

                break;
            }

            case A:
            {
                ParseTreeElement *a = ptElement;
    
                if (ptElement->ruleNo == 0)
                {
                    declareAstNode(nodeDatatype, AST_DATATYPE, Ast_Datatype, datatype);
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_RECORD;
                    nodeToAst(nodeDatatype, datatype)->name = NULL;
                    a->node_syn = nodeDatatype;
                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeDatatype, AST_DATATYPE, Ast_Datatype, datatype);
                    nodeToAst(nodeDatatype, datatype)->datatype = DT_UNION;
                    nodeToAst(nodeDatatype, datatype)->name = NULL;
                    a->node_syn = nodeDatatype;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case ELSE_PART:
            {
                ParseTreeElement *else_part = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement* stmt = &ptElement->children[1];
                    ParseTreeElement* othStmt = &ptElement->children[2];
                    
                    declareAstNode(nodeOtherStmts, AST_OTHERSTMTS, Ast_OtherStmts,
                        otherStmts);
                    
                    nodeToAst(nodeOtherStmts, otherStmts)->otherStmtList = 
                        createAstList();
                    
                    stmt->node_inh = nodeOtherStmts;
                    handleParseTreeElement(stmt);

                    othStmt->node_inh = stmt->node_syn;
                    handleParseTreeElement(othStmt);

                    declareAstNode(nodeElsePart, AST_ELSEPART, Ast_ElsePart, elsePart);
                    nodeToAst(nodeElsePart, elsePart)->otherStmts = nodeToAst(othStmt->node_syn, otherStmts);

                    else_part->node_syn = nodeElsePart;
                   // ptElement->lineNo = ptElement->children[0].lineNo;
                    nodeToAst(nodeElsePart,elsePart)->lineNo = else_part->lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeOtherStmts, AST_OTHERSTMTS, Ast_OtherStmts,
                        otherStmts);
                    
                    nodeToAst(nodeOtherStmts, otherStmts)->otherStmtList = 
                        createAstList();
                    declareAstNode(nodeElsePart, AST_ELSEPART, Ast_ElsePart,
                        elsePart);

                    nodeToAst(nodeElsePart, elsePart)->otherStmts = 
                        nodeToAst(nodeOtherStmts, otherStmts);

                    else_part->node_syn = nodeElsePart;
                   // ptElement->lineNo = ptElement->children[0].lineNo;
                    nodeToAst(nodeElsePart,elsePart)->lineNo = else_part->lineNo;
                }

                break;
            }

            case ARITHMETIC_EXPRESSION:
            {
                ParseTreeElement* aExp = ptElement;
                ParseTreeElement* term = &ptElement->children[0];
                ParseTreeElement* expPrime = &ptElement->children[1];

                handleParseTreeElement(term);
                expPrime->node_inh = term->node_syn;
                handleParseTreeElement(expPrime);

                aExp->node_syn = expPrime->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }
            case TERM:
            {
                ParseTreeElement* term = ptElement;
                ParseTreeElement* factor = &ptElement->children[0];
                ParseTreeElement* tPrime = &ptElement->children[1];

                handleParseTreeElement(factor);
                tPrime->node_inh = factor->node_syn;
                handleParseTreeElement(tPrime);

                term->node_syn = tPrime->node_syn;
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }

            case LOW_PRECEDENCE_OPERATORS:
            {
                ParseTreeElement *low_precedence_operators = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    declareAstNode(nodeAop, AST_ARITHMETICOP, Ast_ArithmeticOperator,
                        arithmeticOp);

                    nodeToAst(nodeAop, arithmeticOp)->op = AOP_PLUS;
                    low_precedence_operators->node_syn = nodeAop;
                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeAop, AST_ARITHMETICOP, Ast_ArithmeticOperator,
                        arithmeticOp);

                    nodeToAst(nodeAop, arithmeticOp)->op = AOP_MINUS;
                    low_precedence_operators->node_syn = nodeAop;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }  

            case FACTOR:
            {
                ParseTreeElement *factor = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    ParseTreeElement *arithmetic_expression = &ptElement->children[1];
                    handleParseTreeElement(arithmetic_expression);
                    factor->node_syn = arithmetic_expression->node_syn;
                }
                else if (ptElement->ruleNo == 1)
                {
                    ParseTreeElement *var = &ptElement->children[0];
                    handleParseTreeElement(var);
                    declareAstNode(nodeAexp, AST_ARITHMETICEXPR, Ast_ArithmeticExpression, arithmeticExpression);
                    nodeToAst(nodeAexp, arithmeticExpression)->aexpType = AEXP_VAR;

                    varAexp(nodeToAst(nodeAexp, arithmeticExpression)) = nodeToAst(var->node_syn, var);
                    nodeToAst(nodeAexp, arithmeticExpression)->lineNo = var->lineNo;
                    factor->node_syn = nodeAexp;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            } 

            case TERM_PRIME:
            {
                ParseTreeElement *term_prime = ptElement;
                if (ptElement->ruleNo == 0){
                    ParseTreeElement* hpOp = &ptElement->children[0];
                    ParseTreeElement* factor = &ptElement->children[1];
                    ParseTreeElement* term_prime1 = &ptElement->children[2];

                    handleParseTreeElement(hpOp);
                    handleParseTreeElement(factor);

                    declareAstNode(nodeAexp, AST_ARITHMETICEXPR, Ast_ArithmeticExpression,
                        arithmeticExpression);
                    expAexp(nodeToAst(nodeAexp, arithmeticExpression)) = malloc(sizeof(ArithmeticExpression));
                    expAexp(nodeToAst(nodeAexp, arithmeticExpression))->op = 
                        nodeToAst(hpOp->node_syn, arithmeticOp)->op;
                    nodeToAst(nodeAexp, arithmeticExpression)->aexpType = AEXP_EXP;
                    
                    Ast_ArithmeticExpression* left = nodeToAst(term_prime->node_inh, arithmeticExpression);
                    Ast_ArithmeticExpression* right = nodeToAst(factor->node_syn, arithmeticExpression);
                    Ast_ArithmeticExpression* aexp = nodeToAst(nodeAexp, arithmeticExpression);

                    expAexp(aexp)->left = left;
                    expAexp(aexp)->right = right;
                    
                    term_prime1->node_inh = nodeAexp;
                    
                    handleParseTreeElement(term_prime1);

                    term_prime->node_syn = term_prime1->node_syn;
                    nodeToAst(nodeAexp, arithmeticExpression)->lineNo = term_prime->lineNo;
                   // ptElement->lineNo = ptElement->children[0].lineNo;

                }
                else if (ptElement->ruleNo == 1)
                {
                    term_prime->node_syn = ptElement->node_inh;
                }
                break;
            }

            case HIGH_PRECEDENCE_OPERATORS:
            {
                ParseTreeElement *high_precedence_operators = ptElement;
                if (ptElement->ruleNo == 0)
                {
                    declareAstNode(nodeAop, AST_ARITHMETICOP, Ast_ArithmeticOperator,
                        arithmeticOp);
                    nodeToAst(nodeAop, arithmeticOp)->op = AOP_MUL;

                    high_precedence_operators->node_syn = nodeAop;
                }
                else if (ptElement->ruleNo == 1)
                {
                    declareAstNode(nodeAop, AST_ARITHMETICOP, Ast_ArithmeticOperator,
                        arithmeticOp);
                    nodeToAst(nodeAop, arithmeticOp)->op = AOP_DIV;

                    high_precedence_operators->node_syn = nodeAop;
                }
               // ptElement->lineNo = ptElement->children[0].lineNo;

                break;
            }  
        }
    }

    else
    {
        Token t = ptElement->elem.symbol;
        maybePrintf("Error !! Calling handleParseTreeELement on terminal symbol %s\n", tokToStr(t));
    }
}