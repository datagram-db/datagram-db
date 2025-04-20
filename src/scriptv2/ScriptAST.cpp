//
// Created by giacomo on 09/04/23.
//

#include <iostream>
#include <utility>
#include "database/SchemaIndexer.h"
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::TRUE;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::FALSE;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::DOUBLET;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::INTEGERT;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::BOOLEANT;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::STRINGT;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::START;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::ANYT;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::BOTCCT;
DPtr<script::structures::ScriptAST> script::structures::ScriptAST::NULLO;
#include <scriptv2/Funzione.h>
 std::unordered_map<std::string, DPtr<script::structures::ScriptAST>> script::structures::ScriptAST::globals;
         TemplateIndexer* script::structures::ScriptAST::idxers = nullptr;
closure* script::structures::ScriptAST::db{nullptr};

void script::structures::ScriptAST::setDBRecursively( closure* datenbanken) {
    db = datenbanken;
    return;
}

bool script::structures::ScriptAST::toBoolean(bool implode)  {
        switch (type) {
            case Boolean:
                return bool_;
            case Integer:
                return integer != 0;
            case Double:
                return doubleValue_ != 0;
            case String:
            case LABELT:
                return !string.empty();
            case Array:
                return !arrayList.empty();
            case Function:
                return !function->body.empty();
            case TupleT:
            case Tuple:
                return !tuple.empty();
            case NullE:
                return false;
            default:
                return run(implode)->toBoolean();
        }

}

double script::structures::ScriptAST::toDouble(bool implode)  {
    switch (type) {
        case Boolean:
            return bool_ ? 1.0 : 0.0;
        case Integer:
            return (double)integer;
        case Double:
            return doubleValue_;
        case String:
            return (double)string.length();
        case Array:
            return (double)arrayList.size();
        case TupleT:
        case Tuple:
            return (double)tuple.size();
        case Function:
            return (double)function->body.size();
        case NullE:
            return 0.0;
        default:
            return run(implode)->toDouble();
    }
}

#include <algorithm>
#include <numeric>

long long script::structures::ScriptAST::toInteger(bool implode)   {
    switch (type) {
        case ObjX:
            return arrayList[0]->toInteger() ;
        case Boolean:
            return bool_ ? 1LL : 0LL;
        case Integer:
            return integer;
        case Double:
            return (long long)doubleValue_;
        case String:
            return (long long)string.length();
        case Array:
            return (long long)arrayList.size();
        case Tuple:
        case TupleT:
            return (long long)tuple.size();
        case Function:
            return (long long)function->body.size();
        case NullE:
            return -1;
        default:
            return run(implode)->toInteger();
    }
}

#include <iomanip>



std::string script::structures::ScriptAST::toString(bool quot, bool implode) const {
    std::string result;
    switch (type) {
        case BooleanT:
            return "bool";
        case Boolean:
            return bool_ ? "tt" : "ff";
        case Integer:
            return std::to_string(integer);
        case IntegerT:
            return "int";
        case Double:
            return std::to_string(doubleValue_);
        case DoubleT:
            return "double";
        case String:
            if (quot) {
                std::stringstream ss;
                ss << std::quoted(string);
                return ss.str();
            } else return string;
        case StringT:
            return "string";
        case Assignment:
            return arrayList[0]->toString(true) +" := ("+arrayList[1]->toString(true)+") ";
        case Function:
            if (function->body.size() == 1)
                return "fun " + function->parameter + " -> {" + function->body[0]->toString(true, false) + "}";
            else
            {
                auto it = function->body.begin();
//                it++;
                return "fun " + function->parameter + " -> {" + std::accumulate(it,
                                function->body.end(), std::string(""),
                                [](const std::string& m, DPtr<script::structures::ScriptAST> obj) {
                                    return (m.empty() ? m : m+"; ") + obj->toString(true, false);
                                }) + "}";
            }
        case Array:
            if (arrayList.empty()) {
                if (implode)
                    return "";
                else
                    return "{}";
            } if (arrayList.size() == 1) {
                if (implode)
                    arrayList[0]->toString(true, true);
                else
                    return "{" + arrayList[0]->toString(true) + "}";
            } else {
                std::string beginning = implode ? "" : "{";
                std::string sep = implode ? " " : "; ";
                std::string end = implode ? "" : "}";
                auto it = arrayList.begin();
//                it++;
                return beginning + std::accumulate(it,
                                             arrayList.end(), std::string(""),
                                                                       [&sep,implode](const std::string& m, DPtr<script::structures::ScriptAST> obj) {
                                                                           return (m.empty() ? m : m+sep) + obj->toString(true, implode);
                                                                       }) + end;
            }
        case ArrayOfT:
            return "(listof (" + arrayList[0]->toString(true, false) +"))";
        case TupleT:
        case Tuple:
            if (tuple.size() == 1) {
                auto it = tuple.begin();
                std::stringstream ss;
                if (type == TupleT)
                    ss << "t< ";
                else
                    ss << "< ";
                ss <<std::quoted(it->first) << " >> " << it->second->toString(true, false) << " >";
                return ss.str();
            }
            else
            {
                auto it = tuple.begin();
//                it++;
                return ((type == TupleT) ? "t< " : "< ") + std::accumulate(it,
                                             tuple.end(), std::string(""),
                                             [](const std::string& m, const std::pair<std::string,DPtr<script::structures::ScriptAST>>& obj) {
                                                 std::stringstream ss;
                                                 ss << std::quoted(obj.first) << " >> " << obj.second->toString(true);
                                                 return (m.empty() ? m : m+"; ") + ss.str();
                                             }) + " >";
            }
        case Variable:
            return string;

        case AndE:
            return "(" + arrayList[0]->toString(true) +") && ("+ arrayList[1]->toString(true)+")";
        case ANDT:
            return "(" + arrayList[0]->toString(true) +") /\\ ("+ arrayList[1]->toString(true)+")";

        case OrE:
            return "(" + arrayList[0]->toString(true) +") || ("+ arrayList[1]->toString(true)+")";
        case ORT:
            return "(" + arrayList[0]->toString(true) +") \\/ ("+ arrayList[1]->toString(true)+")";

        case NotE:
            return "!(" + arrayList[0]->toString(true) +")";

        case EqE:
            return "(" + arrayList[0]->toString(true) +") == ("+ arrayList[1]->toString(true)+")";

        case IfteE:
            return "if (" + arrayList[0]->toString(true) +") then ("+ arrayList[1]->toString(true)+") else (" + arrayList[2]->toString(true)+")";

        case ImplyE:
            return "(" + arrayList[0]->toString(true) +") => ("+ arrayList[1]->toString(true)+")";

        case ApplyE:
            return "((" + arrayList[0]->toString(true) +")  ("+ arrayList[1]->toString(true)+"))";

        case AtE:
            return "(" + arrayList[0]->toString(true) +")  ["+ arrayList[1]->toString(true)+"]";

        case ProjectE:
            return "(" + arrayList[0]->toString(true) +")  [["+ arrayList[1]->toString(true)+"]]";

        case PutE:
            return "(" + arrayList[0]->toString(true) +")  ["+ arrayList[1]->toString(true)+"]:= ("+ arrayList[2]->toString(true)+")";

        case RemoveE:
            return "remove (" + arrayList[0]->toString(true) +") from ("+ arrayList[1]->toString(true)+")";

        case AppendE:
            return "(" + arrayList[0]->toString(true) +") @ ("+ arrayList[1]->toString(true)+")";
//
//        case AssignE:
//            return "(" + arrayList[0]->toString(true) +") := ("+ arrayList[1]->toString(true)+")";

        case ContainsE:
            return "(" + arrayList[0]->toString(true) +") in ("+ arrayList[1]->toString(true)+")";

        case FilterE:
            return "filter(" + arrayList[0]->toString(true) +" : "+ arrayList[1]->toString(true)+")";

        case InvokeE:
            return "(" + arrayList[0]->toString(true) +") . ("+ arrayList[1]->toString(true)+")";

        case MapE:
            return "map(" + arrayList[0]->toString(true) +" : "+ arrayList[1]->toString(true)+")";

        case SubElementsE:
            break;
        case AddE:
            return "(" + arrayList[0]->toString(true) +") + ("+ arrayList[1]->toString(true)+")";

        case DivE:
            return "(" + arrayList[0]->toString(true) +") / ("+ arrayList[1]->toString(true)+")";

        case ModE:
            return "(" + arrayList[0]->toString(true) +") % ("+ arrayList[1]->toString(true)+")";

        case AbsE:
            return "| " + arrayList[0]->toString(true) +" |";

        case MinusE:
            return "(" + arrayList[0]->toString(true) +") - ("+ arrayList[1]->toString(true)+")";

        case MulE:
            return "(" + arrayList[0]->toString(true) +") * ("+ arrayList[1]->toString(true)+")";

        case SinE:
            return "sin(" + arrayList[0]->toString(true) +")";

        case CosE:
            return "cos(" + arrayList[0]->toString(true) +")";

        case TanE:
            return "tan(" + arrayList[0]->toString(true) +")";

        case ExpE:
            return "exp (" + arrayList[0]->toString(true) +")  ("+ arrayList[1]->toString(true)+")";

        case LogE:
            return "log (" + arrayList[0]->toString(true) +")  ("+ arrayList[1]->toString(true)+")";

        case FloorE:
            return "|_" + arrayList[0]->toString(true) +"_|";

        case CeilE:
            return "|-" + arrayList[0]->toString(true) +"-|";

        case ConcatE:
            return "(" + arrayList[0]->toString(true) +") ++ ("+ arrayList[1]->toString(true)+")";

        case GtE:
            return "(" + arrayList[0]->toString(true) +") > ("+ arrayList[1]->toString(true)+")";

        case LtE:
            return "(" + arrayList[0]->toString(true) +") < ("+ arrayList[1]->toString(true)+")";

        case GEqE:
            return "(" + arrayList[0]->toString(true) +") >= ("+ arrayList[1]->toString(true)+")";

        case LEqE:
            return "(" + arrayList[0]->toString(true) +") <= ("+ arrayList[1]->toString(true)+")";

        case NeqE:
            return "(" + arrayList[0]->toString(true) +") != ("+ arrayList[1]->toString(true)+")";

        case PhiX:
            return "(phi (" + arrayList[0]->toString(true) +")  ("+ arrayList[1]->toString(true)+"))";

        case VarPhiX:
            return "varphi (" + arrayList[0]->toString(true) +")";

        case EllX:
            return "ell (" + arrayList[0]->toString(true) +")";

        case XiX:
            return "xi (" + arrayList[0]->toString(true) +")";

        case CrossE:
            return "(" + arrayList[0]->toString(true) +") x ("+ arrayList[1]->toString(true)+")";

        case SelfCrossE:
            return "selfx (" + arrayList[0]->toString(true) +")";

        case SelfZipE:
            return "zip (" + arrayList[0]->toString(true) +")";

        case InjE:
            return "inj (" + arrayList[0]->toString(true) +")";

        case FlatE:
            return "flat (" + arrayList[0]->toString(true) +")";

        case LFoldE:
            return "lfold(" + arrayList[0]->toString(true) +" , "+ arrayList[1]->toString(true)+" : " + arrayList[2]->toString(true)+")";

        case RFoldE:
            return "rfold(" + arrayList[0]->toString(true) +" , "+ arrayList[1]->toString(true)+" : " + arrayList[2]->toString(true)+")";

        case EvalE:
            return "eval(" + arrayList[0]->toString(true) +")";


        case BotT:
            return "null";

        case SigmaT:
            return "sigma (" + arrayList[0]->toString(true) +") where ("+ arrayList[1]->toString(true)+")";

        case StarT:
            return "star";

        case LABELT: {
            std::stringstream ss;
            ss << std::quoted(string);
            return "label "+ ss.str();
        }

        case ENFORCET:
            return "(" + arrayList[0]->toString(true) +") enforce_subtype ("+ arrayList[1]->toString(true)+")";
        case ObjTT:
            return "ObjT (" + arrayList[0]->toString(true) +") ("+ arrayList[1]->toString(true)+")";
        case AnyT:
            return "any";
        case TypeOf:
            return "typeof (" + arrayList[0]->toString(true) +")";
        case ObjX:
            return "OBJ ("+ arrayList[0]->toString(true) +")";
        case AssertE:
            return "assert ("+ arrayList[0]->toString(true) +")";
        case SubtypeOfE:
            return "(" + arrayList[0]->toString(true) +") <: ("+ arrayList[1]->toString(true)+")";
        case CoerceE:
            return "coerce (" + arrayList[0]->toString(true) +") as ("+ arrayList[1]->toString(true)+")";
        case NullE:
            return "null";
    }
    return "";
}

