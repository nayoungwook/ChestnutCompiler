#include "../parser/cn_parser.h"

wchar_t get_char(std::wstring& line, int& _i, int line_number) {

	if (line.size() <= _i) {
		std::wstring w_line;
		w_line.assign(line.begin(), line.end());
		CHESTNUT_THROW_ERROR(L"Unexpected error at : " + w_line, "UNEXPECTED_ERROR", "002", _i);
	}

	if (_i + 1 < line.length()) {
		next_char = line[_i + 1];
	}
	return cur_char = line[_i++];
}

std::wstring get_number_literal(std::wstring& identifier, std::wstring& line, int& i, int line_number) {
	std::wstring number_string = L"";

	while (isdigit(cur_char) || cur_char == '.') {
		number_string += cur_char;

		if (next_char == 'f') {
			number_string += 'f';
			i++;
			break;
		}

		if ((isdigit(next_char) || next_char == '.'))
			get_char(line, i, line_number);
		else
			break;
	}

	identifier = number_string;
	return number_string;
}

bool is_special_characters(const wchar_t& wc) {
	if (wc == L'_') return false;

	if ((wc >= L'!' && wc <= L'/') ||
		(wc >= L':' && wc <= L'@') ||
		(wc >= L'[' && wc <= L'`') ||
		(wc >= L'{' && wc <= L'~')) {
		return true;
	}
	return false;
}

