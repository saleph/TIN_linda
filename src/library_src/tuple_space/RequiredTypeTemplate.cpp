#include "tuple_space/RequiredTypeTemplate.h"

bool RequiredTypeTemplate::matches(const TupleElement &tuple_element)
{
    return tuple_element.get_type() == required_type;
}

RequiredTypeTemplate::RequiredTypeTemplate(TupleElement::Type required_type) : required_type(required_type)
{ }