ArrayList<DPtr<script::structures::ScriptAST>> script::structures::ScriptAST::toList()  {
    ArrayList<DPtr<script::structures::ScriptAST>> result;
    switch (type) {
        case Boolean:
        case Integer:
        case Double:
            result.emplace_back(shared_from_this());
            break;
        case String:
        {
            std::string local;
            for (size_t i = 0; i<string.length(); i++) {
                local = string.at(i);
                result.emplace_back(script::structures::ScriptAST::string_(local));
            }
            break;
        }
        case TupleT:
        case Tuple: {
            for (const auto&[k,v] : tuple) {
                ArrayList<DPtr<script::structures::ScriptAST>> inneres;
                inneres.emplace_back(string_(k));
                inneres.emplace_back(v);
                result.emplace_back(array_(std::move(inneres)));
            }
            break;
        }
        case Array:
            return arrayList;
        case Function:
            return function->body;
        case NullE: {
            return {};
        } default:
            return run()->toList();
    }
    return result;
}

StringMap<DPtr<script::structures::ScriptAST>> script::structures::ScriptAST::toMap()  {
    StringMap<DPtr<script::structures::ScriptAST>> result;
    switch (type) {
        case Boolean:
        case Integer:
        case Double: {
            result["0"] = shared_from_this();
            break;
        }

        case String: {
            for (size_t i = 0, N = string.size(); i<N; i++) {
                result[std::to_string(i)] = string_(""+string.at(i));
            }
            break;
        }

        case Array: {
            for (size_t i = 0, N = arrayList.size(); i<N; i++) {
                result[std::to_string(i)] = arrayList.at(i);
            }
            break;
        }

        case Function: {
            throw std::runtime_error("ERROR: cannot convert a function with a potential infinite domain into a tuple!");
        }

        case TupleT:
        case Tuple: {
            return tuple;
        }

        case NullE: {
            break;
        }

        default:
            return run()->toMap();
    }
    return result;
}

std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>)> script::structures::ScriptAST::toFunction()  {
    switch (type) {
        case Boolean:
        case BooleanT:
        case Integer:
        case IntegerT:
        case Double:
        case DoubleT:
            return [this](auto x) {return shared_from_this(); };

        case String:
            return [this](DPtr<script::structures::ScriptAST> x) {
                size_t i = (size_t)x->toInteger();
                std::string local;
                if (i<string.length()) {
                    local = string.at(i);
                }
                return script::structures::ScriptAST::string_(local);
            };

        case Array:
            return [this](DPtr<script::structures::ScriptAST> x) {
                size_t i = (size_t)x->toInteger();
                ArrayList<DPtr<script::structures::ScriptAST>> vv;
                if (i<arrayList.size()) {
                    return arrayList[i];
                } else {
                    return script::structures::ScriptAST::array_(std::move(vv));
                }
            };

        case Function: {
            return [this](DPtr<script::structures::ScriptAST> x) {
                return function->apply(std::move(x));
            };
        }

        case TupleT:{
            return [this](DPtr<script::structures::ScriptAST> x) {
                std::string i = x->toString();
                auto it = tuple.find(i);
                if (it == tuple.end())
                    return script::structures::ScriptAST::star_T();
                else
                    return it->second;
            };
        }
        case Tuple: {
            return [this](DPtr<script::structures::ScriptAST> x) {
                std::string i = x->toString();
                auto it = tuple.find(i);
                if (it == tuple.end())
                    return script::structures::ScriptAST::null_();
                else
                    return it->second;
            };
        }

        case NullE:
            return [this](DPtr<script::structures::ScriptAST> x) {
                return script::structures::ScriptAST::string_("");
            };

        default:
            return run()->toFunction();
    }
}

DPtr<script::structures::ScriptAST> script::structures::ScriptAST::variableEval()  {
    if (string.contains("N"))
        std::cout << "DEBUG" << std::endl;
//    auto it3 = globals.find(string);
    if (!idxers->contains(string)) {

        if (!optGamma.get()) {
//            std::cerr << "Error: variable cannot be solved: Gamma is null" << std::endl;
            auto it3 = globals.find(string);
            if (it3 == globals.end())
                return null_();
            else
                return it3->second;
        } else {
            auto it = optGamma->find(string);
            if (it != optGamma->end()) {
                return it->second;
            } else
                return null_();
        }
    } else {
        return idxers->get(string);
    }
}

void script::structures::ScriptAST::setContext(DPtr<std::unordered_map<std::string, DPtr<ScriptAST>>>& g, closure* db) {
    if (optGamma.get() != g.get())  {
        optGamma = g;
    }
    if (this->db != db)
        this->db = db;
    if (type == Function) {
        function->setContext(g, db);
    } else {
        for (auto & i : arrayList) {
            i->setContext(g, db);
        }
        for (auto & [k,v] : tuple) {
            v->setContext(g, db);
        }

    }
}

