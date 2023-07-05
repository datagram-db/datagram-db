//
// Created by giacomo on 10/04/23.
//

#ifndef GSM_GSQL_SCRIPTVISITOR_H
#define GSM_GSQL_SCRIPTVISITOR_H

#include <scriptVisitor.h>
#include <gsql_gsm/script_1/java_types.h>
#include <gsql_gsm/gsm_inmemory_db.h>

namespace script {
    namespace structures {
        struct ScriptAST;
    }
    namespace compiler {

        struct ScriptVisitor : public scriptVisitor {
            DPtr<std::unordered_map<std::string, DPtr<script::structures::ScriptAST>>> context;
            static gsm_inmemory_db* db; // WARNING: THIS CANNOT BE USED IN CONCURRENT SETTINGS WHERE MULTIPLE DATABASES ARE USED!
            ScriptVisitor();
            static void bindGSM(gsm_inmemory_db* gsm) { db = gsm; }
            static DPtr<script::structures::ScriptAST> eval(std::istream& is);
            std::any visitScript(scriptParser::ScriptContext *context) override;
            std::any visitSub(scriptParser::SubContext *context) override;
            std::any visitAtom_array(scriptParser::Atom_arrayContext *context) override;
            std::any visitSelect(scriptParser::SelectContext *context) override;
            std::any visitMult(scriptParser::MultContext *context) override;
            std::any visitLt(scriptParser::LtContext *context) override;
            std::any visitSubstring(scriptParser::SubstringContext *context) override;
            std::any visitRemove(scriptParser::RemoveContext *context) override;
            std::any visitPut(scriptParser::PutContext *context) override;
            std::any visitDiv(scriptParser::DivContext *context) override;
            std::any visitParen(scriptParser::ParenContext *context) override;
            std::any visitNot(scriptParser::NotContext *context) override;
            std::any visitGeq(scriptParser::GeqContext *context) override;
            std::any visitAnd(scriptParser::AndContext *context) override;
            std::any visitFunction(scriptParser::FunctionContext *context) override;
            std::any visitLeq(scriptParser::LeqContext *context) override;
            std::any visitNeq(scriptParser::NeqContext *context) override;
            std::any visitMap(scriptParser::MapContext *context) override;
            std::any visitAdd(scriptParser::AddContext *context) override;
            std::any visitOr(scriptParser::OrContext *context) override;
            std::any visitImply(scriptParser::ImplyContext *context) override;
            std::any visitApply(scriptParser::ApplyContext *context) override;
            std::any visitVar(scriptParser::VarContext *context) override;
            std::any visitAtom_bool(scriptParser::Atom_boolContext *context) override;
            std::any visitConcat(scriptParser::ConcatContext *context) override;
            std::any visitEq(scriptParser::EqContext *context) override;
            std::any visitGt(scriptParser::GtContext *context) override;
            std::any visitContains(scriptParser::ContainsContext *context) override;
            std::any visitEval(scriptParser::EvalContext *context) override;
            std::any visitAtom_number(scriptParser::Atom_numberContext *context) override;
            std::any visitAt(scriptParser::AtContext *context) override;
            std::any visitVariable(scriptParser::VariableContext *context) override;
            std::any visitAppend(scriptParser::AppendContext *context) override;
            std::any visitIfte(scriptParser::IfteContext *context) override;
            std::any visitAtom_string(scriptParser::Atom_stringContext *context) override;
            std::any visitAssign(scriptParser::AssignContext *context) override;
            std::any visitVarphi(scriptParser::VarphiContext *context) override;
            std::any visitEll(scriptParser::EllContext *context) override;
            std::any visitPhi(scriptParser::PhiContext *context) override;
            std::any visitXi(scriptParser::XiContext *context) override;
            std::any visitMod(scriptParser::ModContext *context) override;
            std::any visitFlat(scriptParser::FlatContext *context) override;
            std::any visitSelfcross(scriptParser::SelfcrossContext *context) override;
            std::any visitInj(scriptParser::InjContext *context) override;
            std::any visitCross(scriptParser::CrossContext *context) override;
            std::any visitAbs(scriptParser::AbsContext *context) override;
            std::any visitRfold(scriptParser::RfoldContext *context) override;
            std::any visitLfold(scriptParser::LfoldContext *context) override;
            std::any visitFilter(scriptParser::FilterContext *context) override;
            std::any visitCos(scriptParser::CosContext *context) override;
            std::any visitTan(scriptParser::TanContext *context) override;
            std::any visitCeil(scriptParser::CeilContext *context) override;
            std::any visitLog(scriptParser::LogContext *context) override;
            std::any visitPow(scriptParser::PowContext *context) override;
            std::any visitSin(scriptParser::SinContext *context) override;
            std::any visitFloor(scriptParser::FloorContext *context) override;
            std::any visitTuple_pair(scriptParser::Tuple_pairContext *context) override;
            std::any visitType_bool(scriptParser::Type_boolContext *context) override;
            std::any visitEnsure(scriptParser::EnsureContext *context) override;
            std::any visitKind(scriptParser::KindContext *context) override;
            std::any visitType_tuple(scriptParser::Type_tupleContext *context) override;
            std::any visitSigma_type(scriptParser::Sigma_typeContext *context) override;
            std::any visitType_string(scriptParser::Type_stringContext *context) override;
            std::any visitTypeof(scriptParser::TypeofContext *context) override;
            std::any visitType_int(scriptParser::Type_intContext *context) override;
            std::any visitAtom_tuple(scriptParser::Atom_tupleContext *context) override;
            std::any visitType_double(scriptParser::Type_doubleContext *context) override;
            std::any visitType_list(scriptParser::Type_listContext *context) override;
            std::any visitObj(scriptParser::ObjContext *context) override;
            std::any visitSubtype_of(scriptParser::Subtype_ofContext *context) override;
            std::any visitType_or(scriptParser::Type_orContext *context) override;
            std::any visitType_and(scriptParser::Type_andContext *context) override;
            std::any visitProject(scriptParser::ProjectContext *context) override;

            std::any visitType_label(scriptParser::Type_labelContext *context) override;
            std::any visitEnforce(scriptParser::EnforceContext *context) override;

            std::any visitType_lex(scriptParser::Type_lexContext *context) override;

            std::any visitCoerce(scriptParser::CoerceContext *context) override;
        };

    } // script
} // compiler

#endif //GSM_GSQL_SCRIPTVISITOR_H
