.PHONY: help
help: _help

CMPL := g++
EXTRA_FLAGS := 
INLC_PATHS := -I ./src/matcher -I ./src/parser
FLAGS := $(INLC_PATHS) -Wall -Wfatal-errors $(EXTRA_FLAGS)

SRC_DIR := ./src
OBJ_DIR := ./obj

.PHONY: release
release: FLAGS += -O3
release: clean compile

.PHONY: debug
debug: FLAGS += -g
debug: clean compile

# <obj_files_compilation>
# <main>
MAIN_BASE := main
MAIN_SRC := $(SRC_DIR)/$(MAIN_BASE).cpp
MAIN_O := $(OBJ_DIR)/$(MAIN_BASE).o
$(MAIN_O): $(MAIN_SRC)
	$(CMPL) -c $< -o $@ $(FLAGS)

PARSE_OPTS_BASE := parse_opts
PARSE_OPTS_SRC := $(SRC_DIR)/$(PARSE_OPTS_BASE).c
PARSE_OPTS_O := $(OBJ_DIR)/$(PARSE_OPTS_BASE).o
$(PARSE_OPTS_O): $(PARSE_OPTS_SRC)
	$(CMPL) -c $< -o $@ $(FLAGS)
	
MAIN_BLOCKS_O := $(MAIN_O) $(PARSE_OPTS_O)
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

# <parser>
PARSER_SRC_DIR := $(SRC_DIR)/parser
BLOCK_PARSER_BASE := block_parser
BLOCK_PARSER_SRC := $(PARSER_SRC_DIR)/$(BLOCK_PARSER_BASE).cpp
BLOCK_PARSER_HDR := $(PARSER_SRC_DIR)/$(BLOCK_PARSER_BASE).hpp
BLOCK_PARSER_O := $(OBJ_DIR)/$(BLOCK_PARSER_BASE).o
$(BLOCK_PARSER_O): $(BLOCK_PARSER_SRC) $(BLOCK_PARSER_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)

PARSER_IO_BASE := parser_io
PARSER_IO_SRC := $(PARSER_SRC_DIR)/$(PARSER_IO_BASE).cpp
PARSER_IO_HDR := $(PARSER_SRC_DIR)/$(PARSER_IO_BASE).hpp
PARSER_IO_O := $(OBJ_DIR)/$(PARSER_IO_BASE).o
$(PARSER_IO_O): $(PARSER_IO_SRC) $(PARSER_IO_HDR)
	$(CMPL) -c $< -o $@ $(FLAGS)
	
PARSER_O := $(BLOCK_PARSER_O) $(PARSER_IO_O)
# </parser>

# <unte_tests>	
UNIT_TESTS_BASE := unit_tests
UNIT_TESTS_SRC := $(SRC_DIR)/$(UNIT_TESTS_BASE).cpp
UNIT_TESTS_O := $(OBJ_DIR)/$(UNIT_TESTS_BASE).o
$(UNIT_TESTS_O): $(UNIT_TESTS_SRC)
	$(CMPL) -c $< -o $@ $(FLAGS)
# </unte_tests>
# </obj_files_compilation>

# <binary_files_compilation>
BLOCKS_BASE := blocks
BLOCKS_BIN := $(BLOCKS_BASE)
BLOCKS_DEP := $(MAIN_BLOCKS_O) $(MATCHERS_O) $(PARSER_O)
$(BLOCKS_BIN): $(BLOCKS_DEP)
	$(CMPL) $^ -o ./$@ $(FLAGS)
	
UNIT_TESTS_BIN := unit-tests
UNIT_TESTS_DEP := $(UNIT_TESTS_O) $(MATCHERS_O) $(PARSER_O)
$(UNIT_TESTS_BIN): $(UNIT_TESTS_DEP)
	$(CMPL) $^ -o ./$@ $(FLAGS)

.PHONY: compile
compile: $(BLOCKS_BIN) $(UNIT_TESTS_BIN)
# </binary_files_compilation>

# <testing>
TEST_DIR := ./tests

.PHONY: test
test: utest btest

.PHONY: btest
btest:
	cd $(TEST_DIR) && bash run_tests.sh ../$(BLOCKS_BIN)
	
.PHONY: utest
utest:
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
	@echo 'make test    - run all tests'
	@echo 'make btest   - run binary tests'
	@echo 'make utest   - run unit tests'
	@echo 'make clean   - clean up'
	@echo 'make help    - this screen'
# </help>