std::vector<Token*> tokenize(std::wstring line, int line_number) {
	std::vector<Token*> result_tokens;

	int type = 0;
	std::wstring identifier = L"";
	double number = 0;

	int i = 0;

	while (i < line.length()) {
		identifier = L"";

		get_char(line, i, line_number);


		if (isspace(cur_char)) continue;

		if (!(is_special_characters(cur_char) || isdigit(cur_char) || cur_char == ' ')) {
			type = tok_identifier;

			while (!(is_special_characters(next_char) || isdigit(next_char) || next_char == ' ')) {
				if (i >= line.length()) break;
				identifier += cur_char;
				get_char(line, i, line_number);
			}
			identifier += cur_char;

			if (identifier == L"func") type = tok_func;
			if (identifier == L"if") type = tok_if;
			if (identifier == L"else") type = tok_else;
			if (identifier == L"for") type = tok_for;
			if (identifier == L"while") type = tok_while;

			if (identifier == L"number") type = tok_number;
			if (identifier == L"int") type = tok_int;
			if (identifier == L"float") type = tok_float;
			if (identifier == L"bool") type = tok_bool;
			if (identifier == L"char") type = tok_char;
			if (identifier == L"string") type = tok_string;
			if (identifier == L"array") type = tok_array;

			if (identifier == L"cast") type = tok_cast;

			if (identifier == L"public") type = tok_pubilc;
			if (identifier == L"private") type = tok_private;
			if (identifier == L"protected") type = tok_protected;
			if (identifier == L"static") type = tok_static;
			if (identifier == L"const") type = tok_const;

			if (identifier == L"return") type = tok_return;
			if (identifier == L"class") type = tok_class;

			if (identifier == L"new") type = tok_new;
			if (identifier == L"var") type = tok_var;

			if (identifier == L"true") type = tok_true;
			if (identifier == L"false") type = tok_false;
			if (identifier == L"extends") type = tok_extends;
			if (identifier == L"constructor") type = tok_constructor;
			if (identifier == L"scene") type = tok_scene;
			if (identifier == L"object") type = tok_object;
			if (identifier == L"vector") type = tok_vector;
		}
		else if (isdigit(cur_char)) {
			type = tok_number_literal;
			get_number_literal(identifier, line, i, line_number);
		}
		else if (cur_char == L'\"') {
			type = tok_string_identifier;
			bool is_inner_quote = false;

			identifier = L"";

			while (true) {
				identifier += cur_char;

				if (cur_char == L'\\' && next_char == L'\"') {
					is_inner_quote = true;
				}

				get_char(line, i, line_number);

				if (cur_char == L'\"') {
					if (is_inner_quote) {
						is_inner_quote = false;
						continue;
					}
					else {
						identifier += cur_char;
						break;
					}
				}
			}
		}
		// TODO : remove this comment token it occurs error
		else if (cur_char == L'/' && next_char == L'/') {
			type = tok_comment;
			identifier = L"(comment)";
			//result_tokens.push_back(new Token(type, identifier, number, line_number));
			return result_tokens;
		}
		else if (cur_char == L'&') {
			if (next_char == L'&') {
				type = tok_and;
				identifier = L"&&";
				i++;
			}
		}
		else if (cur_char == L'|') {
			if (next_char == L'|') {
				type = tok_or;
				identifier = L"||";
				i++;
			}
		}
		else if (cur_char == L'=') {
			if (next_char == L'=') {
				type = tok_equal;
				identifier = L"==";
				i++;
			}
			else {
				type = tok_assign;
				identifier = L"=";
			}
		}
		else if (cur_char == L'.') {
			type = tok_dot;
			identifier = L".";
		}
		else if (cur_char == L'<') {
			if (next_char == L'=') {
				type = tok_eq_greater;
				identifier = L"<=";
				i++;
			}
			else {
				type = tok_greater;
				identifier = L"<";
			}
		}
		else if (cur_char == L'>') {
			if (next_char == L'=') {
				type = tok_eq_lesser;
				identifier = L">=";
				i++;
			}
			else {
				type = tok_lesser;
				identifier = L">";
			}
		}
		else if (cur_char == L'!') {
			if (next_char == L'=') {
				type = tok_not_equal;
				identifier = L"!=";
				i++;
			}
			else {
				type = tok_not;
				identifier = L"!";
			}
		}
		else if (cur_char == L')') {
			type = tok_r_paren;
			identifier = L")";
		}
		else if (cur_char == L'(') {
			type = tok_l_paren;
			identifier = L"(";
		}
		else if (cur_char == L'}') {
			type = tok_r_bracket;
			identifier = L"}";
		}
		else if (cur_char == L'{') {
			type = tok_l_bracket;
			identifier = L"{";
		}
		else if (cur_char == L']') {
			type = tok_r_sq_bracket;
			identifier = L"]";
		}
		else if (cur_char == L'[') {
			type = tok_l_sq_bracket;
			identifier = L"[";
		}
		else if (cur_char == L'^') {
			type = tok_pow;
			identifier = L"^";
		}
		else if (cur_char == L'+') {
			if (next_char == L'+') {
				type = tok_incre;
				identifier = L"++";
				i++;
			}
			else if (next_char == L'=') {
				type = tok_bin_incre;
				identifier = L"+=";
				i++;
			}
			else {
				type = tok_pls;
				identifier = L"+";
			}
		}
		else if (cur_char == L'-') {
			if (next_char == L'-') {
				type = tok_decre;
				identifier = L"--";
				i++;
			}
			else if (next_char == L'=') {
				type = tok_bin_decre;
				identifier = L"-=";
				i++;
			}
			else {
				type = tok_min;
				identifier = L"-";
			}
		}
		else if (cur_char == L'*') {
			if (next_char == L'=') {
				type = tok_bin_mul;
				identifier = L"*=";
				i++;
			}
			else {
				type = tok_mul;
				identifier = L"*";
			}
		}
		else if (cur_char == L'/') {
			if (next_char == L'=') {
				type = tok_bin_div;
				identifier = L"/=";
				i++;
			}
			else {
				type = tok_div;
				identifier = L"/";
			}
		}
		else if (cur_char == L'#') {

			if (next_char == L' ') {
				type = tok_sharp;
				identifier = L"#";
			}
			else {
				while (isalnum(next_char) || next_char == L'_') {
					if (i >= line.length()) break;
					identifier += cur_char;
					get_char(line, i, line_number);
				}
				identifier += cur_char;

				if (identifier == L"#import")
					type = tok_import;
				else if (identifier == L"#load")
					type = tok_load;
				else if (identifier == L"#font")
					type = tok_font;
				else {
					type = tok_option;
				}
			}

		}
		else if (cur_char == L';') {
			type = tok_semi_colon;
			identifier = L";";
		}
		else if (cur_char == L':') {
			type = tok_colon;
			identifier = L":";
		}
		else if (cur_char == L',') {
			type = tok_comma;
			identifier = L",";
		}
		else if (cur_char == L'%') {
			type = tok_mod;
			identifier = L"%";
		}

		result_tokens.push_back(new Token(type, identifier, line_number));
	}

	return result_tokens;
}
