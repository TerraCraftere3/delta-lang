#include "Parser.h"
#include "Tokenizer.h"
#include "Log.h"
#include "Error.h"

namespace Delta
{
    Parser::Parser(std::vector<Token> tokens) : m_tokens(tokens),
                                                m_allocator(1024 * 1024 * 4) // 4 MB
    {
    }

    std::optional<NodeStatement *> Parser::parseStatement()
    {
        std::optional<NodeStatement *> statement = std::nullopt;
        if (!peek().has_value())
        {
            exit(EXIT_FAILURE);
        }
        // Exit statement: exit(expr);
        if (peek().value().type == TokenType::exit && peek(2).has_value() && peek(2).value().type == TokenType::open_paren)
        {
            consume();
            auto open_paren = consume();
            if (auto node_expr = parseExpression())
            {
                auto *statement_exit = m_allocator.alloc<NodeStatementExit>();
                statement_exit->expression = node_expr.value();
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = statement_exit;
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", open_paren.line);
            try_consume(TokenType::semicolon, "';'", open_paren.line);
        }
        // while(expr){scope}
        if (peek().value().type == TokenType::while_)
        {
            consume();                                                       // while
            try_consume(TokenType::open_paren, "'('", peek(0).value().line); // (
            auto expr = parseExpression();                                   // expr
            if (!expr.has_value())
            {
                LOG_ERROR("Expected Expression in While Loop");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", peek(0).value().line); // )
            auto scope = parseScope();
            if (!scope.has_value())
            {
                LOG_ERROR("Expected Scope for While Loop");
                exit(EXIT_FAILURE);
            }
            auto statement_while = m_allocator.alloc<NodeStatementWhile>();
            statement_while->expr = expr.value();
            statement_while->scope = scope.value();
            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = statement_while;
            statement = stmt;
        }
        // Return statement: return expr?;
        else if (peek().value().type == TokenType::return_)
        {
            auto return_token = consume();
            auto *statement_return = m_allocator.alloc<NodeStatementReturn>();

            // Check if there's an expression (optional for void functions)
            if (peek().has_value() && peek().value().type != TokenType::semicolon)
            {
                if (auto node_expr = parseExpression())
                {
                    statement_return->expression = node_expr.value();
                }
                else
                {
                    LOG_ERROR("Invalid Expression in return statement");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                statement_return->expression = nullptr; // No return value
            }

            auto *stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = statement_return;
            statement = stmt;
            try_consume(TokenType::semicolon, "';'", return_token.line);
        }
        // Const variable declaration: const type identifier = expr;
        else if (peek(1).value().type == TokenType::let &&
                 peek(2).has_value() && peek(2).value().type == TokenType::const_ &&
                 peek(3).has_value() && peek(3).value().type == TokenType::identifier &&
                 peek(4).has_value() && peek(4).value().type == TokenType::colon &&
                 peek(5).has_value() && peek(5).value().type == TokenType::data_type &&
                 peek(6).has_value() && peek(6).value().type == TokenType::equals)
        {
            consume();                                                   // let
            consume();                                                   // const
            auto *statement_let = m_allocator.alloc<NodeStatementLet>(); //
            statement_let->ident = consume();                            // name
            statement_let->isConst = true;                               //
            consume();                                                   // :
            auto data_type = consume();                                  // type
            auto eq = consume();                                         // =
            statement_let->type = stringToType(data_type.value.value());
            if (auto node_expr = parseExpression())
            {
                statement_let->expression = node_expr.value();
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = statement_let;
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semicolon, "';'", eq.line);
        }
        // Variable declaration: let identifier: type = expr;
        else if (peek().value().type == TokenType::let &&
                 peek(2).has_value() && peek(2).value().type == TokenType::identifier &&
                 peek(3).has_value() && peek(3).value().type == TokenType::colon &&
                 peek(4).has_value() && peek(4).value().type == TokenType::data_type &&
                 peek(5).has_value() && peek(5).value().type == TokenType::equals)
        {
            consume();                                                   // let
            auto *statement_let = m_allocator.alloc<NodeStatementLet>(); //
            statement_let->ident = consume();                            // name
            statement_let->isConst = false;                              //
            consume();                                                   // :
            auto data_type = consume();                                  // type
            auto eq = consume();                                         // =
            statement_let->type = stringToType(data_type.value.value());
            if (auto node_expr = parseExpression())
            {
                statement_let->expression = node_expr.value();
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = statement_let;
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semicolon, "';'", eq.line);
        }
        // Assignment: identifier = expr
        else if (peek().value().type == TokenType::identifier && peek(2).has_value() && peek(2).value().type == TokenType::equals)
        {
            auto assign = m_allocator.alloc<NodeStatementAssign>();
            assign->ident = consume(); // identifier
            auto eq = consume();       // ==
            if (auto expr = parseExpression())
            {
                assign->expression = expr.value();
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semicolon, "';'", eq.line);
            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = assign;
            statement = stmt;
        }
        // Decrement: identifier--
        else if (peek().value().type == TokenType::identifier && peek(2).has_value() && peek(2).value().type == TokenType::minus && peek(3).has_value() && peek(3).value().type == TokenType::minus)
        {
            auto assign = m_allocator.alloc<NodeStatementAssign>();
            assign->ident = consume(); // identifier
            auto minus1 = consume();   // -
            auto plus2 = consume();    // -

            // Build expression: ident + 1
            auto expr = m_allocator.alloc<NodeExpression>();
            auto binop = m_allocator.alloc<NodeExpressionBinary>();
            auto subtraction = m_allocator.alloc<NodeExpressionBinarySubtraction>();

            // Left side = ident
            auto left = m_allocator.alloc<NodeExpression>();
            auto left_term = m_allocator.alloc<NodeExpressionTerm>();
            auto left_ident = m_allocator.alloc<NodeTermIdentifier>();
            left_ident->ident = assign->ident;
            left_term->var = left_ident;
            left->var = left_term;

            // Right side = 1
            auto right = m_allocator.alloc<NodeExpression>();
            auto right_term = m_allocator.alloc<NodeExpressionTerm>();
            auto one_lit = m_allocator.alloc<NodeTermIntegerLiteral>();
            one_lit->int_literal = {TokenType::int_literal, plus2.line, "1"};
            right_term->var = one_lit;
            right->var = right_term;

            binop->var = subtraction;
            subtraction->left = left;
            subtraction->right = right;
            expr->var = binop;

            assign->expression = expr;

            try_consume(TokenType::semicolon, "';'", plus2.line);

            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = assign;
            statement = stmt;
        }
        // Increment: identifier++
        else if (peek().value().type == TokenType::identifier && peek(2).has_value() && peek(2).value().type == TokenType::plus && peek(3).has_value() && peek(3).value().type == TokenType::plus)
        {
            auto assign = m_allocator.alloc<NodeStatementAssign>();
            assign->ident = consume(); // identifier
            auto plus1 = consume();    // +
            auto plus2 = consume();    // +

            // Build expression: ident + 1
            auto expr = m_allocator.alloc<NodeExpression>();
            auto binop = m_allocator.alloc<NodeExpressionBinary>();
            auto addition = m_allocator.alloc<NodeExpressionBinaryAddition>();

            // Left side = ident
            auto left = m_allocator.alloc<NodeExpression>();
            auto left_term = m_allocator.alloc<NodeExpressionTerm>();
            auto left_ident = m_allocator.alloc<NodeTermIdentifier>();
            left_ident->ident = assign->ident;
            left_term->var = left_ident;
            left->var = left_term;

            // Right side = 1
            auto right = m_allocator.alloc<NodeExpression>();
            auto right_term = m_allocator.alloc<NodeExpressionTerm>();
            auto one_lit = m_allocator.alloc<NodeTermIntegerLiteral>();
            one_lit->int_literal = {TokenType::int_literal, plus2.line, "1"};
            right_term->var = one_lit;
            right->var = right_term;

            binop->var = addition;
            addition->left = left;
            addition->right = right;
            expr->var = binop;

            assign->expression = expr;

            try_consume(TokenType::semicolon, "';'", plus2.line);

            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = assign;
            statement = stmt;
        }
        // Scope: { statements }
        else if (peek().value().type == TokenType::open_curly)
        {
            if (auto scope = parseScope())
            {
                auto stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = scope.value();
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Scope");
                exit(EXIT_FAILURE);
            }
        }
        // If statement: if(expr) scope pred?
        else if (auto if_ = try_consume(TokenType::if_))
        {
            try_consume(TokenType::open_paren, "'('", if_.value().line);
            auto stmt_if = m_allocator.alloc<NodeStatementIf>();
            if (auto expr = parseExpression())
            {
                stmt_if->expr = expr.value();
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", if_.value().line);
            if (auto scope = parseScope())
            {
                stmt_if->scope = scope.value();
            }
            else
            {
                LOG_ERROR("Invalid Scope");
                exit(EXIT_FAILURE);
            }
            stmt_if->pred = parseIfPred();
            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = stmt_if;
            statement = stmt;
        }
        // Array Assign: expr[expr] = expr
        // Pointer Assign: *ptr = value
        // Expression: a + b, func(), etc
        else if (auto expr = parseExpression())
        {
            // Expression: a + b, func(), etc
            if (auto semi = try_consume(TokenType::semicolon))
            {
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = expr.value();
                statement = stmt;
            }
            // Pointer Assign: *ptr = value
            else if (auto equals = try_consume(TokenType::equals))
            {
                if (auto value_expr = parseExpression())
                {
                    auto *stmt_assign = m_allocator.alloc<NodeStatementPointerAssign>();
                    stmt_assign->ptr_expr = expr.value();
                    stmt_assign->value_expr = value_expr.value();
                    auto *stmt = m_allocator.alloc<NodeStatement>();
                    stmt->var = stmt_assign;
                    statement = stmt;
                    try_consume(TokenType::semicolon, "';'", peek(0).value().line);
                }
                else
                {
                    LOG_ERROR("Expected Expression");
                    exit(EXIT_FAILURE);
                }
            }
            // Array Assign: expr[expr] = expr
            else if (auto open_square = try_consume(TokenType::open_square))
            {
                auto index_expr = parseExpression();
                if (!index_expr.has_value())
                {
                    LOG_ERROR("Expected Index Expression");
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_square, "']'", peek(0).value().line);
                try_consume(TokenType::equals, "'='", peek(0).value().line);
                auto value_expr = parseExpression();
                if (!value_expr.has_value())
                {
                    LOG_ERROR("Expected Value Expression");
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::semicolon, "';'", peek(0).value().line);
                auto *stmt_assign = m_allocator.alloc<NodeStatementArrayAssign>();
                stmt_assign->array_expr = expr.value();       // ptr
                stmt_assign->index_expr = index_expr.value(); // [index]
                stmt_assign->value_expr = value_expr.value(); // = value
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = stmt_assign;
                statement = stmt;
            }
            else
            {
                Error::throwExpected("'=', ';' or Array Acecss", peek(0).value().line);
            }
        }

        return statement;
    }

    std::optional<NodeProgram> Parser::parseProgram()
    {
        NodeProgram program;

        while (peek().has_value())
        {
            // external type name (parameters, ...);
            if (auto external_decl = parseExternalDeclaration())
            {
                program.externals.push_back(external_decl.value());
            }
            // type name (parameters, ...);
            else if (auto func_decl = parseFunctionDeclaration())
            {
                program.functions.push_back(func_decl.value());
            }
            // any statement
            else if (auto statement = parseStatement())
            {
                program.statements.push_back(statement.value());
            }
            else
            {
                LOG_ERROR("Invalid Statement or Function Declaration");
                exit(EXIT_FAILURE);
            }
        }
        return program;
    }

    std::optional<NodeFunctionDeclaration *> Parser::parseFunctionDeclaration()
    {
        // Check if this looks like a function: fn identifier(
        if (peek().has_value() && peek().value().type == TokenType::function &&
            peek(2).has_value() && peek(2).value().type == TokenType::identifier &&
            peek(3).has_value() && peek(3).value().type == TokenType::open_paren)
        {
            auto function_token = consume();
            auto function_name = consume();
            auto open_paren = consume();

            auto *func_decl = m_allocator.alloc<NodeFunctionDeclaration>();
            func_decl->function_name = function_name;

            // Parse parameter list
            if (peek().has_value() && peek().value().type != TokenType::close_paren)
            {
                if (auto param_list = parseParameterList())
                {
                    func_decl->parameters = param_list.value();
                }
                else
                {
                    LOG_ERROR("Invalid parameter list");
                    exit(EXIT_FAILURE);
                }
            }

            try_consume(TokenType::close_paren, "')'", open_paren.line);
            if (try_consume(TokenType::arrow_right).has_value())
            {
                auto return_type_token = consume();
                func_decl->return_type = stringToType(return_type_token.value.value());
            }
            else
            {
                func_decl->return_type = DataType::VOID; // Default to void
            }

            // Parse function body
            if (auto body = parseScope())
            {
                func_decl->body = body.value();
            }
            else
            {
                LOG_ERROR("Expected function body");
                exit(EXIT_FAILURE);
            }

            return func_decl;
        }

        return std::nullopt;
    }

    std::optional<NodeExternalDeclaration *>
    Parser::parseExternalDeclaration()
    {
        {
            // Check if this looks like a function: type identifier(
            if (peek(1).has_value() && peek(1).value().type == TokenType::external &&
                peek(2).has_value() && peek(2).value().type == TokenType::data_type &&
                peek(3).has_value() && peek(3).value().type == TokenType::identifier &&
                peek(4).has_value() && peek(4).value().type == TokenType::open_paren)
            {
                consume();                          // external
                auto return_type_token = consume(); // type
                auto function_name = consume();     // name
                auto open_paren = consume();        // (

                auto *external_decl = m_allocator.alloc<NodeExternalDeclaration>();
                external_decl->return_type = stringToType(return_type_token.value.value());
                external_decl->function_name = function_name;
                external_decl->is_variadic = false; // Default to false

                // Parse parameter list
                if (peek().has_value() && peek().value().type != TokenType::close_paren)
                {
                    while (true)
                    {
                        if (!peek().has_value() || (peek().value().type != TokenType::data_type && peek().value().type != TokenType::ellipsis))
                        {
                            LOG_ERROR("Expected parameter type or ellipsis (...)  in external function declaration");
                            exit(EXIT_FAILURE);
                        }

                        auto type_token = consume();
                        if (type_token.type == TokenType::ellipsis)
                        {
                            external_decl->is_variadic = true;
                            break; // ellipsis must be last paremeter
                        }
                        else
                        {
                            external_decl->parameters.push_back(stringToType(type_token.value.value()));
                        }

                        if (peek().has_value() && peek().value().type == TokenType::comma)
                        {
                            consume(); // ,
                            continue;
                        }

                        break;
                    }
                }

                try_consume(TokenType::close_paren, "')'", open_paren.line); // )
                try_consume(TokenType::semicolon, "';'", peek().value().line);

                return external_decl;
            }

            return std::nullopt;
        }
    }

    std::optional<std::vector<NodeParameter *>> Parser::parseParameterList()
    {
        std::vector<NodeParameter *> parameters;

        // Parse first parameter
        if (auto param = parseParameter())
        {
            parameters.push_back(param.value());
        }
        else
        {
            return std::nullopt;
        }

        // Parse remaining parameters (comma-separated)
        while (peek().has_value() && peek().value().type == TokenType::comma)
        {
            consume(); // consume comma
            if (auto param = parseParameter())
            {
                parameters.push_back(param.value());
            }
            else
            {
                LOG_ERROR("Expected parameter after comma");
                exit(EXIT_FAILURE);
            }
        }

        return parameters;
    }

    std::optional<NodeParameter *> Parser::parseParameter()
    {
        if (peek().has_value() && peek().value().type == TokenType::identifier &&
            peek(2).has_value() && peek(2).value().type == TokenType::colon &&
            peek(3).has_value() && peek(3).value().type == TokenType::data_type)
        {
            auto name_token = consume(); // name
            consume();                   // :
            auto type_token = consume(); // type

            auto *param = m_allocator.alloc<NodeParameter>();
            param->type = stringToType(type_token.value.value());
            param->ident = name_token;

            return param;
        }

        return std::nullopt;
    }

    std::optional<NodeScope *> Parser::parseScope()
    {
        if (auto open_curly = try_consume(TokenType::open_curly))
        {
            auto scope = m_allocator.alloc<NodeScope>();
            while (auto statement = parseStatement())
            {
                scope->statements.push_back(statement.value());
            }
            try_consume(TokenType::close_curly, "'}'", peek(0).value().line);
            return scope;
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<NodeIfPred *> Parser::parseIfPred()
    {
        if (auto _ = try_consume(TokenType::elif))
        {
            try_consume(TokenType::open_paren, "'('", _.value().line);
            auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (auto expr = parseExpression())
            {
                elif->expr = expr.value();
            }
            else
            {
                LOG_ERROR("Expected Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", _.value().line);
            if (auto scope = parseScope())
            {
                elif->scope = scope.value();
            }
            else
                Error::throwExpected("Scope", _.value().line);
            elif->pred = parseIfPred();
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = elif;
            return pred;
        }
        if (auto _ = try_consume(TokenType::else_))
        {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (auto scope = parseScope())
            {
                else_->scope = scope.value();
            }
            else
                Error::throwExpected("Scope", _.value().line);
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = else_;
            return pred;
        }
        return std::nullopt;
    }

    std::optional<NodeExpression *> Parser::parseExpression(int min_prec)
    {
        std::optional<NodeExpressionTerm *> term_lhs = parseTerm();
        if (!term_lhs.has_value())
        {
            return std::nullopt;
        }

        auto expr_lhs = m_allocator.alloc<NodeExpression>();
        expr_lhs->var = term_lhs.value();

        while (true)
        {
            std::optional<Token> curr_token = peek();
            std::optional<int> prec;
            if (curr_token.has_value())
            {
                prec = Tokenizer::getBinaryOPPrec(curr_token.value().type);
                if (!prec.has_value() || prec.value() < min_prec)
                    break;
            }
            else
            {
                break;
            }

            Token op = consume();

            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parseExpression(next_min_prec);
            if (!expr_rhs.has_value())
            {
                LOG_ERROR("Unable to parse Expression");
                exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.alloc<NodeExpressionBinary>();
            auto expr_lhs2 = m_allocator.alloc<NodeExpression>();
            if (op.type == TokenType::plus)
            {
                auto add = m_allocator.alloc<NodeExpressionBinaryAddition>();
                expr_lhs2->var = expr_lhs->var;
                add->left = expr_lhs2;
                add->right = expr_rhs.value();
                expr->var = add;
            }
            else if (op.type == TokenType::minus)
            {
                auto sub = m_allocator.alloc<NodeExpressionBinarySubtraction>();
                expr_lhs2->var = expr_lhs->var;
                sub->left = expr_lhs2;
                sub->right = expr_rhs.value();
                expr->var = sub;
            }
            else if (op.type == TokenType::slash)
            {
                auto div = m_allocator.alloc<NodeExpressionBinaryDivision>();
                expr_lhs2->var = expr_lhs->var;
                div->left = expr_lhs2;
                div->right = expr_rhs.value();
                expr->var = div;
            }
            else if (op.type == TokenType::star)
            {
                auto mult = m_allocator.alloc<NodeExpressionBinaryMultiplication>();
                expr_lhs2->var = expr_lhs->var;
                mult->left = expr_lhs2;
                mult->right = expr_rhs.value();
                expr->var = mult;
            }
            else if (op.type == TokenType::greater)
            {
                auto greater = m_allocator.alloc<NodeExpressionBinaryGreater>();
                expr_lhs2->var = expr_lhs->var;
                greater->left = expr_lhs2;
                greater->right = expr_rhs.value();
                expr->var = greater;
            }
            else if (op.type == TokenType::greater_equals)
            {
                auto greater_eq = m_allocator.alloc<NodeExpressionBinaryGreaterEquals>();
                expr_lhs2->var = expr_lhs->var;
                greater_eq->left = expr_lhs2;
                greater_eq->right = expr_rhs.value();
                expr->var = greater_eq;
            }
            else if (op.type == TokenType::less)
            {
                auto less = m_allocator.alloc<NodeExpressionBinaryLess>();
                expr_lhs2->var = expr_lhs->var;
                less->left = expr_lhs2;
                less->right = expr_rhs.value();
                expr->var = less;
            }
            else if (op.type == TokenType::less_equals)
            {
                auto less_eq = m_allocator.alloc<NodeExpressionBinaryLessEquals>();
                expr_lhs2->var = expr_lhs->var;
                less_eq->left = expr_lhs2;
                less_eq->right = expr_rhs.value();
                expr->var = less_eq;
            }
            else if (op.type == TokenType::double_equals)
            {
                auto eq = m_allocator.alloc<NodeExpressionBinaryEquals>();
                expr_lhs2->var = expr_lhs->var;
                eq->left = expr_lhs2;
                eq->right = expr_rhs.value();
                expr->var = eq;
            }
            else
            {
                assert(false);
            }
            expr_lhs->var = expr;
        }

        return expr_lhs;
    }

    std::optional<NodeExpressionTerm *> Parser::parseTerm()
    {
        // Integer literal: 42
        if (auto int_lit = try_consume(TokenType::int_literal))
        {
            auto term_int_lit = m_allocator.alloc<NodeTermIntegerLiteral>();
            term_int_lit->int_literal = int_lit.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_int_lit;
            return node_term;
        }
        // Float literal: 3.14f
        else if (auto float_lit = try_consume(TokenType::float_literal))
        {
            auto term_float_lit = m_allocator.alloc<NodeTermFloatLiteral>();
            term_float_lit->float_literal = float_lit.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_float_lit;
            return node_term;
        }
        // Double literal: 1.2345
        else if (auto double_lit = try_consume(TokenType::double_literal))
        {
            auto term_double_lit = m_allocator.alloc<NodeTermDoubleLiteral>();
            term_double_lit->double_literal = double_lit.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_double_lit;
            return node_term;
        }
        // String literal: "Hello World"
        else if (auto string_lit = try_consume(TokenType::string_literal))
        {
            auto term_string_lit = m_allocator.alloc<NodeTermStringLiteral>();
            term_string_lit->string_literal = string_lit.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_string_lit;
            return node_term;
        }
        // Char literal: 'A'
        else if (peek().has_value() && peek().value().type == TokenType::apostrophe &&
                 peek(2).has_value() && peek(2).value().type == TokenType::identifier &&
                 peek(3).has_value() && peek(3).value().type == TokenType::apostrophe)
        {
            consume();                           // '
            auto char_literal_token = consume(); // Char
            consume();                           // '
            auto char_str = char_literal_token.value.value();
            char c = char_str.at(0);
            int asciivalue = (int)c; // Convert to ascii

            Token int_lit;
            int_lit.type = TokenType::int_literal;
            int_lit.value = std::to_string(asciivalue);

            auto term_int_lit = m_allocator.alloc<NodeTermIntegerLiteral>();
            term_int_lit->int_literal = int_lit;
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_int_lit;
            return node_term;
        }
        // Cast: (type) value
        else if (peek().has_value() && peek().value().type == TokenType::open_paren &&
                 peek(2).has_value() && peek(2).value().type == TokenType::data_type &&
                 peek(3).has_value() && peek(3).value().type == TokenType::close_paren)
        {
            consume();                   // (
            auto type_token = consume(); // type
            consume();                   // )
            auto term_cast = m_allocator.alloc<NodeTermCast>();
            term_cast->target_type = stringToType(type_token.value.value());
            auto expr = parseExpression();
            if (!expr.has_value())
            {
                LOG_ERROR("Expected Expression after Cast");
                exit(EXIT_FAILURE);
            }
            term_cast->expr = expr.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_cast;
            return node_term;
        }
        // Function call: identifier(args)
        else if (peek().has_value() && peek().value().type == TokenType::identifier &&
                 peek(2).has_value() && peek(2).value().type == TokenType::open_paren)
        {
            auto function_name = consume();
            auto open_paren = consume();

            auto *func_call = m_allocator.alloc<NodeTermFunctionCall>();
            func_call->function_name = function_name;

            // Parse argument list
            if (peek().has_value() && peek().value().type != TokenType::close_paren)
            {
                if (auto arg_list = parseArgumentList())
                {
                    func_call->arguments = arg_list.value();
                }
                else
                {
                    LOG_ERROR("Invalid argument list");
                    exit(EXIT_FAILURE);
                }
            }

            try_consume(TokenType::close_paren, "')'", open_paren.line);

            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = func_call;
            return node_term;
        }
        // Identifier (variable)
        else if (auto id = try_consume(TokenType::identifier))
        {
            auto term_ident = m_allocator.alloc<NodeTermIdentifier>();
            term_ident->ident = id.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_ident;
            return node_term;
        }
        // Parenthesized expression: (expr)
        else if (auto open_paren = try_consume(TokenType::open_paren))
        {
            auto expr = parseExpression();
            if (!expr.has_value())
            {
                LOG_ERROR("Expected Expression after paren");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", open_paren.value().line);
            auto node_term_paren = m_allocator.alloc<NodeTermParen>();
            node_term_paren->expr = expr.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = node_term_paren;
            return node_term;
        }
        // Address of: &ident;
        else if (peek().has_value() && peek().value().type == TokenType::and_ &&
                 peek(2).has_value() && peek(2).value().type == TokenType::identifier)
        {
            consume();
            auto ident = consume();
            auto node_term_aof = m_allocator.alloc<NodeTermAddressOf>();
            node_term_aof->ident = ident;
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = node_term_aof;
            return node_term;
        }
        // Dereference: *ptr;
        else if (peek().has_value() && peek().value().type == TokenType::star)
        {
            consume();
            if (auto expr = parseExpression())
            {
                auto node_term_deref = m_allocator.alloc<NodeTermDereference>();
                node_term_deref->expr = expr.value();
                auto node_term = m_allocator.alloc<NodeExpressionTerm>();
                node_term->var = node_term_deref;
                return node_term;
            }
        }
        // Array: Access expr[index]
        /*else if (auto expr = parseExpression())
        {
            try_consume(TokenType::open_square, "'['", peek(0).value().line);
            auto index_expr = parseExpression();
            if (!index_expr.has_value())
            {
                LOG_ERROR("Expected Index Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_square, "']'", peek(0).value().line);
            auto node_term_access = m_allocator.alloc<NodeTermArrayAccess>();
            node_term_access->array_expr = expr.value();
            node_term_access->index_expr = index_expr.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = node_term_access;
            return node_term;
        }*/
        // STACK OVERFLOW BECAUSE OF TO MUCH RECURSION
        return std::nullopt;
    }

    std::optional<std::vector<NodeExpression *>> Parser::parseArgumentList()
    {
        std::vector<NodeExpression *> arguments;

        // Parse first argument
        if (auto expr = parseExpression())
        {
            arguments.push_back(expr.value());
        }
        else
        {
            return std::nullopt;
        }

        // Parse remaining arguments (comma-separated)
        while (peek().has_value() && peek().value().type == TokenType::comma)
        {
            consume(); // consume comma
            if (auto expr = parseExpression())
            {
                arguments.push_back(expr.value());
            }
            else
            {
                LOG_ERROR("Expected expression after comma in argument list");
                exit(EXIT_FAILURE);
            }
        }

        return arguments;
    }

    std::optional<Token> Parser::peek(int count) const
    {
        if (m_position + count > m_tokens.size())
            return std::nullopt;
        return m_tokens.at(m_position + count - 1);
    }

    Token Parser::consume()
    {
        return m_tokens.at(m_position++);
    }

    Token Parser::try_consume(TokenType type, const std::string &c, int line, int row)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            Error::throwExpected(c, line, row);
        }
        return {};
    }

    std::optional<Token> Parser::try_consume(TokenType type)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            return std::nullopt;
        }
    }
}