#include "tuple_space/TupleTemplate.h"
#include "tuple_space/IntegerComparerTemplate.h"
#include "tuple_space/StringComparerTemplate.h"
#include "tuple_space/parsing/exceptions/ParseException.h"
#include "tuple_space/parsing/TupleTemplateElementParser.h"


TupleTemplateElementParser::TupleTemplateElementParser(std::unique_ptr<Scanner> scanner)
        : scanner(std::move(scanner))
{
    // move the scanner to the first position in the inner stream
    advance();
}

std::unique_ptr<TupleTemplate> TupleTemplateElementParser::parse()
{
    skip(PunctuationMark::LeftParenthesis);
    std::vector<std::unique_ptr<TupleTemplateElement>> elements;
    do
    {
        elements.push_back(parse_tuple_template_element());
    } while (try_skip_comma());
    skip(PunctuationMark::RightParenthesis);
    return std::make_unique<TupleTemplate>(std::move(elements));
}

std::unique_ptr<TupleTemplateElement> TupleTemplateElementParser::parse_tuple_template_element()
{
    TupleElement::Type required_type = read_type();

    skip(PunctuationMark::Colon);
    const Token &token = scanner->get_token();
    if (is(PunctuationMark::Star))
    {
        skip(PunctuationMark::Star);
        return std::make_unique<RequiredTypeTemplate>(required_type);
    }
    if (is_operator())
        return parse_comparer_template(required_type);

    return parse_equal_template(required_type);
}

TupleElement::Type TupleTemplateElementParser::read_type() const
{
    const Token& token = scanner->get_token();
    if (token.get_type() != Token::Type::Identifier)
        throw ParseException("Identifier expected");

    const std::string identifier = token.get_identifier();
    if (identifier != "string" && identifier != "integer")
        throw ParseException("Identifier must be integer or string");

    advance();
    return identifier == "string" ? TupleElement::Type::String : TupleElement::Type::Integer;
}


void TupleTemplateElementParser::advance() const
{
    scanner->read_next();
}

bool TupleTemplateElementParser::is(PunctuationMark punctuation_mark) const
{
    const auto token = scanner->get_token();
    return token.get_type() == Token::Type::PunctuationMark && token.get_punctuation_mark() == punctuation_mark;
}

void TupleTemplateElementParser::skip(PunctuationMark punctuation_mark) const
{

    if (is(punctuation_mark))
        advance();
    else if (is_eof())
        throw EndOfFile("End of file at line: " + std::to_string(scanner->get_current_line()));
    else
        throw ParseException("Parse error: " + to_string(punctuation_mark) + " expected at line "
                             + std::to_string(scanner->get_current_line()));
}

bool TupleTemplateElementParser::is_eof() const
{
    const auto token = scanner->get_token();
    return token.get_type() == Token::Type::Eof;
}

bool TupleTemplateElementParser::try_skip_comma() const
{
    if (is(PunctuationMark::Comma))
    {
        skip(PunctuationMark::Comma);
        return true;
    }
    return false;
}

bool TupleTemplateElementParser::is_operator() const
{
    return scanner->get_token().get_type() == Token::Type::Operator;
}

std::unique_ptr<TupleTemplateElement> TupleTemplateElementParser::parse_comparer_template(TupleElement::Type type)
{
    const Operator operator_ = scanner->get_token().get_operator();
    advance();
    if (type == TupleElement::Type::String)
        return parse_string_comparer_template(operator_);

    if (type == TupleElement::Type::Integer)
        return parse_integer_comparer_template(operator_);

    throw std::invalid_argument("type");
}

std::unique_ptr<TupleTemplateElement> TupleTemplateElementParser::parse_integer_comparer_template(
        const Operator &operator_) const
{
    if (scanner->get_token().get_type() != Token::Type::Integer)
        throw ParseException("Parse error: integer expected at line "
                             + std::__cxx11::to_string(scanner->get_current_line()));

    const int to_compare = scanner->get_token().get_integer();
    advance();
    return std::make_unique<IntegerComparerTemplate>(operator_, to_compare);
}

std::unique_ptr<TupleTemplateElement> TupleTemplateElementParser::parse_string_comparer_template(
        const Operator &operator_) const
{
    if (scanner->get_token().get_type() != Token::Type::StringLiteral)
        throw ParseException("Parse error: string literal expected at line "
                             + std::to_string(scanner->get_current_line()));

    const std::string to_compare = scanner->get_token().get_string_literal();
    advance();
    return std::make_unique<StringComparerTemplate>(operator_, to_compare);
}

std::unique_ptr<TupleTemplateElement> TupleTemplateElementParser::parse_equal_template(TupleElement::Type type)
{
    if (type == TupleElement::Type::Integer)
        return parse_integer_comparer_template(Operator::Equal);

    if (type == TupleElement::Type::String)
        return parse_string_comparer_template(Operator::Equal);

    throw std::invalid_argument("type");
}