#include <math.h>
#include "scriptv2/ScriptVisitor.h"
#include "yaucl/strings/string_utils.h"
#include "yaucl/functional/iterators.h"
#include "yaucl/structures/setoids/basics.h"

DPtr<script::structures::ScriptAST> script::structures::ScriptAST::run(bool implode) {
    switch (type) {
        case Boolean:
        case Integer:
        case Double:
        case String:
        case Array:
        case Function:
        case BooleanT:
        case DoubleT:
        case StringT:
        case StarT:
        case IntegerT:
        case ArrayOfT:
        case Tuple:
        case TupleT:
        case AnyT:
        case ObjTT:
        case BotT:
        case NullE:
            return shared_from_this();

        case LABELT:
            if (optGamma) {
                auto it = optGamma->find(string);
                if (it == optGamma->end()) return shared_from_this();
                auto it2 = it->second->run(implode);
                return ((it2->isType())) ?
                       it2 :
                       shared_from_this();
            } else
                return shared_from_this();

        case Assignment:
//            arrayList[1] = arrayList[1]->run();
            if (arrayList[0]->type == Variable) {
                (*arrayList[0]->optGamma)[arrayList[0]->string] = arrayList[1];
            } else if (arrayList[0]->type == Function) {
                return arrayList[0]->function->apply(arrayList[1]);
            }
            return arrayList[1];
            break;

        case Variable:
            return variableEval();

        case AndE:
            return arrayList[0]->toBoolean() && arrayList[1]->toBoolean() ? true_() : false_();

        case OrE:
            return arrayList[0]->toBoolean() || arrayList[1]->toBoolean() ? true_() : false_();

        case NotE:
            return arrayList[0]->toBoolean() ? false_() : true_();

        case EqE:         {
            auto lx = arrayList[0]->run(implode);
            auto rx = arrayList[1]->run(implode);
            return lx->javaComparator(rx) == 0 ? true_() : false_();
        }

        case IfteE: {
            auto cond = arrayList[0]->toBoolean();
            if (cond) {
                auto tmp = arrayList[1]->run(implode);
                return tmp;
            } else {
                auto tmp = arrayList[2]->run(implode);
                return tmp;
            }
        }
            return arrayList[0]->toBoolean() ? arrayList[1]->run(implode) : arrayList[2]->run(implode);

        case ImplyE:
            return arrayList[0]->toBoolean() ? arrayList[1]->run(implode) : arrayList[0]->run(implode);

        case InvokeE:
        case ApplyE:
        case AtE:
            return arrayList[0]->run(implode)->toFunction()(arrayList[1]->run(implode));

        case ProjectE: {
            auto run = arrayList[0]->run(implode);
            auto run2 = arrayList[1]->toList();
            if (run->type == String) {
                std::string compose;
                size_t N = run->string.size();
                for (const auto& idx : run2) {
                    size_t actual = idx->toInteger();
                    if (actual < N)
                        compose += run->string.at(actual);
                }
                return string_(compose);
            } else if ((run->type == Tuple) || (run->type == TupleT)) {
                StringMap<DPtr<script::structures::ScriptAST>> result;
                for (const auto& key : run2) {
                    auto songs = key->toString();
                    auto it = run->tuple.find(songs);
                    if (it != run->tuple.end()) {
                        result[songs] = it->second;
                        if (!result[songs]->db) result[songs]->db = db;
                    }
                }
                if (run->type == TupleT)
                    return tuple_type_(std::move(result));
                else
                    return tuple_(std::move(result));
            } else {
                auto ls = run->toList();
                ArrayList<DPtr<script::structures::ScriptAST>> result;
                size_t N = ls.size();
                for (const auto& key: run2) {
                    size_t actual = key->toInteger();
                    if (actual < N)
                        result.emplace_back(ls[actual]);
                }
                return array_(std::move(ls));
            }
        }

        case PutE:
        {
            auto run = arrayList[0]->run(implode);
            if (run->type == String) {
                size_t pos = arrayList[1]->toInteger();
                std::string cpy = run->string;
                if (pos < cpy.length()) {
                    auto rpl = arrayList[2]->toString(false,implode);
                    if (rpl.empty())
                        cpy[pos] = ' ';
                    else
                        cpy[pos] = rpl.at(0);
                } else if (pos == cpy.length()) {
                    auto rpl = arrayList[2]->toString(false,implode);
                    if (!rpl.empty())
                        cpy += rpl.at(0);
                } else {
                    auto rpl = arrayList[2]->toString(false,implode);
                    if (!rpl.empty()) {
                        while (cpy.length() < pos)
                            cpy += " ";
                        cpy += rpl.at(0);
                    }
                }
                return string_(cpy);
            } else if ((run->type == Tuple) || (run->type == TupleT)) {
                auto cpy = tuple;
                cpy[arrayList[1]->toString()] = arrayList[2]->run(implode);
                if (run->type == TupleT)
                    return tuple_type_(std::move(cpy));
                else
                    return tuple_(std::move(cpy));
            } else {
                auto list = arrayList[0]->toList();
                size_t pos = arrayList[1]->toInteger();
                if (pos < list.size()) {
                    list[pos] = arrayList[2]->run(implode);
                } else if (pos == list.size()) {
                    list.emplace_back(arrayList[2]->run(implode));
                } else {
                    while (list.size() < pos)
                        list.emplace_back(false_());
                    list.emplace_back(arrayList[2]->run(implode));
                }
                return script::structures::ScriptAST::array_(std::move(list));
            }
        }

        case RemoveE:
        {
            auto run = arrayList[1]->run(implode);
            if (run->type == String) {
                return string_(yaucl::strings::replace_all(run->string, arrayList[0]->toString(), ""));
            } else if ((run->type == Tuple) || (run->type == TupleT)) {
                auto cpy = tuple;
                cpy.erase(arrayList[1]->toString());
                if (run->type == TupleT)
                    return tuple_type_(std::move(cpy));
                else
                    return tuple_(std::move(cpy));
            } else {
                ArrayList<DPtr<script::structures::ScriptAST>> vv;
                auto list = arrayList[0]->toList();
                for (const auto& ref : list) {
                    if (ref->javaComparator(arrayList[0]) != 0)
                        vv.emplace_back(ref);
                }
                return script::structures::ScriptAST::array_(std::move(vv));
            }
        }

        case AppendE:
        {
            auto run = arrayList[0]->run(implode);
            auto run2 = arrayList[1]->run(implode);
            if ((run->type == Tuple) && (run2->type == Tuple)) {
                auto cpy = run->tuple;
                for (const auto& [k,v] : run2->tuple) {
                    cpy[k] = v;
                    if (!cpy[k]->db) cpy[k]->db = db;
                }
                return tuple_(std::move(cpy));
            } else if ((run->type == TupleT) && (run2->type == TupleT)) {
                auto cpy = run->tuple;
                for (const auto& [k,v] : run2->tuple) {
                    cpy[k] = v;
                    if (!cpy[k]->db) cpy[k]->db = db;
                }
                return tuple_type_(std::move(cpy));
            } else {
                auto lsx = arrayList[0]->toList();
                auto lrx = arrayList[1]->toList();
                ArrayList<DPtr<script::structures::ScriptAST>> v;
                v.insert(v.end(), std::make_move_iterator(lsx.begin()), std::make_move_iterator(lsx.end()));
                v.insert(v.end(), std::make_move_iterator(lrx.begin()), std::make_move_iterator(lrx.end()));
                return array_(std::move(v));
            }
        }

        case ContainsE:
        {
            auto transf = arrayList[0]->run(implode);
            auto run = arrayList[1]->run(implode);
            if ((run->type == Tuple) || (run->type == TupleT)) {
                return run->tuple.contains(transf->toString()) ? true_() : false_();
            } else {
                auto list = run->toList();
                for (const auto& ref : list) {
                    if (ref->javaComparator(transf) == 0) return true_();
                }
                return false_();
            }
        }

        case FilterE: {
            auto run = arrayList[1]->run(implode);
            auto prop = arrayList[0]->toFunction();
            if ((run->type == Tuple) || (run->type == TupleT)) {
                StringMap<DPtr<script::structures::ScriptAST>> return_;
                for (const auto& [k,v]: run->tuple) {
                    StringMap<DPtr<script::structures::ScriptAST>> map;
                    bool isDb = v->db != nullptr;
                    map["key"] = string_(k);
                    map["value"] = v;
                    if (!isDb) map["value"]->db = db;
                    if (prop(tuple_(std::move(map)))->toBoolean()) {
                        return_[k] = v;
                        if (!isDb) return_[k]->db = db;
                    }
                }
                return (run->type == Tuple) ? tuple_(std::move(return_)) : tuple_type_(std::move(return_));
            } else {
                ArrayList<DPtr<script::structures::ScriptAST>> vv;
                auto list = arrayList[1]->toList();
                for (const auto& ref : list) {
                    if (prop(ref)->toBoolean())
                        vv.emplace_back(ref);
                }
                return script::structures::ScriptAST::array_(std::move(vv));
            }
        }

        case MapE:
        {
            auto run = arrayList[1]->run(implode);
            auto transf = arrayList[0]->toFunction();
            if ((run->type == Tuple) || (run->type == TupleT)) {
                StringMap<DPtr<script::structures::ScriptAST>> return_;
                for (const auto& [k,v]: run->tuple) {
                    StringMap<DPtr<script::structures::ScriptAST>> map;
                    map["key"] = string_(k);
                    map["value"] = v;
                    auto mapResult = transf(tuple_(std::move(map)))->toMap();
                    auto it = mapResult.find("key");
                    auto it2 = mapResult.find("value");
                    if ((it != mapResult.end()) && (it2 != mapResult.end())) {
                        return_[it->second->toString()] = it2->second;
                    } else {
                        throw std::runtime_error("ERROR: the transformation should return key and value as elements!");
                    }
                }
                return (run->type == Tuple) ? tuple_(std::move(return_)) : tuple_type_(std::move(return_));
            } else {
                auto list = run->toList();
                ArrayList<DPtr<script::structures::ScriptAST>> vv;
                vv.reserve(list.size());
                for (const auto& ref : list) {
                    vv.emplace_back(transf(ref));
                }
                return script::structures::ScriptAST::array_(std::move(vv));
            }

        }

        case SubElementsE: {
            size_t begin = (size_t)arrayList[0]->toInteger();
            size_t end = (size_t)arrayList[1]->toInteger();
            auto run2 = arrayList[2]->run(implode);
            if (run2->type == String) {
                if (end < begin)
                    return string_("");
                else
                    return string_(arrayList[2]->string.substr(begin, end-begin+1));
            } else if (run2->type == TupleT || run2->type == Tuple) {
                std::vector<std::string> ls;
                StringMap<DPtr<script::structures::ScriptAST>> cp;
                ls.reserve(run2->tuple.size());
                for (const auto& [k,v] : run2->tuple) {
                    ls.emplace_back(k);
                }
                remove_duplicates(ls);
                auto first = ls.begin() + begin;
                auto last = ls.begin() + end;
                while (first != last) {
                    cp[*first] = run2->tuple.at(*first);
                    if (!cp[*first]->db) cp[*first]->db = db;
                    first++;
                }
                return (run2->type == Tuple) ? tuple_(std::move(cp)) : tuple_type_(std::move(cp));
            } else {
                auto ls = run2->toList();
                auto first = ls.begin() + begin;
                auto last = ls.begin() + end;
                std::vector<DPtr<script::structures::ScriptAST>> newVec(first, last);
                return array_(std::move(newVec));
            }
        }

        case AddE:
            if (arrayList[0]->isDouble() || arrayList[1]->isDouble())
                return double_(arrayList[0]->toDouble()+arrayList[1]->toDouble());
            else
                return integer_(arrayList[0]->toInteger()+arrayList[1]->toInteger());

        case DivE:
            if (arrayList[0]->isDouble() || arrayList[1]->isDouble())
                return double_(arrayList[0]->toDouble()/arrayList[1]->toDouble());
            else
                return integer_(arrayList[0]->toInteger()/arrayList[1]->toInteger());

        case MinusE:
            if (arrayList[0]->isDouble() || arrayList[1]->isDouble())
                return double_(arrayList[0]->toDouble()-arrayList[1]->toDouble());
            else
                return integer_(arrayList[0]->toInteger()-arrayList[1]->toInteger());

        case MulE:
            if (arrayList[0]->isDouble() || arrayList[1]->isDouble())
                return double_(arrayList[0]->toDouble()*arrayList[1]->toDouble());
            else
                return integer_(arrayList[0]->toInteger()*arrayList[1]->toInteger());

        case SinE:
            return double_(std::sin(arrayList[0]->toDouble()));

        case CosE:
            return double_(std::cos(arrayList[0]->toDouble()));

        case TanE:
            return double_(std::tan(arrayList[0]->toDouble()));

        case ExpE: {
            double val = arrayList[1]->toDouble();
            if (val == 2.0) {
                return double_(std::exp2(arrayList[0]->toDouble()));
            } else if (val == M_E) {
                return double_(std::exp(arrayList[0]->toDouble()));
            } else {
                return double_(std::pow(arrayList[0]->toDouble(),val));
            }
        }
        case LogE:{
            double val = arrayList[1]->toDouble();
            if (val == 10.0) {
                return double_(std::log10(arrayList[0]->toDouble()));
            } else if (val == M_E) {
                return double_(std::log(arrayList[0]->toDouble()));
            } else {
                return double_(std::log(arrayList[0]->toDouble())/std::log(val));
            }
        }

        case FloorE:
            return double_(std::floor(arrayList[0]->toDouble()));

        case CeilE:
            return double_(std::ceil(arrayList[0]->toDouble()));

        case ConcatE: {
            auto f = arrayList[0]->run(implode)->toString(false,true);
            auto s = arrayList[1]->run(implode)->toString(false,true);
            return string_(f+s);
        }

        case GtE:
        {
            auto lx = arrayList[0]->run(implode);
            auto rx = arrayList[1]->run(implode);
            return lx->javaComparator(rx) > 0 ? true_() : false_();
        }

        case LtE:
        {
            auto lx = arrayList[0]->run(implode);
            auto rx = arrayList[1]->run(implode);
            return lx->javaComparator(rx) < 0 ? true_() : false_();
        }

        case GEqE:
        {
            auto lx = arrayList[0]->run(implode);
            auto rx = arrayList[1]->run(implode);
            return lx->javaComparator(rx) >= 0 ? true_() : false_();
        }

        case LEqE:
        {
            auto lx = arrayList[0]->run(implode);
            auto rx = arrayList[1]->run(implode);
            return lx->javaComparator(rx) <= 0 ? true_() : false_();
        }

        case NeqE:
        {
            auto lx = arrayList[0]->run(implode);
            auto rx = arrayList[1]->run(implode);
            return lx->javaComparator(rx) != 0 ? true_() : false_();
        }

        case PhiX: {
            size_t id = -1;
            ArrayList<DPtr<script::structures::ScriptAST>> v;
            if (arrayList[0]->isImmediateInteger()) {
                id = (size_t) arrayList[0]->toInteger();
            } else {
                auto it = arrayList[0]->run(implode);
                if ((it->type == t::NullE) || (it->type == t::String)) {
                    return script::structures::ScriptAST::array_(std::move(v));
                } else {
                    id = it->toInteger();
                }
            }
            auto label = arrayList[1]->toString(false,implode);
            if (db) {
                ;
//                const auto& m = db->phi(id);
//                auto it = m.find(label);
//                if (it != m.cend()) {
                    for (const auto& content2 : db->resolve_content((size_t)idxers->getNativeInt("graph"), id, label)) {
                        StringMap<DPtr<script::structures::ScriptAST>> content;
//                        ArrayList<DPtr<script::structures::ScriptAST>> item;
                        content["id"] = ((script::structures::ScriptAST::integer_((long long)content2.id)));
                        content["score"] = ((script::structures::ScriptAST::double_(content2.score)));
                        v.emplace_back(tuple_(std::move(content)));
                    }
//                }
            } else {
                std::cerr << "WARNING: No DB being associated. Returning an empty array for Phi" << std::endl;
            }
            return script::structures::ScriptAST::array_(std::move(v));
        }

        case VarPhiX:{
            size_t id = -1;
            ArrayList<DPtr<script::structures::ScriptAST>> v;
            if (arrayList[0]->isImmediateInteger()) {
                id = (size_t) arrayList[0]->toInteger();
            } else {
                auto it = arrayList[0]->run(implode);
                if ((it->type == t::NullE) || (it->type == t::String)) {
                    return script::structures::ScriptAST::array_(std::move(v));
                } else {
                    id = it->toInteger();
                }
            }
            if (db) {
                for (const std::string& label : db->phi_keys((size_t)idxers->getNativeInt("graph"), id)) {
                    for (const auto& content2 : db->resolve_content((size_t)idxers->getNativeInt("graph"), id, label)) {
                        v.emplace_back((script::structures::ScriptAST::integer_((long long)content2.id)));
                    }
                }
//                for (size_t val : db->varphi(id))
//                    v.emplace_back(script::structures::ScriptAST::integer_((long long)val));
            } else {
                std::cerr << "WARNING: No DB being associated. Returning an empty array for VarPhi" << std::endl;
            }
            return script::structures::ScriptAST::array_(std::move(v));
        }

        case EllX: {
            size_t id = -1;
            ArrayList<DPtr<script::structures::ScriptAST>> v;
            if (arrayList[0]->isImmediateInteger()) {
                id = (size_t) arrayList[0]->toInteger();
            } else {
                auto it = arrayList[0]->run(implode);
                if ((it->type == t::NullE) || (it->type == t::String)) {
                    return script::structures::ScriptAST::array_(std::move(v));
                } else {
                    id = it->toInteger();
                }
            }
            if (db) {
                for (const std::string& val :  db->resolve_ell((size_t)idxers->getNativeInt("graph"), id))
                    v.emplace_back(script::structures::ScriptAST::string_(val));
            } else {
                std::cerr << "WARNING: No DB being associated. Returning an empty array for Xi" << std::endl;
            }
            return script::structures::ScriptAST::array_(std::move(v));
//            auto id = (size_t) arrayList[0]->toInteger();
////            ArrayList<DPtr<script::structures::ScriptAST>> v;
//            if (db) {
//                return string_(db->ell(id));
//            } else {
//                std::cerr << "WARNING: No DB being associated. Returning an empty array for Ell" << std::endl;
//            }
//            return string_("");
        }

        case XiX:{
            size_t id = -1;
            ArrayList<DPtr<script::structures::ScriptAST>> v;
            if (arrayList[0]->isImmediateInteger()) {
                id = (size_t) arrayList[0]->toInteger();
            } else {
                auto it = arrayList[0]->run(implode);
                if ((it->type == t::NullE) || (it->type == t::String)) {
                    return script::structures::ScriptAST::array_(std::move(v));
                } else {
                    id = it->toInteger();
                }
            }
//            if (id == 3)
//                std::cout << "HERE" << std::endl;
            if (db) {
                for (const std::string& val : db->resolve_xi((size_t)idxers->getNativeInt("graph"), id))
                    v.emplace_back(script::structures::ScriptAST::string_(val));
            } else {
                std::cerr << "WARNING: No DB being associated. Returning an empty array for Xi" << std::endl;
            }
            return script::structures::ScriptAST::array_(std::move(v));
        }

        case ObjX: {
            size_t id = -1;
            StringMap<DPtr<script::structures::ScriptAST>> vv;
            if (arrayList[0]->isImmediateInteger()) {
                id = (size_t) arrayList[0]->toInteger();
            } else {
                auto it = arrayList[0]->run(implode);
                if ((it->type == t::NullE) || (it->type == t::String)) {
                    script::structures::ScriptAST::tuple_(std::move(vv));
                } else {
                    id = it->toInteger();
                }
            }
            if (!db) {
                std::cerr << "WARNING: No DB being associated. Returning an empty array for Xi" << std::endl;
            } else {
//                DPtr<script::structures::ScriptAST> ell;
                ArrayList<DPtr<script::structures::ScriptAST>> xi, ells, errs;
                for (const std::string& val : db->resolve_xi((size_t)idxers->getNativeInt("graph"), id))
                    xi.emplace_back(script::structures::ScriptAST::string_(val));
                for (const std::string& val : db->resolve_ell((size_t)idxers->getNativeInt("graph"), id))
                    ells.emplace_back(script::structures::ScriptAST::string_(val));
                vv["@xi"] = array_(std::move(xi));
                vv["@ell"] = array_(std::move(ells));
                StringMap<DPtr<script::structures::ScriptAST>> cont;
                for (const std::string& label : db->objProperties((size_t)idxers->getNativeInt("graph"), id)) {
//                    ArrayList<DPtr<script::structures::ScriptAST>> local;
                    auto val = db->resolve_GoodProp((size_t)idxers->getNativeInt("graph"), id, label);
                    if (val.has_value()) {
                        if (std::holds_alternative<std::string>(val.value())) {
                            cont[label] = string_(std::get<std::string>(val.value()));
                        } else {
                            cont[label] = double_(std::get<double>(val.value()));
                        }
                    }
//
//                    for (const auto& content2 : ) {
//                        StringMap<DPtr<script::structures::ScriptAST>> content;
//////                        ArrayList<DPtr<script::structures::ScriptAST>> item;
//                        content["id"] = ((script::structures::ScriptAST::integer_((long long)content2.id)));
//                        content["score"] = ((script::structures::ScriptAST::double_(content2.score)));
//                        local.emplace_back(tuple_(std::move(content)));
//
////                        v.emplace_back((script::structures::ScriptAST::integer_((long long)content2.id)));
//                    }
//                    cont[label] = array_(std::move(local));
                }
                vv["@pi"] = tuple_(std::move(cont));
//                for (const std::string& val : db->ell(id))
//                    ell.emplace_back(script::structures::ScriptAST::string_(val));
//                vv["@ell"] = array_(std::move(ell));
//                for (const double val : db->err(id))
//                    errs.emplace_back(script::structures::ScriptAST::double_(val));
//                vv["@err"] = array_(std::move(errs));
                for (const std::string& label : db->phi_keys((size_t)idxers->getNativeInt("graph"), id)) {
                    ArrayList<DPtr<script::structures::ScriptAST>> local;
                    for (const auto& content2 : db->resolve_content((size_t)idxers->getNativeInt("graph"), id, label)) {
                        StringMap<DPtr<script::structures::ScriptAST>> content;
////                        ArrayList<DPtr<script::structures::ScriptAST>> item;
                        content["id"] = ((script::structures::ScriptAST::integer_((long long)content2.id)));
                        content["score"] = ((script::structures::ScriptAST::double_(content2.score)));
                        local.emplace_back(tuple_(std::move(content)));

//                        v.emplace_back((script::structures::ScriptAST::integer_((long long)content2.id)));
                    }
                    vv[label] = array_(std::move(local));
                }
//                for (const auto& [k,v] : db->phi(id)) {
//                    ArrayList<DPtr<script::structures::ScriptAST>> local;
//                    for (const auto& content : v)
//                        local.emplace_back(std::move(fromObjectContent(content)));
//                    vv[k] = array_(std::move(local));
//                }
            }
            return script::structures::ScriptAST::tuple_(std::move(vv));
        }

        case ModE:
            return integer_(arrayList[0]->toInteger() % arrayList[1]->toInteger());

        case AbsE:
            if (arrayList[0]->isDouble())
                return double_(std::abs(arrayList[0]->toDouble()));
            else
                return integer_(std::abs(arrayList[0]->toInteger()));

        case CrossE: {
            auto lx = arrayList[0]->toList();
            auto rx = arrayList[1]->toList();
            ArrayList<DPtr<script::structures::ScriptAST>> result;
            for (const DPtr<script::structures::ScriptAST>& lptr : lx) {
                for (const DPtr<script::structures::ScriptAST>& rptr : rx) {
                    ArrayList<DPtr<script::structures::ScriptAST>> v;
                    v.emplace_back(lptr);
                    v.emplace_back(rptr);
                    result.emplace_back(script::structures::ScriptAST::array_(std::move(v)));
                }
            }
            return script::structures::ScriptAST::array_(std::move(result));
        }

        case SelfCrossE: {
            ArrayList<DPtr<script::structures::ScriptAST>> result;
            ArrayList<ArrayList<DPtr<script::structures::ScriptAST>>> v;
            for (const DPtr<script::structures::ScriptAST>& lptr : arrayList[0]->toList()) {
                v.emplace_back(lptr->toList());
            }
            for (ArrayList<DPtr<script::structures::ScriptAST>>& res : yaucl::iterators::cartesian(v)) {
                result.emplace_back(array_(std::move(res)));
            }
            return array_(std::move(result));
        }

        case SelfZipE: {
            ArrayList<DPtr<script::structures::ScriptAST>> result;
            ArrayList<ArrayList<DPtr<script::structures::ScriptAST>>> v;
            size_t len = std::numeric_limits<size_t>::max();
            for (const DPtr<script::structures::ScriptAST>& lptr : arrayList[0]->toList()) {
                auto ref = lptr->toList();
                if (!ref.empty()) {
                    len = std::min(len, ref.size());
                    v.emplace_back(ref);
                }
            }
            if (!v.empty()) {
                for (size_t i = 0; i<len; i++) {
                    ArrayList<DPtr<script::structures::ScriptAST>> zipped;
                    for (const ArrayList<DPtr<script::structures::ScriptAST>>& vv : v) {
                        zipped.emplace_back(vv[i]);
                    }
                    result.emplace_back(array_(std::move(zipped)));
                }
            }
            return array_(std::move(result));
        }

        case InjE: {
            ArrayList<DPtr<script::structures::ScriptAST>> v;
            v.emplace_back(arrayList[0]);
            return script::structures::ScriptAST::array_(std::move(v));
        }

        case FlatE: {
            ArrayList<DPtr<script::structures::ScriptAST>> v;
            for (const auto& ptr : toList()) {
                auto ls = ptr->toList();
                v.insert(v.end(), std::make_move_iterator(ls.begin()), std::make_move_iterator(ls.end()));
            }
            return script::structures::ScriptAST::array_(std::move(v));
        }

        case LFoldE: {
            auto f = arrayList[0]->toFunction();
            auto obj = arrayList[1];
            auto ls = arrayList[2]->toList();
            if (ls.empty())
                return obj;
            else {
                for (size_t i = 0, N = ls.size(); i<N; i++) {
                    ArrayList<DPtr<script::structures::ScriptAST>> v{obj, ls[i]};
                    obj = f(array_(std::move(v)));
                }
                return obj;
            }
        }

        case RFoldE: {
            auto f = arrayList[0]->toFunction();
            auto obj = arrayList[1];
            auto ls = arrayList[2]->toList();
            if (ls.empty())
                return obj;
            else {
                for (std::ptrdiff_t i = ls.size()-1; i>=0; i--) {
                    ArrayList<DPtr<script::structures::ScriptAST>> v{ls[i], obj};
                    obj = f(array_(std::move(v)));
                }
                return obj;
            }
        }

        case EvalE: {
//            if (casted_type.get()) {
//                if (casted_type->optGamma.get() != optGamma.get())
//                    casted_type->setOptGammaRecursively(optGamma);
//            } else
            {
//                std::stringstream ss;
                auto ss = arrayList[0]->run(implode)->toString(false,implode);
                DPtr<script::structures::ScriptAST> ptr;
                //std::cerr << ss.str() << std::endl;
                return script::compiler::ScriptVisitor::eval3(ss.data(), ss.length(),ptr, optGamma);
//                delete ((((scriptParser*)parser)));
            }
        }

        case TypeOf: {
            return arrayList[0]->typeInference();
        }

        case AssertE:
            if (!arrayList[0]->toBoolean())
                throw std::runtime_error(arrayList[0]->toString()+": condition does not hold!");
            return true_();

        case SigmaT:
            return binop_(optGamma, SigmaT, arrayList[0]->run(), arrayList[1]->run());

        case ANDT:
            return binop_(optGamma, ANDT, arrayList[0]->run(), arrayList[1]->run());

        case ORT:
            return binop_(optGamma, ORT, arrayList[0]->run(), arrayList[1]->run());

        case ENFORCET: {
            auto self = arrayList[0]->run();
            auto superType = arrayList[1]->run();
            bool return_result = false;
            // TODO: checking the admissibility of the enforcement!
            // TODO: if yes, then remember the inference step in the inference graph!
            script::compiler::ScriptVisitor::typecaster.addNewEdgeB(self->toString(), superType->toString(), [](const auto&& x) {
                return x;
            });
            return return_result ? true_() : false_();
        } break;

        case SubtypeOfE: {
            auto x = arrayList[0]->run();
            auto y = arrayList[1]->run();
            return subTyping(x, y).has_value() ? true_() : false_();
        } break;

        case CoerceE: {
            auto self = arrayList[0]->run();
            auto thisType = self->typeInference();
            auto superType = arrayList[1]->run();
            if (superType->type == SigmaT) {
                auto actualSuperType = superType->arrayList[0]->run();
                auto result = subTyping(thisType, actualSuperType);
                if (!result.has_value()) {
                    throw std::runtime_error("ERROR (2): term \"" + self->toString(true) + "\" cannot be casted to " + actualSuperType->toString() + " as the term is of type " + thisType->toString() );
                }
                if (superType->arrayList[1]->toFunction()(self)->toBoolean()) {
                    self = result.value()(std::move(self)); // Using the typecasting resulting from the inference step
                    self->casted_type = superType; // Setting this as the resulting type anyway
                    return self; // returning the casted object
                } else {
                    throw std::runtime_error("ERROR (2): term \"" + self->toString(true) + "\" cannot be casted to " + superType->toString() + " as the term does not satisfy its condition " );
                }
            }
            auto result = subTyping(thisType, superType);
            if (!result.has_value()) {
                throw std::runtime_error("ERROR: term \"" + self->toString(true) + "\" cannot be casted to " + superType->toString() + " as the term is of type " + thisType->toString() );
            } else {
                self = result.value()(std::move(self)); // Using the typecasting resulting from the inference step
                self->casted_type = superType; // Setting this as the resulting type anyway
                return self; // returning the casted object
            }
        }

    }
    return std::make_shared<ScriptAST>();
}

