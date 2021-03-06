#include "tokenizer.hpp"

using token_type = tokenizer::type_t;

void skip_space_and_comment(std::string_view &src){
	if(src.empty()) return;
	switch(src[0]){
		// space
		case ' ': case '\t': case '\n':
			src.remove_prefix(1);
			skip_space_and_comment(src);
			break;
		case '/':
			if(src[1] == '/'){
				size_t i;
				for(i=2; i<src.size(); i++){
					if(src[i] == '\n') break;
				}
				src.remove_prefix(i);
				skip_space_and_comment(src);
			}else if(src[1] == '*'){
				size_t i;
				for(i=3; i<src.size(); i++){
					if(src[i-1] == '*' || src[i] == '/') break;
				}
				src.remove_prefix(i+1);
				skip_space_and_comment(src);
			}
			return;
		default:
			return;
	}
}

tokenizer::tokenizer_t tokenizer::sat = [](std::string_view &src){
	token_t token;
	static std::vector<char> delim_char = {
		// 1文字で区切れるやつ
		' ', '\t', '\n',
		'[', ']',
		'{', '}',
		'(', ')',
		',', ';',
		'?'
	};
	static std::vector<std::string> delim_str = {
		"::", ":",
		"<<=", "<<", "<=", "<-", "<",
		">>=", ">>", ">=", ">"
		"->", "--", "-=", "-",
		"++", "+=", "+",
		"&&", "&=", "&",
		"||", "|=", "|",
		"*=", "*",
		"/=", "/",
		"%=", "%",
		"!=", "!",
		"^=", "^",
		"~=", "~",
	};

	auto check_token_type = [&]{
		auto& t = token.type = token_type::Identifier;
		if(token.empty()) return;
		if(token == "fn") t = token_type::Function;
		else if(token == "if") t = token_type::If;
		else if(token == "loop") t = token_type::Loop;
		else if(token == "return") t = token_type::Return;
		else if(token == "import") t = token_type::Import;
		else if(token[0] == '0'){
			if(token.size() == 1) // ただの'0'
				t = token_type::Decimal;
			else if(token[1] == 'x') // 16進数
				t = token_type::Hex;
			else
				t = token_type::Octal;
		}else if('0'<token[0] && token[0]<='9'){
			t = token_type::Decimal;
			for(const auto& c : token){
				if('0'<c && c<='9'){}
				else{
					t = token_type::Identifier;
					break;
				}
			}
		}
	};

	skip_space_and_comment(src); // 空白とコメントをスキップ

	for(size_t i=0; i<src.size(); i++){
		// 文字列
		if(src[i] == '\'' || src[i] == '\"'){
			if(i != 0) goto default_token;
			size_t siz;
			for(siz=1;siz<src.size();siz++){
				if(src[siz] == '\'' || src[siz] == '\"'){
					if(src[siz-1] == '\\') continue;
					siz++;
					break;
				}
			}
			token = src.substr(0, siz);
			src.remove_prefix(siz);
			token.type = token_type::String;
			return token;
		}

		// 1文字で区切れるやつ
		for(const auto& dc : delim_char){
			if(src[i] != dc) continue;
			if(i != 0) goto default_token;
			token = src.substr(0, 1);
			src.remove_prefix(1);
			token.type = token_type::Delim;
			return token;
		}

		// 複数文字で区切れるやつ
		for(const auto& ds : delim_str){
			token = src.substr(i, ds.size());
			if(ds != token) continue;
			if(i != 0) goto default_token;
			src.remove_prefix(ds.size());
			token.type = (token==":" ? token_type::Delim : token_type::Operator);
			return token;
		}

		// 区切り文字の時はdefault_tokenに行かない
		continue;
default_token:
		token = src.substr(0, i);
		check_token_type();
		src.remove_prefix(i);
		return token;
	}

	return token;
};
