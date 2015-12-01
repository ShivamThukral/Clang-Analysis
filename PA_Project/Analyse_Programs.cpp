#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;


int numFunctions = 0;
int linesOfCode=0;
int forStmtCounter=0;
int ifStmtCounter=0;
int whileStmtCounter=0;
int switchStmtCounter=0;
int functionCallCounter=0;
int recursiveFunctionCalls=0;

string currentFunction="";
std::map<string,int> functionLengthMap;
ofstream fout("Result.txt");
    
void FileWrite(string str)
{
    if(fout.is_open())
		fout<<str;    
    errs()<<str;
}

string BoolToString(bool b)
{
  return b ? "true" : "false";
}

int getFunctionLength(string start,string end)
{
	std::size_t startLocation1=start.find_first_of(":");
	std::size_t startLocation2=start.find_last_of(":");
	
	string startLineStr=start.substr(startLocation1+1,startLocation2-startLocation1-1);
	int startLine=atoi(startLineStr.c_str());
	
	std::size_t endLocation1=end.find_first_of(":");
	std::size_t endLocation2=end.find_last_of(":");
	
	string endLineStr=end.substr(endLocation1+1,endLocation2-endLocation1-1);
	int endLine=atoi(endLineStr.c_str());
	
	return endLine-startLine;	
}

vector<int> CumulativeFunctionAnalysis()
{
	int total=0;
	int average;
	int max,min;
	if(functionLengthMap.size()>0)
	{
		std::map<std::string, int>::iterator it = functionLengthMap.begin();
		max=it->second;
		min=it->second;
		for(;it != functionLengthMap.end();it++)
		{
			total+=it->second;
			if(max<it->second)
				max=it->second;
			if(min>it->second)
				min=it->second;	
		}
		average=total/functionLengthMap.size();
		vector<int> result{average,min,max};
		return result;
	}
	
	vector<int> result{0,0,0};
	return result;
	
}


class MyVisitor : public RecursiveASTVisitor<MyVisitor> {
private:
    ASTContext *astContext;
  
public:
    explicit MyVisitor(CompilerInstance *CI) 
      : astContext(&(CI->getASTContext())) 
    {
       
    }
    
    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        numFunctions++;
        linesOfCode++;
        
        FileWrite("-----------Function Declaration--------------------\n");
        
        string funcName = func->getNameInfo().getName().getAsString();
        currentFunction=funcName;
        string retName = func->getResultType().getAsString();
        
        SourceLocation funcLoc = func->getLocation();
        SourceRange funcRange = func->getSourceRange();
        SourceLocation funcLocBegin=funcRange.getBegin();
        SourceLocation funcLocEnd=funcRange.getEnd();
        
        
        string funcLocation = funcLoc.printToString(astContext->getSourceManager());
        string beginLocation = funcLocBegin.printToString(astContext->getSourceManager());
        string endLocation = funcLocEnd.printToString(astContext->getSourceManager());
        int length=getFunctionLength(beginLocation,endLocation);
        functionLengthMap.insert(std::make_pair(funcName,length));
                
        FileWrite("Function Found:\t" + funcName + "\n");
        FileWrite("Return Type:\t" + retName + "\n");
        FileWrite("Function Location:\t" + funcLocation + "\n");
        FileWrite("Function \t Begin:\t" + beginLocation + "\t End:\t" + endLocation+"\n");
        FileWrite("Function Length:\t"+to_string(length)+"\n");
        
        for(int i=0; i<func->getNumParams(); i++)
		{
			FileWrite( "\tFunction Parameter:   Name:\t" + func->getParamDecl(i)->getQualifiedNameAsString());
			FileWrite( "   Type:\t" + QualType::getAsString(func->getParamDecl(i)->getType().split()) + "\n"); 
		}
        
