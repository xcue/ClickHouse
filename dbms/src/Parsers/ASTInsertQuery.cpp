#include <iomanip>
#include <Parsers/ASTInsertQuery.h>
#include <Parsers/ASTFunction.h>
#include <Common/quoteString.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int INVALID_USAGE_OF_INPUT;
}


void ASTInsertQuery::formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const
{
    frame.need_parens = false;

    settings.ostr << (settings.hilite ? hilite_keyword : "") << "INSERT INTO ";
    if (table_function)
    {
        settings.ostr << (settings.hilite ? hilite_keyword : "") << "FUNCTION ";
        table_function->formatImpl(settings, state, frame);
    }
    else
        settings.ostr << (settings.hilite ? hilite_none : "")
                      << (!database.empty() ? backQuoteIfNeed(database) + "." : "") << backQuoteIfNeed(table);

    if (columns)
    {
        settings.ostr << " (";
        columns->formatImpl(settings, state, frame);
        settings.ostr << ")";
    }

    if (select)
    {
        settings.ostr << " ";
        select->formatImpl(settings, state, frame);
    }
    else
    {
        if (!format.empty())
        {
            settings.ostr << (settings.hilite ? hilite_keyword : "") << " FORMAT " << (settings.hilite ? hilite_none : "") << format;
        }
        else
        {
            settings.ostr << (settings.hilite ? hilite_keyword : "") << " VALUES" << (settings.hilite ? hilite_none : "");
        }
    }

    if (settings_ast)
    {
        settings.ostr << (settings.hilite ? hilite_keyword : "") << "SETTINGS " << (settings.hilite ? hilite_none : "");
        settings_ast->formatImpl(settings, state, frame);
    }
}


static void tryFindInputFunctionImpl(const ASTPtr & ast, ASTPtr & input_function)
{
    if (!ast)
        return;
    for (const auto & child : ast->children)
        tryFindInputFunctionImpl(child, input_function);

    if (const auto * table_function_ast = ast->as<ASTFunction>())
    {
        if (table_function_ast->name == "input")
        {
            if (input_function)
                throw Exception("You can use 'input()' function only once per request.", ErrorCodes::INVALID_USAGE_OF_INPUT);
            input_function = ast;
        }
    }
}


void ASTInsertQuery::tryFindInputFunction(ASTPtr & input_function) const
{
    tryFindInputFunctionImpl(select, input_function);
}

}