#include <limits>

template<class InputIt1, class InputIt2, class Compare>
std::ptrdiff_t int_lexicographical_compare(InputIt1 first1, InputIt1 last1,
                             InputIt2 first2, InputIt2 last2, Compare comp)
{
    for (; (first1 != last1) && (first2 != last2); ++first1, (void) ++first2)
    {
        auto val = (std::ptrdiff_t)comp(*first1, *first2);
        if (val != 0) return val;
    }

    bool hasFirstFinished = first1 == last1;
    if (hasFirstFinished && (first2 == last2))
        return 0;
    else if (hasFirstFinished)
        return -1;
    else return 1;
}

std::ptrdiff_t script::structures::ScriptAST::javaComparator(const DPtr<ScriptAST> &object) {
    if (!object.get()) return 1;
    std::ptrdiff_t cmp = ((std::ptrdiff_t)type) - ((std::ptrdiff_t)object->type);
    if (cmp != 0) return cmp;
    switch (type) {
        case BooleanT:
        case DoubleT:
        case IntegerT:
        case AnyT:
        case StringT:
        case StarT:
            return cmp;

        case Boolean:
            return ((std::ptrdiff_t)bool_) - ((std::ptrdiff_t)object->bool_);
        case Integer:
            return integer - object->integer;
        case Double:
            {
                double cmp = (doubleValue_ - object->doubleValue_);
                if (std::abs(cmp) <= std::numeric_limits<double>::epsilon()) return 0;
                else return (cmp > 0 ? 1 : -1);
            }
        case String:
        case LABELT:
            return string.compare(object->string);

        case Variable:
            return variableEval()->javaComparator(object->variableEval());

        case Array:
            return int_lexicographical_compare(arrayList.begin(), arrayList.end(),
                                               object->arrayList.begin(), object->arrayList.end(),
                                               [](const auto& x, auto& y) {return x->javaComparator(y);});

        case ObjTT:
        {
//            auto x = arrayList[0];
//            auto y = arrayList[1];
            return int_lexicographical_compare(arrayList.begin(), arrayList.end(),
                                               object->arrayList.begin(), object->arrayList.end(),
                                               [](const auto& x, auto& y) {return x->javaComparator(y);});
        }

        case Tuple:
        case TupleT: {
            std::vector<std::pair<std::string, DPtr<script::structures::ScriptAST>>> left, right;
            for (const auto&[k,v] : tuple) {
                left.emplace_back(k, v);
            }
            for (const auto&[k,v] : object->tuple) {
                right.emplace_back(k, v);
            }
            auto f = [](const std::pair<std::string, DPtr<script::structures::ScriptAST>>& x,
                         const std::pair<std::string, DPtr<script::structures::ScriptAST>>& y) {
                return (x.first < y.first) || ((x.first==y.first) && (x.second->javaComparator(y.second)));
            };
            std::sort(left.begin(), left.end(), f);
            std::sort(right.begin(), right.end(), f);
            return int_lexicographical_compare(left.begin(), left.end(),
                                               right.begin(), right.end(),
                                               f);
        }


        default:
            return int_lexicographical_compare(function->body.begin(), function->body.end(),
                                               object->function->body.begin(), object->function->body.end(),
                                               [](const auto& x, auto& y) {return x->javaComparator(y);});
    }
}

