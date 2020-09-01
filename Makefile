# Input files are every data set that is located in the input data folder.
INPUT_FILES     := $(wildcard OriginalFiles/*.txt)
# Get the filename without the path component from the INPUT_FILES
# make variable.
OUTPUT_FILENAMES := $(notdir $(INPUT_FILES))
# Generate the output filenames for where the experimental results
# will be placed.
REPORT_FILES   := $(addprefix ./Build/,$(OUTPUT_FILENAMES:%.txt=%.report))

# Used by the test rule on run the c program to generate its reports
TEST_FILES:=  $(addprefix ./Build/,$(OUTPUT_FILENAMES:%.txt=%.out1.data))

# Default name for the final generated output.
# The report will contain a summary of all results.
REPORT := ./Build/report.txt

C_EXAMPLE := ./Build/Test.out
C_COURCE := ./C_SourceImplementation/decoding.c
C_COURCE += ./C_SourceImplementation/encoding.c
C_COURCE += ./C_SourceImplementation/main_test.cpp

FLAGS := -Wall
FLAGS += -Wextra


# Explicitly tell make what the default goal is. 
.DEFAULT_GOAL: all

# Mark all intermediate files as secondary. 
# this prevents make from deleting them.
.SECONDARY:

all: $(REPORT) $(C_EXAMPLE)

$(C_EXAMPLE): $(C_COURCE)  | ./Build
	g++ -o $@ $(FLAGS) $^

# Report is dependent on outputs files being generated. 
# The output files are simply processed dataset files. 
# The report summarizes all of the results. 
$(REPORT): $(REPORT_FILES) | ./Build ./inputData
	@cat $^ > $@

# Copy target file from originalFiles to inputData folder
./inputData/%.txt: ./OriginalFiles/%.txt | ./inputData
	@cp -r $< $@

# Rule used to generate the build directory folder.
./Build:
	@mkdir $@

# Rule to generate the folder where make will place data sets.
./inputData:
	@mkdir $@

# Run length ad delta encode data. 
# Also calculates some stats based on the input and output 
# streams of bytes, like compression ratio.
./Build/%.report: ./Build/%.out1.txt  ./Python_SourceImplementation/ProcessData.py | ./Build
	@python3 ./Python_SourceImplementation/ProcessData.py $< $@

# Extract relevant data from dataset files
./Build/%.out1.txt: ./inputData/%.txt ./Python_SourceImplementation/ExtractData.sh | ./Build
	@./Python_SourceImplementation/ExtractData.sh $< $@

# Remove all build related files.
.PHONY: clean
clean: 
	@rm -rf ./Build ./inputData

# Display experimental report in the terminal
.PHONY: report
report: $(REPORT)
	@cat $<

.PHONY: test
# Report is used here as the dependency since report will generate test files. 
# This is done indirectly since since the rule that generates report, generates multiple
# files and make is not able to track multiple file outputs of a rule all that well.
test: $(REPORT) $(C_EXAMPLE)
	$(C_EXAMPLE) $(TEST_FILES)