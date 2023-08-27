//Badger
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>

std::string cleanInput(const std::string txt, const char* exclude=" \n") {
	std::string ans="";
	bool has=false;
	for(char a : txt){
		has=false;
		for(int b=0;b<strlen(exclude);b++){
			if(!has){ has = a == exclude[b];}
			else{ break; }
		}
		if(!has){ ans += a; }
	}
	return ans;
}

// Define the tokens for our language
enum class TokenType {
	INT,		// Integer
	PLUS,		// Addition operator
	MINUS,		// Subtraction operator
	MUL,		// Multiplication operator
	DIV,		// Division operator
	MOD,		// Modulous operator
	LPAREN,		// Left parenthesis
	RPAREN,		// Right parenthesis
	EQUAL,		// Equal symbol
	VARIABLE,	// Variable name
	FUNC,		// Function
	FUNC_ARG,	// Function argument
	CMD_END,	// End of command
	END		// End of input
};

struct Token {
	TokenType type;
	std::string value;
};

struct ASTNode {
	TokenType type;
	std::string value;
	ASTNode* left;
	ASTNode* right;
};

class Lexer {
public:
	Lexer(const std::string& input) : input(input), position(0) {}

	// Lexical analysis: convert input into a stream of tokens
	std::vector<Token> Tokenize() {
		std::vector<Token> tokens;
		while (position < input.length()) {
			char currentChar = input[position];
			if (isdigit(currentChar)) {
				tokens.push_back(ScanNumber());
			} else if (currentChar == '+') {
				tokens.push_back({TokenType::PLUS, "+"});
				position++;
			} else if (currentChar == '-') {
				tokens.push_back({TokenType::MINUS, "-"});
				position++;
			} else if (currentChar == '*') {
				tokens.push_back({TokenType::MUL, "*"});
				position++;
			} else if (currentChar == '/') {
				tokens.push_back({TokenType::DIV, "/"});
				position++;
			} else if (currentChar == '%') {
				tokens.push_back({TokenType::MOD, "%"});
				position++;
			} else if (currentChar == '(') {
				tokens.push_back({TokenType::LPAREN, "("});
				position++;
			} else if (currentChar == ')') {
				tokens.push_back({TokenType::RPAREN, ")"});
				position++;
			} else if (currentChar == '=') {
				tokens.push_back({TokenType::EQUAL, "="});
				position++;
			} else if (isalpha(currentChar)) {
				std::string variableName;
				while (position < input.length() && (isalnum(currentChar) || currentChar == '_')) {
					variableName += currentChar;
					position++;
					currentChar = input[position];
				}
				try{
				if (input[position] == '(') {
					tokens.push_back({TokenType::FUNC, variableName});
					position++;
					currentChar=input[position];
					std::string funcArg;
					while (position < input.length()  && (isalnum(currentChar) || currentChar == '_')) {
						funcArg += currentChar;
						position++;
						currentChar = input[position];
					}
					tokens.push_back({TokenType::FUNC_ARG, funcArg});
					if(input[position] == ')'){position++;}
					else{std::cerr << "Missing ending parenthesis." << std::endl;}
				}
				else {
					tokens.push_back({TokenType::VARIABLE, variableName});
				}
				}
				catch(...){std::cerr << "Error attempting to locate function.\n\tLikely missing ending parenthesis after function argument." << std::endl;}
			} else if (currentChar == ';') {
				tokens.push_back({TokenType::CMD_END, ";"});
				position++;
			} else {
				// Handle invalid characters or unknown tokens here
				std::cerr << "Invalid character: " << currentChar << std::endl;
				exit(1);
			}
		}
		tokens.push_back({TokenType::END, ""});
		return tokens;
	}

private:
	Token ScanNumber() {
		std::string number;
		while (position < input.length() && isdigit(input[position])) {
			number += input[position];
			position++;
		}
		return {TokenType::INT, number};
	}

	std::string input;
	size_t position;
};

class Parser {
public:
	Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

	// Parse the tokens into an abstract syntax tree (AST)
	ASTNode* Parse() {
		return ParseStatement();
	}

private:
	// Helper functions for parsing
	ASTNode* ParseStatement();
	ASTNode* ParseFunctionCall();
	ASTNode* ParseExpression();
	ASTNode* ParseTerm();
	ASTNode* ParseFactor();

	const std::vector<Token>& tokens;
	size_t position;
	// Symbol table for variable assignments
	std::unordered_map<std::string, int> symbolTable;
};

int Evaluate(ASTNode* node);

ASTNode* Parser::ParseStatement() {
	Token token = tokens[position];
	if (token.type == TokenType::FUNC) {
		return ParseFunctionCall(); // Handle print statements
	}
	else if (token.type == TokenType::VARIABLE) {
		std::string identifier = token.value;
		position++;

		token = tokens[position];
		if (token.type == TokenType::EQUAL) {
			position++; // Consume the '=' token
			ASTNode* expression = ParseExpression();

			// Store the result of the expression in the symbol table
			symbolTable[identifier] = Evaluate(expression);

			// Return a dummy ASTNode for assignments
			ASTNode* assignmentNode = new ASTNode();
			assignmentNode->type = TokenType::EQUAL;
			return assignmentNode;
		} else {
			std::cerr << "Error: Expected '=' after identifier." << std::endl;
			exit(1);
		}
	} else {
		// If it's not an identifier, treat it as an expression
		return ParseExpression();
	}
}