DPtr<script::structures::ScriptAST> script::structures::ScriptAST::typeInference() {
    if (type == Variable) {
        return variableEval()->typeInference();
    } else if (type == ObjX) {
        // Objects should be typed before being converted to a tuple, as they should be first class citizens
        return upTypeObjX(arrayList[0]->toInteger());
    }
    auto self = run();
    if (self->casted_type) {
        return self->casted_type;
    } else {
        DPtr<script::structures::ScriptAST> result;
        switch (self->type) {
            case Variable:
                result = self->variableEval()->typeInference();
                break;

            case AnyT:
            case BooleanT:
            case IntegerT:
            case DoubleT:
            case StringT:
            case TupleT:
            case ArrayOfT:
            case StarT:
            case SigmaT:
            case ANDT:
            case ORT:
            case LABELT:
                result = star_T();
                break;

            case Boolean:
                result = boolean_T();
                break;

            case Integer:
                result = integer_T();
                break;

            case Double:
                result = double_T();
                break;

            case String:
                result = string_T();
                break;

            case Tuple: {
                StringMap<DPtr<script::structures::ScriptAST>> type_tuple;
                for (const auto&[k,v] : self->tuple) {
                    auto cp = v;
                    if (!cp->db) cp->db = db;
                    type_tuple[k] = cp->typeInference();
                }
                result = tuple_type_(std::move(type_tuple));
                break;
            }

            case Array: {
                ArrayList<DPtr<script::structures::ScriptAST>> types;
                types.reserve(self->arrayList.size());
                for (const auto& val : self->arrayList) {
                    types.emplace_back(val->typeInference());
                }
                remove_duplicates(types, [](const auto& x, const auto& y) {
                    return x->javaComparator(y) < 0;
                });
                if (types.empty()) {
                    result =   array_type(any_T());
                } else if (types.size() > 1) {
                    throw std::runtime_error("ERROR: at this stage, I cannot unify distinct types together! This requires typing inference with relaxation!");
                } else {
                    result = array_type(std::move(types[0]));
                }

            }
            break;
            default:
                throw std::runtime_error("Cannot determine the type of an unevaluated expression! This would require a more expressive type system!");
        }
        casted_type = result;
        return casted_type;
    }

}

