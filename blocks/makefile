.PHONY: help
help:
	@echo 'make release - release build'
	@echo 'make debug   - debug build'
	@echo 'make blocks  - compile only blocks'
	@echo 'make unit    - compile only unit tests'
	@echo 'make test    - run all tests'
	@echo 'make btest   - run binary tests'
	@echo 'make utest   - run unit tests'
	@echo 'make clean   - clean up'
	@echo 'make help    - this screen'

CMPL := g++
EXTRA_FLAGS := 
FLAGS := -Wall -Wfatal-errors $(EXTRA_FLAGS)

SRC_DIR := ./src
OBJ_DIR := ./obj

.PHONY: release
release: FLAGS += -O3
release: clean compile

.PHONY: debug
debug: FLAGS += -g
debug: clean compile

# <obj_files_compilation>
MAIN_BASE := main
MAIN_SRC := $(SRC_DIR)/$(MAIN_BASE).cpp
MAIN_O := $(OBJ_DIR)/$(MAIN_BASE).o
$(MAIN_O): $(MAIN_SRC)
	$(CMPL) -c $? -o $@ $(FLAGS)

PARSER_IO_BASE := parser_io
PARSER_IO_SRC := $(SRC_DIR)/$(PARSER_IO_BASE).cpp
PARSER_IO_O := $(OBJ_DIR)/$(PARSER_IO_BASE).o
$(PARSER_IO_O): $(PARSER_IO_SRC)
	$(CMPL) -c $? -o $@ $(FLAGS)

BLOCK_PARSER_BASE := block_parser
BLOCK_PARSER_SRC := $(SRC_DIR)/$(BLOCK_PARSER_BASE).cpp
BLOCK_PARSER_O := $(OBJ_DIR)/$(BLOCK_PARSER_BASE).o
$(BLOCK_PARSER_O): $(BLOCK_PARSER_SRC)
	$(CMPL) -c $? -o $@ $(FLAGS)

PARSE_OPTS_BASE := parse_opts
PARSE_OPTS_SRC := $(SRC_DIR)/$(PARSE_OPTS_BASE).c
PARSE_OPTS_O := $(OBJ_DIR)/$(PARSE_OPTS_BASE).o
$(PARSE_OPTS_O): $(PARSE_OPTS_SRC)
	$(CMPL) -c $? -o $@ $(FLAGS)
	
UNIT_TESTS_BASE := unit_tests
UNIT_TESTS_SRC := $(SRC_DIR)/$(UNIT_TESTS_BASE).cpp
UNIT_TESTS_O := $(OBJ_DIR)/$(UNIT_TESTS_BASE).o
$(UNIT_TESTS_O): $(UNIT_TESTS_SRC)
	$(CMPL) -c $? -o $@ $(FLAGS)
# </obj_files_compilation>

# <binary_files_compilation>
BLOCKS_BASE := blocks
BLOCKS_BIN := $(BLOCKS_BASE)
BLOCKS_DEP := $(MAIN_O) $(PARSER_IO_O) $(BLOCK_PARSER_O) $(PARSE_OPTS_O)
$(BLOCKS_BIN): $(BLOCKS_DEP)
	$(CMPL) $? -o ./$@ $(FLAGS)
	
UNIT_TESTS_BIN := unit-tests
UNIT_TESTS_DEP := $(UNIT_TESTS_O) $(PARSER_IO_O) $(BLOCK_PARSER_O)
$(UNIT_TESTS_BIN): $(UNIT_TESTS_DEP)
	$(CMPL) $? -o ./$@ $(FLAGS)

.PHONY: compile
compile: $(BLOCKS_BIN) $(UNIT_TESTS_BIN)
# </binary_files_compilation>

# <cleanup>
.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/* ./$(BLOCKS_BIN) ./$(UNIT_TESTS_BIN)
# </cleanup>
