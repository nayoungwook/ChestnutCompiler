#include "token.h"

Token::Token(token_type type, std::wstring const& identifier, int line_number)
	:type(type), identifier(identifier), line_number(line_number) {

}

std::wstring Token::get_identifier() {
	return identifier;
}

token_type Token::get_type() {
	return type;
}

int Token::get_line_number() {
	return line_number;
}