DPtr<script::structures::ScriptAST> script::structures::ScriptAST::upTypeObjX(size_t id)  {
    StringMap<DPtr<ScriptAST>> vv;
    if (!db) {
        std::cerr << "WARNING: No DB being associated. Returning an empty array for Xi" << std::endl;
    } else {
        auto phi_object = db->phi_keys((size_t)idxers->getNativeInt("graph"), id);
//        const auto& phi_object = db->resolve_content(id);
        if (phi_object.empty()) {
            auto objProps = db->objProperties((size_t)idxers->getNativeInt("graph"), id);
            const auto& XI = db->resolve_xi((size_t)idxers->getNativeInt("graph"), id);
            if (objProps.empty()) {
                void* parser{nullptr};
//                std::stringstream ss;
                const auto& s = db->resolve_ell((size_t)idxers->getNativeInt("graph"), id).at(0);
                DPtr<script::structures::ScriptAST> ptr;
//                auto lfst = ;
                auto lfst = compiler::ScriptVisitor::eval3(s.data(),s.length(), ptr, optGamma);
//                delete ((((scriptParser*)parser)));
                if (!lfst->isType()) {
                    ArrayList<DPtr<ScriptAST>> xi;
                    for (const std::string& val : XI) {
                        DPtr<script::structures::ScriptAST> ptr;
//                        std::stringstream ss2;
//                        ss2 << val;
                        DPtr<script::structures::ScriptAST> local = compiler::ScriptVisitor::eval3(val.data(),val.length(), ptr, optGamma);
                        xi.emplace_back(local->typeInference());
//                        delete ((((scriptParser*)parser)));
                    }
                    auto x = mgu(xi); // Actually this?
                    if (!x->isType())
                        throw std::runtime_error("ERROR: the xi of an object should be uniform!");
                    else
                        return x;
                } else
                    return lfst;
            } else {
//                std::stringstream ss;
                const auto& s =  db->resolve_ell((size_t)idxers->getNativeInt("graph"), id).at(0);
                DPtr<script::structures::ScriptAST> ptr;
//                DPtr<script::structures::ScriptAST> lfst;
                auto lfst =
                compiler::ScriptVisitor::eval3(s.data(),s.length(), ptr, optGamma);
//                delete ((scriptParser*)(ptr));
                if (!lfst->isType())
                    throw std::runtime_error("ERROR: the label of an object should express a type!");
                ArrayList<DPtr<ScriptAST>> xi;
                for (const std::string& val : XI) {
                    DPtr<script::structures::ScriptAST> ptr4;
//                    std::stringstream ss2;
//                    ss2 << val;
                    DPtr<script::structures::ScriptAST> lfst2 = compiler::ScriptVisitor::eval3(val.data(),val.size(), ptr4, optGamma);
                    xi.emplace_back(lfst2->typeInference());
//                    delete (scriptParser*)(ptr);
                }
                auto x = mgu(xi); // Actually this?
                if (x->type == TupleT) {
                    return lex_type_(std::move(lfst), std::move(x->tuple));
                } else {
                    return binop_(optGamma, ANDT, std::move(lfst), std::move(x));;
                }
            }
        } else {
            for (const std::string& label : phi_object) {
                auto v = db->resolve_content((size_t)idxers->getNativeInt("graph"), id, label);
                ArrayList<DPtr<ScriptAST>> local;
                for (const auto& content : v)
                    local.emplace_back(upTypeObjX(content.id));
                vv[label] = mgu(local);
            }
//            for (const auto& [k,v] : db->phi(id)) {
//                ArrayList<DPtr<ScriptAST>> local;
//                for (const auto& content : v)
//                    local.emplace_back(upTypeObjX(content.id));
//                vv[k] = mgu(local);
//            }
            void* ptr{nullptr};
//                    std::stringstream ss2;
//                    ss2 << val;
            DPtr<script::structures::ScriptAST>ptr4;

            const auto& s = db->resolve_ell((size_t)idxers->getNativeInt("graph"), id).at(0);
//            auto lfst =
            DPtr<script::structures::ScriptAST> lfst = compiler::ScriptVisitor::eval3(s.data(),s.length(), ptr4, optGamma);
            if (!lfst->isType())
                throw std::runtime_error("ERROR: the label of an object should express a type!");
            delete (scriptParser*)(ptr);
            return lex_type_(std::move(lfst), std::move(vv));
        }
    }
    return lex_type_(std::move(any_T()), std::move(vv));
}

