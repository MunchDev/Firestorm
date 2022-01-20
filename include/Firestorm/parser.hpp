//
// Created by Nguyen Thai Binh on 17/1/22.
//
#ifndef FIRESTORM_PARSER_HPP
#define FIRESTORM_PARSER_HPP

#include <map>
#include <memory>
#include <string>

namespace Firestorm::Lexing {
    class TokenStream;
}

namespace Firestorm::AST {
    class Expr;

    class Prototype;

    class Function;
}

namespace Firestorm::Parsing {
    using ExprPtr = std::unique_ptr<AST::Expr>;
    using ProtoPtr = std::unique_ptr<AST::Prototype>;
    using FunctionPtr = std::unique_ptr<AST::Function>;

    std::map<std::string, int> &getPrecedenceTable();

    class Parser {
        Lexing::TokenStream &stream;
        std::map<std::string, int> precedence_table;

    public:
        explicit Parser(Lexing::TokenStream &s) : stream(s), precedence_table(getPrecedenceTable()) {}

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

        // for_expr     :=  FOR ID EQUALS expr COMMA expr step_clause THEN expr
        //
        // step_clause  :=
        //              :=  COMMA expr
        ExprPtr parseForExpr();

        // Util method for bin_op_rhs
        int getOperatorPrecedence();
    };
}

#endif //FIRESTORM_PARSER_HPP