        FileWrite("Some Information of Function:");
        FileWrite("\n Is Function Defined:\t\t\t"+BoolToString(func->isDefined()));
        FileWrite("\n Function marked as virtual explicitly:\t"+BoolToString(func->isVirtualAsWritten()));
        FileWrite("\n Function has body:\t\t\t"+BoolToString(func->hasBody()));
        FileWrite("\n Function is pure virtual function:\t"+BoolToString(func->isPure()));
        FileWrite("\n");
        FileWrite("---------------------------------------------------\n");
        return true;
    }

    virtual bool VisitStmt(Stmt *st) {
		
		if(AsmStmt *asmStmt = dyn_cast<AsmStmt>(st))
		{
			linesOfCode++;
			FileWrite("-----------Assignment Statement--------------------");
			SourceLocation asmLocation=asmStmt->getAsmLoc();
			FileWrite("\nFound Assignment at:\t"+asmLocation.printToString(astContext->getSourceManager()));
			FileWrite("---------------------------------------------------\n");
		}
		
		if(BinaryOperator *expr=dyn_cast<BinaryOperator>(st))
		{
			linesOfCode++;
			FileWrite("-------------Binary Operator ----------------------");
			SourceLocation binaryOpLoc=expr->getExprLoc();
			FileWrite("\nLine Number:\t"+binaryOpLoc.printToString(astContext->getSourceManager()));
			
			if (expr->isLogicalOp())
				{
					FileWrite("\tLogical Operator\n");
				}
			else if (expr->isRelationalOp())
				{
					FileWrite( "\tRelational Operator " + expr->getOpcodeStr().str() + "\n");
				}
			else if (expr->isEqualityOp())
				{
					FileWrite( "\tEquality Operator " + expr->getOpcodeStr().str() + "\n");
				}
			else if (expr->isMultiplicativeOp() )
				{
					FileWrite( "\tMultiplicative Operator " + expr->getOpcodeStr().str() + "\n");
				}
			else if (expr->isAdditiveOp() )
				{
					FileWrite( "\tAdditive Operator " + expr->getOpcodeStr().str() + "\n");
				}
			else if (expr->isAssignmentOp() )
				{
					FileWrite( "\tAssignment Operator " + expr->getOpcodeStr().str() + "\n");
				}
			else if (expr->isComparisonOp() )
				{
					FileWrite( "\tComparison Operator " + expr->getOpcodeStr().str() + "\n");
				} 	
			FileWrite("---------------------------------------------------\n");
		}
		
		
		if(WhileStmt *whileStmt = dyn_cast<WhileStmt>(st)){
			linesOfCode++;
			whileStmtCounter++;
			FileWrite("-------------While Statement ----------------------");
			SourceLocation whileLocation = whileStmt->getWhileLoc();
			Expr *expr=whileStmt->getCond();
			
			bool invalid;
			CharSourceRange conditionRange = CharSourceRange::getTokenRange(expr->getLocStart(), expr->getLocEnd());
			StringRef str = Lexer::getSourceText(conditionRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
			FileWrite("\nFound While Loop at:\t" + whileLocation.printToString(astContext->getSourceManager())+"\n");
			
			if (invalid==false) {
				FileWrite("Condition: " + str.str() + "\n");
				}	
			if(VarDecl* cond=whileStmt->getConditionVariable())
			{
				FileWrite("Condition Variable:\t"+cond->getNameAsString()+"\n");
			}
				
			FileWrite("---------------------------------------------------\n");	
			
		}
		
		if(ForStmt *forStmt = dyn_cast<ForStmt>(st)){
			linesOfCode++;
			forStmtCounter++;
			FileWrite("-------------- For Statement ----------------------");
			SourceLocation forLocation = forStmt->getForLoc();
			Expr *expr=forStmt->getCond();
			
			bool invalid;
			CharSourceRange conditionRange = CharSourceRange::getTokenRange(expr->getLocStart(), expr->getLocEnd());
			StringRef str = Lexer::getSourceText(conditionRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
			FileWrite("\nFound For Loop at:\t"+forLocation.printToString(astContext->getSourceManager())+"\n");
			if (invalid==false) {
				FileWrite( "Condition: " + str.str() +"\n");
				}
			
			if(VarDecl* cond=forStmt->getConditionVariable())
			{
				FileWrite("Condition Variable:\t"+cond->getNameAsString()+"\n");
			}
			if(Expr* inc=forStmt->getInc())
			{
				bool invalid;
				CharSourceRange conditionRange = CharSourceRange::getTokenRange(inc->getLocStart(), inc->getLocEnd());
				StringRef str = Lexer::getSourceText(conditionRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
				if (invalid==false) {
					FileWrite( "Variable Changed ( Incremented ) : " + str.str() + "\n");
				}
			}	
			FileWrite("---------------------------------------------------\n");
		}
		
		if(IfStmt *ifStmt= dyn_cast<IfStmt>(st)){
			linesOfCode++;
			ifStmtCounter++;
			FileWrite("--------------- If Statement ----------------------");
			SourceLocation ifLocation=ifStmt->getIfLoc();
			SourceLocation ElseLocation=ifStmt->getElseLoc();
			Expr *expr = ifStmt->getCond();
			bool invalid;
			CharSourceRange conditionRange = CharSourceRange::getTokenRange(expr->getLocStart(), expr->getLocEnd());
			StringRef str = Lexer::getSourceText(conditionRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
			FileWrite("\nFound If Block at:\t"+ifLocation.printToString(astContext->getSourceManager())+"\n");
			if (invalid==false) {
				FileWrite( "Condition: " + str.str() + "\n");
				}
			if(VarDecl* cond=ifStmt->getConditionVariable())
			{
				FileWrite("Condition Variable:\t"+cond->getNameAsString()+"\n");
			}
			if(ifStmt->getElse())
				FileWrite("Found Else Block at:\t"+ElseLocation.printToString(astContext->getSourceManager())+"\n");
			FileWrite("Return Type of If Condition:"+QualType::getAsString(expr->getType().split())+"\n");
			expr = expr->IgnoreParenCasts();		/*Helps in casting of functions*/
			if (CallExpr *call = dyn_cast<CallExpr>(expr)) {
				FileWrite( "Type:\t" + QualType::getAsString(call->getCallReturnType().split()) + "\n"); 
				bool invalid;
				CharSourceRange methodRange = CharSourceRange::getTokenRange(call->getLocStart(), call->getLocEnd());
				StringRef str = Lexer::getSourceText(methodRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
				FileWrite( "Function Name:\t"+str.str()+"\n");
			}
			FileWrite("---------------------------------------------------\n");
		}
		
        if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st)) {
			linesOfCode++;
			FileWrite("--------------- Return Statement ------------------");
			SourceLocation returnLoc=ret->getRetValue()->getLocStart();
			FileWrite("\nFound Return Statement at:" + returnLoc.printToString(astContext->getSourceManager()) + "\n");
			FileWrite("Return Type: " + QualType::getAsString(ret->getRetValue()->getType().split()) + "\n");
			FileWrite("---------------------------------------------------\n"); 
        }   
        
        if(DeclStmt *declStmt=dyn_cast<DeclStmt>(st))
        {
			linesOfCode++;
			FileWrite("--------------- Declaration Statement ------------------\n");
			clang::DeclStmt::decl_iterator end = declStmt->decl_end();
			for (clang::DeclStmt::decl_iterator decl = declStmt->decl_begin();decl != end;++decl) {
					if (clang::VarDecl* var = llvm::dyn_cast<clang::VarDecl>(*decl)) {
							if(var->isFunctionOrMethodVarDecl()) {
										FileWrite( "Local Variable: " + var->getNameAsString() + "\t");
										FileWrite("Parent Function Name:  "+ ((CXXMethodDecl*)(var->getParentFunctionOrMethod()))->getQualifiedNameAsString() + "\t");
										FileWrite("\n");
									}
									
							else if(var->hasGlobalStorage()) {
										FileWrite( "Global Variable: " + var->getNameAsString() + "\t");
										FileWrite( "\n");
									}		
						}
			}
			FileWrite("---------------------------------------------------\n"); 
		}     
		
        if (CallExpr *call = dyn_cast<CallExpr>(st)) {
			linesOfCode++;
			functionCallCounter++;
			FileWrite("--------------- Function Call Statement ------------------\n");
			std::string callee = call->getDirectCallee()->getQualifiedNameAsString();
			FileWrite("Calling Function:\t" + currentFunction+"\n");
			if(callee.compare(currentFunction)==0)
				{
					FileWrite("***Recursive Call Found***\n");
					recursiveFunctionCalls++;
				}
			SourceLocation callLocation=call->getLocStart();
			FileWrite("Found Function Call at:" + callLocation.printToString(astContext->getSourceManager()) + "\n");
			bool invalid;
            CharSourceRange methodRange =
				CharSourceRange::getTokenRange(call->getLocStart(), call->getLocEnd());
			StringRef str =
				Lexer::getSourceText(methodRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
            FileWrite( "Function Name:\t" + str.str() + "\n");
            FileWrite("---------------------------------------------------\n"); 
        }
        
        if(SwitchStmt *switchStmt = dyn_cast<SwitchStmt>(st))
        {
			linesOfCode++;
			switchStmtCounter++;
			FileWrite("--------------- Switch Statement ------------------\n");
			SourceLocation switchLocation = switchStmt->getSwitchLoc();
			Expr *expr=switchStmt->getCond();
			
			bool invalid;
			CharSourceRange conditionRange = CharSourceRange::getTokenRange(expr->getLocStart(), expr->getLocEnd());
			StringRef str = Lexer::getSourceText(conditionRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
			FileWrite("\nFound Switch Statement at:\t"+switchLocation.printToString(astContext->getSourceManager())+"\n");
			if (invalid==false) {
				FileWrite( "Condition: " + str.str() + "\n");
				}
				
		for (SwitchCase *c = switchStmt->getSwitchCaseList(); c != NULL; c = c->getNextSwitchCase()) {
            if (isa<CaseStmt>(c)) {
                CaseStmt *caseStmt = cast<CaseStmt>(c);
				Expr *caseExpr=caseStmt->getLHS();	
				bool invalid;
				CharSourceRange caseRange = CharSourceRange::getTokenRange(caseExpr->getLocStart(), caseExpr->getLocEnd());
				StringRef str = Lexer::getSourceText(caseRange, astContext->getSourceManager(), astContext->getLangOpts(), &invalid);
					if (invalid==false) {
				FileWrite( "Case Stmt:\t" + str.str() + "\n");
				}
			
            }
    }	
			
			
			FileWrite("---------------------------------------------------\n"); 
		}
        
        return true;
    }

};



class MyASTConsumer : public ASTConsumer {
private:
    MyVisitor *visitor; 

public:
    
    explicit MyASTConsumer(CompilerInstance *CI)
        : visitor(new MyVisitor(CI)) 
    { }

   
    virtual void HandleTranslationUnit(ASTContext &Context) {
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }
};



class MyFrontendAction : public ASTFrontendAction {
public:
    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return new MyASTConsumer(&CI); 
    }
};





void CumulativeAnalysis()
{
	FileWrite("\n===========================================================");
	FileWrite("\n------------------Cumulative Analysis----------------------");
	FileWrite("\n===========================================================");
	FileWrite("\nNumber of Functions:\t\t"+to_string(numFunctions));
	FileWrite("\nNumber of IfStmts:\t\t"+to_string(ifStmtCounter));
	FileWrite("\nNumber of ForStmts:\t\t"+to_string(forStmtCounter));
	FileWrite("\nNumber of whileStmts:\t\t"+to_string(whileStmtCounter));
	FileWrite("\nNumber of switchStmts:\t\t"+to_string(switchStmtCounter));
	FileWrite("\nNumber of function calls:\t"+to_string(functionCallCounter));	
	FileWrite("\nNumber of Recursive Calls:\t"+to_string(recursiveFunctionCalls));
	FileWrite("\nNumber of Lines Scanned:\t"+to_string(linesOfCode));
	FileWrite("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	vector<int> result=CumulativeFunctionAnalysis();
	FileWrite("\nAverage Function Size:\t\t"+to_string(result.at(0)));
	FileWrite("\nMinimum Function Size:\t\t"+to_string(result.at(1)));
	FileWrite("\nMaximum Function Size:\t\t"+to_string(result.at(2)));
	FileWrite("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	FileWrite("\n===========================================================\n");

}




int main(int argc, const char **argv) {
    CommonOptionsParser op(argc, argv);        
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    int result = Tool.run(newFrontendActionFactory<MyFrontendAction>());
   // FileWrite("Hello1");
    CumulativeAnalysis();
  
    return result;
}
