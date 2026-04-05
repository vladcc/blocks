.PHONY: help
help: _help

AWK := awk
CMPL := g++
EXTRA_FLAGS :=
INCL_PATHS := -I ./src/matcher -I ./src/lexer -I ./src/parser
INCL_PATHS += -I ./src/parse_opts -I ./src/cli_opts
WARN_FLAGS := -Wall -Wfatal-errors
FLAGS := $(INCL_PATHS) $(WARN_FLAGS) $(EXTRA_FLAGS)

SRC_DIR := ./src
OBJ_DIR := ./obj

.PHONY: release
release: FLAGS += -O3
release: clean compile

.PHONY: debug
debug: FLAGS += -g
debug: clean compile

# <parse_opts>
PARSE_OPTS_SRC_DIR := $(SRC_DIR)/parse_opts

PARSE_OPTS_GEN := $(PARSE_OPTS_SRC_DIR)/parse-opts-gen.awk
PARSE_OPTS_BASE := parse_opts
PARSE_OPTS_SRC := $(PARSE_OPTS_SRC_DIR)/$(PARSE_OPTS_BASE).c
PARSE_OPTS_O := $(OBJ_DIR)/$(PARSE_OPTS_BASE).o
$(PARSE_OPTS_O): $(PARSE_OPTS_SRC)
	$(CMPL) -c $< -o $@ $(FLAGS)
# </parse_opts>

# <cli_opts>
CLI_SRC_DIR := $(SRC_DIR)/cli_opts

CLI_DEFN_SRC := $(CLI_SRC_DIR)/cli_opts_defn.txt
CLI_DEFN_IC := $(CLI_SRC_DIR)/opts_definitions.ic
CLI_PROC_IC := $(CLI_SRC_DIR)/opts_process.ic
CLI_IMPL_IC := $(CLI_SRC_DIR)/opts_impl.ic
CLI_GEN := $(AWK) -f $(PARSE_OPTS_GEN) -vOutDir="$(CLI_SRC_DIR)" $(CLI_DEFN_SRC)

.PHONY: cli_opts
cli_opts: $(CLI_DEFN_IC)

# regenerates both CLI_DEFN_IC and CLI_PROC_IC
$(CLI_DEFN_IC): $(CLI_DEFN_SRC)
	$(CLI_GEN)
# </cli_opts>

# <main>
MAIN_SRC_DIR := $(SRC_DIR)

MAIN_BASE := main
MAIN_SRC := $(MAIN_SRC_DIR)/$(MAIN_BASE).cpp
MAIN_CLI := $(CLI_IMPL_IC) $(CLI_DEFN_IC) $(CLI_PROC_IC)
MAIN_O := $(OBJ_DIR)/$(MAIN_BASE).o
$(MAIN_O): $(MAIN_SRC) $(MAIN_CLI)
	$(CMPL) -c $< -o $@ $(FLAGS)
# </main>

# <matchers>
MATCHERS_SRC_DIR := $(SRC_DIR)/matcher

STRING_MATCHER_BASE := string_matcher
STRING_MATCHER_SRC := $(MATCHERS_SRC_DIR)/$(STRING_MATCHER_BASE).cpp
STRING_MATCHER_HDR := $(MATCHERS_SRC_DIR)/$(STRING_MATCHER_BASE).hpp
STRING_MATCHER_O := $(OBJ_DIR)/$(STRING_MATCHER_BASE).o
$(STRING_MATCHER_O): $(STRING_MATCHER_SRC) $(STRING_MATCHER_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)

REGEX_MATCHER_BASE := regex_matcher
REGEX_MATCHER_SRC := $(MATCHERS_SRC_DIR)/$(REGEX_MATCHER_BASE).cpp
REGEX_MATCHER_HDR := $(MATCHERS_SRC_DIR)/$(REGEX_MATCHER_BASE).hpp
REGEX_MATCHER_O := $(OBJ_DIR)/$(REGEX_MATCHER_BASE).o
$(REGEX_MATCHER_O): $(REGEX_MATCHER_SRC) $(REGEX_MATCHER_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)