ASTNode* Parser::ParseFunctionCall() {
	Token token = tokens[position];
	if(token.value == "sqrt"){
		//implement square root func., etc.
	}
}

ASTNode* Parser::ParseExpression() {
	ASTNode* left = ParseTerm();

	while (position < tokens.size()) {
		Token token = tokens[position];

		if (token.type == TokenType::PLUS || token.type == TokenType::MINUS) {
			position++;
			ASTNode* right = ParseTerm();

			ASTNode* expressionNode = new ASTNode();
			expressionNode->type = token.type;
			expressionNode->value = token.value;
			expressionNode->left = left;
			expressionNode->right = right;

			left = expressionNode;
		} else {
			break;
		}
	}

	return left;
}

ASTNode* Parser::ParseTerm() {
	ASTNode* left = ParseFactor();

	while (position < tokens.size()) {
		Token token = tokens[position];

		if (token.type == TokenType::MUL || token.type == TokenType::DIV || token.type == TokenType::MOD) {
			position++;
			ASTNode* right = ParseFactor();

			ASTNode* termNode = new ASTNode();
			termNode->type = token.type;
			termNode->value = token.value;
			termNode->left = left;
			termNode->right = right;

			left = termNode;
		} else {
			break;
		}
	}

	return left;
}

ASTNode* Parser::ParseFactor() {
	Token token = tokens[position++];

	if (token.type == TokenType::INT) {
		ASTNode* intNode = new ASTNode();
		intNode->type = token.type;
		intNode->value = token.value;
		intNode->left = nullptr;
		intNode->right = nullptr;

		return intNode;
	} else if (token.type == TokenType::LPAREN) {
		ASTNode* expressionNode = ParseExpression();

		// Check for a closing parenthesis
		if (position < tokens.size() && tokens[position].type == TokenType::RPAREN) {
			position++;  // Consume the closing parenthesis
			return expressionNode;
		} else {
			std::cerr << "Error: Missing closing parenthesis." << std::endl;
			exit(1);
		}
	} else {
		std::cerr << "Error: Unexpected token: " << token.value << std::endl;
		exit(1);
	}
}

// Evaluate the AST
int Evaluate(ASTNode* node) {
	if (node->type == TokenType::INT) {
		return std::stoi(node->value);
	} else if (node->type == TokenType::PLUS) {
		return Evaluate(node->left) + Evaluate(node->right);
	} else if (node->type == TokenType::MINUS) {
		return Evaluate(node->left) - Evaluate(node->right);
	} else if (node->type == TokenType::MUL) {
		return Evaluate(node->left) * Evaluate(node->right);
	} else if (node->type == TokenType::DIV) {
		return Evaluate(node->left) / Evaluate(node->right);
	} else if (node->type == TokenType::MOD) {
		return Evaluate(node->left) % Evaluate(node->right);
	} else {
		// Handle other cases if needed
		return 0;
	}
}

int main() {
	// Sample input code
	std::string code = "a = 2 + 3 * (4 - 2) / 2;\n5+9;\n(((5+9)-(4+5))*20)%24";
	std::cout << code << std::endl;
	code=cleanInput(code);

	std::cout << "\n--------------------\n" << std::endl;

	// Lexical analysis
	Lexer lexer(code);
	std::vector<Token> tokens = lexer.Tokenize();

	// Display the tokens
	for (const Token& token : tokens) {
		std::cout << "Token: " << static_cast<int>(token.type) << " Value: " << token.value << std::endl;
		if(token.value == ";" || token.value == ""){
			std::cout << "------------------" << std::endl;
		}
	}

	// Parsing and evaluation for multiple statements
	size_t startPosition = 0;
	while (startPosition < tokens.size()) {
		// Find the end position of the current statement
		size_t endPosition = startPosition;
		while (endPosition < tokens.size() && tokens[endPosition].type != TokenType::CMD_END) {
			endPosition++;
		}

		// Extract tokens for the current statement
		std::vector<Token> statementTokens(tokens.begin() + startPosition, tokens.begin() + endPosition);

		// Parsing
		Parser parser(statementTokens);
		ASTNode* root = parser.Parse();

		// Evaluate and display the result for the current statement
		int result = Evaluate(root);
		std::cout << "Result: " << result << std::endl;

		// Move the start position to the next statement
		startPosition = endPosition + 1; // Skip the semicolon token
	}

	return 0;
}
// TODO: step-by-step to solve problem
// TODO: substitute value for variable instead of just saying that the identifier "=" is not found.
// TODO: algebraic solving on both sides. ex.: a+2=2+3 => a=3:
	//allow for algebraic expression solving such as `a+2=2+3` where the output will be `a=3` since the 2's cancel out. this solving for both sides will apply for all operators.
// TODO: the parser should reduce to improper fractions when the output isn't an integer, but should also provide the approximated value as well as the improper fraction. you should make a function to simplify roots such as `sqrt(8)`=`2*sqrt(2)` because 8/2=4/2=2, so there's a pair of 2's and a leftover 2 in the sqrt.
// TODO:add functions such as: sqrt, log, and ln to the tokens.
// TODO: add global/pre-defined variables such as: i(imaginary) & e(euler's #)