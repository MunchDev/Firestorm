//
// Created by Nguyen Thai Binh on 17/1/22.
//

#ifndef FIRESTORM_PARSER_HPP
#define FIRESTORM_PARSER_HPP

#include <map>

#include "utility.hpp"
#include "lexer.hpp"
#include "ast.hpp"

namespace {
    std::map<std::string, int> getPrecedenceTable() {
        std::map<std::string, int> table;

        table["=="] = 100;
        table["!="] = 100;
        table[">="] = 100;
        table["<="] = 100;
        table[">"] = 100;
        table["<"] = 100;

        table["+"] = 200;
        table["-"] = 200;
        table["*"] = 300;
        table["/"] = 300;

        return table;
    }
}

class ParsingError : public FirestormError {
public:
    explicit ParsingError(const char *msg) : FirestormError(msg) {}
};

class Parser {
    TokenStream &stream;
    std::map<std::string, int> precedence_table;

public:
    explicit Parser(TokenStream &s) : stream(s), precedence_table(getPrecedenceTable()) {}

    std::vector<ExprPtr> parse();

private:
    // program      :=  stmts
    std::vector<ExprPtr> parseProgram();

    // stmts        :=  stmt SEMICOLON
    //              :=  stmt SEMICOLON stmts
    std::vector<ExprPtr> parseStmts();

    // stmt         :=  tlo_stmt
    //              :=  otr_stmt
    ExprPtr parseStmt();

    // tlo_stmt     :=  extern_stmt
    //              :=  define_stmt
    ExprPtr parseTLOStmt();

    // extern_stmt  :=  EXTERN proto
    ProtoPtr parseExternStmt();

    // proto        :=  ID LPAREN ids RPAREN
    //
    // ids          :=
    //              :=  ID
    //              :=  ID COMMA ids
    ProtoPtr parseProto();

    // define_stmt  :=  DEFINE proto expr
    FunctionPtr parseDefineStmt();

    // otr_stmt     :=  expr
    ExprPtr parseOtrStmt();

    // expr         :=  primary
    //              :=  primary bin_op_rhs
    ExprPtr parseExpr();

    // bin_op_rhs   :=  op primary
    //              :=  op primary bin_op_rhs
    //
    // op           :   PLUS
    //              |   MINUS
    //              |   TIMES
    //              |   DIVIDE
    //              |   EQL
    //              |   NEQ
    //              |   GTE
    //              |   LTE
    //              |   GT
    //              |   LT
    ExprPtr parseBinOpRHS(int exprPre, ExprPtr lhs);

    // primary      :=  num_expr
    //              :=  id_expr
    //              :=  paren_expr
    //              :=  if_expr
    ExprPtr parsePrimary();

    // num_expr     :=  NUMBER
    ExprPtr parseNumExpr();

    // id_expr      :=  ID
    //              :=  ID LPAREN args RPAREN
    //
    // args         :=
    //              :=  expr
    //              :=  expr COMMA args
    ExprPtr parseIdExpr();

    // paren_expr   :=  LPAREN expr RPAREN
    ExprPtr parseParenExpr();

    // if_expr      :=  IF expr THEN expr ELSE expr
    ExprPtr parseIfExpr();

    // Util method for bin_op_rhs
    int getOperatorPrecedence();
};

#endif //FIRESTORM_PARSER_HPP