MATCHER_FACTORY_BASE := matcher_factory
MATCHER_FACTORY_SRC := $(MATCHERS_SRC_DIR)/$(MATCHER_FACTORY_BASE).cpp
MATCHER_FACTORY_HDR := $(MATCHERS_SRC_DIR)/$(MATCHER_FACTORY_BASE).hpp
MATCHER_FACTORY_O := $(OBJ_DIR)/$(MATCHER_FACTORY_BASE).o
$(MATCHER_FACTORY_O): $(MATCHER_FACTORY_SRC) $(MATCHER_FACTORY_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)

MATCHERS_O := $(STRING_MATCHER_O) $(REGEX_MATCHER_O) $(MATCHER_FACTORY_O)
# </matchers>

# <lexer>
LEXER_SRC_DIR := $(SRC_DIR)/lexer

LEXER_BASE := lexer
LEXER_SRC := $(LEXER_SRC_DIR)/$(LEXER_BASE).cpp
LEXER_HDR := $(LEXER_SRC_DIR)/$(LEXER_BASE).hpp
LEXER_O := $(OBJ_DIR)/$(LEXER_BASE).o
$(LEXER_O): $(LEXER_SRC) $(LEXER_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)
# </lexer>

# <parser>
PARSER_SRC_DIR := $(SRC_DIR)/parser

PARSER_BASE := block_parser
PARSER_SRC := $(PARSER_SRC_DIR)/$(PARSER_BASE).cpp
PARSER_HDR := $(PARSER_SRC_DIR)/$(PARSER_BASE).hpp
PARSER_O := $(OBJ_DIR)/$(PARSER_BASE).o
$(PARSER_O): $(PARSER_SRC) $(PARSER_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)
# </parser>

# <unte_tests>
UNIT_TESTS_SRC_DIR := $(SRC_DIR)/unit_tests

UNIT_TESTS_BASE := unit_tests
UNIT_TESTS_SRC := $(UNIT_TESTS_SRC_DIR)/$(UNIT_TESTS_BASE).cpp
UNIT_TESTS_O := $(OBJ_DIR)/$(UNIT_TESTS_BASE).o
$(UNIT_TESTS_O): $(UNIT_TESTS_SRC)
	$(CMPL) -c $< -o $@ $(FLAGS)
# </unte_tests>

# <blocks>
BLOCKS_BASE := blocks
BLOCKS_BIN := $(BLOCKS_BASE)
BLOCKS_DEP := $(MAIN_O) $(PARSE_OPTS_O) $(MATCHERS_O) $(LEXER_O) $(PARSER_O)
$(BLOCKS_BIN): $(BLOCKS_DEP)
	$(CMPL) $^ -o ./$@ $(FLAGS)

UNIT_TESTS_BIN := unit-tests
UNIT_TESTS_DEP := $(UNIT_TESTS_O) $(MATCHERS_O) $(LEXER_O) $(PARSER_O)
$(UNIT_TESTS_BIN): $(UNIT_TESTS_DEP)
	$(CMPL) $^ -o ./$@ $(FLAGS)
# </blocks>

.PHONY: compile
compile: $(BLOCKS_BIN) $(UNIT_TESTS_BIN)

# <testing>
TEST_DIR := ./tests

.PHONY: test
test: blocks utest btest

.PHONY: btest
btest: blocks
	cd $(TEST_DIR) && bash run_tests.sh

.PHONY: btestv
btestv: blocks
	cd $(TEST_DIR) && bash run_tests.sh x

.PHONY: utest
utest: unit-tests
	./$(UNIT_TESTS_BIN)
# </testing>

# <cleanup>
.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/* ./$(BLOCKS_BIN) ./$(UNIT_TESTS_BIN)
# </cleanup>

# <help>
.PHONY: _help
_help:
	@echo 'make release    - release build'
	@echo 'make debug      - debug build'
	@echo 'make $(BLOCKS_BIN)     - compile blocks'
	@echo 'make $(UNIT_TESTS_BIN) - compile unit tests'
	@echo 'make cli_opts   - generate the command line options'
	@echo 'make test       - run all tests'
	@echo 'make btest      - run binary tests'
	@echo 'make btestv     - run binary tests verbose'
	@echo 'make utest      - run unit tests'
	@echo 'make clean      - clean up'
	@echo 'make help       - this screen'
# </help>