std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>> script::structures::ScriptAST::subTyping(DPtr<script::structures::ScriptAST> &left, DPtr<script::structures::ScriptAST> &right) {
    constexpr auto f = [](DPtr<script::structures::ScriptAST> &&x) {
        return x;
    };
    if (left->javaComparator(right) == 0) return {f}; // if those are the same type, then by reflexivity they are the same type
    // TODO! check
    auto L = left->run();
    switch (L->type) {
        case ANDT: {
            /**
             *  T1 <: C /\ T2 <: C
             *  -------------------
             *     T1 /\ T2 <: C
             */
            auto leftType = L->arrayList[0]->run();
            auto rightType = L->arrayList[1]->run();
            auto lr = subTyping(leftType, right);
            auto rr = subTyping(rightType, right);
            if ((!(lr.has_value())) || (!(rr.has_value()))) {
                return {}; // no viable conversion
            } else {
                return {[&rr, &lr, &leftType, &rightType, this](DPtr<script::structures::ScriptAST> &&x) {
                    auto t = (x->typeInference());
                    auto st1 = subTyping(t, leftType);
                    if (st1.has_value())
                        return lr.value()(std::move(st1.value()(std::move(x))));
                    else //if (rr.has_value())
                    {
                        auto st2 = subTyping(t, rightType);
                        return rr.value()(std::move(st2.value()(std::move(x))));
                    }
                }};
            }
        }

        case ORT: {
            /**
             *  T1 <: C \/ T2 <: C
             *  -------------------
             *     T1 \/ T2 <: C
             */
            auto leftType = L->arrayList[0]->run();
            auto rightType = L->arrayList[1]->run();
            auto lr = subTyping(leftType, right);
            auto rr = subTyping(rightType, right);
            if ((!(lr.has_value())) && (!(rr.has_value()))) {
                return {}; // no viable conversion
            } else {
                return {[&rr, &lr, &leftType, &rightType, this](DPtr<script::structures::ScriptAST> &&x) {
                    auto t = (x->typeInference());
                    auto st1 = subTyping(t, leftType);
                    if (st1.has_value())
                        return lr.value()(std::move(st1.value()(std::move(x))));
                    else
                    {
                        auto st2 = subTyping(t, rightType);
                        return rr.value()(std::move(st2.value()(std::move(x))));
                    }
                }};
            }
        }

        case TypeOf: {
            throw std::runtime_error("Something unexpected: cannot make an inference for the current left operand to a type! this is weird...");
        }

        case AnyT:
            return (right->run()->type == AnyT) ?
            std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>>{f}
            : std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>>{}; // The most general type is only equal to itself!

        case StarT:
            return (right->run()->type == StarT)
            ? std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>>{f}
            : std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>>{}; // Similarly for sorts, sorts are only subtyped by themselves

        case LABELT: {
            auto right_run = right->run();
            if (right_run->type == AnyT) {
                return {f};
            } else if (right_run->type == LABELT) {
                return script::compiler::ScriptVisitor::isEnforcedWithCast(L, right_run);
            } else {
                throw std::runtime_error("ERROR: a label type could be only a subtype of Any or of any other enforced LabelT subtype");
            }
        } break;

        case ObjTT: {
            auto right_run = right->run();
            if (right_run->type == AnyT) {
                return {f};
            } else if (right_run->type == ObjTT) {
                // Not equal: this is already handled above!
                auto rewriteLeft = subTyping(L->arrayList[0], right_run->arrayList[0]);
                if (!rewriteLeft.has_value()) // If the types are not compatible, I cannot cast the type at all
                    return {};
                auto rewriteRight = subTyping(L->arrayList[1], right_run->arrayList[1]);
                if (!rewriteRight.has_value())
                    return {};
                return [&right_run,this](DPtr<script::structures::ScriptAST> &&x) {
                    x->casted_type = right_run;
                    for (const auto& [k,v] : right_run->arrayList[1]->tuple)
                        if (x->tuple.contains(k)) {
                            (x->tuple[k])->casted_type = v; // forcibly casting the null to v
                            if (!x->tuple[k]->casted_type->db) x->tuple[k]->casted_type->db = db;
                        }
                    return x;
                };
            } else {
                auto l = L->arrayList[0]->run();
                return subTyping(l, right_run);
            }
        } break;

        case BooleanT: {
            auto right_run = right->run();
            if (right_run->type == AnyT) {
                return {f};
            } else if (right_run->type == IntegerT) {
                return {[](DPtr<script::structures::ScriptAST> &&x) {
                    return script::structures::ScriptAST::integer_(x->toInteger());
                }};
            } else if (right_run->type == DoubleT) {
                return {[](DPtr<script::structures::ScriptAST> &&x) {
                    return script::structures::ScriptAST::double_(x->toDouble());
                }};
            } else if (right_run->type == LABELT) {
                auto test = script::compiler::ScriptVisitor::isEnforcedWithCast(L, right_run);
                if (test) return test;
                test = script::compiler::ScriptVisitor::isEnforcedWithCast(integer_T(), right_run);
                if (test) return test;
                test = script::compiler::ScriptVisitor::isEnforcedWithCast(double_T(), right_run);
                return test;
            } else
                return {};
        } break;

        case IntegerT: {
            auto right_run = right->run();
            if (right_run->type == AnyT) {
                return {f};
            } else if (right_run->type == DoubleT) {
                return {[](DPtr<script::structures::ScriptAST> &&x) {
                    return script::structures::ScriptAST::double_(x->toDouble());
                }};
            } else if (right_run->type == LABELT) {
                auto test = script::compiler::ScriptVisitor::isEnforcedWithCast(L, right_run);
                if (test) return test;
                test = script::compiler::ScriptVisitor::isEnforcedWithCast(double_T(), right_run);
                return test;
            } else
                return {};
        } break;

        case DoubleT: {
            auto right_run = right->run();
            if (right_run->type == AnyT) {
                return {f};
            } else if (right_run->type == LABELT) {
                auto test = script::compiler::ScriptVisitor::isEnforcedWithCast(L, right_run);
                if (test) return test;
                test = script::compiler::ScriptVisitor::isEnforcedWithCast(double_T(), right_run);
                return test;
            } else
                return {};
        } break;

        case StringT: {
            auto right_run = right->run();
            if (right_run->type == AnyT) {
                return {f};
            } else if (right_run->type == LABELT) {
                return script::compiler::ScriptVisitor::isEnforcedWithCast(L, right_run);
            } else
                return {};
        } break;

        case TupleT: {
            auto right_run = right->run();
            size_t enforcing = script::compiler::ScriptVisitor::isEnforced(L, right_run);
            if (enforcing != -1) {
                return {script::compiler::ScriptVisitor::typecaster.getEdgeLabel(enforcing)};
            }
            if (right_run->type != TupleT)
                return {};
            // A tuple is a subtype of the other if each label from the left appears on the
            // right and the corresponding types are sub-types
            for (const auto& [k,v] : L->tuple) {
                auto it = right_run->tuple.find(k);
                if (it == right_run->tuple.end())
                    return {};
                auto cp = v;
                if (!cp->db) cp->db = db;
                auto tmp = subTyping(cp, it->second);
                if (!tmp.has_value())
                    return {};
            }
            // Then, the super-typed tuple will have
            return {[&right_run,this](DPtr<script::structures::ScriptAST> &&x) {
                x->casted_type = right_run;
                for (const auto& [k,v] : right_run->tuple) {
                    if (!x->tuple.contains(k)) {
                        (x->tuple[k] = null_())->casted_type = v; // forcibly casting the null to v
                        if (!x->tuple[k]->db) x->tuple[k]->db = db;
                    }
                }
                return x;
            }};
        }
        case ArrayOfT: {
            auto right_run = right->run();
            size_t enforcing = script::compiler::ScriptVisitor::isEnforced(L, right_run);
            if (enforcing != -1) {
                return {script::compiler::ScriptVisitor::typecaster.getEdgeLabel(enforcing)};
            }
            // Using a type invariance which is actually correct, from Benjamin Pierce's book.
            return (L->arrayList[0]->javaComparator(right_run->arrayList[0]) == 0)
            ? std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>>{f}
            : std::optional<std::function<DPtr<script::structures::ScriptAST>(DPtr<script::structures::ScriptAST>&&)>>{};
        }

        case Boolean:
        case Integer:
        case String:
        case Double:
        case Assignment:
        case Function:
        case Array:
        case Tuple:
        case Variable:
        case AndE:
        case OrE:
        case NotE:
        case EqE:
        case IfteE:
        case ImplyE:
        case ApplyE:
        case AtE:
        case PutE:
        case RemoveE:
        case AppendE:
        case ContainsE:
        case FilterE:
        case InvokeE:
        case MapE:
        case SubElementsE:
        case AddE:
        case DivE:
        case ModE:
        case AbsE:
        case MinusE:
        case MulE:
        case SinE:
        case CosE:
        case TanE:
        case ExpE:
        case LogE:
        case FloorE:
        case CeilE:
        case ConcatE:
        case GtE:
        case ProjectE:
        case LtE:
        case GEqE:
        case LEqE:
        case NeqE:
        case PhiX:
        case VarPhiX:
        case EllX:
        case XiX:
        case ObjX:
        case CrossE:
        case SelfCrossE:
        case SelfZipE:
        case InjE:
        case FlatE:
        case LFoldE:
        case RFoldE:
        case EvalE:
        case AssertE:
        case ENFORCET:
        case SubtypeOfE:
        case NullE:
            throw std::runtime_error("ERROR: " + L->toString() +" is not a type!");
        case CoerceE:
            throw std::runtime_error("ERROR: coercing a type to be another type is not allowed!");

        case BotT: {
            // A null can always be cased
            auto R = right->run();
            if (!R->isType())
                return {};
            return {[&R](DPtr<script::structures::ScriptAST> &&x) {
                x->casted_type = R;
                return x;
            }};
        } break;

        case SigmaT: {
            // TODO: implement also the subtyping based on the predicate inclusion. Still, the two functions should be syntactally equivalent!
            auto R = right->run();
            if (R->type == AnyT)
                return {f};
            else if (R->type == SigmaT) {
                auto isSubTypeMain = subTyping(L->arrayList[0], R->arrayList[0]);
                if ((!isSubTypeMain.has_value()) || (L->arrayList[1]->javaComparator(R->arrayList[1])))
                    return {};
                else return {[&R](DPtr<script::structures::ScriptAST> &&x) {
                        x->casted_type = R;
                        return x;
                    }};
            } else
                return subTyping(L->arrayList[0], R);
        } break;
    }
    return {};
}

void
script::structures::ScriptAST::setOptGammaRecursively(DPtr<std::unordered_map<std::string, DPtr<ScriptAST>>> &gamma) {
    if (optGamma.get() != gamma.get()) {
        optGamma = gamma;
        for (auto & ref : arrayList)
            ref->setOptGammaRecursively(gamma);
        for (auto & [k,v] : tuple)
            v->setOptGammaRecursively(gamma);
        if (function)
            function->setContext(gamma);
//            if (optGamma)
//                for (auto & [k,v] : *optGamma)
//                    v->setOptGammaRecursively(gamma);
    }
}

void
script::structures::ScriptAST::setOptGammaRecursively(std::unordered_map<std::string, DPtr<ScriptAST>>&& gamma) {
    *optGamma = std::move(gamma);
    for (auto & ref : arrayList)
        ref->setOptGammaRecursively(optGamma);
    for (auto & [k,v] : tuple)
        v->setOptGammaRecursively(optGamma);
    if (function)
        function->setContext(optGamma);
//            if (optGamma)
//                for (auto & [k,v] : *optGamma)
//                    v->setOptGammaRecursively(gamma);
}